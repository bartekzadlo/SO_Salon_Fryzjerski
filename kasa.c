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