#ifndef KLIENT_H
#define KLIENT_H
#include <semaphore.h>
#include <pthread.h>
#include "salon.h"

struct Salon;

typedef struct Klient
{
    int id;
    int portfel_10;
    int portfel_20;
    int portfel_50;
    pthread_cond_t czekaj_na_zaplate;
    pthread_mutex_t mutex;
} Klient;

void inicjalizuj_klienta(Klient *klient, int id)                               // Inicjalizacja klienta, nadanie mu id
    void zarabiaj_pieniadze(Klient *klient);                                   // Losowanie i dodawanie banknotów do portfela klienta
void klient_przychodzi_do_salon(Salon *salon, Klient *klient, int Tp, int Tk); // Klient przychodzi do salonu
void zaplac_za_usluge(Klient *klient, Salon *salon);                           // Klient płaci za usługę
void otrzymaj_reszte(Klient *klient);                                          // Klient otrzymuje resztę (tymczasowa funkcjonalność)

#endif
