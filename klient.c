#include "common.h"

long id;
key_t klucz;
int kolejka;

int main()
{
    long id = get_pid();       // Pobieramy identyfikator klienta
    char log_buffer[MSG_SIZE]; // Bufor do przechowywania komunikatów logujących

    klucz = ftok(".", "M");
    kolejka = utworz_kolejke(klucz);

    klucz = ftok(".", "P");
    poczekalnia = utworz_semafor(klucz);

    while (salon_open && !close_all_clients) // Pętla działania klienta – działa, dopóki salon jest otwarty i nie otrzymano sygnału zamknięcia dla klientów
    {
        /* Symulacja "zarabiania pieniędzy":
         * Klient czeka losowo od 1 do 5 sekund, aby zasymulować okres, w którym "zarabia" pieniądze.
         */
        int earning_time = rand() % 5 + 1;
        sleep(0); // domyślnie earning_time

        if (!salon_open || close_all_clients) // Jeżeli salon jest zamknięty lub otrzymano sygnał zamknięcia klientów, kończymy pętlę
            break;

        if (rand() % 2 == 0)
            klient->payment = 30;
        else
            klient->payment = 50;

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
