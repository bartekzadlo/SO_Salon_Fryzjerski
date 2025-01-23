#include <stdio.h>
#include "kasa.h"
#include <pthread.h>

int main()
{
    Kasa kasa;
    inicjalizuj_kase(&kasa);

    // Dodawanie banknotów
    dodaj_banknoty(&kasa, 10, 5); // Dodajemy 5 banknotów 10zł
    dodaj_banknoty(&kasa, 20, 3); // Dodajemy 3 banknoty 20zł
    dodaj_banknoty(&kasa, 50, 2); // Dodajemy 2 banknoty 50zł

    // Wyświetlanie stanu kasy
    pthread_mutex_lock(&kasa.mutex); // Blokowanie kasy na czas wyświetlania
    printf("Stan kasy:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);
    pthread_mutex_unlock(&kasa.mutex); // Odblokowanie kasy

    // Odejmowanie banknotów
    odejmij_banknoty(&kasa, 10, 3); // Odejmujemy 3 banknoty 10zł
    odejmij_banknoty(&kasa, 50, 1); // Odejmujemy 1 banknot 50zł

    // Wyświetlanie stanu kasy po odejmowaniu
    pthread_mutex_lock(&kasa.mutex); // Blokowanie kasy na czas wyświetlania
    printf("\nStan kasy po odejmowaniu:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);
    pthread_mutex_unlock(&kasa.mutex); // Odblokowanie kasy

    zamknij_kase(&kasa); // Zamykanie mutexa po zakończeniu

    return 0;
}
