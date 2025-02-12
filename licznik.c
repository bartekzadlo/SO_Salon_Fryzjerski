#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <semaphore.h>
#include <string.h>
#include "common.h"

void *simulation_timer_thread(void *arg)
{
    (void)arg;
    int remaining = sim_duration; // Zmienna do przechowywania czasu pozostałego do zakończenia symulacji
    char buf[64];                 // Bufor do przechowywania komunikatu o pozostałym czasie

    while (remaining > 0) // Pętla działa dopóki nie minie czas symulacji lub nie otrzymano sygnału zamknięcia salonu
    {
        snprintf(buf, sizeof(buf), "Czas pozostały: %d s", remaining); // Tworzenie komunikatu o pozostałym czasie
        printf(MAGENTA "%s\n" RESET, buf);                             // Wyświetlanie pozostałego czasu w kolorze magenta
        sleep(0);                                                      // Symulacja upływu czasu - domyślnie 1
        remaining--;                                                   // Zmniejszenie liczby pozostałych sekund
    }
    send_message("Czas symulacji upłynął. Wysłany sygnał 2: Salon zamykany."); // Wysłanie komunikatu o zakończeniu symulacji
    return NULL;                                                               // Zakończenie wątku
}