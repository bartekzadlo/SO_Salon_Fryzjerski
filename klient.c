#include "common.h"

long id;
key_t klucz;
int kolejka;
int poczekalnia_semafor;
int platnosc;
int wolne_miejsce;
int id_fryzjer_obslugujacy;

volatile sig_atomic_t client_stop = 0;
volatile sig_atomic_t w_poczekalni = 0;
volatile sig_atomic_t klient_komunikat_poczekalnia = 0;
volatile sig_atomic_t pobranie_z_poczekalni = 0;
volatile sig_atomic_t zaplacone = 0;
volatile sig_atomic_t otrzymana_reszta = 0;

int main()
{
    long id = get_pid();
    char log_buffer[MSG_SIZE];
    struct komunikat kom;

    klucz = ftok(".", "M");
    kolejka = utworz_kolejke(klucz);
    klucz = ftok(".", "P");
    poczekalnia_semafor = utworz_semafor(klucz);

    while (1)
    {
        if (client_stop)
        {
            break;
        }
        int earning_time = rand() % 5 + 1;
        sleep(earning_time);
        snprintf(log_buffer, MSG_SIZE, "Klient %d: Próbuję wejść do poczekalni", id);
        send_message(log_buffer);
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

            if (pobranie_z_poczekalni != 1)
            {
                odbierz_komunikat(kolejka, &kom, id);
                pobranie_z_poczekalni = 1;
            }

            if (w_poczekalni)
            {
                sem_v(poczekalnia_semafor, 1);
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

            if (otrzymana_reszta != 1)
            {
                odbierz_komunikat(kolejka, &kom, id);
                otrzymana_reszta = 1;
            }

            klient_komunikat_poczekalnia = 0;
            pobranie_z_poczekalni = 0;
            zaplacone = 0;
            otrzymana_reszta = 0;

            snprintf(log_buffer, MSG_SIZE, "Klient %d: zostałem obsłużony i opuszczam salon.", id);
            send_message(log_buffer);
        }
        else
        {
            snprintf(log_buffer, MSG_SIZE, "Klient %d: poczekalnia jest pełna. Wracam do pracy.", id);
            send_message(log_buffer);
        }
        if (w_poczekalni)
        {
            snprintf(log_buffer, MSG_SIZE, "Klient %d: zwalniam swoje miejsce w poczekalni.", id);
            send_message(log_buffer);
            sem_v(poczekalnia_semafor, 1);
        }
        return NULL;
    }
}
