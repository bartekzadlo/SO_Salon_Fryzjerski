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

    while (remaining > 0 && !close_all_clients) // Pętla działa dopóki nie minie czas symulacji lub nie otrzymano sygnału zamknięcia salonu
    {
        snprintf(buf, sizeof(buf), "Czas pozostały: %d s", remaining); // Tworzenie komunikatu o pozostałym czasie
        printf(MAGENTA "%s\n" RESET, buf);                             // Wyświetlanie pozostałego czasu w kolorze magenta
        sleep(0);                                                      // Symulacja upływu czasu - domyślnie 1
        remaining--;                                                   // Zmniejszenie liczby pozostałych sekund
    }
    if (!close_all_clients) // Jeśli salon nie został wcześniej zamknięty
    {
        close_all_clients = 1;                        // Ustawiamy flagę zamknięcia salonu
        salon_open = 0;                               // Zamykamy salon, aby nowe wejścia nie były przyjmowane
        pthread_mutex_lock(&poczekalniaMutex);        // Zabezpieczenie przed równoczesnym dostępem do poczekalni
        pthread_cond_broadcast(&poczekalniaNotEmpty); // Budzimy fryzjerów, informując ich o zamknięciu salonu
        pthread_cond_broadcast(&kasa.uzupelnienie);
        while (poczekalniaCount > 0) // Dopóki są klienci w poczekalni
        {
            // Obsługuje każdego klienta w poczekalni
            Klient *klient = poczekalnia[poczekalniaFront]; // Pobieramy klienta z poczekalni
            poczekalniaFront = (poczekalniaFront + 1) % K;  // Przesuwamy wskaźnik frontu poczekalni
            poczekalniaCount--;                             // Zmniejszamy liczbę oczekujących klientów
            sem_post(&klient->served);                      // Informujemy klienta, że może być obsłużony przez fryzjera
        }
        pthread_mutex_unlock(&poczekalniaMutex);
        send_message("Czas symulacji upłynął. Wysłany sygnał 2: Salon zamykany."); // Wysłanie komunikatu o zakończeniu symulacji
    }
    return NULL; // Zakończenie wątku
}