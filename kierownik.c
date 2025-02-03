#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

/* ------------------------------ */
/* Funkcja managera (kierownika) */
void *manager_thread(void *arg)
{
    (void)arg; // nieużywany parametr
    /* Po 30 sekundach wysyłamy sygnał 1 do fryzjera 0 */
    sleep(30);
    barber_stop[0] = 1;
    printf("Kierownik: wysyłam sygnał 1 do fryzjera 0 (skończenie pracy przed zamknięciem salonu).\n");

    /* Po kolejnych 30 sekundach wysyłamy sygnał 2 – wszyscy klienci natychmiast opuszczają salon */
    sleep(30);
    close_all_clients = 1;
    pthread_mutex_lock(&poczekalniaMutex);
    pthread_cond_broadcast(&poczekalniaNotEmpty);
    pthread_mutex_unlock(&poczekalniaMutex);

    pthread_mutex_lock(&kasa.mutex_kasa);
    pthread_cond_broadcast(&kasa.uzupelnienie);
    pthread_mutex_unlock(&kasa.mutex_kasa);

    printf("Kierownik: wysyłam sygnał 2 (wszyscy klienci opuszczają salon) oraz zamykam salon.\n");
    salon_open = 0;
    return NULL;
}