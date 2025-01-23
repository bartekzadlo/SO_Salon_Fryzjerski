#include "fotel.h"
#include <stdio.h>
#include <stdlib.h>

void inicjalizuj_fotel(Fotel *fotel, int liczba_foteli)
{
    sem_init(&fotel->wolne_fotele, 0, liczba_foteli);
    pthread_mutex_init(&fotel->mutex, NULL);
}

void zajmij_fotel(Fotel *fotel)
{
    sem_wait(&fotel->wolne_fotele);
    pthread_mutex_lock(&fotel->mutex);
    printf("Fryzjer zajmował fotel.\n");
    pthread_mutex_unlock(&fotel->mutex);
}

void zwolnij_fotel(Fotel *fotel)
{
    pthread_mutex_lock(&fotel->mutex);
    sem_post(&fotel->wolne_fotele);
    printf("Fryzjer zwolnił fotel.\n");
    pthread_mutex_unlock(&fotel->mutex);
}

void usun_fotel(Fotel *fotel)
{
    sem_destroy(&fotel->wolne_fotele);
    pthread_mutex_destroy(&fotel->mutex);
}
