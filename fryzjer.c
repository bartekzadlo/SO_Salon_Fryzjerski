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
            printf("Fryzjer %d pobiera klienta z poczekalni.\n", fryzjer->id);
            printf("W poczekalni pozostało %d wolnych miejsc.\n", salon->max_klientow - salon->klienci_w_poczekalni); // informacja o liczbie miejsc
            zajmij_fotel(Fotel);
            printf("Fryzjer %d przygotowuje się do przyjęcia płatności od klienta %d.\n", fryzjer->id, klient->id);
            pthread_cond_signal(&klient->czekaj_na_zaplate); // Wysłanie sygnału
            dodaj_banknoty_do_kasy(salon);
            printf("Fryzjer wykonuje obsługę");
            zwolnij_fotel(Fotel);
        }
        pthread_mutex_unlock(&salon->mutex_poczekalnia); // Zwolnienie mutexa
        zajmij_fotel(&salon->fotel);                     // Używamy semafora wolnych foteli
        printf("Fryzjer %d obsługuje klienta.\n", fryzjer->id);
        zwolnij_fotel(&salon->fotel); // Zwolnienie semafora foteli
        printf("Fryzjer %d zwolnił fotel, klient zakończył wizytę.\n", fryzjer->id);
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
