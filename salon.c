#include "salon.h"
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>

void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli)
{
    sem_init(&salon->poczekalnia, 0, max_klientow);      // Inicjalizowanie semafora dla poczekalni
    pthread_mutex_init(&salon->mutex_poczekalnia, NULL); // Inicjalizowanie mutexu dla poczekalni
    salon->klienci_w_poczekalni = 0;

    // Inicjalizacja fotela (semafor dla wolnych foteli)
    sem_init(&salon->fotel.wolne_fotele, 0, liczba_foteli);
    pthread_mutex_init(&salon->fotel.mutex, NULL);
}

void zajmij_fotel(Fotel *fotel)
{
    sem_wait(&fotel->wolne_fotele);    // Czekaj na wolny fotel
    pthread_mutex_lock(&fotel->mutex); // Zablokuj mutex, aby fotel był zajęty przez jednego klienta
    printf("Klient zajmuje fotel.\n");
    pthread_mutex_unlock(&fotel->mutex); // Zwolnij mutex
}

void zwolnij_fotel(Fotel *fotel)
{
    pthread_mutex_lock(&fotel->mutex); // Zablokuj mutex przed zwolnieniem fotela
    sem_post(&fotel->wolne_fotele);    // Zwolnij fotel
    printf("Fotel został zwolniony.\n");
    pthread_mutex_unlock(&fotel->mutex); // Zwolnij mutex
}

void zamknij_salon(Salon *salon)
{
    sem_destroy(&salon->poczekalnia);                 // Zwalniamy semafor dla poczekalni
    pthread_mutex_destroy(&salon->mutex_poczekalnia); // Zwalniamy mutex dla poczekalni

    // Niszczenie zasobów związanych z fotelami
    sem_destroy(&salon->fotel.wolne_fotele);    // Zwalniamy semafor dla foteli
    pthread_mutex_destroy(&salon->fotel.mutex); // Zwalniamy mutex dla foteli
}
