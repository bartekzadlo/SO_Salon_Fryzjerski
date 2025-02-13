#include "common.h"

long id;
int msg_qid;
int poczekalnia_semafor;
int id_fryzjer_obslugujacy;

volatile sig_atomic_t sygnal_klient = 0; // sygnal przerwanie pracy klienta
volatile sig_atomic_t w_poczekalni = 0;
volatile sig_atomic_t klient_komunikat_poczekalnia = 0;
volatile sig_atomic_t pobranie_z_poczekalni = 0;
volatile sig_atomic_t zaplacone = 0;
volatile sig_atomic_t otrzymana_reszta = 0;

int main()
{
    srand(time(NULL) + getpid());
    id = getpid();

    if (signal(SIGINT, sygnal_2) == SIG_ERR)
    {
        error_exit("Blad obslugi sygnalu 2");
    }
    struct komunikat komunikat;
    key_t msg_qkey;
    key_t sem_key_p;
    int wolne_miejsce;
    int platnosc;

    msg_qkey = ftok(".", 'M');
    msg_qid = stworz_kolejke_komunikatow(msg_qkey);

    sem_key_p = ftok(".", 'P');
    poczekalnia_semafor = utworz_semafor(sem_key_p);

    while (1)
    {
        printf(BLUE "Klient %ld: Przyszedłem do salonu.\n", id);
        if (sygnal_klient)
        {
            break;
        }
        int earning_time = rand() % 10 + 3;
        sleep(earning_time); // domyslni earning_time
        printf(BLUE "Klient %ld: Próbuję wejść do poczekalni\n" RESET, id);
        if (!w_poczekalni)
        {
            wolne_miejsce = sem_try_wait(poczekalnia_semafor, 1);
        }

        if (wolne_miejsce == 0)
        {
            w_poczekalni = 1;
            printf(BLUE "Klient %ld: wchodzę do poczekalni. Liczba wolnych miejsc: %d.\n" RESET, id, sem_getval(poczekalnia_semafor));

            komunikat.mtype = 1;
            komunikat.nadawca = id;
            wyslij_komunikat_do_kolejki(msg_qid, &komunikat);
            klient_komunikat_poczekalnia = 1;
            printf(BLUE "Klient %ld: wyslalem komunikat, że jestem w poczekalni.\n" RESET, id);

            if (pobranie_z_poczekalni != 1)
            {
                printf(BLUE "Klient %ld: zostałem pobrany przez fryzjera.\n" RESET, id);
                pobierz_komunikat_z_kolejki(msg_qid, &komunikat, id);
                pobranie_z_poczekalni = 1;
            }

            if (w_poczekalni)
            {
                printf(BLUE "Klient %ld: opuściłem poczekalnie\n" RESET, id);
                sem_v(poczekalnia_semafor, 1);
                w_poczekalni = 0;
            }

            id_fryzjer_obslugujacy = komunikat.nadawca;

            platnosc = (rand() % 2 == 0) ? 30 : 50;

            komunikat.mtype = id_fryzjer_obslugujacy;
            komunikat.nadawca = id;
            komunikat.platnosc = platnosc;

            if (zaplacone != 1)
            {
                printf(BLUE "Klient %ld: płacę %d zł.\n" RESET, id, platnosc);
                wyslij_komunikat_do_kolejki(msg_qid, &komunikat);
                printf(BLUE "Klient %ld: wysłałem komunikat\n" RESET, id);
                zaplacone = 1;
            }
            if (otrzymana_reszta != 1)
            {
                pobierz_komunikat_z_kolejki(msg_qid, &komunikat, id);
                printf(BLUE "Klient %ld: otrzymałem komunikat o zakończeniu obsługi.\n" RESET, id);
                otrzymana_reszta = 1;
            }

            klient_komunikat_poczekalnia = 0;
            pobranie_z_poczekalni = 0;
            zaplacone = 0;
            otrzymana_reszta = 0;

            printf(BLUE "Klient %ld: zostałem obsłużony i opuszczam salon.\n" RESET, id);
        }
        else
        {
            printf(BLUE "Klient %ld: poczekalnia jest pełna. Wracam do pracy.\n" RESET, id);
        }

        if (w_poczekalni)
        {
            sem_v(poczekalnia_semafor, 1);
        }
    }
    printf(BLUE "Klient %ld: kończę pracę\n" RESET, id);
}

void sygnal_2(int sig)
{
    printf(RED "Klient %ld: Otrzymałem sygnał 2.\n" RESET, id);

    if (klient_komunikat_poczekalnia == 1)
    {
        sygnal_klient = 1;

        if (pobranie_z_poczekalni != 1)
        {
            pobranie_z_poczekalni = -1;
        }
        else if (zaplacone != 1)
        {
            zaplacone = -1;
        }
        else if (otrzymana_reszta != 1)
        {
            otrzymana_reszta = -1;
        }
    }
    else
    {
        if (w_poczekalni)
        {
            printf(BLUE "Klient %ld: Zwalniam swoje miejsce w poczekalni.\n" RESET, id);
            sem_v(poczekalnia_semafor, 1);
        }
        printf(BLUE "Klient %ld: kończę pracę\n" RESET, id);
        exit(EXIT_SUCCESS);
    }
}