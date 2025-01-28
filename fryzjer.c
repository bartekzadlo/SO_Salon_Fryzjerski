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
        printf("Fryzjer %d czeka na klienta w poczekalni.\n", fryzjer->id); // fryzjer czeka na klienta
        sem_wait(&salon->poczekalnia);                                      // wywołanie semafora poczekalni

        pthread_mutex_lock(&salon->mutex_poczekalnia); // blokujemy muteks poczekalni
        if (salon->klienci_w_poczekalni > 0)
        {
            salon->klienci_w_poczekalni--; // zmniejszenie liczby klientów w poczekalni
            fryzjer->klient = pobierz_klienta_z_kolejki(salon);
            printf("Fryzjer %d pobiera klienta %d.\n", fryzjer->id, fryzjer->klient->id);
            printf("W poczekalni pozostało %d wolnych miejsc.\n", salon->max_klientow - salon->klienci_w_poczekalni); // informacja o liczbie miejsc
        }
        pthread_mutex_unlock(&salon->mutex_poczekalnia); // Odblokowujemy mutex poczekalni

        zajmij_fotel(&salon->fotel);
        printf("Fryzjer %d przygotowuje się do przyjęcia płatności od klienta.\n", fryzjer->id);
        pthread_cond_signal(&fryzjer->klient->czekaj_na_zaplate); // Wysłanie sygnału
        dodaj_banknoty_do_kasy(salon);
        printf("Fryzjer wykonuje obsługę");
        zwolnij_fotel(&salon->fotel);
        wydaj_reszte(&salon->kasa, salon->reszta);
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