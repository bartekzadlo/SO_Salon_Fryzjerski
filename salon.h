#ifndef SALON_H
#define SALON_H
#include <semaphore.h>
#include <pthread.h>

typedef struct
{
    int id;
    int portfel_10;
    int portfel_20;
    int portfel_50;
    pthread_cond_t czekaj_na_zaplate;
    pthread_mutex_t mutex;
} Klient;

typedef struct
{
    Klient *klienci[100]; // Tablica wskaźników na klientów
    int poczatek;         // Indeks początku kolejki
    int koniec;           // Indeks końca kolejki
} KolejkaKlientow;

// Struktura Fotel
typedef struct
{
    sem_t wolne_fotele;          // Semafor dla wolnych foteli
    pthread_mutex_t mutex_fotel; // Mutex do synchronizacji dostępu do fotela
} Fotel;

// Struktura Kasa
typedef struct
{
    int banknot_10;
    int banknot_20;
    int banknot_50;
    int wydane_10;
    int wydane_20;
    int wydane_50;
    pthread_mutex_t mutex_kasa;  // Mutex do synchronizacji dostępu do kasy
    pthread_cond_t uzupelnienie; // Warunek dla uzupełnienia kasy
    pthread_cond_t zaplata;
    pthread_cond_t wydano_reszte;
} Kasa;

// Struktura Salon
typedef struct
{
    sem_t poczekalnia;                 // Semafor dla poczekalni
    pthread_mutex_t mutex_poczekalnia; // Mutex do synchronizacji dostępu do poczekalni
    int klienci_w_poczekalni;          // Liczba klientów w poczekalni
    int max_klientow;
    Fotel fotel; // Fotel z semaforem i mutexem
    Kasa kasa;   // Kasa z banknotami i mutexem
    int zaplacone_10;
    int zaplacone_20;
    int zaplacone_50;
    int zaplacona_kwota;
    int reszta;
    KolejkaKlientow kolejka;
} Salon;

// Funkcje operujące na salonie
void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli);
void zajmij_fotel(Fotel *fotel);
void zwolnij_fotel(Fotel *fotel);
void zamknij_salon(Salon *salon);

// Funkcje operujące na kasie
void inicjalizuj_kase(Kasa *kasa);
void zamknij_kase(Kasa *kasa);
void dodaj_banknoty_do_kasy(Salon *salon);
void wydaj_reszte(Kasa *kasa, int reszta);

#endif // SALON_H
