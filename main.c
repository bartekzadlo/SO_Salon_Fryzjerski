#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "kasa.h"
#include "klient.h"
#include "salon.h"

int main()
{
    int Tp, Tk;
    int liczba_foteli = 5; // Przykładowa liczba foteli
    int max_klientow = 10; // Przykładowa liczba maksymalnych klientów w poczekalni

    // Prośba o podanie godzin otwarcia i zamknięcia salonu
    printf("Podaj godzinę otwarcia salonu (Tp): ");
    scanf("%d", &Tp);
    printf("Podaj godzinę zamknięcia salonu (Tk): ");
    scanf("%d", &Tk);

    if (Tp < 0 || Tp > 24 || Tk < 0 || Tk > 24 || Tp >= Tk)
    {
        printf("Podano nieprawidłowe godziny otwarcia lub zamknięcia salonu.\n");
        return 1;
    }

    printf("Salon działa od %d:00 do %d:00.\n\n", Tp, Tk);

    // Inicjalizacja kasy
    Kasa kasa;
    inicjalizuj_kase(&kasa);

    // Inicjalizacja klienta
    Klient klient;
    inicjalizuj_portfel(&klient);

    // Inicjalizacja salonu
    Salon salon;
    inicjalizuj_salon(&salon, max_klientow, liczba_foteli);

    // Klient zarabia pieniądze
    zarabiaj_pieniadze(&klient);

    // Wyświetlanie stanu portfela klienta
    printf("Stan portfela klienta po zarabianiu:\n");
    printf("Banknoty 10z: %d\n", klient.portfel_10);
    printf("Banknoty 20z: %d\n", klient.portfel_20);
    printf("Banknoty 50z: %d\n", klient.portfel_50);

    // Klient dodaje pieniądze do kasy
    dodaj_banknoty(&kasa, 10, klient.portfel_10);
    dodaj_banknoty(&kasa, 20, klient.portfel_20);
    dodaj_banknoty(&kasa, 50, klient.portfel_50);

    // Wyświetlanie stanu kasy
    printf("\nStan kasy po wpłacie klienta:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);

    // Kasjer wydaje pieniądze (odejmujemy z kasy)
    odejmij_banknoty(&kasa, 10, 1);
    odejmij_banknoty(&kasa, 50, 1);

    // Wyświetlanie stanu kasy po operacjach
    printf("\nStan kasy po wydaniu pieniędzy:\n");
    printf("Banknoty 10z: %d\n", kasa.banknot_10);
    printf("Banknoty 20z: %d\n", kasa.banknot_20);
    printf("Banknoty 50z: %d\n", kasa.banknot_50);

    // Zamknięcie salonu
    zamknij_salon(&salon);

    return 0;
}
