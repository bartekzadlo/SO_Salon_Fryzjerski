#ifndef KASA_H
#define KASA_H

typedef struct {
    int banknot_10;
    int banknot_20;
    int banknot_50;
} Kasa;

void inicjalizuj_kase(Kasa *kasa);

#endif
