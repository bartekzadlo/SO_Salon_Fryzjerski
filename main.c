#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "salon.h"
#include "klient.h"
#include "fryzjer.h"

int main()
{
    // Parametry wejściowe: liczba fryzjerów, wielkość poczekalni, liczba klientów, liczba foteli
    int liczba_fryzjerow = 1;                           // Liczba fryzjerów w salonie
    int wielkosc_poczekalni = MAX_KLIENCI_W_POCZEKALNI; // Wielkość poczekalni
    int liczba_klientow = 1;                            // Liczba istniejących klientów
    int liczba_foteli = 3;                              // Liczba foteli w salonie

    // Tworzymy salon i inicjalizujemy go
    Salon salon;
    inicjalizuj_salon(&salon, wielkosc_poczekalni, liczba_foteli);
    /*
        Funkcja inicjalizuje salon, ustawiając maksymalną liczbę klientów w poczekalni,
        liczby dostępnych foteli i synchronizując dostęp do tych zasobów.
        Tworzy semafor dla poczekalni (max_klientow) oraz mutexy i semafory dla foteli i poczekalni,
        aby zapewnić prawidłową synchronizację wątku podczas korzystania z tych zasobów.
        Dodatkowo inicjalizuje kasę (poprzez funkcję inicjalizuj_kase), aby zapewnić prawidłowe operacje kasowe.
        Funkcja inicjalizuje kasę w salonie, ustawiając początkowe ilości banknotów (wszystkie na 0).
        Inicjalizuje również mutex dla kasy oraz zmienną warunkową do synchronizacji operacji uzupełniania kasy.
    */

    // Tworzymy fryzjerów
    Fryzjer fryzjerowie[liczba_fryzjerow]; // Tablica przechowująca fryzjerów
    for (int i = 0; i < liczba_fryzjerow; i++)
    {
        inicjalizuj_fryzjera(&fryzjerowie[i], &salon, i + 1); // Inicjalizujemy fryzjerów
    }

    // Tworzymy klientów
    Klient klienci[liczba_klientow]; // Tablica przechowująca klientów
    for (int i = 0; i < liczba_klientow; i++)
    {
        inicjalizuj_klienta(&klienci[i], i); // Przekazanie numeru klienta jako ID
    }

    // Kończenie pracy fryzjerów
    for (int i = 0; i < liczba_fryzjerow; i++)
    {
        zakoncz_fryzjera(&fryzjerowie[i]); // Kończymy pracę fryzjerów
    }

    // Kończenie pracy klientów
    for (int i = 0; i < liczba_klientow; i++)
    {
        zakoncz_klienta(&klienci[i]); // Kończymy pracę klientów
    }

    // Zamykanie zasobów salonu
    zamknij_salon(&salon); // Zwalniamy wszystkie zasoby związane z salonem
    return 0;
}
