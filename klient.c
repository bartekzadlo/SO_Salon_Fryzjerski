#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "common.h"

/* ------------------------------ */
/* Funkcja klienta */
void *client_thread(void *arg)
{
    int id = *((int *)arg);
    free(arg);

    while (salon_open && !close_all_clients)
    {
        /* Symulacja „zarabiania pieniędzy” – oczekiwanie 1–5 s */
        int earning_time = rand() % 5 + 1;
        sleep(earning_time);

        /* Przygotowanie danych klienta */
        Klient *klient = malloc(sizeof(Klient));
        if (!klient)
        {
            perror("malloc");
            exit(1);
        }
        klient->id = id;
        /* Losujemy sposób płatności – 50% szans na 20 zł, 50% na 50 zł */
        if (rand() % 2 == 0)
            klient->payment = 20;
        else
            klient->payment = 50;

        /* Inicjalizacja semafora POSIX z wartością 0 */
        if (sem_init(&klient->served, 0, 0) != 0)
        {
            perror("sem_init");
            exit(1);
        }

        /* Próba wejścia do poczekalni */
        pthread_mutex_lock(&poczekalniaMutex);
        if (poczekalniaCount >= MAX_WAITING)
        {
            pthread_mutex_unlock(&poczekalniaMutex);
            printf("Klient %d: poczekalnia pełna, opuszczam salon.\n", id);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwalniamy pamięć
            continue;                     // klient wraca „zarabiać pieniądze”
        }
        int index = (poczekalniaFront + poczekalniaCount) % MAX_WAITING;
        poczekalnia[index] = klient;
        poczekalniaCount++;
        printf("Klient %d: wchodzę do poczekalni. Liczba oczekujących: %d.\n", id, poczekalniaCount);
        pthread_cond_signal(&poczekalniaNotEmpty);
        pthread_mutex_unlock(&poczekalniaMutex);

        /* Klient czeka na zakończenie obsługi */
        sem_wait(&klient->served);

        if (close_all_clients)
        {
            printf("Klient %d: salon zamknięty – opuszczam salon.\n", id);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwalniamy pamięć
            break;
        }

        printf("Klient %d: zostałem obsłużony i opuszczam salon.\n", id);
        sem_destroy(&klient->served); // Zwalniamy semafor
        free(klient);                 // Zwalniamy pamięć
    }
    printf("Klient %d: kończę pracę w symulacji salonu.\n", id);
    return NULL;
}
