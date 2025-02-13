#include "common.h"

int TP = 0;
int TK = 0;
int sim_duration = 0;
pid_t klienci[P];
pid_t fryzjerzy[F];
pthread_t timer_thread;
int fotele_semafor;
int kasa_semafor;
int poczekalnia_semafor;
int kolejka;
int shm_id;
int *banknoty; // pamiec dzielona

int main()
{
    if (signal(SIGINT, szybki_koniec) == SIG_ERR)
    {
        error_exit("Błąd obsługi sygnału");
    }

    set_process_limit();
    srand(time(NULL));

    key_t klucz;

    klucz = ftok(".", 'M');
    kolejka = utworz_kolejke(klucz);

    klucz = ftok(".", 'K');
    kasa_semafor = utworz_semafor(klucz);
    ustaw_semafor(kasa_semafor, 1);

    klucz = ftok(".", 'F');
    fotele_semafor = utworz_semafor(klucz);
    ustaw_semafor(fotele_semafor, N);

    klucz = ftok(".", 'P');
    poczekalnia_semafor = utworz_semafor(klucz);
    ustaw_semafor(poczekalnia_semafor, K);

    printf(YELLOW "Zainicjalizowano poczekalnię, ilość miejsc: %d.\n" RESET, sem_getval(poczekalnia_semafor));
    printf(YELLOW "Zainicjalizowano fotele, ilość foteli: %d.\n" RESET, sem_getval(fotele_semafor));

    klucz = ftok(".", 'S');
    shm_id = utworz_pamiec_dzielona(klucz);
    banknoty = dolacz_pamiec_dzielona(shm_id);

    banknoty[0] = 10; // banknoty o nominale 10
    banknoty[1] = 10; // banknoty o nominale 20
    banknoty[2] = 10; // banknoty o nominale 50

    printf(YELLOW "Zainicjalizowane kasę, stan początkowy kasy - Banknoty 10 zł: %d, Banknoty 20 zł: %d, Banknoty 50 zł: %d\n" RESET, banknoty[0], banknoty[1], banknoty[2]);

    if (F <= 1 || N >= F)
    {
        error_exit("Błąd: Warunek F > 1 oraz N < F nie jest spełniony.\n");
    }

    printf(CYAN "Podaj czas otwarcia (TP) w sekundach: \n" RESET);
    if (scanf("%d", &TP) != 1)
    {
        error_exit("Błąd odczytu TP\n");
    }
    printf(CYAN "Podaj czas zamknięcia (TK) w sekundach: \n" RESET);
    if (scanf("%d", &TK) != 1)
    {
        error_exit("Błąd odczytu TK\n");
    }
    if (TK <= TP || TK < 0 || TP < 0)
    {
        error_exit("Błąd: TK musi być większe od TP i jednostki muszą być dodatnie\n");
    }
    sim_duration = TK - TP;

    if (pthread_create(&timer_thread, NULL, simulation_timer_thread, NULL) != 0)
    {
        error_exit("Blad utworzenia watku symulacji czasu\n");
    }

    tworz_fryzjerow();
    tworz_klientow();

    char menu;
    while (menu != '3')
    {
        printf(CYAN "1 - Zakończ pracę fryzjera\n");
        printf("2 - Zakończ pracę klientów\n");
        printf("3 - Zwolnij zasoby i zakończ program\n" RESET);

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
            printf(RED "Niepoprawna opcja\n" RESET);
            break;
        }
    }
    error_exit("Błąd kierownik.c\n");
}

void koniec(int s)
{
    printf(RED "Wywołano koniec." RESET);
    zakoncz_symulacje_czasu();
    wyslij_s3();
    wyslij_s2();
    zwolnij_zasoby_kierownik();
    exit(EXIT_SUCCESS);
}

void szybki_koniec(int s)
{
    printf(RED "Wywołano szybki koniec." RESET);
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
    for (int i = 0; i < 1; i++)
    {
        kill(fryzjerzy[i], 1);
    }
    czekaj_na_procesy(1);
}

void wyslij_s2()
{
    for (int i = 0; i < P; i++)
    {
        kill(klienci[i], 2);
    }
    czekaj_na_procesy(P);
}

void wyslij_s3()
{
    for (int i = 0; i < F; i++)
    {
        kill(fryzjerzy[i], 1);
    }
    czekaj_na_procesy(F);
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
            printf(YELLOW "Koniec procesu: %d, status: %d\n" RESET, wPID, status);
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
    while (1)
    {
        if (TP > 0)
        {
            sleep(TP); // Jeśli TP (czas opóźnienia otwarcia salonu) > 0, czekamy przez TP sekundy - domyślnie sleep(TP)
        }
        printf(YELLOW "Salon otwarty.\n" RESET);
        sem_v(poczekalnia_semafor, K);

        int remaining = sim_duration; // Zmienna do przechowywania czasu pozostałego do zakończenia symulacji
        while (remaining > 0)         // Pętla działa dopóki nie minie czas symulacji
        {
            printf(MAGENTA "Czas pozostały: %d s\n", remaining); // Tworzenie komunikatu o pozostałym czasie
            sleep(1);                                            // Symulacja upływu czasu - domyślnie 1
            remaining--;                                         // Zmniejszenie liczby pozostałych sekund
        }
        sem_p(poczekalnia_semafor, P);
        return NULL; // Zakończenie wątku
    }
}

void zwolnij_zasoby_kierownik()
{
    usun_kolejke(kolejka);
    usun_semafor(kasa_semafor);
    usun_semafor(fotele_semafor);
    usun_semafor(poczekalnia_semafor);
    odlacz_pamiec_dzielona(banknoty);
    usun_pamiec_dzielona(shm_id);

    printf(YELLOW "Zasoby zwolnione\n" RESET);
}

void tworz_fryzjerow()
{
    for (int i = 0; i < F; i++)
    {
        fryzjerzy[i] = fork();
        if (fryzjerzy[i] == 0)
        {
            execl("./fryzjer", "fryzjer", NULL);
        }
        printf(YELLOW "Nowy fryzjer %d\n", fryzjerzy[i]);
    }
}

void tworz_klientow()
{
    for (int i = 0; i < P; i++)
    {
        klienci[i] = fork();
        if (klienci[i] == 0)
        {
            execl("./klient", "klient", NULL);
        }
        printf(YELLOW "Nowy klient %d\n", klienci[i]);
    }
}