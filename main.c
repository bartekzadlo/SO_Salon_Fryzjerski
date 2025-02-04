#include <stdio.h>     // Dla funkcji printf, perror i innych
#include <stdlib.h>    // Dla exit() i innych funkcji
#include <pthread.h>   // Dla operacji na wątkach i mutexach
#include <unistd.h>    // Dla sleep()
#include <sys/ipc.h>   // Dla ftok()
#include <sys/shm.h>   // Dla shmget, shmat i innych funkcji pamięci dzielonej
#include <semaphore.h> // Dla semaforów
#include <errno.h>     // Do obsługi błędów systemowych
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

void init_kasa()
{
    kasa.banknot_10 = 10;
    kasa.banknot_20 = 5;
    kasa.banknot_50 = 2;
    pthread_mutex_init(&kasa.mutex_kasa, NULL);
    pthread_cond_init(&kasa.uzupelnienie, NULL);
}

int main()
{
    srand(time(NULL));
    sem_init(&fotele_semafor, 0, N);
    kasa.banknot_10 = 1;
    kasa.banknot_20 = 1;
    kasa.banknot_50 = 0;
    pthread_mutex_init(&kasa.mutex_kasa, NULL);
    pthread_cond_init(&kasa.uzupelnienie, NULL);
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

    /* Sprzątanie – niszczenie mutexów i zmiennych warunkowych */
    pthread_mutex_destroy(&poczekalniaMutex);
    pthread_cond_destroy(&poczekalniaNotEmpty);
    pthread_mutex_destroy(&kasa.mutex_kasa);
    pthread_cond_destroy(&kasa.uzupelnienie);

    printf("Symulacja salonu fryzjerskiego zakończona.\n");
    return 0;
}
