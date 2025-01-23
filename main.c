#include <stdio.h>
#include "kierownik.h"
#include "fryzjer.h"
#include "klient.h"

int main() {

    int klient1_id = 1;
    int klient2_id = 2;

    przyjdz_do_salonu(klient1_id);
    przyjdz_do_salonu(klient2_id);

    zaplac_za_usluge(klient1_id, 30);
    zaplac_za_usluge(klient2_id, 50);

    przyjmij_zaplate(klient1_id, 30, 40);
    przyjmij_zaplate(klient2_id, 50, 50);

    wydaj_reszte(klient1_id);
    wydaj_reszte(klient2_id);

    wyslij_sygnal_1(1);
    wyslij_sygnal_2();

    return 0;
}