#ifndef KLIENT_H
#define KLIENT_H

#include <pthread.h>

typedef struct
{
    int banknot_10;
    int banknot_20;
    int banknot_50;
    pthread_mutex_t mutex;
} Portfel;

typedef struct
{
    Portfel portfel;
} Klient;

void inicjalizuj_portfel(Portfel *portfel);
void dodaj_pieniadze_do_portfela(Portfel *portfel, int nominal, int ilosc);
void zarabiaj_pieniadze(Klient *klient);
void zamknij_portfel(Portfel *portfel);

#endif
