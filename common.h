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
#define RED "\033[31m"    // bledy
#define GREEN "\033[32m"  // kolor fryzjera
#define YELLOW "\033[33m" // otwarcie, zamkniecie salonu
#define BLUE "\033[34m"   // kolor klienta
#define MAGENTA "\033[35m"
#define CYAN "\033[36m"

/* Godziny – podawane w sekundach od startu symulacji */
extern int TP;           // początek (przed otwarciem salonu)
extern int TK;           // koniec
extern int sim_duration; // TK - TP

/*
 * Funkcja init_kasa:
 * Inicjalizuje kasę salonu, ustawiając początkowe wartości banknotów oraz
 * inicjalizując mutex i zmienną warunkową używaną do synchronizacji operacji na kasie.
 */

void *manager_input_thread(void *arg);
void *simulation_timer_thread(void *arg);
void *simulation_starter_thread(void *arg);

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