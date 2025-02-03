#include <stdio.h>     // Dla funkcji printf, perror i innych
#include <stdlib.h>    // Dla exit() i innych funkcji
#include <pthread.h>   // Dla operacji na wątkach i mutexach
#include <unistd.h>    // Dla sleep()
#include <sys/ipc.h>   // Dla ftok()
#include <sys/shm.h>   // Dla shmget, shmat i innych funkcji pamięci dzielonej
#include <semaphore.h> // Dla semaforów
#include "common.h"

int main()
{
    key_t key;
    key = ftok("./unikalny_klucz.txt", 'A');
    if (key == -1)
    {
        perror("Błąd: ftok nie powiódł się");
        exit(EXIT_FAILURE);
    }

    int shm_id = shmget(key, sizeof(Salon), IPC_CREAT | 0600);
    if (shm_id == -1)
    {
        perror("Błąd: nie udało się utworzyć segmentu pamięci współdzielonej");
        exit(EXIT_FAILURE);
    }

    // Wczytanie parametrów od użytkownika
    int liczba_fryzjerow, wielkosc_poczekalni, liczba_klientow, liczba_foteli;

    printf("Podaj liczbę fryzjerów (F > 1): ");
    if (scanf("%d", &liczba_fryzjerow) != 1 || liczba_fryzjerow <= 1)
    {
        fprintf(stderr, "Błąd: liczba fryzjerów musi być większa niż 1.\n");
        exit(EXIT_FAILURE);
    }

    printf("Podaj liczbę foteli (N < F): ");
    if (scanf("%d", &liczba_foteli) != 1 || liczba_foteli >= liczba_fryzjerow || liczba_foteli <= 0)
    {
        fprintf(stderr, "Błąd: liczba foteli musi być dodatnia i mniejsza niż liczba fryzjerów.\n");
        exit(EXIT_FAILURE);
    }

    printf("Podaj wielkość poczekalni (>= 0): ");
    if (scanf("%d", &wielkosc_poczekalni) != 1 || wielkosc_poczekalni < 0)
    {
        fprintf(stderr, "Błąd: wielkość poczekalni nie może być ujemna.\n");
        exit(EXIT_FAILURE);
    }

    printf("Podaj liczbę klientów (>= 0): ");
    if (scanf("%d", &liczba_klientow) != 1 || liczba_klientow < 0)
    {
        fprintf(stderr, "Błąd: liczba klientów nie może być ujemna.\n");
        exit(EXIT_FAILURE);
    }

    printf("Parametry poprawne. Uruchamianie symulacji...\n");

    // Tworzymy salon i inicjalizujemy go
    Salon *salon = (Salon *)shmat(shm_id, NULL, 0);
    if (salon == (void *)-1)
    {
        perror("Błąd przy podłączaniu pamięci dzielonej");
        exit(EXIT_FAILURE);
    }
    printf("Inicjalizowanie salonu...\n");
    inicjalizuj_salon(salon, wielkosc_poczekalni, liczba_foteli);
    printf("Salon został zainicjowany. Wielkość poczekalni: %d, liczba foteli: %d\n", wielkosc_poczekalni, liczba_foteli);
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
    printf("Inicjalizowanie fryzjerów...\n");
    for (int i = 0; i < liczba_fryzjerow; i++)
    {
        inicjalizuj_fryzjera(&fryzjerowie[i], salon, i); // Inicjalizujemy fryzjerów
        printf("Fryzjer %d został zainicjowany.\n", i);
    }
    // Tworzymy klientów
    Klient klienci[liczba_klientow]; // Tablica przechowująca klientów
    printf("Inicjalizowanie klientów...\n");
    for (int i = 0; i < liczba_klientow; i++)
    {
        inicjalizuj_klienta(&klienci[i], i);           // Przekazanie numeru klienta zaczynającego się od 0
        printf("Klient %d został zainicjowany.\n", i); // Numeracja od 0
    }

    sleep(10);

    // Kończenie pracy fryzjerów
    printf("Kończenie pracy fryzjerów...\n");
    for (int i = 0; i < liczba_fryzjerow; i++)
    {
        zakoncz_fryzjera(&fryzjerowie[i]);          // Kończymy pracę fryzjerów
        printf("Fryzjer %d zakończył pracę.\n", i); // Numeracja od 0
    }

    // Kończenie pracy klientów
    printf("Kończenie pracy klientów...\n");
    for (int i = 0; i < liczba_klientow; i++)
    {
        zakoncz_klienta(&klienci[i]);              // Kończymy pracę klientów
        printf("Klient %d zakończył pracę.\n", i); // Numeracja od 0
    }

    // Zamykanie zasobów salonu
    printf("Zamykanie salonu...\n");
    zamknij_salon(salon, shm_id); // Zwalniamy wszystkie zasoby związane z salonem
    printf("Salon został zamknięty.\n");

    return 0;
}
