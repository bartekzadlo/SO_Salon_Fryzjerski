#include "common.h" // Plik nagłówkowy "common.h" – zawiera definicje stałych, struktur oraz prototypów funkcji wykorzystywanych w aplikacji
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
    if (TK <= TP || TK < 0 || TP < 0)
    {
        fprintf(stderr, RED "Błąd: TK musi być większe od TP i jednostki muszą być dodatnie\n" RESET);
        exit(EXIT_FAILURE);
    }
    sim_duration = TK - TP;

    /* ----------------- Pamięć współdzielona ----------------- */
    key_t shm_key = ftok(".", 'S'); // Utworzenie klucza dla segmentu pamięci współdzielonej za pomocą ftok()
    if (shm_key == -1)
    {
        error_exit("ftok dla pamięci współdzielonej");
    }
    shmid = shmget(shm_key, IPC_CREAT | 0600);
    if (shmid < 0)
    {
        error_exit("shmget");
    }
    // Utworzenie segmentu pamięci współdzielonej dla struktury
    // Przypięcie segmentu pamięci do przestrzeni adresowej procesu
    // Inicjalizacja pamięci współdzielonej

    /* ----------------- Kolejka komunikatów ----------------- */
    key_t msg_key = ftok(".", 'Y'); // Utworzenie klucza dla kolejki komunikatów
    if (msg_key == -1)
    {
        error_exit("ftok dla kolejki komunikatów");
    }
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
    // Po zakończeniu symulacji anulujemy wątek managera, aby nie blokował zakończenia programu
    pthread_cancel(manager_thread);
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
}
