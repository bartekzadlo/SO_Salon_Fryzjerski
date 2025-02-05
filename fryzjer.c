#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

/*
 * Funkcja barber_thread
 * ---------------------
 * Reprezentuje pracę fryzjera w symulacji. Każdy fryzjer wykonuje tę funkcję
 * w osobnym wątku. Funkcja wykonuje pętlę, w której fryzjer:
 *   - Czeka na klientów, którzy pojawią się w poczekalni.
 *   - Pobiera klienta z kolejki.
 *   - Realizuje usługę strzyżenia, pobiera opłatę i aktualizuje kasę.
 *   - W razie potrzeby wydaje resztę.
 *   - Aktualizuje statystyki oraz sygnalizuje klientowi zakończenie obsługi.
 * W przypadku otrzymania sygnału zakończenia pracy, fryzjer przerywa pętlę i wychodzi.
 */
void *barber_thread(void *arg)
{
    int id = *((int *)arg); // Pobranie identyfikatora fryzjera przekazanego jako argument wątku
    free(arg);              // Zwolnienie pamięci przydzielonej dla argumentu

    char log_buffer[MSG_SIZE]; // Bufor do przechowywania komunikatów logujących

    while (1) // Główna pętla pracy fryzjera
    {
        /* Sprawdzenie, czy dla tego fryzjera wysłano sygnał zakończenia pracy.
         * Jeśli flaga barber_stop dla danego fryzjera jest ustawiona, kończymy pracę.
         */
        if (barber_stop[id])
        {
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem sygnał 1, kończę pracę.", id);
            send_message(log_buffer); // Wysłanie komunikatu do loggera
            break;
        }

        /* Czekanie, aż pojawi się klient w poczekalni.
         * Blokujemy dostęp do zmiennych poczekalni przy użyciu mutexa.
         */
        pthread_mutex_lock(&poczekalniaMutex);
        while (poczekalniaCount == 0 && salon_open && !close_all_clients)
        {
            pthread_cond_wait(&poczekalniaNotEmpty, &poczekalniaMutex); // Czekamy, aż warunek (poczekalnia nie jest pusta) zostanie spełniony.
        }
        if ((poczekalniaCount == 0 && !salon_open) || close_all_clients) // Jeśli poczekalnia jest pusta i salon jest zamknięty lub mamy sygnał zakończenia, odblokowujemy mutex i wychodzimy z pętli
        {
            pthread_mutex_unlock(&poczekalniaMutex);
            break;
        }

        /* Pobieramy klienta z poczekalni.
         * Klient znajduje się na pozycji wskazywanej przez poczekalniaFront.
         * Następnie aktualizujemy indeks i zmniejszamy licznik klientów w poczekalni.
         */
        Klient *klient = poczekalnia[poczekalniaFront];
        poczekalniaFront = (poczekalniaFront + 1) % K;
        poczekalniaCount--;
        pthread_mutex_unlock(&poczekalniaMutex); // Odblokowanie mutexa poczekalni

        snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: rozpoczynam obsługę klienta %d (płatność: %d zł).", id, klient->id, klient->payment);
        send_message(log_buffer);

        /* Rezerwacja fotela – symulujemy zajęcie fotela przez klienta.
         * Semafor fotele_semafor kontroluje liczbę dostępnych foteli.
         * Funkcja sem_wait blokuje wątek do momentu, aż fotel będzie dostępny.
         */
        sem_wait(&fotele_semafor);

        snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: zająłem fotel.", id);
        send_message(log_buffer);
        /* Pobieranie opłaty od klienta i aktualizacja kasy.
         * Blokujemy dostęp do kasy używając mutexa, aby zapewnić spójność danych.
         */
        pthread_mutex_lock(&kasa.mutex_kasa);
        if (klient->payment == 30)
        {
            kasa.banknot_10++;
            kasa.banknot_20++; // Klient płaci 20 zł – zwiększamy licznik banknotów 20 zł
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem 20 i 10 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
        }
        else if (klient->payment == 50)
        {
            kasa.banknot_50++; // Klient płaci 50 zł – zwiększamy licznik banknotów 50 zł
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem 50 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
        }
        pthread_mutex_unlock(&kasa.mutex_kasa);

        /* Realizacja usługi – symulacja czasu strzyżenia.
         * Losujemy czas trwania usługi (od 1 do 3 sekund) i "usypiamy" wątek.
         */
        int service_time = rand() % 3 + 1;
        int elapsed = 0;
        while (elapsed < service_time)
        {
            if (close_all_clients)
                break;
            sleep(1);
            elapsed++;
        }
        if (close_all_clients)
        {
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: przerwałem obsługę klienta %d z powodu zamknięcia salonu.",
                     id, klient->id);
            send_message(log_buffer);
        }
        else
        {
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: zakończyłem strzyżenie klienta %d (czas usługi: %d s).",
                     id, klient->id, service_time);
            send_message(log_buffer);
            __sync_fetch_and_add(&sharedStats->total_services_done, 1);
        }

        /* Zwalnianie fotela – klient opuszcza fotel po zakończeniu usługi.
         * Uwalniamy semafor, co umożliwia zajęcie fotela kolejnemu klientowi.
         */
        sem_post(&fotele_semafor);

        /* Wydawanie reszty klientowi, jeśli zapłacił 50 zł.
         * Reszta wynosi 30 zł, składająca się z banknotów 10 zł i 20 zł.
         */
        if (klient->payment == 50 && !close_all_clients)
        {
            pthread_mutex_lock(&kasa.mutex_kasa);
            while ((kasa.banknot_10 < 1 || kasa.banknot_20 < 1) && !close_all_clients) // Czekamy, aż w kasie będą dostępne wymagane banknoty (10 zł oraz 20 zł)
            {
                pthread_cond_wait(&kasa.uzupelnienie, &kasa.mutex_kasa);
            }
            if (close_all_clients) // Jeśli salon się zamyka, przerywamy operację wydawania reszty
            {
                pthread_mutex_unlock(&kasa.mutex_kasa);
                snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: salon zamknięty, nie wydaję reszty klientowi %d.", id, klient->id);
                send_message(log_buffer);
                sem_post(&klient->served); // Sygnalizujemy klientowi, że został obsłużony (choć reszta nie została wydana)
                continue;
            }
            // Wydajemy resztę – zmniejszamy liczbę banknotów w kasie

            kasa.banknot_20--;
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: wydaję resztę 20 zł klientowi %d. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, klient->id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
            pthread_mutex_unlock(&kasa.mutex_kasa);
        }

        /* Powiadomienie klienta o zakończeniu obsługi.
         * Funkcja sem_post sygnalizuje, że klient może kontynuować działanie (np. odebrać informację).
         */
        sem_post(&klient->served);
    }

    // Po wyjściu z pętli pracy fryzjera, logujemy informację o zakończeniu pracy
    snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: wychodzę z pracy.", id);
    send_message(log_buffer);
    return NULL;
}