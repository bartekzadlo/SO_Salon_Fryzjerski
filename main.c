#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "salon.h"
#include "klient.h"
#include "fryzjer.h"

void wyswietl_wartosc_semafora(sem_t *semafor)
{
    int semafor_value;
    if (sem_getvalue(semafor, &semafor_value) == 0)
    {
        printf("Wartość semafora: %d\n", semafor_value);
    }
    else
    {
        perror("Błąd przy pobieraniu wartości semafora");
    }
}

int main()
{
    int Tp, Tk;                   // Godziny otwarcia i zamknięcia salonu
    int liczba_fryzjerow = 1;     // Liczba fryzjerów w salonie
    int wielkosc_poczekalni = 20; // Wielkość poczekalni
    int liczba_klientow = 1;      // Liczba istniejących klientów

    // Prośba o podanie godzin otwarcia i zamknięcia salonu
    printf("Podaj godzinę otwarcia salonu (Tp): ");
    scanf("%d", &Tp);
    printf("Podaj godzinę zamknięcia salonu (Tk): ");
    scanf("%d", &Tk);

    // Tworzymy salon i inicjalizujemy go
    Salon salon;
    inicjalizuj_salon(&salon, wielkosc_poczekalni, 3); // Zakładamy 1 fotel
    int poczekalnia_value;
    wyswietl_wartosc_semafora(&salon.poczekalnia); // Używamy wskaźnika do semafora

    // Tworzymy fryzjerów
    Fryzjer fryzjerowie[liczba_fryzjerow];
    for (int i = 0; i < liczba_fryzjerow; i++)
    {
        inicjalizuj_fryzjera(&fryzjerowie[i], &salon, i + 1);
    }

    // Tworzymy klientów
    Klient klienci[liczba_klientow];
    for (int i = 0; i < liczba_klientow; i++)
    {
        inicjalizuj_klienta(&klienci[i], i); // Przekazanie numeru klienta jako id
        pthread_t klient_watek;
        pthread_create(&klient_watek, NULL, (void *)klient_przychodzi_do_salon, &klienci[i]);
    }

    // Kończenie pracy fryzjerów
    for (int i = 0; i < liczba_fryzjerow; i++)
    {
        zakoncz_fryzjera(&fryzjerowie[i]);
    }

    // Zamykanie zasobów
    zamknij_salon(&salon);
    return 0;
}
