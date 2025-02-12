#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

int TP = 0;
int TK = 0;
int sim_duration = 0;
int sygnal1 = 0;
int sygnal2 = 0;
pid_t klienci[P];
pid_t fryzjerzy[F];
int kolejka;
int fotele_semafor;
int kasa_semafor;
int poczekalnia_semafor;
int shm_id;
int banknoty;

int main()
{
    srand(time(NULL));
    set_process_limit();

    key_t klucz;
    klucz = ftok(".", "M");
    kolejka = utworz_kolejke(klucz);
    klucz = ftok(".", "K");
    kasa_semafor = utworz_semafor(klucz);
    setval_semafor(kasa_semafor, 1);
    klucz = ftok(".", "F");
    fotele_semafor = utworz_semafor(klucz);
    setval_semafor(fotele_semafor, N);
    klucz = ftok(",", "P");
    poczekalnia_semafor = utworz_semafor(klucz);
    setval_semafor(poczekalnia_semafor, K);
    klucz = ftok(".", "S");
    shm_id = utworz_pamiec_dzielona(klucz);
    banknoty = dolacz_pamiec_dzielona(shm_id);

    banknoty[0] = 10; // banknoty o nominale 10
    banknoty[1] = 10; // banknoty o nominale 20
    banknoty[2] = 10; // banknoty o nominale 50

    if (TP > 0)
    {
        sleep(0); // Jeśli TP (czas opóźnienia otwarcia salonu) > 0, czekamy przez TP sekundy - domyślnie sleep(TP)
    }

    printf(YELLOW "Salon otwarty." RESET);

    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, simulation_timer_thread, NULL) != 0)
    {
        error_exit("Blad utworzenia watku symulacji czasu");
    }

    for (int i = 0; i < F; i++)
    {
        fryzjerzy[i] = fork();
        if (fryzjerzy[i] == 0)
        {
            execl("./fryzjer", "fryzjer", NULL);
        }
    }

    for (int i = 0; i < P; i++)
    {
        klienci[i] = fork();
        if (klienci[i] == 0)
        {
            execl("./klient", "klient", NULL);
        }
    }
}
