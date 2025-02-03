#ifndef COMMON_H
#define COMMON_H
#include <semaphore.h>
#include <pthread.h>

/* Parametry symulacji */
#define F 3           // liczba fryzjerów (F > 1)
#define N 2           // liczba foteli (N < F)
#define K 5           // maksymalna liczba klientów w poczekalni
#define P 10          // liczba klientów
#define MAX_WAITING K // rozmiar kolejki poczekalni

/* Struktura opisująca klienta */
typedef struct
{
    int id;         // Id Klienta
    int portfel_10; // Zawartość portfela klienta
    int portfel_20; // Zawartość portfela klienta
    int portfel_50; // Zawartość portfela klienta
    sem_t served;   // semafor, na którym klient czeka na zakończenie obsługi
} Klient;

/* Kolejka poczekalni – implementowana jako tablica cykliczna */
Klient *poczekalnia[MAX_WAITING];
int poczekalniaFront = 0; // indeks pierwszego oczekującego klienta
int poczekalniaCount = 0; // liczba klientów w poczekalni
pthread_mutex_t poczekalniaMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t poczekalniaNotEmpty = PTHREAD_COND_INITIALIZER;

// Struktura Kasa
typedef struct
{
    int banknot_10;              // ilosc banknotow w kasie
    int banknot_20;              // ilosc banknotow w kasie
    int banknot_50;              // ilosc banknotow w kasie
    pthread_cond_t uzupelnienie; // Sygnał dla fryzjera, że kasa uzupełniona
    pthread_mutex_t mutex_kasa;  // Mutex do synchronizacji dostępu do kasy
} Kasa;

// Funkcje operujące na salonie
void inicjalizuj_salon(Salon *salon, int max_klientow, int liczba_foteli);
void zajmij_fotel(Fotel *fotel);
void zwolnij_fotel(Fotel *fotel);
void zamknij_salon(Salon *salon, int shm_id);
void inicjalizuj_kase(Kasa *kasa);
void zamknij_kase(Kasa *kasa);

// Funkcje watku klienta
void inicjalizuj_klienta(Klient *klient, int id);              // Inicjalizacja klienta, nadanie mu id
void klient_przychodzi_do_salon(Salon *salon, Klient *klient); // Klient przychodzi do salonu
void zaplac_za_usluge(Klient *klient, Salon *salon);           // Klient płaci za usługę
void odbierz_reszte(Klient *klient, Kasa *kasa);
void zakoncz_klienta(Klient *klient);

// Funkcja watku klienta
void *fryzjer_praca(void *arg);
void inicjalizuj_fryzjera(Fryzjer *fryzjer, Salon *salon, int id);
void zakoncz_fryzjera(Fryzjer *fryzjer);
void dodaj_banknoty_do_kasy(Salon *salon);
void wydaj_reszte(Kasa *kasa, int reszta);

#endif // COMMON_H
