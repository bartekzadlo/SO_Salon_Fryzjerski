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
    int id;       // Id Klienta
    int payment;  // kwota przekazywana (20 lub 50 zł)
    sem_t served; // semafor, na którym klient czeka na zakończenie obsługi
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

/* Flagi sterujące symulacją */
int salon_open = 1;        // salon czynny
int close_all_clients = 0; // sygnał 2: wszyscy klienci natychmiast opuszczają salon
int barber_stop[F] = {0};  // dla każdego fryzjera – sygnał 1, aby zakończył pracę

#endif // COMMON_H
