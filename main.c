#include <stdio.h>
#include "kasa.h"

int main() {
    Kasa kasa;
    inicjalizuj_kase(&kasa);
    printf("Stan kasy po inicjalizacji:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);

    return 0;
}
