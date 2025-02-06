#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"

void *simulation_starter_thread(void *arg)
{
    (void)arg;
    char log_buffer[MSG_SIZE]; // Bufor do logowania

    if (TP > 0)
    {
        sleep(TP); // Jeśli TP (czas opóźnienia otwarcia salonu) > 0, czekamy przez TP sekundy - domyślnie sleep(TP)
    }

    salon_open = 1;                 // Otwieramy salon
    send_message("Salon otwarty."); // Wysyłamy komunikat o otwarciu salonu

    // Tworzymy wątki dla fryzjerów
    pthread_t fryzjerzy[F]; // Tablica wątków dla fryzjerów
    for (int i = 0; i < F; i++)
    {
        int *arg = malloc(sizeof(*arg)); // Alokujemy pamięć dla argumentu wątku (numer fryzjera)
        if (!arg)
        {
            perror("Błąd malloc dla fryzjera"); // Błąd alokacji pamięci
            exit(EXIT_FAILURE);
        }
        *arg = i; // Ustawiamy numer fryzjera
        // Tworzymy wątek dla fryzjera
        if (pthread_create(&fryzjerzy[i], NULL, barber_thread, arg) != 0)
        {
            perror("Błąd pthread_create barber"); // Błąd przy tworzeniu wątku fryzjera
            exit(EXIT_FAILURE);
        }
    }
    // Tworzymy wątki dla klientów
    pthread_t klienci[P]; // Tablica wątków dla klientów
    for (int i = 0; i < P; i++)
    {
        int *arg = malloc(sizeof(*arg)); // Alokujemy pamięć dla argumentu wątku (numer klienta)
        if (!arg)
        {
            perror("Błąd malloc dla klienta"); // Błąd alokacji pamięci
            exit(EXIT_FAILURE);
        }
        *arg = i + 1; // Ustawiamy numer klienta (zaczynamy od 1)
        // Tworzymy wątek dla klienta
        if (pthread_create(&klienci[i], NULL, client_thread, arg) != 0)
        {
            perror("Błąd pthread_create client"); // Błąd przy tworzeniu wątku klienta
            exit(EXIT_FAILURE);
        }
    }

    // Tworzymy wątek do odliczania czasu symulacji
    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, simulation_timer_thread, NULL) != 0)
    {
        perror("Błąd pthread_create timer"); // Błąd przy tworzeniu wątku timera
        exit(EXIT_FAILURE);
    }

    // Czekamy na zakończenie wszystkich wątków fryzjerów
    for (int i = 0; i < F; i++)
    {
        pthread_join(fryzjerzy[i], NULL); // Dołączamy wątki fryzjerów
    }
    // Czekamy na zakończenie wszystkich wątków klientów
    for (int i = 0; i < P; i++)
    {
        pthread_join(klienci[i], NULL); // Dołączamy wątki klientów
    }

    // Czekamy na zakończenie wątku timera
    pthread_join(timer_thread, NULL);
    // Logujemy wynik symulacji
    snprintf(log_buffer, MSG_SIZE, "Symulacja zakończona. Statystyki: Klienci odeszli: %d, Usługi wykonane: %d",
             sharedStats->total_clients_left, sharedStats->total_services_done);
    send_message(log_buffer); // Wysyłamy komunikat o zakończeniu symulacji
    return NULL;              // Kończymy wątek
}