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
    pthread_mutex_init(&kasa->mutex_kasa, NULL);
    pthread_cond_init(&kasa->uzupelnienie, NULL); // Inicjalizacja warunku
}

void zamknij_kase(Kasa *kasa)
{
    pthread_mutex_destroy(&kasa->mutex_kasa);
    pthread_cond_destroy(&kasa->uzupelnienie); // Niszczenie warunku
}

void dodaj_banknoty_do_kasy(Salon *salon)
{
    pthread_mutex_lock(&salon->mutex_kasa); // Blokowanie mutexu kasy

    // Oczekiwanie na sygnał o zapłacie
    pthread_cond_wait(&salon->kasa.uzupelnienie, &salon->mutex_kasa);

    int zaplacona_kwota = salon->zaplacona_kwota;
    int reszta = zaplacona_kwota - salon->zaplacona_kwota; // Obliczanie reszty

    // Dodawanie banknotów do kasy
    if (salon->zaplacone_50 > 0)
    {
        dodaj_banknoty(&salon->kasa, 50, salon->zaplacone_50);
    }
    if (salon->zaplacone_20 > 0)
    {
        dodaj_banknoty(&salon->kasa, 20, salon->zaplacone_20);
    }
    if (salon->zaplacone_10 > 0)
    {
        dodaj_banknoty(&salon->kasa, 10, salon->zaplacone_10);
    }

    // Zapamiętanie reszty
    salon->reszta = reszta; // Przechowywanie reszty do późniejszego wydania

    pthread_mutex_unlock(&salon->mutex_kasa); // Zwolnienie mutexu kasy
}

void odejmij_banknoty(Kasa *kasa, int nominal, int ilosc)
{
    pthread_mutex_lock(&kasa->mutex_kasa);

    if (nominal == 10)
    {
        if (kasa->banknot_10 >= ilosc)
        {
            kasa->banknot_10 -= ilosc;
        }
        else
        {
            printf("Brak wystarczającej ilości banknotów 10z\n");
        }
    }
    else if (nominal == 20)
    {
        if (kasa->banknot_20 >= ilosc)
        {
            kasa->banknot_20 -= ilosc;
        }
        else
        {
            printf("Brak wystarczającej ilości banknotów 20z\n");
        }
    }
    else if (nominal == 50)
    {
        if (kasa->banknot_50 >= ilosc)
        {
            kasa->banknot_50 -= ilosc;
        }
        else
        {
            printf("Brak wystarczającej ilości banknotów 50z\n");
        }
    }

    pthread_mutex_unlock(&kasa->mutex_kasa);
}

void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli)
{
    salon->max_klientow = max_klientow;                  // Przypisanie wartości max_klientow
    sem_init(&salon->poczekalnia, 0, max_klientow);      // Inicjalizowanie semafora
    pthread_mutex_init(&salon->mutex_poczekalnia, NULL); // Mutex dla poczekalni
    salon->klienci_w_poczekalni = 0;

    // Inicjalizacja fotela
    sem_init(&salon->fotel.wolne_fotele, 0, liczba_foteli);
    pthread_mutex_init(&salon->fotel.mutex_fotel, NULL);
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
    sem_destroy(&salon->poczekalnia);                 // Zwalniamy semafor dla poczekalni
    pthread_mutex_destroy(&salon->mutex_poczekalnia); // Zwalniamy mutex dla poczekalni

    // Niszczenie zasobów związanych z fotelami
    sem_destroy(&salon->fotel.wolne_fotele);          // Zwalniamy semafor dla foteli
    pthread_mutex_destroy(&salon->fotel.mutex_fotel); // Zwalniamy mutex dla foteli
}
