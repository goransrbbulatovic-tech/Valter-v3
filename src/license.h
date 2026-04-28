#ifndef LICENSE_H
#define LICENSE_H

#include <QString>

class License {
public:
    // Generiši novi ključ (za keygen)
    static QString generateKey();

    // Provjeri samo format ključa (bez machine check)
    static bool isValidKey(const QString &key);

    // Provjeri da li je program aktiviran NA OVOM RAČUNARU
    static bool isActivated();

    // Aktiviraj ključ i vežu za ovaj računar
    // Vraća: 0=OK, 1=pogrešan ključ, 2=ključ već aktiviran na drugom računaru
    static int activate(const QString &key);

    // Vrati ID ovog računara (za prikaz korisniku)
    static QString machineId();

    // Vrati sačuvani ključ
    static QString savedKey();

    // Deaktiviraj (za slučaj reinstalacije / prenosa licence)
    static void deactivate();

private:
    static quint32 computeChecksum(const QString &p1, const QString &p2, const QString &p3);
    static QString computeBinding(const QString &key, const QString &mid);
    static QString encodePart(quint32 value);
    static QString getMachineId();

    static const QString CHARS;
    static const quint32 SEED1;
    static const quint32 SEED2;
    static const quint32 SEED3;
    static const quint32 SEED4;
};

#endif
