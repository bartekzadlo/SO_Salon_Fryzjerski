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
    char log_buffer[MSG_SIZE];
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

        /* Inicjalizacja semafora z wartością 0 */
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
            snprintf(log_buffer, MSG_SIZE, "Klient %d: poczekalnia pełna, opuszczam salon.", id);
            send_message(log_buffer);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwalniamy pamięć
            continue;                     // klient wraca „zarabiać pieniądze”
        }
        int index = (poczekalniaFront + poczekalniaCount) % MAX_WAITING;
        poczekalnia[index] = klient;
        poczekalniaCount++;
        pthread_cond_signal(&poczekalniaNotEmpty);
        pthread_mutex_unlock(&poczekalniaMutex);
        snprintf(log_buffer, MSG_SIZE, "Klient %d: wchodzę do poczekalni. Liczba oczekujących: %d.", id, poczekalniaCount);
        send_message(log_buffer);

        /* Klient czeka na zakończenie obsługi */
        sem_wait(&klient->served);

        if (close_all_clients)
        {
            snprintf(log_buffer, MSG_SIZE, "Klient %d: salon zamknięty – opuszczam salon.", id);
            send_message(log_buffer);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwalniamy pamięć
            break;
        }

        snprintf(log_buffer, MSG_SIZE, "Klient %d: zostałem obsłużony i opuszczam salon.", id);
        send_message(log_buffer);
        sem_destroy(&klient->served); // Zwalniamy semafor
        free(klient);                 // Zwalniamy pamięć
    }
    snprintf(log_buffer, MSG_SIZE, "Klient %d: kończę pracę w symulacji salonu.", id);
    send_message(log_buffer);
    return NULL;
}
