#include "common.h"

int TP = 0;             // czas otwarcia salonu
int TK = 0;             // czas zamknięcia salonu
int sim_duration = 0;   // długość trwania otwartego salonu
pid_t klienci[P];       // tablica procesów klientów
pid_t fryzjerzy[F];     // tablica procesów fryzjerów
pthread_t timer_thread; // wątek symulacji czasu
// semafory
int fotele_semafor;
int kasa_semafor;
int poczekalnia_semafor;
// kolejka komunikatów
int msg_qid;
volatile sig_atomic_t jeden_fryzjer_zabity = 0;
// volatile sig_atomic_t koniec_symulacji = 0;
int shm_id;
int *kasa; // kasa -  pamiec dzielona

int main()
{
    if (signal(SIGINT, sig_handler_int) == SIG_ERR) // Ustawienie obsługi sygnału SIGINT na sig_handler_int.
    {
        error_exit("Błąd obsługi sygnału szybkiego końca");
    }

    set_process_limit(); // ustawiamy i sprawdzamy czy nie został przekroczony limit procesów

    if (F <= 1 || N >= F) // walidacja danych F i N
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
    if (TK <= TP || TK < 0 || TP < 0) // walidacja danych TP i TK - porównanie
    {
        error_exit("Błąd: TK musi być większe od TP i jednostki muszą być dodatnie\n");
    }

    sim_duration = TK - TP; // obliczenie czasu trwania symulacji salonu

    // Deklaracja wszystkich kluczy
    key_t msg_qkey;
    key_t sem_key_k;
    key_t sem_key_f;
    key_t sem_key_p;
    key_t shm_key;
    // Tworzenie kluczy
    msg_qkey = ftok(".", 'M');
    shm_key = ftok(".", 'S');
    sem_key_k = ftok(".", 'K');
    sem_key_f = ftok(".", 'F');
    sem_key_p = ftok(".", 'P');
    // Tworzenie struktur przy pomocy naszych kluczy
    msg_qid = stworz_kolejke_komunikatow(msg_qkey);
    shm_id = stworz_pamiec_dzielona(shm_key);
    kasa_semafor = stworz_semafor(sem_key_k);
    fotele_semafor = stworz_semafor(sem_key_f);
    poczekalnia_semafor = stworz_semafor(sem_key_p);
    // Dołączenie pamięci współdzielonej i inicjalizacja banknotów w kasie
    kasa = dolacz_do_pamieci_dzielonej(shm_id);
    zainicjalizuj_kase();

    sem_setval(kasa_semafor, 1);   // ustawiamy semafor kasa na 1 - mamy jedną wspólną kasę, tylko jedna osoba może ją obsługiwać
    sem_setval(fotele_semafor, N); // ustawiamy semafor foteli na N - ilość foteli

    printf(YELLOW "Zainicjalizowano fotele, ilość foteli: %d.\n" RESET, sem_getval(fotele_semafor));
    printf(YELLOW "Zainicjalizowane kasę, stan początkowy kasy - Banknoty 10 zł: %d, Banknoty 20 zł: %d, Banknoty 50 zł: %d\n" RESET, kasa[0], kasa[1], kasa[2]);

    // tworzenie wątku symulacji czasu - otwiera ona i zamyka salon (nie kończy programu - otwiera i zamyka poczekalnię)
    if (pthread_create(&timer_thread, NULL, simulation_timer_thread, NULL) != 0)
    {
        error_exit("Błąd utworzenia wątku symulacji czasu\n");
    }

    tworz_fryzjerow(); // wywolanie funkcji tworzenia fryzjerów
    tworz_klientow();  // wywołanie funkcji tworzenia klientów

    // menu obsługi sygnałów
    char menu = 0;
    while (menu != '3' /*&& !koniec_symulacji*/)
    {
        printf(CYAN "1 - Zakończ pracę fryzjera\n");
        printf("2 - Natychmiastowo zamknij salon\n");
        printf("3 - Zwolnij zasoby i zakończ program\n" RESET);

        while (getchar() != '\n')
            ; // Czeszczenie bufora
        while (scanf("%c", &menu) != 1)
            ;

        switch (menu)
        {
        case '1':
            zabij_fryzjera(); // sygnał 1 kończy pracę jednego fryzjera
            break;
        case '2':
            sig_handler_int(0); // sygnał 2 kończy natychmiastowo pracę wszystkich klientów
            break;
        case '3':
            koniec(0); // zamyka program, zwalniając ps i ipcs
            break;
        default:
            printf(RED "Niepoprawna opcja\n" RESET);
            break;
        }
    }
    // pthread_join(timer_thread, NULL);
    // return 0;
}

void sig_handler_int(int s) // obsługa szybkiego końca - zabicia programu
{
    printf(RED "Wywołano szybki koniec.\n" RESET);
    stop_timer_thread();        // kończymy wątek symulacji czasu
    zwolnij_zasoby_kierownik(); // zwalniamy zasoby
    // konczymy fryzjerów i klientów
    if (jeden_fryzjer_zabity == 1)
    {
        for (int i = 1; i < F; i++)
        {
            kill(fryzjerzy[i], SIGKILL);
        }
    }
    else
    {
        for (int i = 0; i < F; i++)
        {
            kill(fryzjerzy[i], SIGKILL);
        }
    }

    for (int i = 0; i < P; i++)
    {
        kill(klienci[i], SIGKILL);
    }
    if (jeden_fryzjer_zabity == 1)
    {
        wait_for_process(F - 1);
    }
    else
    {
        wait_for_process(F); // czekamy na zakończenie procesów
    }
    wait_for_process(P);
    exit(EXIT_SUCCESS);
}

void wait_for_process(int count)
{
    int child_pid, status;
    for (int i = 0; i < count; i++) // Pętla oczekująca na zakończenie n procesów potomnych
    {
        child_pid = wait(&status); // Oczekiwanie na zakończenie procesu potomnego
        if (child_pid == -1)       // Sprawdzenie błędu funkcji wait()
        {
            error_exit("Błąd oczekiwania na proces potomny");
        }
        else
        {
            printf(YELLOW "Koniec procesu: %d, status: %d\n" RESET, child_pid, status); // Wyświetlenie informacji o zakończonym procesie i jego statusie // Wyświetlenie informacji o zakończonym procesie i jego statusie
        }
    }
}

void stop_timer_thread()
{
    sem_setval(poczekalnia_semafor, 0);
    pthread_cancel(timer_thread);     // Anulowanie wątku zegara (timer_thread)
    pthread_join(timer_thread, NULL); // Oczekiwanie na zakończenie wątku zegara, aby upewnić się, że został poprawnie zatrzymany
}

void koniec(int s) // zamykanie salonu
{
    printf(RED "Wywołano koniec.\n" RESET);
    stop_timer_thread();        // kończymy wątek symulacji czasu - cancel i join
    zabij_klientow();           // wysyłamy sygnał - zamknięcie procesów wszystkich klientów
    zabij_fryzjerow();          // wysyłamy sygnał - zamknięcie procesów wszystkich fryzjerów
    zwolnij_zasoby_kierownik(); // zwalniamy wszystkie zasoby
    exit(EXIT_SUCCESS);
}

void zabij_fryzjera()
{
    jeden_fryzjer_zabity = 1;
    for (int i = 0; i < 1; i++) // zabicie jednego fryzjera
    {
        kill(fryzjerzy[i], 1); // 1- SYGNAŁ SIGHUP
    }
    wait_for_process(1);
}

void zabij_fryzjerow()
{
    if (jeden_fryzjer_zabity == 1)
    {
        for (int i = 1; i < F; i++) // zabicie wszystkich fryzjerów
        {
            kill(fryzjerzy[i], 1); // 1 - SYGNAŁ SIGHUP
        }
        wait_for_process(F - 1);
    }
    else
    {
        for (int i = 0; i < F; i++) // zabicie wszystkich fryzjerów
        {
            kill(fryzjerzy[i], 1); // 1 - SYGNAŁ SIGHUP
        }
        wait_for_process(F);
    }
}

void zabij_klientow()
{
    for (int i = 0; i < P; i++) // zabicie wszystkich klientów
    {
        kill(klienci[i], 2); // 2 - SYGNAŁ SIGINT
    }
    wait_for_process(P);
}

void *simulation_timer_thread(void *arg)
{
    if (TP > 0)
    {
        sleep(TP); // Jeśli TP (czas opóźnienia otwarcia salonu) > 0, czekamy przez TP sekundy - domyślnie sleep(TP)
    }
    printf(YELLOW "Salon otwarty.\n" RESET);                                                                   // Informacja o otwarciu salonu
    sem_setval(poczekalnia_semafor, K);                                                                        // Inicjalizacji poczekalni - ilość K wolnych miejsc
    printf(YELLOW "Zainicjalizowano poczekalnię, ilość miejsc: %d.\n" RESET, sem_getval(poczekalnia_semafor)); // Informacja o inicjalizacji poczekalni

    int remaining = sim_duration; // Zmienna do przechowywania czasu pozostałego do zakończenia symulacji
    while (remaining > 0)         // Pętla działa dopóki nie minie czas symulacji
    {
        printf(MAGENTA "Czas pozostały: %d s\n", remaining); // Informacja o pozostałym czasie
        sleep(1);                                            // Symulacja upływu czasu - MUSI BYĆ USTAWIONY CZAS 1
        remaining--;                                         // Zmniejszenie liczby pozostałych sekund
    }
    sem_setval(poczekalnia_semafor, 0); // NALEŻY ZAKOMENTOWAĆ GDY SLEEP(0) Zamknięcie poczekalni po upływie czasu - równoznaczne z zamknięciem salonu - kolejni klienci nie wejdą
    printf(YELLOW "Salon zamknięty.\n" RESET);
    zabij_klientow();
    zabij_fryzjerow();
    zwolnij_zasoby_kierownik();
    printf(RED "Symulacja zakończona można zakończyć program.\n" RESET);
    // koniec_symulacji = 1;
    //  pthread_cancel(timer_thread);
    //   pthread_join(timer_thread, NULL);
    exit(EXIT_SUCCESS);
    // return NULL;
    //   kill(getpid(), SIGKILL);
}

void zwolnij_zasoby_kierownik() // funkcja zwalniająca wszelkie zasoby
{
    usun_kolejke_komunikatow(msg_qid);
    usun_semafor(kasa_semafor);
    usun_semafor(fotele_semafor);
    usun_semafor(poczekalnia_semafor);
    odlacz_pamiec_dzielona(kasa);
    usun_pamiec_dzielona(shm_id);
    printf(YELLOW "Zasoby zwolnione\n" RESET);
}

void tworz_fryzjerow() // funkcja tworząca fryzjerów
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

void tworz_klientow() // funkcja tworząca klientów
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

void zainicjalizuj_kase()
{
    kasa[0] = 10; // banknoty o nominale 10
    kasa[1] = 10; // banknoty o nominale 20
    kasa[2] = 10; // banknoty o nominale 50
}