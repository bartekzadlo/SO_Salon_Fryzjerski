#include <stdio.h>
#include "kierownik.h"
#include "fryzjer.h"
#include "klient.h"

int main() {
    srand(time(NULL));

    Klient klient = {1, 0, 0, 0};
    Salon salon = {5, true};

    cykliczna_obsluga_klienta(&klient, &salon, 30);
    return 0;
}
