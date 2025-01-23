#include <stdio.h>
#include "kasa.h"
#include <pthread.h>

int main()
{
    Kasa kasa;
    inicjalizuj_kase(&kasa);
    dodaj_banknoty(&kasa, 10, 5);
    dodaj_banknoty(&kasa, 20, 3);
    dodaj_banknoty(&kasa, 50, 2);
    printf("Stan kasy po inicjalizacji:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);
    zamknij_kase(&kasa);
    return 0;
}
