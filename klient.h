#ifndef KLIENT_H
#define KLIENT_H
#include "salon.h"

typedef struct Klient
{
    int portfel_10;
    int portfel_20;
    int portfel_50;
} Klient;

void inicjalizuj_portfel(Klient *klient);                                      // Inicjalizacja pustego portfela klienta
void zarabiaj_pieniadze(Klient *klient);                                       // Losowanie i dodawanie banknotów do portfela klienta
void klient_przychodzi_do_salon(Salon *salon, Klient *klient, int Tp, int Tk); // Klient przychodzi do salonu
void zaplac_za_usluge(Klient *klient);                                         // Klient płaci za usługę
void otrzymaj_reszte(Klient *klient);                                          // Klient otrzymuje resztę (tymczasowa funkcjonalność)

#endif
