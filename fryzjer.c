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
        // Fryzjer czeka na klienta w poczekalni
        printf("Fryzjer %d czeka na klienta w poczekalni.\n", fryzjer->id);
        sem_wait(&salon->poczekalnia);

        pthread_mutex_lock(&salon->mutex_poczekalnia);
        if (salon->klienci_w_poczekalni > 0)
        {
            salon->klienci_w_poczekalni--;
            printf("Fryzjer %d pobiera klienta z poczekalni.\n", fryzjer->id);
        }
        pthread_mutex_unlock(&salon->mutex_poczekalnia);

        // Obsługa klienta
        zajmij_fotel(&salon->fotel);
        printf("Fryzjer %d obsługuje klienta.\n", fryzjer->id);

        // Symulacja czasu obsługi klienta
        int czas_uslugi = rand() % 5 + 1; // Czas obsługi w zakresie 1-5 sekund
        sleep(czas_uslugi);

        // Przyjęcie zapłaty
        int cena = (rand() % 10 + 1) * 10; // Cena w wielokrotności 10
        printf("Fryzjer %d: Cena za usługę: %d zł.\n", fryzjer->id, cena);

        pthread_mutex_lock(&salon->kasa.mutex);
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
                pthread_cond_wait(&salon->kasa.uzupelnienie, &salon->kasa.mutex);
            }
        }
        pthread_mutex_unlock(&salon->kasa.mutex);

        // Zwolnienie fotela
        zwolnij_fotel(&salon->fotel);
        printf("Fryzjer %d zwolnił fotel, klient zakończył wizytę.\n", fryzjer->id);
    }
    return NULL;
}

void inicjalizuj_fryzjera(Fryzjer *fryzjer, Salon *salon, int id)
{
    fryzjer->salon = salon;
    fryzjer->id = id;
    pthread_create(&fryzjer->watek, NULL, fryzjer_praca, fryzjer);
}

void zakoncz_fryzjera(Fryzjer *fryzjer)
{
    pthread_cancel(fryzjer->watek);
    pthread_join(fryzjer->watek, NULL);
}