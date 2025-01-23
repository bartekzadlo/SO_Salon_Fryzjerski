#include "salon.h"
#include <stdio.h>
#include <stdbool.h> 

void inicjalizuj_salon(Salon *salon, int miejsca_w_poczekalni) {
    salon->miejsca_w_poczekalni = miejsca_w_poczekalni;
    salon->fryzjer_dostepny = true;
    printf("Salon został zainicjalizowany z %d miejscami w poczekalni.\n", salon->miejsca_w_poczekalni);
}
