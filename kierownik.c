#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

void *manager_input_thread(void *arg)
{
    (void)arg; // Nieużywany parametr – brak potrzeby przekazywania argumentów do managera
    char input[16];
    fd_set read_fds;
    struct timeval timeout;

    /* Wyświetlamy prompt raz, przed wejściem w pętlę */
    printf(YELLOW "Wprowadź sygnał (1: fryzjer 0 kończy pracę, 2: zamknięcie salonu):\n" RESET);
    fflush(stdout);

    while (!close_all_clients)
    {
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);
    }
}