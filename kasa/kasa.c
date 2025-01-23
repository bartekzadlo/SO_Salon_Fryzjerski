#include "kasa.h"

void inicjalizuj_kase(Kasa *kasa) {
    kasa->banknot_10 = 0;
    kasa->banknot_20 = 0;
    kasa->banknot_50 = 0;
}

    while (reszta >= 50 && kasa->banknot_50 > 0) {
        reszta -= 50;
        kasa->banknot_50--;
    }

    while (reszta >= 20 && kasa->banknot_20 > 0) {
        reszta -= 20;
        kasa->banknot_20--;
    }

    while (reszta >= 10 && kasa->banknot_10 > 0) {
        reszta -= 10;
        kasa->banknot_10--;
    }

    return reszta == 0;
}

bool wydaj_reszte(Kasa *kasa, int kwota) {
    if (czy_ma_wystarczajaca_reszte(kasa, kwota)) {
        return true;
    }
    return false;
}

void dodaj_do_kasy(Kasa *kasa, int wartosc, int ilosc) {
    if (wartosc == 10) {
        kasa->banknot_10 += ilosc;
    } else if (wartosc == 20) {
        kasa->banknot_20 += ilosc;
    } else if (wartosc == 50) {
        kasa->banknot_50 += ilosc;
    }
}