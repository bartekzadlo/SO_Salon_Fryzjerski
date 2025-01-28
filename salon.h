#ifndef SALON_H
#define SALON_H
#include <semaphore.h>
#include <pthread.h>

// Struktura Fotel
typedef struct
{
    // Semafor i mutex dla foteli
    sem_t wolne_fotele;          // Semafor dla wolnych foteli
    pthread_mutex_t mutex_fotel; // Mutex do synchronizacji dostępu do fotela

    // Semafor i mutex dla poczekalni
    sem_t poczekalnia;                 // Semafor dla poczekalni
    pthread_mutex_t mutex_poczekalnia; // Mutex do synchronizacji dostępu do poczekalni
    int klienci_w_poczekalni;          // Liczba klientów w poczekalni

    // Struktura kasy
    int banknot_10;
    int banknot_20;
    int banknot_50;
    pthread_mutex_t mutex_kasa;  // Mutex do synchronizacji dostępu do kasy
    pthread_cond_t uzupelnienie; // Warunek dla uzupełnienia kasy
} Salon;

// Funkcje operujące na salonie
void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli);
void zajmij_fotel(Fotel *fotel);
void zwolnij_fotel(Fotel *fotel);
void zamknij_salon(Salon *salon);

// Funkcje operujące na kasie
void inicjalizuj_kase(Kasa *kasa);
void zamknij_kase(Kasa *kasa);
void dodaj_banknoty(Kasa *kasa, int nominal, int ilosc);
void odejmij_banknoty(Kasa *kasa, int nominal, int ilosc);

#endif // SALON_H
