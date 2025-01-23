#include <stdbool.h>
#include <stdio.h>
#include "fryzjer.h"

void inicjalizuj_kase(Kasa *kasa) {
    kasa->banknot_10 = 0;
    kasa->banknot_20 = 0;
    kasa->banknot_50 = 0;
}

bool czy_ma_wystarczajaca_reszte(Kasa *kasa, int kwota) {
    int banknoty_50 = kasa->banknot_50;
    int banknoty_20 = kasa->banknot_20;
    int banknoty_10 = kasa->banknot_10;

    for (int i = 0; i <= banknoty_50; i++) {
        for (int j = 0; j <= banknoty_20; j++) {
            for (int k = 0; k <= banknoty_10; k++) {
                int suma = i * 50 + j * 20 + k * 10;
                if (suma == kwota) {
                    return true;
                }
            }
        }
    }

    return false;
}

bool wydaj_reszte(Kasa *kasa, int kwota) {
    if (!czy_ma_wystarczajaca_reszte(kasa, kwota)) {
        return false;
    }

    int banknoty_50 = kasa->banknot_50;
    int banknoty_20 = kasa->banknot_20;
    int banknoty_10 = kasa->banknot_10;

    for (int i = 0; i <= banknoty_50; i++) {
        for (int j = 0; j <= banknoty_20; j++) {
            for (int k = 0; k <= banknoty_10; k++) {
                int suma = i * 50 + j * 20 + k * 10;
                if (suma == kwota) {
                    kasa->banknot_50 -= i;
                    kasa->banknot_20 -= j;
                    kasa->banknot_10 -= k;
                    return true;
                }
            }
        }
    }

    return false;
}

void dodaj_do_kasy(Kasa *kasa, int wartosc, int ilosc) {
    if (wartosc != 10 && wartosc != 20 && wartosc != 50) {
        printf("Nieprawidłowa wartość banknotu: %d\n", wartosc);
        return;
    }

    if (wartosc == 10) {
        kasa->banknot_10 += ilosc;
    } else if (wartosc == 20) {
        kasa->banknot_20 += ilosc;
    } else if (wartosc == 50) {
        kasa->banknot_50 += ilosc;
    }
}

void obsluz_klienta(Kasa *kasa, int zaplata, int koszt_uslugi) {
    dodaj_do_kasy(kasa, 10, zaplata / 10);
    int reszta = zaplata - koszt_uslugi;

    if (wydaj_reszte(kasa, reszta)) {
        printf("Reszta w kwocie %d zł została wydana.\n", reszta);
    } else {
        printf("Nie można wydać reszty w kwocie %d zł. Proszę poczekać.\n", reszta);
    }
}
