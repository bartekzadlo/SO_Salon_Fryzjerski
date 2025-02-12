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
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
#include <sys/resource.h>

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
 * Struktura Message dla loggera:
 * mtype – typ komunikatu (np. zdarzenie lub exit)
 * mtext – tekst komunikatu (do 128 znaków)
 */
typedef struct
{
    long mtype;
    char mtext[MSG_SIZE];
} Message;

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
    int platnosc;
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
void setval_semafor(int id, int max);
int sem_getval(int id);
void sem_p(int id, int n);
void sem_v(int id, int n);

#endif // COMMON_H