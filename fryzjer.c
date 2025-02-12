#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

long id;
int kolejka;
key_t klucz;
struct komunikat kom;
int fotele_semafor;
int kasa_semafor;
int shm_id;
int *banknoty;
long id_obslugiwany_klient;
volatile sig_atomic_t sygnal_fryzjer = 0;
volatile sig_atomic_t fryzjer_komunikat_poczekalnia = 0;
volatile sig_atomic_t fotel = 0;
volatile sig_atomic_t kasa = 0;
volatile sig_atomic_t czeka_na_zaplate = 0;
volatile sig_atomic_t odbiera_zaplate = 0;
volatile sig_atomic_t wyslalem_reszte = 0;

int main()
{
    srand(time(NULL));
    long id = getpid(); // Pobranie identyfikatora fryzjera

    if (signal(SIGHUP, sygnal_1) == SIG_ERR)
    {
        error_exit("Blad obslugi sygnalu nr 1")
    }

    klucz = ftok(".", "M");
    kolejka = utworz_kolejke(klucz);
    klucz = ftok(".", "F");
    fotele_semafor = utworz_semafor(klucz);
    klucz = ftok(".", "K");
    kasa_semafor = utworz_semafor(klucz);
    klucz = ftok(".", "S");
    shm_id = utworz_pamiec_dzielona(klucz);
    banknoty = dolacz_pamiec_dzielona(shm_id);

    while (1)
    {
        if (sygnal_fryzjer)
        {
            printf(GREEN "Fryzjer %d: otrzymałem sygnał, kończę pracę." RESET, id);
            break;
        }

        if (fryzjer_komunikat_poczekalnia != 1)
        {
            odbierz_komunikat(kolejka, &kom, 1);
            fryzjer_komunikat_poczekalnia = 1;
        }

        id_obslugiwany_klient = kom.nadawca;

        if (!fotel)
        {
            sem_p(fotele_semafor, 1);
            fotel = 1;
        }
        printf(GREEN "Fryzjer %d: rozpoczynam obsługę klienta %d zajmując fotel" RESET, id, id_obslugiwany_klient);

        kom.mtype = id_obslugiwany_klient;
        kom.nadawca = id;

        if (czeka_na_zaplate != 1)
        {
            wyslij_komunikat(kolejka, &kom);
            czeka_na_zaplate = 1;
        }

        if (odbiera_zaplate != 1)
        {
            odbierz_komunikat(kolejka, &kom, id);
            odbiera_zaplate = 1;
        }

        sem_p(kasa_semafor, 1);
        kasa = 1;

        if (kom.platnosc == 30)
        {
            // Klient płaci 20 i 10 zł – zwiększamy licznik banknotów 20 i 10 zł
            banknoty[0] += 1;
            banknoty[1] += 1;
            printf(GREEN "Fryzjer %d: otrzymałem 20 i 10 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d." RESET,
                   id, banknoty[0], banknoty[1], banknoty[2]);
        }
        else if (kom.platnosc == 50)
        {
            banknoty[2] += 1;
            printf(GREEN "Fryzjer %d: otrzymałem 50 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d." RESET,
                   id, banknoty[0], banknoty[1], banknoty[2]);
        }

        sem_v(kasa_semafor, 1);
        kasa = 0;

        int service_time = rand() % 3 + 1;
        sleep(service_time);
        printf(GREEN "Fryzjer %d: zakończyłem strzyżenie klienta %d (czas usługi: %d s)." RESET,
               id, id_obslugiwany_klient, service_time);

        if (fotel)
        {
            sem_v(fotele_semafor, 1);
            fotel = 0;
        }

        if (kom.platnosc == 50)
        {
            sem_p(kasa_semafor, 1);
            kasa = 1;
            while ((banknoty[0] < 2 && banknoty[1] < 1)) // Czekamy, aż w kasie będą dostępne wymagane banknoty (10 zł oraz 20 zł)
            {
                printf(GREEN "Fryzjer %d: Nie mogę wydać reszty klientowi %d. Czekam na uzupełnienie" RESET, id, id_obslugiwany_klient);
                sem_v(kasa_semafor, 1);
                kasa = 0;
                sleep(0); // oczekiwanie na wpłatę
                sem_p(kasa_semafor, 1);
                kasa = 1;
            }
            if (banknoty[1] >= 1)
            {
                banknoty[1] -= 1;
                printf(GREEN "Fryzjer %d: wydaję resztę 20 zł klientowi %d. Kasa: 10zł=%d, 20zł=%d, 50zł=%d." RESET,
                       id, id_obslugiwany_klient, banknoty[0], banknoty[1], banknoty[2]);
            }
            else
            {
                banknoty[0] -= 2;
                printf(GREEN "Fryzjer %d: wydaję resztę 2 x 10 zł klientowi %d. Kasa: 10zł=%d, 20zł=%d, 50zł=%d." RESET,
                       id, id_obslugiwany_klient, banknoty[0], banknoty[1], banknoty[2]);
            }
            sem_v(kasa_semafor, 1);
            kasa = 0;
        }
        if (wyslalem_reszte != 1)
        {
            wyslij_komunikat(kolejka, &kom);
            wyslalem_reszte = 1;
        }
        fryzjer_komunikat_poczekalnia = 0;
        czeka_na_zaplate = 0;
        odbiera_zaplate = 0;
        wyslalem_reszte = 0;
    }
    // Po wyjściu z pętli pracy fryzjera, logujemy informację o zakończeniu pracy
    printf(GREEN "Fryzjer %d: wychodzę z pracy." RESET, id);
    return NULL;
}

void sygnal_1(int sig)
{
    printf(GREEN "Fryzjer %d: otrzymałem sygnał nr 1." RESET, id);

    // Ustaw flagi
    if (fryzjer_komunikat_poczekalnia)
    {
        sygnal_fryzjer = 1;

        if (czeka_na_zaplate != 1)
        {
            czeka_na_zaplate = -1;
        }
        else if (odbiera_zaplate != 1)
        {
            odbiera_zaplate = -1;
        }
        else if (wyslalem_reszte != 1)
        {
            wyslalem_reszte = -1;
        }
    }
    else
    {
        zwolnij_zasoby();
        printf(GREEN "Fryzjer %d: wychodzę z pracy." RESET, id);
    }
}

void zwolnij_zasoby()
{
    // Zwolnij semafory
    if (fotel)
    {
        printf(GREEN "Fryzjer %d: Zwalniam fotel." RESET, id);
        sem_v(fotele_semafor, 1);
    }
    if (kasa)
    {
        printf(GREEN "Fryzjer %d: Zwalniam kasę." RESET, id);
        sem_v(kasa_semafor, 1);
    }
    // Odłącz pamięć
    odlacz_pamiec_dzielona(banknoty);
}