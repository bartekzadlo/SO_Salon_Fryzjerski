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
#define F 20   // Liczba fryzjerów (F > 1) – ilość wątków reprezentujących fryzjerów
#define N 10   // Liczba foteli (N < F) – ograniczenie liczby jednocześnie obsługiwanych klientów
#define K 10   // Maksymalna liczba klientów w poczekalni
#define P 2000 // Liczba klientów – ilość wątków reprezentujących klientów

/* Kolory ANSI dla logowania w konsoli */
#define RESET "\033[0m"
#define RED "\033[31m"     // bledy
#define GREEN "\033[32m"   // kolor fryzjera
#define YELLOW "\033[33m"  // informacje o salonie, procesach
#define BLUE "\033[34m"    // kolor klienta
#define MAGENTA "\033[35m" // komunikat o czasie
#define CYAN "\033[36m"    // zwrot do użytkownika

struct Message
{
    long message_type;
    long nadawca;
    int platnosc;
};
// obsługa kolejki komunikatów
int stworz_kolejke_komunikatow(key_t msg_qkey);
void usun_kolejke_komunikatow(int msg_qid);
void wyslij_komunikat_do_kolejki(int msg_qid, struct Message *msg);
void pobierz_komunikat_z_kolejki(int msg_qid, struct Message *msg, long odbiorca_id);
// obsługa pamięci współdzielonej
int stworz_pamiec_dzielona(key_t shm_key);
int *dolacz_do_pamieci_dzielonej(int shm_id);
void odlacz_pamiec_dzielona(int *shm_ptr);
void usun_pamiec_dzielona(int shm_id);
// operacje na semaforach
int stworz_semafor(key_t sem_key);
void usun_semafor(int sem_id);
void sem_setval(int sem_id, int value);
int sem_try_wait(int id, int n);
int sem_getval(int id);
void sem_p(int id, int n);
void sem_v(int id, int n);
// dodatkowe utils.c
void error_exit(const char *msg);
void set_process_limit();
// fryzjer.c
void sig_handler_fryzjer(int sig);
void fryzjer_exit();
void wydaj_reszte();
void zajmij_kase();
void zwolnij_kase();
// kierownik.c
void zainicjalizuj_kase();
void zwolnij_zasoby_kierownik();
void *simulation_timer_thread(void *arg);
void stop_timer_thread();
void wait_for_process(int n);
void zabij_fryzjera();
void zabij_klientow();
void zabij_fryzjerow();
void sig_handler_int(int s);
void koniec(int s);
void tworz_fryzjerow();
void tworz_klientow();
// klient.c
void sig_handler_klient(int sig);

#endif // COMMON_H