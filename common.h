#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <errno.h>
#include <sys/shm.h>
#include <sys/msg.h>

/* Parametry symulacji */
#define F 7    // Liczba fryzjerów (F > 1) – ilość wątków reprezentujących fryzjerów
#define N 5    // Liczba foteli (N < F) – ograniczenie liczby jednocześnie obsługiwanych klientów
#define K 10   // Maksymalna liczba klientów w poczekalni
#define P 5000 // Liczba klientów – ilość wątków reprezentujących klientów
#define MAX_PROCESSES 8192

/* Kolory ANSI dla logowania w konsoli */
#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

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

/* Struktura opisująca klienta     */
typedef struct
{
    int id;       // Unikalny identyfikator klienta
    int payment;  // Kwota przekazywana przez klienta (może wynosić 20 lub 50 zł)
    sem_t served; // Semafor używany przez klienta do oczekiwania na zakończenie obsługi przez fryzjera
} Klient;

extern pthread_mutex_t poczekalniaMutex;   // Mutex służący do synchronizacji dostępu do kolejki poczekalni
extern pthread_cond_t poczekalniaNotEmpty; // Zmienna warunkowa, która sygnalizuje, że poczekalnia nie jest pusta

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

/* Godziny – podawane w sekundach od startu symulacji */
extern int TP;           // początek (przed otwarciem salonu)
extern int TK;           // koniec
extern int sim_duration; // TK - TP

/* Globalny identyfikator kolejki komunikatów */
extern int msgqid; // Identyfikator kolejki komunikatów używanej do logowania zdarzeń

/* Prototypy funkcji i procedur  */

/*
 * Funkcja init_kasa:
 * Inicjalizuje kasę salonu, ustawiając początkowe wartości banknotów oraz
 * inicjalizując mutex i zmienną warunkową używaną do synchronizacji operacji na kasie.
 */
void init_kasa();

void *manager_input_thread(void *arg);
void *simulation_timer_thread(void *arg);
void *simulation_starter_thread(void *arg);

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

struct komunikat
{
    long mtype;
    long nadawca;
};
// obsługa kolejki komunikatów
int utworz_kolejke(key_t klucz);
void usun_kolejke(int kolejka);
void wyslij_komunikat(int kolejka, struct komunikat *kom);
void odbierz_komunikat(int kolejka, struct komunikat *kom, long odbiorca);
// obsługa pamięci współdzielonej
int utworz_pamiec_dzielona(key_t klucz);
int dolacz_pamiec_dzielona(int shm_id);
void odlacz_pamiec_dzielona(int *ptr);
void usun_pamiec_dzielona(int shm_id);
// operacje na semaforach
int utworz_semafor(key_t klucz);
int sem_try_wait(int id, int n);
int sem_getval(int id);

#endif // COMMON_H