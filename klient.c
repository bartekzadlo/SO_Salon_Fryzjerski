#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "common.h"

/*
 * Funkcja klienta
 * ---------------
 * Reprezentuje działanie klienta w symulacji salonu fryzjerskiego.
 * Każdy klient (wątek) wykonuje następujące kroki:
 *   1. "Zarabia" pieniądze, symulowane opóźnieniem 1-5 sekund.
 *   2. Przygotowuje swoje dane, w tym unikalny identyfikator oraz losowany sposób płatności (20 zł lub 50 zł).
 *   3. Próbuje wejść do poczekalni:
 *      - Jeśli poczekalnia jest pełna, klient opuszcza salon i wraca "zarabiać pieniądze".
 *      - W przeciwnym razie, klient zajmuje miejsce w poczekalni i czeka na obsługę.
 *   4. Klient czeka, aż fryzjer zakończy jego obsługę (synchronizacja za pomocą semafora).
 *   5. Po obsłużeniu klient opuszcza salon, a statystyki są aktualizowane.
 *   6. W przypadku zamknięcia salonu, klient odpowiednio reaguje i kończy działanie.
 */

void *client_thread(void *arg)
{
    int id = *((int *)arg); // Pobieramy identyfikator klienta przekazany jako argument
    free(arg);              // Zwolnienie pamięci zajmowanej przez argument

    char log_buffer[MSG_SIZE]; // Bufor do przechowywania komunikatów logujących

    while (salon_open && !close_all_clients) // Pętla działania klienta – działa, dopóki salon jest otwarty i nie otrzymano sygnału zamknięcia dla klientów
    {
        /* Symulacja "zarabiania pieniędzy":
         * Klient czeka losowo od 1 do 5 sekund, aby zasymulować okres, w którym "zarabia" pieniądze.
         */
        int earning_time = rand() % 5 + 1;
        sleep(earning_time);

        /* Przygotowanie danych klienta:
         * Alokacja pamięci na strukturę reprezentującą klienta.
         * Ustawienie unikalnego identyfikatora klienta.
         */
        Klient *klient = malloc(sizeof(Klient));
        if (!klient)
        {
            perror("malloc");
            exit(1);
        }
        klient->id = id;

        /* Losowanie sposobu płatności:
         * 50% szans na płatność 20 zł, 50% szans na płatność 50 zł.
         */
        if (rand() % 2 == 0)
            klient->payment = 20;
        else
            klient->payment = 50;

        /* Inicjalizacja semafora dla klienta:
         * Semafor 'served' służy do synchronizacji – klient czeka, aż fryzjer zakończy jego obsługę.
         * Inicjalizujemy semafor z wartością 0.
         */
        if (sem_init(&klient->served, 0, 0) != 0)
        {
            perror("sem_init");
            exit(1);
        }

        /* Próba wejścia do poczekalni:
         * Blokujemy mutex poczekalni, aby bezpiecznie sprawdzić, czy jest dostępne miejsce.
         */
        pthread_mutex_lock(&poczekalniaMutex);
        if (poczekalniaCount >= MAX_WAITING) // Jeśli poczekalnia jest pełna, klient odpuszcza i opuszcza salon
        {
            pthread_mutex_unlock(&poczekalniaMutex);
            snprintf(log_buffer, MSG_SIZE, "Klient %d: poczekalnia pełna, opuszczam salon.", id);
            send_message(log_buffer);
            __sync_fetch_and_add(&sharedStats->total_clients_left, 1);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwalniamy pamięć
            continue;                     // klient wraca „zarabiać pieniądze”
        }
        int index = (poczekalniaFront + poczekalniaCount) % MAX_WAITING; // Dodajemy klienta do poczekalni na pozycji obliczonej na podstawie aktualnej liczby oczekujących
        poczekalnia[index] = klient;
        poczekalniaCount++;
        pthread_cond_signal(&poczekalniaNotEmpty); // Sygnalizujemy, że poczekalnia nie jest pusta (budzimy ewentualne wątki fryzjerów)
        pthread_mutex_unlock(&poczekalniaMutex);
        snprintf(log_buffer, MSG_SIZE, "Klient %d: wchodzę do poczekalni. Liczba oczekujących: %d.", id, poczekalniaCount);
        send_message(log_buffer);

        /* Klient czeka na zakończenie obsługi:
         * sem_wait blokuje wątek klienta do momentu, aż fryzjer zakończy obsługę.
         */
        sem_wait(&klient->served);

        if (close_all_clients) // Sprawdzamy, czy w trakcie oczekiwania został wysłany sygnał zamknięcia salonu
        {
            snprintf(log_buffer, MSG_SIZE, "Klient %d: salon zamknięty – opuszczam salon.", id);
            send_message(log_buffer);
            sem_destroy(&klient->served); // Zwalniamy semafor
            __sync_fetch_and_add(&sharedStats->total_clients_left, 1);
            free(klient); // Zwolnienie pamięci klienta
            break;        // Przerywamy pętlę – klient kończy pracę
        }

        /* Obsługa zakończona:
         * Klient zostaje obsłużony i opuszcza salon.
         */
        snprintf(log_buffer, MSG_SIZE, "Klient %d: zostałem obsłużony i opuszczam salon.", id);
        send_message(log_buffer);

        __sync_fetch_and_add(&sharedStats->total_clients_served, 1); // Aktualizacja statystyk

        // Zwalniamy zasoby klienta: niszczymy semafor i zwalniamy pamięć
        sem_destroy(&klient->served); // Zwalniamy semafor
        free(klient);                 // Zwalniamy pamięć
    }

    /* Po zakończeniu pętli, klient kończy pracę w symulacji */
    snprintf(log_buffer, MSG_SIZE, "Klient %d: kończę pracę w symulacji salonu.", id);
    send_message(log_buffer);
    return NULL;
}
