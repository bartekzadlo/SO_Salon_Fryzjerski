#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

/* ------------------------------ */
/* Funkcja fryzjera */
void *barber_thread(void *arg)
{
    int id = *((int *)arg);
    free(arg);

    char log_buffer[MSG_SIZE];

    while (1)
    {
        /* Sprawdzenie, czy dla tego fryzjera wysłano sygnał 1 */
        if (barber_stop[id])
        {
            printf("Fryzjer %d: otrzymałem sygnał 1, kończę pracę.\n", id);
            send_message(log_buffer);
            break;
        }

        /* Czekaj, aż pojawi się klient w poczekalni */
        pthread_mutex_lock(&poczekalniaMutex);
        while (poczekalniaCount == 0 && salon_open && !close_all_clients)
        {
            pthread_cond_wait(&poczekalniaNotEmpty, &poczekalniaMutex);
        }
        if ((poczekalniaCount == 0 && !salon_open) || close_all_clients)
        {
            pthread_mutex_unlock(&poczekalniaMutex);
            break;
        }

        /* Pobierz klienta z kolejki */
        Klient *klient = poczekalnia[poczekalniaFront];
        poczekalniaFront = (poczekalniaFront + 1) % MAX_WAITING;
        poczekalniaCount--;
        pthread_mutex_unlock(&poczekalniaMutex);

        snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: rozpoczynam obsługę klienta %d (płatność: %d zł).", id, klient->id, klient->payment);
        send_message(log_buffer);

        /* Rezerwacja fotela (czekamy, aż fotel będzie wolny) */
        sem_wait(&fotele_semafor);

        /* Pobieranie opłaty – pieniądze trafiają do wspólnej kasy */
        pthread_mutex_lock(&kasa.mutex_kasa);
        if (klient->payment == 20)
        {
            kasa.banknot_20++;
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem 20 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
        }
        else if (klient->payment == 50)
        {
            kasa.banknot_50++;
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem 50 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
        }
        pthread_mutex_unlock(&kasa.mutex_kasa);

        /* Realizacja usługi – symulacja czasu strzyżenia (losowo 1–3 s) */
        int service_time = rand() % 3 + 1;
        sleep(service_time);
        snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: zakończyłem strzyżenie klienta %d (czas usługi: %d s).", id, klient->id, service_time);
        send_message(log_buffer);

        /* Aktualizacja statystyk w pamięci współdzielonej */
        __sync_fetch_and_add(&sharedStats->total_services_done, 1);

        /* Zwalniamy fotel */
        sem_post(&fotele_semafor);

        /* Wydawanie reszty, jeśli klient zapłacił 50 zł (reszta 30 zł = 10+20) */
        if (klient->payment == 50)
        {
            pthread_mutex_lock(&kasa.mutex_kasa);
            while ((kasa.banknot_10 < 1 || kasa.banknot_20 < 1) && !close_all_clients)
            {
                pthread_cond_wait(&kasa.uzupelnienie, &kasa.mutex_kasa);
            }
            if (close_all_clients)
            {
                pthread_mutex_unlock(&kasa.mutex_kasa);
                snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: salon zamknięty, nie wydaję reszty klientowi %d.", id, klient->id);
                send_message(log_buffer);
                sem_post(&klient->served);
                continue;
            }
            kasa.banknot_10--;
            kasa.banknot_20--;
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: wydaję resztę 30 zł (10+20) klientowi %d. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, klient->id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
            pthread_mutex_unlock(&kasa.mutex_kasa);
        }

        /* Powiadomienie klienta o zakończeniu obsługi */
        sem_post(&klient->served);
    }

    snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: wychodzę z pracy.", id);
    send_message(log_buffer);
    return NULL;
}