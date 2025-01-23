#include "fryzjer.h"
#include <stdio.h>
#include <stdlib.h>

void przyjmij_zaplate(int klient_id, int kwota, int zaplacona_kwota) {
    printf("Fryzjer przyjął zapłatę od klienta %d: %d zł.\n", klient_id, zaplacona_kwota);
    int reszta = zaplacona_kwota - kwota;
    printf("Obliczono resztę do wydania: %d zł.\n", reszta);
}

void wydaj_reszte(int klient_id) {
    printf("Fryzjer wydaje resztę klientowi %d.\n", klient_id);
}