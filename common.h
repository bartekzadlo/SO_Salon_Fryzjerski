#ifndef COMMON_H
#define COMMON_H
#include <semaphore.h>
#include <pthread.h>

#define MAX_KLIENCI_W_POCZEKALNI 5

typedef struct
{
    int id;                           // Id Klienta
    int portfel_10;                   // Zawartość portfela klienta
    int portfel_20;                   // Zawartość portfela klienta
    int portfel_50;                   // Zawartość portfela klienta
    pthread_cond_t czekaj_na_zaplate; // Warunek dla klienta od fryzjera że ten czeka na zapłate
    pthread_mutex_t mutex_klient;     // Mutex dostępu do klienta
    pthread_t watek;                  // Wątek klienta
} Klient;

// Struktura Fotel
typedef struct
{
    sem_t wolne_fotele;          // Semafor dla wolnych foteli
    pthread_mutex_t mutex_fotel; // Mutex do synchronizacji dostępu do fotela
} Fotel;

// Struktura Kasa
typedef struct
{
    int banknot_10;               // ilosc banknotow w kasie
    int banknot_20;               // ilosc banknotow w kasie
    int banknot_50;               // ilosc banknotow w kasie
    int wydane_10;                // uzywane przy zapamiętaniu ile wydajemy
    int wydane_20;                // uzywane przy zapamiętaniu ile wydajemy
    int wydane_50;                // uzywane przy zapamiętaniu ile wydajemy
    pthread_cond_t uzupelnienie;  // Sygnał dla fryzjera, że kasa uzupełniona
    pthread_cond_t zaplata;       // Sygnał dla fryzjera, że klient zapłacił
    pthread_cond_t wydano_reszte; // Warunek dla klienta, że fryzjer wydaje resztę
    pthread_mutex_t mutex_kasa;   // Mutex do synchronizacji dostępu do kasy
} Kasa;

// Struktura Salon
typedef struct
{
    sem_t poczekalnia;                 // Semafor dla poczekalni
    pthread_mutex_t mutex_poczekalnia; // Mutex do synchronizacji dostępu do poczekalni
    int klienci_w_poczekalni;          // Liczba klientów w poczekalni
    int max_klientow;
    Fotel fotel;         // Fotel z semaforem i mutexem
    Kasa kasa;           // Kasa z banknotami i mutexem
    int zaplacone_10;    // uzywane przy placeniu za usluge
    int zaplacone_20;    // uzywane przy placeniu za usluge
    int zaplacone_50;    // uzywane przy placeniu za usluge
    int zaplacona_kwota; // uzywane przy placeniu za usluge
    int reszta;
} Salon;

typedef struct
{
    int id;          // ID fryzjera
    Salon *salon;    // Wskaźnik do salonu
    Klient *klient;  // Wskaźnik na aktualnego klienta
    pthread_t watek; // Wątek fryzjera
} Fryzjer;

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
