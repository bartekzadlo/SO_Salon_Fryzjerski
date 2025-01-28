#include <stdio.h>
#include "kasa.h"
#include "klient.h"
#include <pthread.h>

int main()
{
    // Inicjalizacja kasy
    Kasa kasa;
    inicjalizuj_kase(&kasa);

    // Inicjalizacja klienta
    Klient klient;
    inicjalizuj_portfel(&klient.portfel);

    // Klient zarabia pieniądze
    zarabiaj_pieniadze(&klient);

    // Wyświetlanie stanu portfela klienta
    pthread_mutex_lock(&klient.portfel.mutex);
    printf("Stan portfela klienta po zarabianiu:\n");
    printf("Banknoty 10z: %d\n", klient.portfel.banknot_10);
    printf("Banknoty 20z: %d\n", klient.portfel.banknot_20);
    printf("Banknoty 50z: %d\n", klient.portfel.banknot_50);
    pthread_mutex_unlock(&klient.portfel.mutex);

    // Klient dodaje pieniądze do kasy
    dodaj_banknoty(&kasa, 10, klient.portfel.banknot_10);
    dodaj_banknoty(&kasa, 20, klient.portfel.banknot_20);
    dodaj_banknoty(&kasa, 50, klient.portfel.banknot_50);

    // Wyświetlanie stanu kasy
    pthread_mutex_lock(&kasa.mutex);
    printf("\nStan kasy po wpłacie klienta:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);
    pthread_mutex_unlock(&kasa.mutex);

    // Kasjer wydaje pieniądze (odejmujemy z kasy)
    odejmij_banknoty(&kasa, 10, 1);
    odejmij_banknoty(&kasa, 50, 1);

    // Wyświetlanie stanu kasy po operacjach
    pthread_mutex_lock(&kasa.mutex);
    printf("\nStan kasy po wydaniu pieniędzy:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);
    pthread_mutex_unlock(&kasa.mutex);

    // Zamknięcie portfela klienta i kasy
    zamknij_portfel(&klient.portfel);
    zamknij_kase(&kasa);

    return 0;
}
