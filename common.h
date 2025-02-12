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
void setval_semafor(int id, int max);
int sem_try_wait(int id, int n);
int sem_getval(int id);
void sem_p(int id, int n);
void sem_v(int id, int n);
// dodatkowe utils.c
void error_exit(const char *msg);
void set_process_limit();
// fryzjer.c
void sygnal_1(int sig);
void zwolnij_zasoby_fryzjer();
// kierownik.c
void zwolnij_zasoby_kierownik();
void *simulation_timer_thread(void *arg);
void zakoncz_symulacje_czasu();
void czekaj_na_procesy(int n);
void wyslij_s1();
void wyslij_s2();
void szybki_koniec(int s);
void koniec(int s);
// klient.c
void sygnal_2(int sig);

#endif // COMMON_H