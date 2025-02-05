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
    }
}