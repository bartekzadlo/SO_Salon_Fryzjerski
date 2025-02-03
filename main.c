#include <stdio.h>     // Dla funkcji printf, perror i innych
#include <stdlib.h>    // Dla exit() i innych funkcji
#include <pthread.h>   // Dla operacji na wątkach i mutexach
#include <unistd.h>    // Dla sleep()
#include <sys/ipc.h>   // Dla ftok()
#include <sys/shm.h>   // Dla shmget, shmat i innych funkcji pamięci dzielonej
#include <semaphore.h> // Dla semaforów
#include <errno.h>     // Do obsługi błędów systemowych
#include "common.h"

void sprawdz_blad(int warunek, const char *komunikat)
{
    if (warunek)
    {
        perror(komunikat);
        exit(EXIT_FAILURE);
    }
}

int main()
{
    key_t key = ftok("./unikalny_klucz.txt", 'A');
    sprawdz_blad(key == -1, "Błąd: ftok nie powiódł się");

    int shm_id = shmget(key, sizeof(Salon), IPC_CREAT | 0600);
    sprawdz_blad(shm_id == -1, "Błąd: nie udało się utworzyć segmentu pamięci współdzielonej");

    // Tworzymy salon i inicjalizujemy go
    Salon *salon = (Salon *)shmat(shm_id, NULL, 0);
    if (salon == (void *)-1)
    {
        perror("Błąd przy podłączaniu pamięci dzielonej");
        exit(EXIT_FAILURE);
    }
    printf("Inicjalizowanie salonu...\n");
    inicjalizuj_salon(salon, K, N);
    printf("Salon został zainicjowany. Wielkość poczekalni: %d, liczba foteli: %d\n", K, N);
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
    Fryzjer fryzjerowie[F]; // Tablica przechowująca fryzjerów
    printf("Inicjalizowanie fryzjerów...\n");
    for (int i = 0; i < F; i++)
    {
        inicjalizuj_fryzjera(&fryzjerowie[i], salon, i); // Inicjalizujemy fryzjerów
        printf("Fryzjer %d został zainicjowany.\n", i);
    }
    // Tworzymy klientów
    Klient klienci[P]; // Tablica przechowująca klientów
    printf("Inicjalizowanie klientów...\n");
    for (int i = 0; i < P; i++)
    {
        inicjalizuj_klienta(&klienci[i], i);           // Przekazanie numeru klienta zaczynającego się od 0
        printf("Klient %d został zainicjowany.\n", i); // Numeracja od 0
    }

    sleep(10);

    // Kończenie pracy fryzjerów
    printf("Kończenie pracy fryzjerów...\n");
    for (int i = 0; i < F; i++)
    {
        zakoncz_fryzjera(&fryzjerowie[i]);          // Kończymy pracę fryzjerów
        printf("Fryzjer %d zakończył pracę.\n", i); // Numeracja od 0
    }

    // Kończenie pracy klientów
    printf("Kończenie pracy klientów...\n");
    for (int i = 0; i < P; i++)
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
