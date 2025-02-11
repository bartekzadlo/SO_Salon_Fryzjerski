#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
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
        sleep(0); // domyślnie earning_time

        if (!salon_open || close_all_clients) // Jeżeli salon jest zamknięty lub otrzymano sygnał zamknięcia klientów, kończymy pętlę
            break;
        /* Przygotowanie danych klienta:
         * Alokacja pamięci na strukturę reprezentującą klienta.
         * Ustawienie unikalnego identyfikatora klienta.
         */
        Klient *klient = malloc(sizeof(Klient));
        if (!klient)
        {
            perror("Błąd malloc klienta");
            exit(EXIT_FAILURE);
        }
        klient->id = id;
        if (rand() % 2 == 0)
            klient->payment = 30;
        else
            klient->payment = 50;

        if (sem_init(&klient->served, 0, 0) != 0) // Inicjalizacja semafora dla klienta
        {
            perror("sem_init");
            free(klient); // Zwolnienie pamięci w przypadku błędu
            exit(1);
        }

        /* Próba wejścia do poczekalni:
         * Blokujemy mutex poczekalni, aby bezpiecznie sprawdzić, czy jest dostępne miejsce.
         */
        pthread_mutex_lock(&poczekalniaMutex);
        if (!salon_open || close_all_clients) // Jeśli salon jest zamknięty lub otrzymano sygnał zamknięcia, klient opuszcza salon
        {
            pthread_mutex_unlock(&poczekalniaMutex);
            free(klient); // Zwolnienie pamięci
            break;
        }
        if (poczekalniaCount >= K) // Jeśli poczekalnia jest pełna, klient opuszcza salon i wraca "zarabiać pieniądze"
        {
            pthread_mutex_unlock(&poczekalniaMutex);
            snprintf(log_buffer, MSG_SIZE, "Klient %d: poczekalnia pełna, opuszczam salon.", id);
            send_message(log_buffer);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwalniamy pamięć
            continue;                     // klient wraca „zarabiać pieniądze”
        }
        // Dodanie klienta do poczekalni
        int index = (poczekalniaFront + poczekalniaCount) % K; // Dodajemy klienta do poczekalni na pozycji obliczonej na podstawie aktualnej liczby oczekujących
        poczekalnia[index] = klient;
        poczekalniaCount++;                        // Zwiększanie liczby oczekujących klientów
        pthread_cond_signal(&poczekalniaNotEmpty); // Sygnalizujemy, że poczekalnia nie jest pusta (budzimy ewentualne wątki fryzjerów)
        pthread_mutex_unlock(&poczekalniaMutex);

        snprintf(log_buffer, MSG_SIZE, "Klient %d: wchodzę do poczekalni. Liczba oczekujących: %d.", id, poczekalniaCount);
        send_message(log_buffer); // Logowanie komunikatu

        /* Klient czeka na zakończenie obsługi:
         * sem_wait blokuje wątek klienta do momentu, aż fryzjer zakończy obsługę.
         */
        sem_wait(&klient->served);

        if (close_all_clients) // Sprawdzamy, czy w trakcie oczekiwania został wysłany sygnał zamknięcia salonu
        {
            snprintf(log_buffer, MSG_SIZE, "Klient %d: salon zamknięty – opuszczam salon.", id);
            send_message(log_buffer);
            sem_destroy(&klient->served); // Zwalniamy semafor
            free(klient);                 // Zwolnienie pamięci klienta
            break;                        // Przerywamy pętlę – klient kończy pracę
        }

        /* Obsługa zakończona:
         * Klient zostaje obsłużony i opuszcza salon.
         */
        snprintf(log_buffer, MSG_SIZE, "Klient %d: zostałem obsłużony i opuszczam salon.", id);
        send_message(log_buffer);

        // Zwalniamy zasoby klienta: niszczymy semafor i zwalniamy pamięć
        sem_destroy(&klient->served); // Zwalniamy semafor
        free(klient);                 // Zwalniamy pamięć
    }
    return NULL;
}
