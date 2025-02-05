#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

void *simulation_starter_thread(void *arg)
{
    (void)arg;
    char log_buffer[MSG_SIZE];

    if (TP > 0)
    {
        sleep(TP);
    }
    salon_open = 1;
    send_message("Salon otwarty.");

    pthread_t fryzjerzy[F];
    for (int i = 0; i < F; i++)
    {
        int *arg = malloc(sizeof(*arg));
        if (!arg)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *arg = i;
        if (pthread_create(&fryzjerzy[i], NULL, barber_thread, arg) != 0)
        {
            perror("pthread_create barber");
            exit(EXIT_FAILURE);
        }
    }
    pthread_t klienci[P];
    for (int i = 0; i < P; i++)
    {
        int *arg = malloc(sizeof(*arg));
        if (!arg)
        {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        *arg = i + 1;
        if (pthread_create(&klienci[i], NULL, client_thread, arg) != 0)
        {
            perror("pthread_create client");
            exit(EXIT_FAILURE);
        }
    }
    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, simulation_timer_thread, NULL) != 0)
    {
        perror("pthread_create timer");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < F; i++)
    {
        pthread_join(fryzjerzy[i], NULL);
    }
    for (int i = 0; i < P; i++)
    {
        pthread_join(klienci[i], NULL);
    }
    pthread_join(timer_thread, NULL);
    snprintf(log_buffer, MSG_SIZE, "Symulacja zakończona. Statystyki: Klienci odeszli: %d, Usługi wykonane: %d",
             sharedStats->total_clients_left, sharedStats->total_services_done);
    send_message(log_buffer);
    return NULL;
}