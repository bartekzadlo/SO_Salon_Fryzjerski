#include <stdio.h>     // Dla funkcji printf, perror i innych
#include <stdlib.h>    // Dla exit() i innych funkcji
#include <pthread.h>   // Dla operacji na wątkach i mutexach
#include <unistd.h>    // Dla sleep()
#include <sys/ipc.h>   // Dla ftok()
#include <sys/shm.h>   // Dla shmget, shmat i innych funkcji pamięci dzielonej
#include <sys/msg.h>   // Dla msgsnd, msgrcv i innych funkcji kolejki komunikatów
#include <sys/wait.h>  // Dla wait
#include <semaphore.h> // Dla semaforów
#include <string.h>    // Dla strncpy
#include "common.h"

Klient *poczekalnia[MAX_WAITING] = {NULL};
int poczekalniaFront = 0;
int poczekalniaCount = 0;
pthread_mutex_t poczekalniaMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t poczekalniaNotEmpty = PTHREAD_COND_INITIALIZER;

Kasa kasa;

sem_t fotele_semafor;

int salon_open = 1;
int close_all_clients = 0;
int barber_stop[F] = {0};

int msgqid; // Definicja zmiennej globalnej

void init_kasa()
{
    kasa.banknot_10 = 50;
    kasa.banknot_20 = 50;
    kasa.banknot_50 = 50;
    pthread_mutex_init(&kasa.mutex_kasa, NULL);
    pthread_cond_init(&kasa.uzupelnienie, NULL);
}

/* Funkcja pomocnicza wysyłająca komunikat do kolejki */
void send_message(const char *text)
{
    Message msg;
    msg.mtype = MSG_TYPE_EVENT;
    strncpy(msg.mtext, text, MSG_SIZE - 1);
    msg.mtext[MSG_SIZE - 1] = '\0';
    if (msgsnd(msgqid, &msg, sizeof(msg.mtext), 0) == -1)
    {
        perror("msgsnd");
    }
}

/* Proces loggera – odbiera komunikaty z kolejki i wypisuje je */
void logger_process()
{
    while (1)
    {
        Message msg;
        ssize_t ret = msgrcv(msgqid, &msg, sizeof(msg.mtext), 0, 0);
        if (ret == -1)
        {
            perror("msgrcv in logger");
            break;
        }
        if (msg.mtype == MSG_TYPE_EXIT)
        {
            printf("LOGGER: Otrzymano komunikat zakończenia.\n");
            break;
        }
        printf("LOGGER: %s\n", msg.mtext);
    }
    exit(0);
}

int main()
{
    srand(time(NULL));
    /* Utworzenie kolejki komunikatów */
    key_t msg_key = ftok(".", 'M');
    msgqid = msgget(msg_key, IPC_CREAT | 0666);
    if (msgqid < 0)
    {
        perror("msgget");
        exit(1);
    }
    /* Fork – tworzymy oddzielny proces loggera */
    pid_t logger_pid = fork();
    if (logger_pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (logger_pid == 0)
    {
        /* Proces dziecka: logger */
        logger_process();
    }
    sem_init(&fotele_semafor, 0, N);
    init_kasa();
    /* Tworzenie wątków fryzjerów */
    pthread_t fryzjerzy[F];
    for (int i = 0; i < F; i++)
    {
        int *arg = malloc(sizeof(*arg));
        if (!arg)
        {
            perror("malloc");
            exit(1);
        }
        *arg = i;
        if (pthread_create(&fryzjerzy[i], NULL, barber_thread, arg) != 0)
        {
            perror("pthread_create barber");
            exit(1);
        }
    }

    /* Tworzenie wątków klientów */
    pthread_t klienci[P];
    for (int i = 0; i < P; i++)
    {
        int *arg = malloc(sizeof(*arg));
        if (!arg)
        {
            perror("malloc");
            exit(1);
        }
        *arg = i + 1;
        if (pthread_create(&klienci[i], NULL, client_thread, arg) != 0)
        {
            perror("pthread_create client");
            exit(1);
        }
    }

    /* Tworzenie wątku managera */
    pthread_t manager;
    if (pthread_create(&manager, NULL, manager_thread, NULL) != 0)
    {
        perror("pthread_create manager");
        exit(1);
    }

    /* Czekamy na zakończenie pracy managera */
    pthread_join(manager, NULL);

    /* Dajemy chwilę na zakończenie bieżących obsług */
    sleep(5);

    /* Łączenie (join) wątków fryzjerów */
    for (int i = 0; i < F; i++)
    {
        pthread_join(fryzjerzy[i], NULL);
    }

    /* Łączenie (join) wątków klientów */
    for (int i = 0; i < P; i++)
    {
        pthread_join(klienci[i], NULL);
    }

    /* Wysyłamy komunikat zakończenia do loggera */
    Message exit_msg;
    exit_msg.mtype = MSG_TYPE_EXIT;
    strncpy(exit_msg.mtext, "EXIT", MSG_SIZE - 1);
    exit_msg.mtext[MSG_SIZE - 1] = '\0';
    if (msgsnd(msgqid, &exit_msg, sizeof(exit_msg.mtext), 0) == -1)
    {
        perror("msgsnd exit message");
    }

    /* Czekamy na zakończenie procesu loggera */
    wait(NULL);

    /* Sprzątanie – niszczenie mutexów i zmiennych warunkowych */
    pthread_mutex_destroy(&poczekalniaMutex);
    pthread_cond_destroy(&poczekalniaNotEmpty);
    pthread_mutex_destroy(&kasa.mutex_kasa);
    pthread_cond_destroy(&kasa.uzupelnienie);

    printf("Symulacja salonu fryzjerskiego zakończona.\n");
    return 0;
}
