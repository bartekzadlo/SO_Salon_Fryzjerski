#include <stdio.h>
#include "kierownik.h"
#include "fryzjer.h"
#include "klient.h"

int main() {
    Kasa kasa;
    inicjalizuj_kase(&kasa);

    dodaj_do_kasy(&kasa, 50, 3); 
    dodaj_do_kasy(&kasa, 20, 2); 
    dodaj_do_kasy(&kasa, 10, 0); 

    printf("Czy można wydać 60 zł: %s\n", czy_ma_wystarczajaca_reszte(&kasa, 60) ? "Tak" : "Nie");

    if (wydaj_reszte(&kasa, 60)) {
        printf("Reszta wydana.\n");
    } else {
        printf("Nie można wydać reszty.\n");
    }

    return 0;
}
