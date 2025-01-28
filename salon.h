#ifndef SALON_H
#define SALON_H

#include <pthread.h>
#include <semaphore.h>

typedef struct Fotel
{
    sem_t wolne_fotele;    // Semafor do kontrolowania liczby wolnych foteli
    pthread_mutex_t mutex; // Mutex dla foteli, aby zablokować dostęp do fotela
} Fotel;

typedef struct Salon
{
    sem_t poczekalnia;                 // Semafor do kontrolowania liczby wolnych miejsc w poczekalni
    pthread_mutex_t mutex_poczekalnia; // Mutex dla poczekalni
    Fotel fotel;                       // Fotel w salonie
    int klienci_w_poczekalni;          // Liczba klientów w poczekalni
} Salon;

void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli);
void zajmij_fotel(Fotel *fotel);
void zwolnij_fotel(Fotel *fotel);
void zamknij_salon(Salon *salon);

#endif // SALON_H
