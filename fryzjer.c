#include "fryzjer.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "salon.h"
#include "klient.h"

void *fryzjer_praca(void *arg)
{
    Fryzjer *fryzjer = (Fryzjer *)arg;
    Salon *salon = fryzjer->salon;

    while (1)
    {
        printf("Fryzjer %d czeka na klienta w poczekalni.\n", fryzjer->id);

        // Fryzjer czeka na klienta w poczekalni
        sem_wait(&salon->poczekalnia);

        pthread_mutex_lock(&salon->mutex_poczekalnia);

        // Sprawdzamy, czy są klienci w poczekalni
        if (salon->klienci_w_poczekalni > 0)
        {
            salon->klienci_w_poczekalni--;                      // Zmniejszamy liczbę klientów w poczekalni
            fryzjer->klient = pobierz_klienta_z_kolejki(salon); // Pobieramy klienta
            printf("Fryzjer %d pobiera klienta %d.\n", fryzjer->id, fryzjer->klient->id);
            printf("W poczekalni pozostało %d wolnych miejsc.\n", salon->max_klientow - salon->klienci_w_poczekalni);

            pthread_mutex_unlock(&salon->mutex_poczekalnia);

            // Dopiero teraz fryzjer zajmuje fotel
            zajmij_fotel(&salon->fotel);
            printf("Fryzjer %d przygotowuje się do przyjęcia płatności od klienta.\n", fryzjer->id);

            // Przekazanie sygnału do klienta, aby poczekał na zapłatę
            pthread_cond_signal(&fryzjer->klient->czekaj_na_zaplate);

            // Fryzjer wykonuje obsługę
            dodaj_banknoty_do_kasy(salon);
            printf("Fryzjer %d wykonuje obsługę.\n", fryzjer->id);

            // Po obsłudze fryzjer zwalnia fotel
            zwolnij_fotel(&salon->fotel);

            // Fryzjer wydaje resztę
            wydaj_reszte(&salon->kasa, salon->reszta);
        }
        else
        {
            pthread_mutex_unlock(&salon->mutex_poczekalnia);
            // Jeśli brak klientów, fryzjer czeka na następnego klienta
            printf("Brak klientów w poczekalni. Fryzjer %d czeka.\n", fryzjer->id);
        }
    }

    return NULL;
}

void inicjalizuj_fryzjera(Fryzjer *fryzjer, Salon *salon, int id)
{
    fryzjer->salon = salon;                                        // Przypisanie salonu do fryzjera
    fryzjer->id = id;                                              // Przypisanie ID fryzjera
    pthread_create(&fryzjer->watek, NULL, fryzjer_praca, fryzjer); // Tworzenie wątku dla fryzjera
}

void zakoncz_fryzjera(Fryzjer *fryzjer)
{
    pthread_cancel(fryzjer->watek);     // Zakończenie wątku fryzjera
    pthread_join(fryzjer->watek, NULL); // Dołączenie wątku
}

Klient *pobierz_klienta_z_kolejki(Salon *salon)
{
    Klient *klient = salon->kolejka.klienci[salon->kolejka.poczatek];
    salon->kolejka.poczatek = (salon->kolejka.poczatek + 1) % 100; // Obsługa cyklicznej kolejki
    return klient;
}