#include "common.h"

long id;
key_t klucz;
int kolejka;
int poczekalnia;

volatile sig_atomic_t salon_open;
volatile sig_atomic_t close_all_clients;
volatile sig_atomic_t w_poczekalni = 0;
volatile sig_atomic_t klient_komunikat_poczekalnia = 0;

int main()
{
    long id = get_pid();       // Pobieramy identyfikator klienta
    char log_buffer[MSG_SIZE]; // Bufor do przechowywania komunikatów logujących
    int wolne_miejsce;         // do sprawdzania czy istnieje wolne miejsce w poczekalni
    struct komunikat kom;

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

        if (w_poczekalni == 0)
        {
            wolne_miejsce = sem_try_wait(poczekalnia, 1);
        }

        if (wolne_miejsce == 0)
        {
            w_poczekalni = 1;
            snprintf(log_buffer, MSG_SIZE, "Klient %d: wchodzę do poczekalni. Liczba wolnych miejsc: %d.", id, sem_getval(poczekalnia));
            send_message(log_buffer);

            kom.mtype = 1;
            kom.nadawca = id;
            wyslij_komunikat(kolejka, &kom);
            klient_komunikat_poczekalnia = 1;
        }

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
