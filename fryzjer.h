#ifndef FRYZJER_H
#define FRYZJER_H
#include <semaphore.h>
#include <pthread.h>
#include "salon.h"
#include "klient.h"

typedef struct
{
    int id;          // ID fryzjera
    Salon *salon;    // Wskaźnik do salonu
    Klient *klient;  // Wskaźnik na aktualnego klienta
    pthread_t watek; // Wątek fryzjera
} Fryzjer;

// Funkcja reprezentująca pracę fryzjera
void *fryzjer_praca(void *arg);

// Inicjalizacja fryzjera
void inicjalizuj_fryzjera(Fryzjer *fryzjer, Salon *salon, int id);

// Zakończenie pracy fryzjera
void zakoncz_fryzjera(Fryzjer *fryzjer);

#endif // FRYZJER_H
