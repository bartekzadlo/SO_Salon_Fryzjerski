#ifndef KASA_H
#define KASA_H

#include <pthread.h>

typedef struct
{
    int banknot_10;
    int banknot_20;
    int banknot_50;
    pthread_mutex_t mutex;
} Kasa;

void inicjalizuj_kase(Kasa *kasa);
void zamknij_kase(Kasa *kasa);
void dodaj_banknoty(Kasa *kasa, int nominal, int ilosc);

#endif
