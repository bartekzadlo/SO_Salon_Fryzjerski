#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

/*
 * Funkcja managera (kierownika)
 * -----------------------------
 * Funkcja managera nadzoruje przebieg symulacji salonu fryzjerskiego.
 * Manager podejmuje decyzje dotyczące zakończenia pracy fryzjerów oraz
 * zmusza klientów do opuszczenia salonu w odpowiednich momentach.
 *
 * Działanie funkcji:
 * 1. Po 30 sekundach wysyła sygnał do fryzjera 0, informując go o konieczności
 *    zakończenia pracy (skończenie obsługi przed zamknięciem salonu).
 * 2. Po kolejnych 30 sekundach manager wysyła sygnał, aby wszyscy klienci natychmiast
 *    opuścili salon, a także rozpoczyna procedurę zamknięcia salonu.
 *
 * Używane mechanizmy synchronizacji:
 * - Sleep: opóźnia wykonanie operacji, symulując upływ czasu.
 * - Mutex i zmienne warunkowe: służą do powiadamiania wątków oczekujących w kolejce o zmianach
 *   stanu (np. gdy salon zamyka się lub gdy kasa wymaga uzupełnienia).
 */
void *manager_thread(void *arg)
{
    (void)arg; // Nieużywany parametr – brak potrzeby przekazywania argumentów do managera

    char log_buffer[MSG_SIZE]; // Bufor na komunikaty logujące

    /* Po 30 sekundach wysyłamy sygnał 1 do fryzjera 0, aby zakończył pracę */
    sleep(30);
    barber_stop[0] = 1; // Ustawiamy flagę zatrzymania dla fryzjera o identyfikatorze 0
    snprintf(log_buffer, MSG_SIZE, "Kierownik: wysyłam sygnał 1 do fryzjera 0 (skończenie pracy przed zamknięciem salonu).");
    send_message(log_buffer); // Logowanie komunikatu o wysłaniu sygnału do fryzjera

    /* Po kolejnych 30 sekundach wysyłamy sygnał 2 – wszyscy klienci natychmiast opuszczają salon */
    sleep(30);
    close_all_clients = 1;
    pthread_mutex_lock(&poczekalniaMutex);
    pthread_cond_broadcast(&poczekalniaNotEmpty); // Budzimy wszystkie wątki czekające na warunek poczekalni
    pthread_mutex_unlock(&poczekalniaMutex);
    /* Powiadamiamy wątki związane z kasą, że może nastąpić zmiana (np. rezygnacja z wydawania reszty) */
    pthread_mutex_lock(&kasa.mutex_kasa);
    pthread_cond_broadcast(&kasa.uzupelnienie); // Budzimy wszystkie wątki czekające na uzupełnienie kasy
    pthread_mutex_unlock(&kasa.mutex_kasa);

    /* Logowanie wysłania sygnału 2 oraz informacji o zamknięciu salonu */
    snprintf(log_buffer, MSG_SIZE, "Kierownik: wysyłam sygnał 2 (wszyscy klienci opuszczają salon) oraz zamykam salon.");
    send_message(log_buffer);
    salon_open = 0; /* Ustawienie flagi zamknięcia salonu, co sygnalizuje, że nie przyjmujemy już nowych klientów */
    return NULL;    // Zakończenie wątku managera
}