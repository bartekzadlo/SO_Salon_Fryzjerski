#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "klient.h"
#include "salon.h"

void inicjalizuj_klienta(Klient *klient, int id)
{
    klient->id = id;
    klient->portfel_10 = 0;
    klient->portfel_20 = 0;
    klient->portfel_50 = 0;
    pthread_create(&klient->watek, NULL, dzialanie_klienta, klient);
}

void zarabiaj_pieniadze(Klient *klient)
{
    srand(time(NULL));
    int liczba_banknotow = rand() % 10 + 1;

    for (int i = 0; i < liczba_banknotow; i++)
    {
        int banknot = rand() % 3;
        if (banknot == 0)
        {
            klient->portfel_10 += 1;
        }
        else if (banknot == 1)
        {
            klient->portfel_20 += 1;
        }
        else
        {
            klient->portfel_50 += 1;
        }
    }
    printf("Klient zarobił:\n 10z: %d\n 20z: %d\n 50z: %d\n",
           klient->portfel_10, klient->portfel_20, klient->portfel_50);
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
    pthread_cond_signal(&salon->kasa.uzupelnienie);
}

void klient_przychodzi_do_salon(Salon *salon, Klient *klient, int Tp, int Tk)
{
    int czas_przyjscia = Tp + rand() % (Tk - Tp + 1);
    printf("Klient pojawia się o godzinie %d:00.\n", czas_przyjscia);

    while (1)
    {
        if (sem_trywait(&salon->poczekalnia) == 0)
        {
            printf("Klient wchodzi do poczekalni.\n");       // klient wchodzi do poczekalni
            pthread_mutex_lock(&salon->mutex_poczekalnia);   // klient blokuje mutex na poczekalnie
            salon->klienci_w_poczekalni++;                   // dodajemy osobe do poczekalni
            pthread_mutex_unlock(&salon->mutex_poczekalnia); // odblokowujemy mutex
            pthread_cond_wait(&klient->czekaj_na_zaplate, &klient->mutex);
            zaplac_za_usluge(klient);
            printf("Fryzjer obsługuje klienta.\n");

            printf("Tu informacja od fryzjera o reszcie.\n");

            pthread_mutex_lock(&salon->mutex_poczekalnia);
            salon->klienci_w_poczekalni--;
            pthread_mutex_unlock(&salon->mutex_poczekalnia);
            sem_post(&salon->poczekalnia);
            break;
        }
        else
        {
            printf("Brak wolnych miejsc w poczekalni. Klient wraca do zarabiania.\n");
            zarabiaj_pieniadze(klient);
        }
    }
}

void otrzymaj_reszte(Klient *klient)
{
    printf("Tu informacja od fryzjera o reszcie.\n");
}