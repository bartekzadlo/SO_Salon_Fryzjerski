#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "klient.h"
#include "kasa.h"
#include "salon.h"

void inicjalizuj_portfel(Klient *klient)
{
    klient->portfel_10 = 0;
    klient->portfel_20 = 0;
    klient->portfel_50 = 0;
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

void klient_przychodzi_do_salon(Salon *salon, Klient *klient, int Tp, int Tk)
{
    int czas_przyjscia = rand() % (Tk - Tp + 1) + Tp;
    printf("Klient pojawia się o godzinie %d:00.\n", czas_przyjscia);

    while (1)
    {
        if (sem_trywait(&salon->poczekalnia) == 0)
        {
            printf("Klient wchodzi do poczekalni.\n");

            pthread_mutex_lock(&salon->mutex_poczekalnia);
            salon->klienci_w_poczekalni++;
            pthread_mutex_unlock(&salon->mutex_poczekalnia);

            zajmij_fotel(&salon->fotel);
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

void zaplac_za_usluge(Klient *klient)
{
    int cena = (rand() % 10 + 1) * 10;
    printf("Cena za usługę: %dzł.\n", cena);

    int kwota_do_zaplaty = 0;

    if (klient->portfel_50 >= cena / 50)
    {
        kwota_do_zaplaty = cena;
        klient->portfel_50 -= cena / 50;
    }
    else if (klient->portfel_20 >= cena / 20)
    {
        kwota_do_zaplaty = cena;
        klient->portfel_20 -= cena / 20;
    }
    else if (klient->portfel_10 >= cena / 10)
    {
        kwota_do_zaplaty = cena;
        klient->portfel_10 -= cena / 10;
    }
    else
    {
        printf("Klient nie ma wystarczającej ilości pieniędzy. Wraca do zarabiania.\n");
        zarabiaj_pieniadze(klient);
    }

    printf("Klient płaci za usługę: %d zł.\n", kwota_do_zaplaty);
}

void otrzymaj_reszte(Klient *klient)
{
    printf("Tu informacja od fryzjera o reszcie.\n");
}
