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
        sem_wait(&salon->poczekalnia); // Wywołanie semafora poczekalni

        pthread_mutex_lock(&salon->mutex_poczekalnia); // Zamek na mutexie poczekalni
        if (salon->klienci_w_poczekalni > 0)
        {
            salon->klienci_w_poczekalni--; // Zmniejszenie liczby klientów
            printf("Fryzjer %d pobiera klienta z poczekalni.\n", fryzjer->id);
            printf("W poczekalni pozostało %d wolnych miejsc.\n", salon->max_klientow - salon->klienci_w_poczekalni);
        }
        pthread_mutex_unlock(&salon->mutex_poczekalnia); // Zwolnienie mutexa

        zajmij_fotel(&salon->fotel); // Używamy semafora wolnych foteli
        printf("Fryzjer %d obsługuje klienta.\n", fryzjer->id);

        pthread_mutex_lock(&salon->kasa.mutex_kasa);
        while (1)
        {
            if (salon->kasa.banknot_10 * 10 + salon->kasa.banknot_20 * 20 + salon->kasa.banknot_50 * 50 >= cena)
            {
                odejmij_banknoty(&salon->kasa, 50, cena / 50);
                cena %= 50;
                odejmij_banknoty(&salon->kasa, 20, cena / 20);
                cena %= 20;
                odejmij_banknoty(&salon->kasa, 10, cena / 10);
                break;
            }
            else
            {
                printf("Fryzjer %d: Brak pieniędzy w kasie na resztę. Czekanie na uzupełnienie.\n", fryzjer->id);
                pthread_cond_wait(&salon->kasa.uzupelnienie, &salon->kasa.mutex_kasa); // Czekanie na uzupełnienie kasy
            }
        }
        pthread_mutex_unlock(&salon->kasa.mutex_kasa);

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
