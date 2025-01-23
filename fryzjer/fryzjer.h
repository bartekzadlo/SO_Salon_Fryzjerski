#define FRYZJER_H
#include <stdbool.h>

typedef struct {
    int banknot_10;
    int banknot_20; 
    int banknot_50; 
} Kasa;


void inicjalizuj_kase(Kasa *kasa);
bool czy_ma_wystarczajaca_reszte(Kasa *kasa, int kwota);
bool wydaj_reszte(Kasa *kasa, int kwota);
void dodaj_do_kasy(Kasa *kasa, int wartosc, int ilosc);
void obsluz_klienta(Kasa *kasa, int zaplata, int koszt_uslugi);
