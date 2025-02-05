#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "common.h"

void *simulation_timer_thread(void *arg)
{
    (void)arg;
    int remaining = sim_duration;
    char buf[64];

    while (remaining > 0 && !close_all_clients)
    {
        snprintf(buf, sizeof(buf), "Czas pozostały: %d s", remaining);
        printf(GREEN "%s\n" RESET, buf);
        sleep(1);
        remaining--;
    }
    if (!close_all_clients)
    {
        close_all_clients = 1;
        salon_open = 0; // Zamykamy salon, aby nowe wejścia nie były przyjmowane
        pthread_mutex_lock(&poczekalniaMutex);
        pthread_cond_broadcast(&poczekalniaNotEmpty); // budzimy fryzjerów
        while (poczekalniaCount > 0)
        {
            Klient *klient = poczekalnia[poczekalniaFront];
            poczekalniaFront = (poczekalniaFront + 1) % K;
            poczekalniaCount--;
            sem_post(&klient->served);
        }
        pthread_mutex_unlock(&poczekalniaMutex);
        send_message("Czas symulacji upłynął. Wysłany sygnał 2: Salon zamykany.");
    }
    return NULL;
}