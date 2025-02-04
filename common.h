#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>
#include <pthread.h>
#include <sys/msg.h>

/* Parametry symulacji */
#define F 3           // liczba fryzjerów (F > 1)
#define N 2           // liczba foteli (N < F)
#define K 5           // maksymalna liczba klientów w poczekalni
#define P 10          // liczba klientów
#define MAX_WAITING K // rozmiar kolejki poczekalni

/* Definicja struktury wiadomości */
#define MSG_SIZE 128
#define MSG_TYPE_EVENT 1
#define MSG_TYPE_EXIT 999

typedef struct
{
    long mtype;
    char mtext[MSG_SIZE];
} Message;

/* Struktura opisująca klienta */
typedef struct
{
    int id;       // Id Klienta
    int payment;  // kwota przekazywana (20 lub 50 zł)
    sem_t served; // semafor, na którym klient czeka na zakończenie obsługi
} Klient;

/* Kolejka poczekalni – implementowana jako tablica cykliczna */
extern Klient *poczekalnia[MAX_WAITING];
extern int poczekalniaFront; // indeks pierwszego oczekującego klienta
extern int poczekalniaCount; // liczba klientów w poczekalni
extern pthread_mutex_t poczekalniaMutex;
extern pthread_cond_t poczekalniaNotEmpty;

extern sem_t fotele_semafor;

// Struktura Kasa
typedef struct
{
    int banknot_10;              // ilość banknotów w kasie
    int banknot_20;              // ilość banknotów w kasie
    int banknot_50;              // ilość banknotów w kasie
    pthread_cond_t uzupelnienie; // Sygnał dla fryzjera, że kasa uzupełniona
    pthread_mutex_t mutex_kasa;  // Mutex do synchronizacji dostępu do kasy
} Kasa;
extern Kasa kasa;

/* Flagi sterujące symulacją */
extern int salon_open;        // salon czynny
extern int close_all_clients; // sygnał 2: wszyscy klienci natychmiast opuszczają salon
extern int barber_stop[F];    // dla każdego fryzjera – sygnał 1, aby zakończył pracę

/* Globalny identyfikator kolejki komunikatów */
extern int msgqid;

/* Funkcja inicjalizująca kasę */
void init_kasa();

/* Prototypy funkcji wątków */
void *barber_thread(void *arg);
void *client_thread(void *arg);
void *manager_thread(void *arg);

/* Funkcja pomocnicza wysyłająca komunikat do kolejki */
void send_message(const char *text);

/* Proces loggera – odbiera komunikaty z kolejki i wypisuje je */
void logger_process();

#endif // COMMON_H