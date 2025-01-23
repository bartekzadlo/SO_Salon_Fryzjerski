#include "kierownik.h"
#include <stdio.h>
#include <stdlib.h>

void wyslij_sygnal_1(int fryzjer_id) {
    printf("Kierownik wysyła sygnał, aby fryzjer %d opuścił salon.\n", fryzjer_id);
}

void wyslij_sygnal_2() {
    printf("Kierownik wysyła sygnał, aby wszyscy klienci opuścili salon.\n");
}
