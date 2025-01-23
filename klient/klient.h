#define KLIENT_H

#include "salon.h"

typedef struct {
    int id;
    int banknot_10;
    int banknot_20;
    int banknot_50;
} Klient;

void zarabiaj_pieniadze(Klient *klient);
bool przyjdz_do_salonu(Klient *klient, Salon *salon);
void czekaj_na_fryzjera(Klient *klient, Salon *salon);
void zaplac_za_usluge(Klient *klient, int koszt_uslugi, Salon *salon);
void czekaj_na_reszte(Klient *klient, int reszta);
void opuszcz_salon(Klient *klient, Salon *salon);
void cykliczna_obsluga_klienta(Klient *klient, Salon *salon, int koszt_uslugi);

