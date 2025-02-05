#include <stdio.h>        // Standardowa biblioteka wejścia/wyjścia – umożliwia korzystanie z funkcji takich jak printf oraz perror
#include <stdlib.h>       // Biblioteka standardowa – dostarcza funkcje zarządzania pamięcią, takie jak exit() oraz malloc()
#include <pthread.h>      // Biblioteka do obsługi wątków POSIX – zawiera funkcje do tworzenia i synchronizacji wątków (pthread_create, pthread_join, mutexy, zmienne warunkowe)
#include <unistd.h>       // Biblioteka unistd – zapewnia dostęp do funkcji systemowych takich jak sleep()
#include <sys/ipc.h>      // Biblioteka do obsługi kluczy IPC – umożliwia korzystanie z funkcji ftok() do generowania unikalnych kluczy
#include <sys/shm.h>      // Biblioteka do operacji na pamięci współdzielonej – pozwala na tworzenie i zarządzanie segmentami pamięci współdzielonej (shmget, shmat)
#include <sys/msg.h>      // Biblioteka do obsługi kolejek komunikatów – zawiera funkcje msgsnd oraz msgrcv, służące do wysyłania i odbierania komunikatów
#include <sys/wait.h>     // Biblioteka do obsługi funkcji wait – umożliwia oczekiwanie na zakończenie procesów potomnych (wait)
#include <semaphore.h>    // Biblioteka do obsługi semaforów – dostarcza funkcje inicjalizacji oraz operacji na semaforach (sem_init)
#include <string.h>       // Biblioteka do operacji na ciągach znakowych – umożliwia korzystanie z funkcji takich jak strncpy
#include <sys/resource.h> // dla struct rlimit, getrlimit, setrlimit, RLIMIT_NPROC
#include "common.h"       // Plik nagłówkowy "common.h" – zawiera definicje stałych, struktur oraz prototypów funkcji wykorzystywanych w aplikacji

// Globalne zmienne do zarządzania poczekalnią klientów

Klient *poczekalnia[K] = {NULL};                               // Tablica wskaźników na klientów, reprezentująca poczekalnię salonu
int poczekalniaFront = 0;                                      // Indeks pierwszego elementu w kolejce poczekalni
int poczekalniaCount = 0;                                      // Aktualna liczba klientów oczekujących w poczekalni
pthread_mutex_t poczekalniaMutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex zabezpieczający dostęp do zmiennych poczekalni
pthread_cond_t poczekalniaNotEmpty = PTHREAD_COND_INITIALIZER; // Zmienna warunkowa sygnalizująca, że poczekalnia nie jest pusta

SalonStats *sharedStats = NULL; // Wskaźnik na strukturę statystyk salonu przechowywanych w pamięci współdzielonej
Kasa kasa;                      // Struktura reprezentująca kasę salonu, zawiera liczbę dostępnych banknotów
sem_t fotele_semafor;           // Semafor ograniczający dostęp do foteli w salonie – określa maksymalną liczbę klientów, którzy mogą jednocześnie zajmować fotele

// Flagi kontrolujące stan salonu i zakończenie symulacji
int salon_open = 1;        // Flaga informująca, czy salon jest nadal otwarty
int close_all_clients = 0; // Flaga sygnalizująca konieczność zakończenia obsługi klientów
int barber_stop[F] = {0};  // Tablica flag określających, kiedy poszczególni fryzjerzy mają zakończyć pracę

/* Godziny – podawane w sekundach od startu symulacji */
int TP = 0;           // początek (przed otwarciem salonu)
int TK = 0;           // koniec
int sim_duration = 0; // TK - TP

int msgqid; // Globalny identyfikator kolejki komunikatów, używany przez loggera oraz inne funkcje komunikacji międzyprocesowej

/* Funkcja do obsługi błędów – wypisuje komunikat i kończy program */
void error_exit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
/*
 * Funkcja init_kasa()
 * -------------------
 * Inicjalizuje kasę salonu poprzez ustawienie początkowej liczby banknotów oraz
 * inicjalizację mutexa i zmiennej warunkowej używanych do synchronizacji operacji na kase.
 */
void init_kasa()
{
    kasa.banknot_10 = 0; // Ustawienie początkowej liczby banknotów o nominale 10
    kasa.banknot_20 = 0; // Ustawienie początkowej liczby banknotów o nominale 20
    kasa.banknot_50 = 0; // Ustawienie początkowej liczby banknotów o nominale 50
    if (pthread_mutex_init(&kasa.mutex_kasa, NULL) != 0)
    {
        error_exit("pthread_mutex_init kasy");
    }
    if (pthread_cond_init(&kasa.uzupelnienie, NULL) != 0)
    {
        error_exit("pthread_cond_init kasy");
    }
}

/*
 * Funkcja send_message()
 * ----------------------
 * Wysyła komunikat do kolejki komunikatów.
 * Parametr 'text' – tekst komunikatu, który ma zostać wysłany.
 */
void send_message(const char *text)
{
    Message msg;
    msg.mtype = MSG_TYPE_EVENT;                           // Ustawienie typu komunikatu jako zdarzenie (event)
    strncpy(msg.mtext, text, MSG_SIZE - 1);               // Kopiowanie tekstu do pola wiadomości, zapewniając, że nie przekroczymy rozmiaru
    msg.mtext[MSG_SIZE - 1] = '\0';                       // Gwarancja zakończenia ciągu znakowego null-em
    if (msgsnd(msgqid, &msg, sizeof(msg.mtext), 0) == -1) // Wysłanie komunikatu do kolejki
    {
        perror("msgsnd"); // Wypisanie błędu w przypadku niepowodzenia
    }
}

/* Funkcja ustawiająca limit procesów na MAX_PROCESSES */
void set_process_limit()
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NPROC, &rl) != 0)
    {
        error_exit("getrlimit");
    }
    if (rl.rlim_cur < MAX_PROCESSES)
    {
        rl.rlim_cur = MAX_PROCESSES;
        if (setrlimit(RLIMIT_NPROC, &rl) != 0)
        {
            error_exit("setrlimit");
        }
    }
}

/*
 * Funkcja logger_process()
 * ------------------------
 * Proces loggera – odbiera komunikaty z kolejki komunikatów i wypisuje je na standardowe wyjście.
 * Działa w nieskończonej pętli do momentu otrzymania komunikatu zakończenia (MSG_TYPE_EXIT).
 */
void logger_process()
{
    while (1)
    {
        Message msg;
        ssize_t ret = msgrcv(msgqid, &msg, sizeof(msg.mtext), 0, 0); // Odbiór komunikatu z kolejki
        if (ret == -1)
        {
            perror("msgrcv in logger");
            break;
        }
        if (msg.mtype == MSG_TYPE_EXIT)
        {
            printf("LOGGER: Otrzymano komunikat zakończenia.\n" RESET);
            break;
        }
        if (strstr(msg.mtext, "Fryzjer") != NULL)
        {
            printf(GREEN "LOGGER: %s\n" RESET, msg.mtext);
        }
        else if (strstr(msg.mtext, "Klient") != NULL)
        {
            printf(YELLOW "LOGGER: %s\n" RESET, msg.mtext);
        }
        else if (strstr(msg.mtext, "Kierownik") != NULL)
        {
            printf(CYAN "LOGGER: %s\n" RESET, msg.mtext);
        }
        else
        {
            printf(RED "LOGGER: %s\n" RESET, msg.mtext);
        }
    }
    exit(0);
}
/*
 * Funkcja main()
 * --------------
 * Główna funkcja programu, która inicjalizuje zasoby, tworzy wątki i procesy,
 * uruchamia symulację salonu fryzjerskiego, a następnie dokonuje sprzątania.
 */

int main()
{
    srand(time(NULL)); // Inicjalizacja generatora liczb losowych, wykorzystywana przy symulacji zdarzeń

    if (F <= 1 || N >= F)
    {
        fprintf(stderr, RED "Błąd: Warunek F > 1 oraz N < F nie jest spełniony.\n" RESET);
        exit(EXIT_FAILURE);
    }

    set_process_limit();

    printf("Podaj czas otwarcia (TP) w sekundach: ");
    if (scanf("%d", &TP) != 1)
    {
        error_exit("Błąd odczytu TP");
    }
    printf("Podaj czas zamknięcia (TK) w sekundach: ");
    if (scanf("%d", &TK) != 1)
    {
        error_exit("Błąd odczytu TK");
    }
    if (TK <= TP)
    {
        fprintf(stderr, RED "Błąd: TK musi być większe od TP.\n" RESET);
        exit(EXIT_FAILURE);
    }
    sim_duration = TK - TP;

    /* ----------------- Pamięć współdzielona ----------------- */
    key_t shm_key = ftok(".", 'S'); // Utworzenie klucza dla segmentu pamięci współdzielonej za pomocą ftok()
    if (shm_key == -1)
    {
        error_exit("ftok dla pamięci współdzielonej");
    }
    int shm_id = shmget(shm_key, sizeof(SalonStats), IPC_CREAT | 0600); // Utworzenie segmentu pamięci współdzielonej dla struktury SalonStats
    if (shm_id < 0)
    {
        error_exit("shmget");
    }

    sharedStats = (SalonStats *)shmat(shm_id, NULL, 0); // Przypięcie segmentu pamięci do przestrzeni adresowej procesu
    if (sharedStats == (void *)-1)
    {
        error_exit("shmat");
    }
    // Inicjalizacja statystyk salonu
    sharedStats->total_clients_served = 0;
    sharedStats->total_clients_left = 0;
    sharedStats->total_services_done = 0;

    /* ----------------- Kolejka komunikatów ----------------- */
    key_t msg_key = ftok(".", 'M'); // Utworzenie klucza dla kolejki komunikatów
    msgqid = msgget(msg_key, IPC_CREAT | 0600);
    if (msgqid < 0)
    {
        error_exit("msgget");
    }

    /* ----------------- Proces Loggera ----------------- */
    pid_t logger_pid = fork(); // Utworzenie nowego procesu przy użyciu fork() w celu uruchomienia loggera
    if (logger_pid < 0)
    {
        error_exit("fork loggera");
    }
    else if (logger_pid == 0)
    {
        logger_process(); // Proces potomny – uruchomienie loggera, który będzie odbierał i wypisywał komunikaty
    }

    sem_init(&fotele_semafor, 0, N); // Inicjalizacja semafora dla foteli, ograniczającego liczbę jednocześnie obsługiwanych klientów

    init_kasa(); // Ustawienie początkowych wartości banknotów i inicjalizacja synchronizacji dla operacji na kasie

    pthread_t manager_thread;
    if (pthread_create(&manager_thread, NULL, manager_input_thread, NULL) != 0)
    {
        error_exit("pthread_create wątku managera");
    }

    pthread_t starter_thread;
    if (pthread_create(&starter_thread, NULL, simulation_starter_thread, NULL) != 0)
    {
        error_exit("pthread_create wątku startującego symulację");
    }

    pthread_join(starter_thread, NULL);
    pthread_join(manager_thread, NULL);
    /* ----------------- Wysłanie komunikatu zakończenia do loggera ----------------- */
    // Przygotowanie komunikatu kończącego działanie loggera
    Message exit_msg;
    exit_msg.mtype = MSG_TYPE_EXIT; // Typ komunikatu ustawiony na zakończenie
    strncpy(exit_msg.mtext, "EXIT", MSG_SIZE - 1);
    exit_msg.mtext[MSG_SIZE - 1] = '\0';
    if (msgsnd(msgqid, &exit_msg, sizeof(exit_msg.mtext), 0) == -1)
    {
        error_exit("msgsnd komunikatu zakończenia");
    }

    /* ----------------- Oczekiwanie na zakończenie procesu loggera ----------------- */
    wait(NULL);

    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
    {
        error_exit("shmctl");
    }
    if (msgctl(msgqid, IPC_RMID, NULL) == -1)
    {
        error_exit("msgctl");
    }

    /* ----------------- Sprzątanie zasobów ----------------- */
    // Niszczenie mutexów oraz zmiennych warunkowych używanych w programie
    pthread_mutex_destroy(&poczekalniaMutex);
    pthread_cond_destroy(&poczekalniaNotEmpty);
    pthread_mutex_destroy(&kasa.mutex_kasa);
    pthread_cond_destroy(&kasa.uzupelnienie);

    // Wypisanie komunikatu informującego o zakończeniu symulacji
    printf(GREEN "Symulacja zakończona.\n" RESET);
    printf(GREEN "Statystyki: Obsłużonych klientów: %d, Odeszło: %d, Wykonanych usług: %d\n" RESET,
           sharedStats->total_clients_served, sharedStats->total_clients_left, sharedStats->total_services_done);
    shmdt(sharedStats);
}
