#include "salon.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

void inicjalizuj_kase(Kasa *kasa)
{
    kasa->banknot_10 = 0;
    kasa->banknot_20 = 0;
    kasa->banknot_50 = 0;
    kasa->wydane_10 = 0;
    kasa->wydane_20 = 0;
    kasa->wydane_50 = 0;

    // Inicjalizowanie semaforów
    if (sem_init(&kasa->mutex_kasa, 1, 1) != 0)
    { // 1 dla semafora binarnego (mutex)
        perror("Błąd inicjalizacji semafora mutex_kasa");
        exit(EXIT_FAILURE);
    }

    // Inicjalizowanie zmiennych warunkowych
    // W przypadku pamięci dzielonej musimy zadbać o to, by były współdzielone przez wszystkie procesy
    if (pthread_cond_init(&kasa->uzupelnienie, NULL) != 0)
    {
        perror("Błąd inicjalizacji zmiennej warunkowej uzupelnienie");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&kasa->zaplata, NULL) != 0)
    {
        perror("Błąd inicjalizacji zmiennej warunkowej zaplata");
        exit(EXIT_FAILURE);
    }
    if (pthread_cond_init(&kasa->wydano_reszte, NULL) != 0)
    {
        perror("Błąd inicjalizacji zmiennej warunkowej wydano_reszte");
        exit(EXIT_FAILURE);
    }
}

void zamknij_kase(Kasa *kasa)
{
    pthread_mutex_destroy(&kasa->mutex_kasa);   // Zwalniamy mutex
    pthread_cond_destroy(&kasa->uzupelnienie);  // Niszczenie zmiennej warunkowej uzupelnienie
    pthread_cond_destroy(&kasa->zaplata);       // Niszczenie zmiennej warunkowej zaplata
    pthread_cond_destroy(&kasa->wydano_reszte); // Niszczenie zmiennej warunkowej wydano_reszte
}

void dodaj_banknoty_do_kasy(Salon *salon)
{
    // Oczekiwanie na sygnał o zapłacie
    pthread_cond_wait(&salon->kasa.zaplata, &salon->kasa.mutex_kasa);
    pthread_mutex_lock(&salon->kasa.mutex_kasa); // Blokowanie mutexu kasy

    int zaplacona_kwota = salon->zaplacona_kwota;
    int reszta = zaplacona_kwota - salon->zaplacona_kwota; // Obliczanie reszty

    // Dodawanie banknotów do kasy
    if (salon->zaplacone_50 > 0)
    {
        salon->kasa.banknot_50 += salon->zaplacone_50; // Dodaj banknoty 50 do kasy
    }
    if (salon->zaplacone_20 > 0)
    {
        salon->kasa.banknot_20 += salon->zaplacone_20; // Dodaj banknoty 20 do kasy
    }
    if (salon->zaplacone_10 > 0)
    {
        salon->kasa.banknot_10 += salon->zaplacone_10; // Dodaj banknoty 10 do kasy
    }

    // Zapamiętanie reszty
    salon->reszta = reszta; // Przechowywanie reszty do późniejszego wydania

    // Sygnalizowanie, że kasa jest uzupełniona
    pthread_cond_signal(&salon->kasa.uzupelnienie);

    pthread_mutex_unlock(&salon->kasa.mutex_kasa); // Zwolnienie mutexu kasy
}

void wydaj_reszte(Kasa *kasa, int reszta)
{
    pthread_mutex_lock(&kasa->mutex_kasa); // Blokowanie mutexu na kasie

    // Zmienna przechowująca liczbę wydanych banknotów
    int wydane_10 = 0, wydane_20 = 0, wydane_50 = 0;

    // Próba wydania reszty
    while (reszta > 0)
    {
        // Sprawdzamy, czy możemy wydać 50z
        if (reszta >= 50 && kasa->banknot_50 > 0)
        {
            kasa->banknot_50--;
            reszta -= 50;
            wydane_50++;
            printf("Wydano banknot 50z. Pozostała reszta: %dz.\n", reszta);
        }
        // Sprawdzamy, czy możemy wydać 20z
        else if (reszta >= 20 && kasa->banknot_20 > 0)
        {
            kasa->banknot_20--;
            reszta -= 20;
            wydane_20++;
            printf("Wydano banknot 20z. Pozostała reszta: %dz.\n", reszta);
        }
        // Sprawdzamy, czy możemy wydać 10z
        else if (reszta >= 10 && kasa->banknot_10 > 0)
        {
            kasa->banknot_10--;
            reszta -= 10;
            wydane_10++;
            printf("Wydano banknot 10z. Pozostała reszta: %dz.\n", reszta);
        }
        else
        {
            // Jeśli brak banknotów, czekamy na uzupełnienie kasy
            printf("Brak wystarczających banknotów do wydania reszty. Czekam na uzupełnienie.\n");
            pthread_cond_wait(&kasa->uzupelnienie, &kasa->mutex_kasa); // Czekamy na sygnał
        }
    }

    // Zapamiętanie liczby wydanych banknotów
    kasa->wydane_10 = wydane_10;
    kasa->wydane_20 = wydane_20;
    kasa->wydane_50 = wydane_50;

    pthread_mutex_unlock(&kasa->mutex_kasa); // Zwolnienie mutexu

    // Wysyłamy sygnał o wydaniu reszty
    pthread_cond_signal(&kasa->wydano_reszte);
}

void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli)
{
    salon->klienci_w_poczekalni = 0;
    salon->max_klientow = max_klientow;

    // Inicjalizowanie semafora poczekalni
    if (sem_init(&salon->poczekalnia, 1, max_klientow) != 0)
    {
        perror("Błąd inicjalizacji semafora poczekalni");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja mutexu dla poczekalni
    if (pthread_mutex_init(&salon->mutex_poczekalnia, NULL) != 0)
    {
        perror("Błąd inicjalizacji mutexu poczekalni");
        exit(EXIT_FAILURE);
    }

    // Inicjalizowanie semafora fotela
    if (sem_init(&salon->fotel.wolne_fotele, 1, liczba_foteli) != 0)
    {
        perror("Błąd inicjalizacji semafora foteli");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja mutexu dla fotela
    if (pthread_mutex_init(&salon->fotel.mutex_fotel, NULL) != 0)
    {
        perror("Błąd inicjalizacji mutexu fotela");
        exit(EXIT_FAILURE);
    }

    // Inicjalizacja kasy
    inicjalizuj_kase(&salon->kasa);
}

void zajmij_fotel(Fotel *fotel)
{
    sem_wait(&fotel->wolne_fotele);          // Czekaj na wolny fotel
    pthread_mutex_lock(&fotel->mutex_fotel); // Zablokuj mutex, aby fotel był zajęty przez jednego klienta
    printf("Klient zajmuje fotel.\n");
    pthread_mutex_unlock(&fotel->mutex_fotel); // Zwolnij mutex
}

void zwolnij_fotel(Fotel *fotel)
{
    pthread_mutex_lock(&fotel->mutex_fotel); // Zablokuj mutex przed zwolnieniem fotela
    sem_post(&fotel->wolne_fotele);          // Zwolnij fotel
    printf("Fotel został zwolniony.\n");
    pthread_mutex_unlock(&fotel->mutex_fotel); // Zwolnij mutex
}

void zamknij_salon(Salon *salon)
{
    // Zwalnianie zasobów semaforów i mutexów związanych z salonem
    sem_destroy(&salon->poczekalnia);                 // Zwalniamy semafor dla poczekalni
    pthread_mutex_destroy(&salon->mutex_poczekalnia); // Zwalniamy mutex dla poczekalni

    // Zwalnianie zasobów związanych z fotelami
    sem_destroy(&salon->fotel.wolne_fotele);          // Zwalniamy semafor dla foteli
    pthread_mutex_destroy(&salon->fotel.mutex_fotel); // Zwalniamy mutex dla foteli

    // Zwalnianie zasobów kasy
    zamknij_kase(&salon->kasa); // Zwalniamy zasoby związane z kasą

    // Zwalnianie pamięci dzielonej
    if (shmctl(salon->segment_pamieci_id, IPC_RMID, NULL) == -1)
    {
        perror("Błąd przy usuwaniu segmentu pamięci dzielonej");
        exit(EXIT_FAILURE);
    }

    printf("Pamięć dzielona została pomyślnie usunięta.\n");
}
