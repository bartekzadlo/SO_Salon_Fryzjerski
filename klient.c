#include "common.h"

long id;
int kolejka;
int poczekalnia_semafor;
int platnosc;
int id_fryzjer_obslugujacy;

volatile sig_atomic_t sygnal_klient = 0; // sygnal przerwanie pracy klienta
volatile sig_atomic_t w_poczekalni = 0;
volatile sig_atomic_t klient_komunikat_poczekalnia = 0;
volatile sig_atomic_t pobranie_z_poczekalni = 0;
volatile sig_atomic_t zaplacone = 0;
volatile sig_atomic_t otrzymana_reszta = 0;

int main()
{
    srand(time(NULL));
    id = getpid();

    if (signal(SIGINT, sygnal_2) == SIG_ERR)
    {
        error_exit("Blad obslugi sygnalu 2");
    }
    struct komunikat kom;
    key_t klucz;
    int wolne_miejsce;

    klucz = ftok(".", 'M');
    kolejka = utworz_kolejke(klucz);

    klucz = ftok(".", 'P');
    poczekalnia_semafor = utworz_semafor(klucz);

    while (1)
    {
        printf(BLUE "Klient %ld: Przyszedłem do salonu.\n", id);
        if (sygnal_klient)
        {
            break;
        }
        int earning_time = rand() % 5 + 1;
        sleep(earning_time);
        printf(BLUE "Klient %ld: Próbuję wejść do poczekalni\n" RESET, id);
        if (!w_poczekalni)
        {
            wolne_miejsce = sem_try_wait(poczekalnia_semafor, 1);
        }

        if (wolne_miejsce == 0)
        {
            w_poczekalni = 1;
            printf(BLUE "Klient %ld: wchodzę do poczekalni. Liczba wolnych miejsc: %d.\n" RESET, id, sem_getval(poczekalnia_semafor));

            kom.mtype = 1;
            kom.nadawca = id;
            wyslij_komunikat(kolejka, &kom);
            klient_komunikat_poczekalnia = 1;
            printf(BLUE "Klient %ld: wyslalem komunikat, że jestem w poczekalni.\n" RESET, id);

            if (pobranie_z_poczekalni != 1)
            {
                printf(BLUE "Klient %ld: zostałem pobrany przez fryzjera.\n" RESET, id);
                odbierz_komunikat(kolejka, &kom, id);
                pobranie_z_poczekalni = 1;
            }

            if (w_poczekalni)
            {
                printf(BLUE "Klient %ld: opuściłem poczekalnie\n" RESET, id);
                sem_v(poczekalnia_semafor, 1);
                w_poczekalni = 0;
            }

            id_fryzjer_obslugujacy = kom.nadawca;

            if (rand() % 2 == 0)
            {
                platnosc = 30;
            }
            else
            {
                platnosc = 50;
            }

            kom.mtype = id_fryzjer_obslugujacy;
            kom.nadawca = id;
            kom.platnosc = platnosc;

            if (zaplacone != 1)
            {
                printf(BLUE "Klient %ld: płacę %d zł.\n" RESET, id, platnosc);
                wyslij_komunikat(kolejka, &kom);
                printf(BLUE "Klient %ld: wysłałem komunikat" RESET, id);
                zaplacone = 1;
            }
            if (otrzymana_reszta != 1)
            {
                odbierz_komunikat(kolejka, &kom, id);
                otrzymana_reszta = 1;
                printf(BLUE "Klient %ld: otrzymałem komunikat o zakończeniu obsługi.\n" RESET, id);
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
    printf(BLUE "Klient %ld: Otrzymałem sygnał 2.\n" RESET, id);

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