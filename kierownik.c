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
pthread_t timer_thread;
int kolejka;
int fotele_semafor;
int kasa_semafor;
int poczekalnia_semafor;
int shm_id;
int *banknoty; // pamiec dzielona

int main()
{
    if (signal(SIGINT, szybki_koniec) == SIG_ERR)
    {
        perror("Blad obslugi sygnalu");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    set_process_limit();

    key_t klucz;
    klucz = ftok(".", 'M');
    kolejka = utworz_kolejke(klucz);
    klucz = ftok(".", 'K');
    kasa_semafor = utworz_semafor(klucz);
    setval_semafor(kasa_semafor, 1);
    klucz = ftok(".", 'F');
    fotele_semafor = utworz_semafor(klucz);
    setval_semafor(fotele_semafor, N);
    klucz = ftok(",", 'P');
    poczekalnia_semafor = utworz_semafor(klucz);
    setval_semafor(poczekalnia_semafor, K);
    klucz = ftok(".", 'S');
    shm_id = utworz_pamiec_dzielona(klucz);
    banknoty = dolacz_pamiec_dzielona(shm_id);

    banknoty[0] = 10; // banknoty o nominale 10
    banknoty[1] = 10; // banknoty o nominale 20
    banknoty[2] = 10; // banknoty o nominale 50

    if (F <= 1 || N >= F)
    {
        fprintf(stderr, RED "Błąd: Warunek F > 1 oraz N < F nie jest spełniony.\n" RESET);
        exit(EXIT_FAILURE);
    }

    printf("Podaj czas otwarcia (TP) w sekundach: \n");
    if (scanf("%d", &TP) != 1)
    {
        error_exit("Błąd odczytu TP\n");
    }
    printf("Podaj czas zamknięcia (TK) w sekundach: \n");
    if (scanf("%d", &TK) != 1)
    {
        error_exit("Błąd odczytu TK\n");
    }
    if (TK <= TP || TK < 0 || TP < 0)
    {
        fprintf(stderr, RED "Błąd: TK musi być większe od TP i jednostki muszą być dodatnie\n" RESET);
        exit(EXIT_FAILURE);
    }
    sim_duration = TK - TP;

    if (TP > 0)
    {
        sleep(TP); // Jeśli TP (czas opóźnienia otwarcia salonu) > 0, czekamy przez TP sekundy - domyślnie sleep(TP)
    }

    sem_v(poczekalnia_semafor, K);
    printf(YELLOW "Salon otwarty.\n" RESET);

    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, simulation_timer_thread, NULL) != 0)
    {
        error_exit("Blad utworzenia watku symulacji czasu\n");
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

    char menu;
    while (menu != '3')
    {
        printf("1 - zakończ danego fryzjera\n");
        printf("2 - odeślij klientów do domu\n");
        printf("3 - koniec\n");

        while (getchar() != '\n')
            ;
        while (scanf("%c", &menu) != 1)
            ;
        switch (menu)
        {
        case '1':
            wyslij_s1();
            break;
        case '2':
            wyslij_s2();
            break;
        case '3':
            koniec(0);
            break;
        default:
            printf("Niepoprawna opcja\n");
            break;
        }
    }
    error_exit("Błąd kierownik.c\n");
}

void koniec(int s)
{
    zakoncz_symulacje_czasu();
    wyslij_s1();
    wyslij_s2();
    zwolnij_zasoby_kierownik();
    exit(EXIT_SUCCESS);
}

void szybki_koniec(int s)
{
    zwolnij_zasoby_kierownik();
    for (int i = 0; i < F; i++)
    {
        kill(fryzjerzy[i], SIGKILL);
    }
    for (int i = 0; i < P; i++)
    {
        kill(klienci[i], SIGKILL);
    }
    czekaj_na_procesy(F);
    czekaj_na_procesy(P);

    zakoncz_symulacje_czasu();

    exit(EXIT_SUCCESS);
}

void wyslij_s1()
{
    kill(fryzjerzy[0], 1);
    czekaj_na_procesy(1);
}

void wyslij_s2()
{
    szybki_koniec(0);
}

void czekaj_na_procesy(int n)
{
    int wPID, status;
    for (int i = 0; i < n; i++)
    {
        wPID = wait(&status);
        if (wPID == -1)
        {
            error_exit("Błąd wait");
        }
        else
        {
            printf(MAGENTA "Koniec procesu: %d, status: %d\n" RESET, wPID, status);
        }
    }
}

void zakoncz_symulacje_czasu()
{
    pthread_cancel(timer_thread);
    pthread_join(timer_thread, NULL);
}

void *simulation_timer_thread(void *arg)
{
    (void)arg;
    int remaining = sim_duration; // Zmienna do przechowywania czasu pozostałego do zakończenia symulacji
    char buf[64];                 // Bufor do przechowywania komunikatu o pozostałym czasie

    while (remaining > 0) // Pętla działa dopóki nie minie czas symulacji lub nie otrzymano sygnału zamknięcia salonu
    {
        printf(MAGENTA "Czas pozostały: %d s\n", remaining); // Tworzenie komunikatu o pozostałym czasie
        sleep(1);                                            // Symulacja upływu czasu - domyślnie 1
        remaining--;                                         // Zmniejszenie liczby pozostałych sekund
    }
    sem_p(poczekalnia_semafor, P);
    return NULL; // Zakończenie wątku
}

void zwolnij_zasoby_kierownik()
{
    usun_kolejke(kolejka);
    usun_semafor(kasa_semafor);
    usun_semafor(fotele_semafor);
    usun_semafor(poczekalnia_semafor);
    odlacz_pamiec_dzielona(banknoty);
    usun_pamiec_dzielona(shm_id);

    printf(RED "Zasoby zwolnione\n" RESET);
}