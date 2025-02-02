#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "klient.h"
#include "salon.h"

void inicjalizuj_klienta(Klient *klient, int id)
{
    // Inicjalizacja danych klienta
    klient->id = id;
    klient->portfel_10 = 5;
    klient->portfel_20 = 5;
    klient->portfel_50 = 5;

    // Inicjalizacja zmiennej warunkowej (dla synchronizacji)
    if (pthread_cond_init(&klient->czekaj_na_zaplate, NULL) != 0)
    {
        printf("Błąd inicjalizacji zmiennej warunkowej.\n");
        exit(1);
    }

    // Inicjalizacja mutexu
    if (pthread_mutex_init(&klient->mutex_klient, NULL) != 0)
    {
        printf("Błąd inicjalizacji mutexu.\n");
        exit(1);
    }

    // Tworzenie wątku dla klienta
    if (pthread_create(&klient->watek, NULL, (void *)klient_przychodzi_do_salon, klient) != 0)
    {
        printf("Błąd tworzenia wątku dla klienta %d.\n", klient->id);
        exit(1);
    }
}

void zaplac_za_usluge(Klient *klient, Salon *salon)
{
    int cena = (rand() % 10 + 1) * 10;
    printf("Cena za usługę: %dzł.\n", cena);

    // Przygotowanie informacji o zapłacie
    salon->zaplacona_kwota = cena;
    salon->zaplacone_50 = 0;
    salon->zaplacone_20 = 0;
    salon->zaplacone_10 = 0;

    // Obliczanie nominalów
    while (cena >= 50 && klient->portfel_50 > 0)
    {
        salon->zaplacone_50++;
        cena -= 50;
        klient->portfel_50--;
    }

    while (cena >= 20 && klient->portfel_20 > 0)
    {
        salon->zaplacone_20++;
        cena -= 20;
        klient->portfel_20--;
    }

    while (cena >= 10 && klient->portfel_10 > 0)
    {
        salon->zaplacone_10++;
        cena -= 10;
        klient->portfel_10--;
    }

    printf("Zapłacono: %dzł w nominalach: %d x 50z, %d x 20z, %d x 10z.\n",
           salon->zaplacona_kwota, salon->zaplacone_50, salon->zaplacone_20, salon->zaplacone_10);

    // Sygnał dla fryzjera, że klient zapłacił
    pthread_cond_signal(&salon->kasa.zaplata);
}

void klient_przychodzi_do_salon(Salon *salon, Klient *klient)
{
    while (1)
    {
        if (sem_trywait(&salon->poczekalnia) == 0)
        {
            printf("Klient wchodzi do poczekalni.\n");
            pthread_mutex_lock(&salon->mutex_poczekalnia);
            salon->klienci_w_poczekalni++;
            salon->kolejka.klienci[salon->kolejka.koniec] = klient;
            salon->kolejka.koniec = (salon->kolejka.koniec + 1) % 100;
            pthread_mutex_unlock(&salon->mutex_poczekalnia);

            // Czekaj na zapłatę
            pthread_cond_wait(&klient->czekaj_na_zaplate, &klient->mutex_klient);

            // Zapłać i odbierz resztę
            zaplac_za_usluge(klient, salon);
            odbierz_reszte(klient, &salon->kasa);

            // Zwalniamy miejsce w poczekalni po zakończeniu
            sem_post(&salon->poczekalnia);
            break;
        }
        else
        {
            usleep(1000000); // Czekaj, aż pojawi się miejsce
            printf("Brak wolnych miejsc w poczekalni. Klient wraca do zarabiania.\n");
        }
    }
}

void odbierz_reszte(Klient *klient, Kasa *kasa)
{
    pthread_mutex_lock(&klient->mutex_klient);

    // Oczekiwanie na sygnał o wydanej reszcie
    pthread_cond_wait(&kasa->wydano_reszte, &klient->mutex_klient);

    // Dodanie wydanych banknotów do portfela klienta
    klient->portfel_10 += kasa->wydane_10;
    klient->portfel_20 += kasa->wydane_20;
    klient->portfel_50 += kasa->wydane_50;

    printf("Klient odebrał resztę: %d x 10z, %d x 20z, %d x 50z.\n",
           kasa->wydane_10, kasa->wydane_20, kasa->wydane_50);

    pthread_mutex_unlock(&klient->mutex_klient);
}

void zakoncz_klienta(Klient *klient)
{
    pthread_cancel(klient->watek);     // Zakończenie wątku klienta
    pthread_join(klient->watek, NULL); // Dołączenie wątku
}