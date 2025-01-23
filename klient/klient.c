#include "klient.h"
#include <stdio.h>
#include <stdlib.h>

void przyjdz_do_salonu(int klient_id) {
    printf("Klient %d przychodzi do salonu.\n", klient_id);
}

void zaplac_za_usluge(int klient_id, int kwota) {
    printf("Klient %d płaci za usługę: %d zł.\n", klient_id, kwota);
}