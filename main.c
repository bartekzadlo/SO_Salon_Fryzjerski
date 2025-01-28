#include <stdio.h>
#include <stdlib.h>
#include "salon.h"
#include "klient.h"

int main()
{
    int Tp, Tk;              // Godziny otwarcia i zamknięcia salonu
    int liczba_fryzjerow;    // Liczba fryzjerów w salonie
    int wielkosc_poczekalni; // Wielkość poczekalni (maksymalna liczba klientów)
    int liczba_klientow;     // Liczba istniejących klientów

    // Prośba o podanie godzin otwarcia i zamknięcia salonu
    printf("Podaj godzinę otwarcia salonu (Tp): ");
    scanf("%d", &Tp);
    printf("Podaj godzinę zamknięcia salonu (Tk): ");
    scanf("%d", &Tk);

    // Prośba o liczbę fryzjerów
    printf("Podaj liczbę fryzjerów w salonie: ");
    scanf("%d", &liczba_fryzjerow);

    // Prośba o wielkość poczekalni
    printf("Podaj maksymalną liczbę klientów w poczekalni: ");
    scanf("%d", &wielkosc_poczekalni);

    // Prośba o liczbę istniejących klientów
    printf("Podaj liczbę istniejących klientów: ");
    scanf("%d", &liczba_klientow);

    printf("\nSalon działa od %d:00 do %d:00.\n", Tp, Tk);
    printf("Liczba fryzjerów: %d\n", liczba_fryzjerow);
    printf("Maksymalna liczba klientów w poczekalni: %d\n", wielkosc_poczekalni);
    printf("Istniejący klienci: %d\n", liczba_klientow);

    return 0;
}