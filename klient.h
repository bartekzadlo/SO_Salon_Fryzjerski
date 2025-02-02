#ifndef KLIENT_H
#define KLIENT_H
#include <semaphore.h>
#include <pthread.h>
#include "salon.h"

void inicjalizuj_klienta(Klient *klient, int id);              // Inicjalizacja klienta, nadanie mu id
void zarabiaj_pieniadze(Klient *klient);                       // Losowanie i dodawanie banknotów do portfela klienta
void klient_przychodzi_do_salon(Salon *salon, Klient *klient); // Klient przychodzi do salonu
void zaplac_za_usluge(Klient *klient, Salon *salon);           // Klient płaci za usługę
void odbierz_reszte(Klient *klient, Kasa *kasa);
void zakoncz_klienta(Klient *klient);
Klient *pobierz_klienta_z_kolejki(Salon *salon);

#endif
