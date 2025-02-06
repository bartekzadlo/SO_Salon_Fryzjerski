#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include "common.h"

/* Funkcja pomocnicza usuwająca białe znaki z początku i końca łańcucha */
char *trim_whitespace(char *str)
{
    char *end;
    // Pomijamy białe znaki na początku łańcucha
    while (*str && isspace((unsigned char)*str))
        str++;
    if (*str == 0) // Jeśli łańcuch jest pusty, zwracamy pusty wskaźnik
        return str;
    end = str + strlen(str) - 1; // Pomijamy białe znaki na końcu łańcucha
    while (end > str && isspace((unsigned char)*end))
        end--;
    *(end + 1) = '\0'; // Kończymy łańcuch na ostatnim niebiałym znaku
    return str;
}

void *manager_input_thread(void *arg)
{
    (void)arg; // Nieużywany parametr – brak potrzeby przekazywania argumentów do managera
    char input[16];
    fd_set read_fds;
    struct timeval timeout;

    /* Ustawienie trybu anulowalności na asynchroniczny */
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    // Instrukcja wyświetlająca użytkownikowi informację o dostępnych sygnałach
    printf(YELLOW "Wprowadź sygnał (1: fryzjer 0 kończy pracę, 2: zamknięcie salonu):\n" RESET);
    fflush(stdout);

    while (!close_all_clients) // Pętla oczekująca na sygnały, dopóki nie ma sygnału zamknięcia salonu
    {
        FD_ZERO(&read_fds);                                                  // Zerowanie zbioru descriptorów plików
        FD_SET(STDIN_FILENO, &read_fds);                                     // Dodanie standardowego wejścia (klawiatura) do zbioru
        timeout.tv_sec = 1;                                                  // Ustawienie timeoutu na 1 sekundę
        timeout.tv_usec = 0;                                                 // Brak dodatkowych mikros sekund
        int ret = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout); // Funkcja select oczekuje na dane z wejścia lub timeout

        if (close_all_clients) // Sprawdzanie, czy nie nadszedł sygnał zamknięcia
            break;
        if (ret > 0 && FD_ISSET(STDIN_FILENO, &read_fds)) // Jeśli dane zostały odczytane z wejścia
        {
            if (fgets(input, sizeof(input), stdin) != NULL) // Odczytanie wprowadzonego ciągu znaków z klawiatury
            {
                char *trimmed = trim_whitespace(input); // Usuwanie białych znaków z początku i końca łańcucha
                if (strcmp(trimmed, "1") == 0)
                {
                    if (F <= 1) // Sprawdzamy, czy jest więcej niż jeden fryzjer
                    {
                        printf(RED "Błąd: Nie można wysłać sygnału 1, ponieważ jest tylko jeden fryzjer.\n" RESET);
                    }
                    else
                    {
                        barber_stop[0] = 1; // Ustawiamy flagę dla fryzjera 0 i wysyłamy komunikat
                        send_message("Manager: Odczytano sygnał 1 - fryzjer 0 kończy pracę przed zamknięciem salonu.");
                    }
                }
                else if (strcmp(trimmed, "2") == 0) // Obsługa sygnału 2 – zamknięcie salonu
                {
                    // Ustawienie flag zamknięcia salonu
                    close_all_clients = 1;
                    salon_open = 0;
                    pthread_cond_broadcast(&kasa.uzupelnienie);
                    pthread_mutex_lock(&poczekalniaMutex);
                    pthread_cond_broadcast(&poczekalniaNotEmpty); // Powiadomienie o zamknięciu salonu
                    while (poczekalniaCount > 0)                  // Obsługa wszystkich oczekujących klientów
                    {
                        Klient *klient = poczekalnia[poczekalniaFront];
                        poczekalniaFront = (poczekalniaFront + 1) % K;
                        poczekalniaCount--;
                        sem_post(&klient->served); // Umożliwiamy klientowi kontynuację
                    }
                    pthread_mutex_unlock(&poczekalniaMutex);
                    // Wysyłanie komunikatu o zamknięciu salonu
                    send_message("Manager: Odczytano sygnał 2. Wszyscy klienci opuszczają salon, salon zamykany.");
                }
            }
        }
        else if (ret < 0) // Obsługa błędu funkcji select
        {
            perror("Błąd select w wątku managera");
        }
    }
    return NULL; // Zakończenie wątku managera
}