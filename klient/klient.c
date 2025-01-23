#include "klient.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void zarabiaj_pieniadze(Klient *klient) {
    int banknoty[] = {10, 20, 50};
    int wybierany_banknot = banknoty[rand() % 3];

    if (wybierany_banknot == 10) {
        klient->banknot_10++;
    } else if (wybierany_banknot == 20) {
        klient->banknot_20++;
    } else if (wybierany_banknot == 50) {
        klient->banknot_50++;
    }

    printf("Klient %d zarobił %d zł.\n", klient->id, wybierany_banknot);
}

bool przyjdz_do_salonu(Klient *klient, Salon *salon) {
    printf("Klient %d przychodzi do salonu.\n", klient->id);

    if (salon->miejsca_w_poczekalni > 0) {
        salon->miejsca_w_poczekalni--;
        printf("Klient %d siada w poczekalni. Pozostało %d miejsc.\n", klient->id, salon->miejsca_w_poczekalni);
        return true;
    } else {
        printf("Brak miejsca w poczekalni, klient %d opuszcza salon.\n", klient->id);
        return false;
    }
}

void czekaj_na_fryzjera(Klient *klient, Salon *salon) {
    if (salon->fryzjer_dostepny) {
        printf("Fryzjer jest dostępny. Klient %d jest obsługiwany.\n", klient->id);
        salon->fryzjer_dostepny = false;
    } else {
        printf("Fryzjer zajęty. Klient %d czeka na fryzjera.\n", klient->id);
    }
}

void zaplac_za_usluge(Klient *klient, int koszt_uslugi, Salon *salon) {
    int calkowita_kwota = klient->banknot_10 * 10 + klient->banknot_20 * 20 + klient->banknot_50 * 50;

    if (calkowita_kwota >= koszt_uslugi) {
        printf("Klient %d płaci za usługę %d zł.\n", klient->id, koszt_uslugi);
        int pozostalo_do_zaplaty = koszt_uslugi;

        while (pozostalo_do_zaplaty >= 50 && klient->banknot_50 > 0) {
            klient->banknot_50--;
            pozostalo_do_zaplaty -= 50;
        }
        while (pozostalo_do_zaplaty >= 20 && klient->banknot_20 > 0) {
            klient->banknot_20--;
            pozostalo_do_zaplaty -= 20;
        }
        while (pozostalo_do_zaplaty >= 10 && klient->banknot_10 > 0) {
            klient->banknot_10--;
            pozostalo_do_zaplaty -= 10;
        }

        printf("Klient %d zapłacił. Pozostało mu: 10 zł: %d, 20 zł: %d, 50 zł: %d\n",
            klient->id, klient->banknot_10, klient->banknot_20, klient->banknot_50);
    } else {
        printf("Klient %d nie ma wystarczająco pieniędzy. Opuszcza salon.\n", klient->id);
    }
}


void czekaj_na_reszte(Klient *klient, int reszta) {
    if (reszta > 0) {
        printf("Klient %d czeka na resztę w wysokości %d zł.\n", klient->id, reszta);
    }
}

void opuszcz_salon(Klient *klient, Salon *salon) {
    salon->miejsca_w_poczekalni++;
    salon->fryzjer_dostepny = true;
    printf("Klient %d opuszcza salon.\n", klient->id);
    printf("Klient %d zapłacił. Pozostało mu: 10 zł: %d, 20 zł: %d, 50 zł: %d\n",
    klient->id, klient->banknot_10, klient->banknot_20, klient->banknot_50);
}

void cykliczna_obsluga_klienta(Klient *klient, Salon *salon, int koszt_uslugi) {
    while (true) {
        zarabiaj_pieniadze(klient);
        if (przyjdz_do_salonu(klient, salon)) {
            czekaj_na_fryzjera(klient, salon);
            if (!salon->fryzjer_dostepny) {
                zaplac_za_usluge(klient, koszt_uslugi, salon);
                czekaj_na_reszte(klient, koszt_uslugi);
                opuszcz_salon(klient, salon);
                break;
            }
        }
    }
}