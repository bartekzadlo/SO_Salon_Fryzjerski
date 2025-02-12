#include "common.h"

long id;
key_t klucz;
int kolejka;
int poczekalnia;
int platnosc;

volatile sig_atomic_t salon_open;
volatile sig_atomic_t close_all_clients;
volatile sig_atomic_t w_poczekalni = 0;
volatile sig_atomic_t klient_komunikat_poczekalnia = 0;
volatile sig_atomic_t pobranie_z_poczekalni = 0;
volatile sig_atomic_t zaplacone = 0;

int main()
{
    long id = get_pid();       // Pobieramy identyfikator klienta
    char log_buffer[MSG_SIZE]; // Bufor do przechowywania komunikatów logujących
    int wolne_miejsce;         // do sprawdzania czy istnieje wolne miejsce w poczekalni
    int id_fryzjer_obslugujacy;
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

        if (pobranie_z_poczekalni != 1) // w tym miejscu klient dowiaduje sie ze jest obslugiwany a wiec zaraz zwolni miejsce w poczekalni i przejdzie do zaplaty
        {
            odbierz_komunikat(kolejka, &kom, id);
            pobranie_z_poczekalni = 1;
        }

        if (w_poczekalni)
        {
            sem_v(poczekalnia, 1);
            w_poczekalni = 0;
        }

        id_fryzjer_obslugujacy = kom.nadawca;

        if (rand() % 2 == 0)
            int platnosc = 30;
        else
            int platnosc = 50;

        kom.mtype = id_fryzjer_obslugujacy;
        kom.nadawca = id;
        kom.platnosc = platnosc;
        if (zaplacone != 1)
        {
            wyslij_komunikat(kolejka, &kom);
            zaplacone = 1;
        }

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
