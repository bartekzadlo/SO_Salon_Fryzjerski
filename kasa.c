#include <stdio.h>
#include "kasa.h"

void inicjalizuj_kase(Kasa *kasa)
{
    kasa->banknot_10 = 0;
    kasa->banknot_20 = 0;
    kasa->banknot_50 = 0;
    pthread_mutex_init(&kasa->mutex, NULL);
}

void zamknij_kase(Kasa *kasa)
{
    pthread_mutex_destroy(&kasa->mutex);
}

void dodaj_banknoty(Kasa *kasa, int nominal, int ilosc)
{
    pthread_mutex_lock(&kasa->mutex);

    if (nominal == 10)
    {
        kasa->banknot_10 += ilosc;
    }
    else if (nominal == 20)
    {
        kasa->banknot_20 += ilosc;
    }
    else if (nominal == 50)
    {
        kasa->banknot_50 += ilosc;
    }

    pthread_mutex_unlock(&kasa->mutex);
}

void odejmij_banknoty(Kasa *kasa, int nominal, int ilosc)
{
    pthread_mutex_lock(&kasa->mutex);

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

    pthread_mutex_unlock(&kasa->mutex);
}