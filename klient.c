#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "klient.h"

void inicjalizuj_portfel(Portfel *portfel)
{
    portfel->banknot_10 = 0;
    portfel->banknot_20 = 0;
    portfel->banknot_50 = 0;
    pthread_mutex_init(&portfel->mutex, NULL);
}

void dodaj_pieniadze_do_portfela(Portfel *portfel, int nominal, int ilosc)
{
    pthread_mutex_lock(&portfel->mutex);

    if (nominal == 10)
    {
        portfel->banknot_10 += ilosc;
    }
    else if (nominal == 20)
    {
        portfel->banknot_20 += ilosc;
    }
    else if (nominal == 50)
    {
        portfel->banknot_50 += ilosc;
    }
    else
    {
        printf("Nieprawidłowy nominal: %d\n", nominal);
    }

    pthread_mutex_unlock(&portfel->mutex);
}

void zarabiaj_pieniadze(Klient *klient)
{
    srand(time(NULL));

    // Losowo generujemy liczbę banknotów dla każdego nominalu
    int ilosc_10 = rand() % 5 + 1; // Od 1 do 5
    int ilosc_20 = rand() % 5 + 1; // Od 1 do 5
    int ilosc_50 = rand() % 5 + 1; // Od 1 do 5

    printf("Klient zarabia:\n");
    printf("- %d banknotów po 10 PLN\n", ilosc_10);
    printf("- %d banknotów po 20 PLN\n", ilosc_20);
    printf("- %d banknotów po 50 PLN\n", ilosc_50);

    // Dodajemy zarobione pieniądze do portfela
    dodaj_pieniadze_do_portfela(&klient->portfel, 10, ilosc_10);
    dodaj_pieniadze_do_portfela(&klient->portfel, 20, ilosc_20);
    dodaj_pieniadze_do_portfela(&klient->portfel, 50, ilosc_50);
}

void zamknij_portfel(Portfel *portfel)
{
    pthread_mutex_destroy(&portfel->mutex);
}
