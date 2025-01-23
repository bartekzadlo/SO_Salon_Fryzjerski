#define SALON_H
#include <stdbool.h> 

typedef struct {
    int miejsca_w_poczekalni;
    bool fryzjer_dostepny;
} Salon;

void inicjalizuj_salon(Salon *salon, int miejsca_w_poczekalni);

