#include "license.h"
#include <QSettings>
#include <QRandomGenerator>
#include <QSysInfo>
#include <QCryptographicHash>
#include <QNetworkInterface>

// Dozvoljeni znakovi — bez dvosmislenih (0, O, 1, I)
const QString License::CHARS = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";

// Tajni seedovi — NE MIJENJATI nakon distribucije!
const quint32 License::SEED1 = 0x7A3FC92E;
const quint32 License::SEED2 = 0xB4D1F865;
const quint32 License::SEED3 = 0x3E8A1D4F;
const quint32 License::SEED4 = 0xC2F09B31;

// ── Privatne pomoćne funkcije ─────────────────────────────────────────────────

quint32 License::computeChecksum(const QString &p1, const QString &p2, const QString &p3) {
    quint32 h = SEED1;
    for (QChar c : p1 + p2 + p3) {
        h = ((h << 5) + h) ^ (quint32)c.unicode();
        h ^= SEED3;
    }
    return h ^ SEED2;
}

QString License::encodePart(quint32 value) {
    QString result;
    for (int i = 0; i < 5; i++) {
        result += CHARS[value % (quint32)CHARS.size()];
        value /= (quint32)CHARS.size();
    }
    return result;
}

// Generiše jedinstveni ID ovog računara
// Kombinuje: Qt machineUniqueId + MAC adresa + hostname
QString License::getMachineId() {
    QByteArray raw;

    // 1. Qt ugrađeni machine ID (najstabilniji — ostaje i nakon reinstalacije app)
    QByteArray qtId = QSysInfo::machineUniqueId();
    raw += qtId;

    // 2. Prva ne-loopback MAC adresa kao backup
    for (const QNetworkInterface &iface : QNetworkInterface::allInterfaces()) {
        if (iface.flags() & QNetworkInterface::IsLoopBack) continue;
        QString mac = iface.hardwareAddress();
        if (!mac.isEmpty() && mac != "00:00:00:00:00:00") {
            raw += mac.toLatin1();
            break;
        }
    }

    // 3. Hostname kao treći sloj
    raw += QSysInfo::machineHostName().toLatin1();

    // Hash sve to u 32 bajta
    QByteArray hash = QCryptographicHash::hash(raw, QCryptographicHash::Sha256);
    // Vrati prvih 16 hex znakova — dovoljno jedinstveno, ne previše
    return QString::fromLatin1(hash.toHex()).left(16).toUpper();
}

// Izračunaj binding: hash(ključ + machineId) → 10-znakovna string
QString License::computeBinding(const QString &key, const QString &mid) {
    quint32 h1 = SEED4;
    quint32 h2 = SEED1 ^ SEED3;
    for (QChar c : key + "|" + mid) {
        h1 = ((h1 << 5) + h1) ^ (quint32)c.unicode();
        h2 = ((h2 << 3) + h2) ^ (quint32)c.unicode();
        h1 ^= SEED2;
        h2 ^= SEED3;
    }
    return encodePart(h1) + "-" + encodePart(h2);
}

// ── Javne funkcije ────────────────────────────────────────────────────────────

QString License::generateKey() {
    auto *rng = QRandomGenerator::global();
    QString p1 = encodePart(rng->generate());
    QString p2 = encodePart(rng->generate());
    QString p3 = encodePart(rng->generate());
    QString p4 = encodePart(computeChecksum(p1, p2, p3));
    return QString("VALTR-%1-%2-%3-%4").arg(p1).arg(p2).arg(p3).arg(p4);
}

bool License::isValidKey(const QString &key) {
    QString clean = key.trimmed().toUpper(); clean.remove(' ');
    QStringList parts = clean.split('-');
    if (parts.size() != 5)   return false;
    if (parts[0] != "VALTR") return false;
    for (int i = 1; i < 5; i++) {
        if (parts[i].size() != 5) return false;
        for (QChar c : parts[i])
            if (!CHARS.contains(c)) return false;
    }
    quint32 expected = computeChecksum(parts[1], parts[2], parts[3]);
    return parts[4] == encodePart(expected);
}

// Vraća: 0=OK, 1=pogrešan ključ, 2=ključ vezan za drugi računar
int License::activate(const QString &key) {
    if (!isValidKey(key)) return 1;

    QSettings s("KCBar", "Valter");

    // Da li je ovaj ključ već aktiviran negdje?
    // (Ne možemo znati za drugi komp — ali možemo spriječiti lokalno ponavljanje
    //  sa drugim ključem i provjeriti binding ako postoji)
    QString mid     = getMachineId();
    QString binding = computeBinding(key.trimmed().toUpper().remove(' '), mid);

    s.setValue("License/Key",     key.trimmed().toUpper().remove(' '));
    s.setValue("License/Binding", binding);
    s.sync();
    return 0;
}

bool License::isActivated() {
    QSettings s("KCBar", "Valter");
    QString key     = s.value("License/Key",     "").toString();
    QString binding = s.value("License/Binding", "").toString();

    if (!isValidKey(key)) return false;

    // Provjeri da binding odgovara OVOM računaru
    QString mid             = getMachineId();
    QString expectedBinding = computeBinding(key, mid);

    return binding == expectedBinding;
}

QString License::machineId() {
    return getMachineId();
}

QString License::savedKey() {
    QSettings s("KCBar", "Valter");
    return s.value("License/Key", "").toString();
}

void License::deactivate() {
    QSettings s("KCBar", "Valter");
    s.remove("License/Key");
    s.remove("License/Binding");
    s.sync();
}
