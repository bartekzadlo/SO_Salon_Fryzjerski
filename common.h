#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>
#include <pthread.h>

/* Parametry symulacji            */
#define F 3           // Liczba fryzjerów (F > 1) – ilość wątków reprezentujących fryzjerów
#define N 2           // Liczba foteli (N < F) – ograniczenie liczby jednocześnie obsługiwanych klientów
#define K 5           // Maksymalna liczba klientów w poczekalni
#define P 10          // Liczba klientów – ilość wątków reprezentujących klientów
#define MAX_WAITING K // Rozmiar kolejki poczekalni – wykorzystujemy K jako maksymalną liczbę oczekujących klientów

/* Definicja struktury wiadomości */
#define MSG_SIZE 128      // Maksymalny rozmiar tekstu w komunikacie
#define MSG_TYPE_EVENT 1  // Typ komunikatu: zdarzenie (np. log zdarzeń)
#define MSG_TYPE_EXIT 999 // Typ komunikatu: sygnał zakończenia (używany do zamknięcia loggera)

/*
 * Struktura Message:
 * Służy do komunikacji między procesami (np. do logowania zdarzeń) przy użyciu kolejki komunikatów.
 * mtype – typ komunikatu (np. zdarzenie lub exit)
 * mtext – tekst komunikatu (do 128 znaków)
 */
typedef struct
{
    long mtype;
    char mtext[MSG_SIZE];
} Message;

/* Struktura przechowywana w pamięci współdzielonej */
typedef struct
{
    int total_clients_served; // Liczba klientów, którzy zostali obsłużeni
    int total_clients_left;   // Liczba klientów, którzy opuścili salon (np. gdy poczekalnia była pełna)
    int total_services_done;  // Liczba wykonanych usług (np. ilość przeprowadzonych strzyżeń)
} SalonStats;
extern SalonStats *sharedStats; // Wskaźnik do struktury statystyk salonu (przechowywanych w pamięci współdzielonej)

/* Struktura opisująca klienta     */
typedef struct
{
    int id;       // Unikalny identyfikator klienta
    int payment;  // Kwota przekazywana przez klienta (może wynosić 20 lub 50 zł)
    sem_t served; // Semafor używany przez klienta do oczekiwania na zakończenie obsługi przez fryzjera
} Klient;

/* Kolejka poczekalni – implementowana jako tablica cykliczna */
extern Klient *poczekalnia[MAX_WAITING];   // Tablica wskaźników na klientów reprezentująca poczekalnię
extern int poczekalniaFront;               // Indeks pierwszego oczekującego klienta (początek kolejki)
extern int poczekalniaCount;               // Liczba klientów aktualnie oczekujących w poczekalni
extern pthread_mutex_t poczekalniaMutex;   // Mutex służący do synchronizacji dostępu do kolejki poczekalni
extern pthread_cond_t poczekalniaNotEmpty; // Zmienna warunkowa, która sygnalizuje, że poczekalnia nie jest pusta

/* Semafor foteli                 */
extern sem_t fotele_semafor; // Semafor kontrolujący liczbę dostępnych foteli (ograniczenie liczby obsługiwanych klientów)

/* Struktura Kasa                 */
typedef struct
{
    int banknot_10;              // Liczba banknotów o nominale 10 zł dostępnych w kasie
    int banknot_20;              // Liczba banknotów o nominale 20 zł dostępnych w kasie
    int banknot_50;              // Liczba banknotów o nominale 50 zł dostępnych w kasie
    pthread_cond_t uzupelnienie; // Zmienna warunkowa sygnalizująca fryzjerom, że kasa została uzupełniona
    pthread_mutex_t mutex_kasa;  // Mutex służący do synchronizacji dostępu do kasy
} Kasa;
extern Kasa kasa; // Globalna struktura kasy salonu

/* Flagi sterujące symulacją      */
extern int salon_open;        // Flaga informująca, czy salon jest czynny (1 – otwarty, 0 – zamknięty)
extern int close_all_clients; // Flaga sygnalizująca, że wszyscy klienci muszą natychmiast opuścić salon (sygnał 2)
extern int barber_stop[F];    // Tablica flag dla fryzjerów – dla każdego fryzjera sygnał (1), aby zakończył pracę

/* Globalny identyfikator kolejki komunikatów */
extern int msgqid; // Identyfikator kolejki komunikatów używanej do logowania zdarzeń

/* Prototypy funkcji i procedur  */

/*
 * Funkcja init_kasa:
 * Inicjalizuje kasę salonu, ustawiając początkowe wartości banknotów oraz
 * inicjalizując mutex i zmienną warunkową używaną do synchronizacji operacji na kasie.
 */
void init_kasa();

/*
 * Prototypy funkcji wątków:
 * - barber_thread: Funkcja reprezentująca pracę fryzjera.
 * - client_thread: Funkcja reprezentująca działanie klienta.
 * - manager_thread: Funkcja reprezentująca pracę managera (kierownika) salonu.
 */
void *barber_thread(void *arg);
void *client_thread(void *arg);
void *manager_thread(void *arg);

/*
 * Funkcja send_message:
 * Wysyła komunikat do kolejki komunikatów.
 * Używana głównie do logowania zdarzeń w symulacji.
 */
void send_message(const char *text);

/*
 * Funkcja logger_process:
 * Proces loggera, który odbiera komunikaty z kolejki i wypisuje je na standardowe wyjście.
 * Proces kończy działanie po otrzymaniu komunikatu zakończenia (MSG_TYPE_EXIT).
 */
void logger_process();

#endif // COMMON_H