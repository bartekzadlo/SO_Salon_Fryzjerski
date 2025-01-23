#ifndef FOTEL_H
#define FOTEL_H

#include <pthread.h>
#include <semaphore.h>

typedef struct
{
    sem_t wolne_fotele;
    pthread_mutex_t mutex;
} Fotel;

void inicjalizuj_fotel(Fotel *fotel, int liczba_foteli);
void zajmij_fotel(Fotel *fotel);
void zwolnij_fotel(Fotel *fotel);

#endif
