#include "common.h"

long id;
int msg_qid;
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
        error_exit("Blad obslugi sygnalu nr 1");
    }

    key_t msg_qkey;
    key_t sem_key_f;
    key_t sem_key_k;
    key_t shm_key;
    struct Message msg;
    int platnosc;

    msg_qkey = ftok(".", 'M');
    shm_key = ftok(".", 'S');
    sem_key_f = ftok(".", 'F');
    sem_key_k = ftok(".", 'K');

    msg_qid = stworz_kolejke_komunikatow(msg_qkey);
    shm_id = stworz_pamiec_dzielona(shm_key);
    fotele_semafor = stworz_semafor(sem_key_f);
    kasa_semafor = stworz_semafor(sem_key_k);

    banknoty = dolacz_do_pamieci_dzielonej(shm_id);

    while (1)
    {
        if (sygnal_fryzjer)
        {
            printf(GREEN "Fryzjer %ld: otrzymałem sygnał, kończę pracę.\n" RESET, id);
            break;
        }
        printf(GREEN "Fryzjer %ld: czekam na klientów w poczekalni.\n" RESET, id);
        if (fryzjer_komunikat_poczekalnia != 1)
        {
            pobierz_komunikat_z_kolejki(msg_qid, &msg, 1);
            fryzjer_komunikat_poczekalnia = 1;
        }

        id_obslugiwany_klient = msg.nadawca;
        printf(GREEN "Fryzjer %ld: zaczynam obsługę klienta %ld.\n" RESET, id, id_obslugiwany_klient);

        if (!fotel)
        {
            sem_p(fotele_semafor, 1);
            fotel = 1;
        }
        printf(GREEN "Fryzjer %ld: zajmuję fotel\n" RESET, id);

        printf(GREEN "Fryzjer %ld: Proszę klienta %ld o zapłatę za strzyżenie.\n" RESET, id, id_obslugiwany_klient);
        msg.message_type = id_obslugiwany_klient;
        msg.nadawca = id;

        if (czeka_na_zaplate != 1)
        {
            printf(GREEN "Fryzjer %ld: czekam na zapłatę.\n" RESET, id);
            wyslij_komunikat_do_kolejki(msg_qid, &msg);
            czeka_na_zaplate = 1;
        }

        if (odbiera_zaplate != 1)
        {
            pobierz_komunikat_z_kolejki(msg_qid, &msg, id);
            printf(GREEN "Fryzjer %ld: otrzymałem komunikat o zapłacie.\n" RESET, id);
            odbiera_zaplate = 1;
        }

        platnosc = msg.platnosc;

        sem_p(kasa_semafor, 1);
        kasa = 1;

        if (platnosc == 30)
        {
            // Klient płaci 20 i 10 zł – zwiększamy licznik banknotów 20 i 10 zł
            banknoty[0] += 1;
            banknoty[1] += 1;
            printf(GREEN "Fryzjer %ld: otrzymałem 20 i 10 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
                   id, banknoty[0], banknoty[1], banknoty[2]);
        }
        else if (platnosc == 50)
        {
            banknoty[2] += 1;
            printf(GREEN "Fryzjer %ld: otrzymałem 50 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
                   id, banknoty[0], banknoty[1], banknoty[2]);
        }

        sem_v(kasa_semafor, 1);
        kasa = 0;

        int service_time = rand() % 3 + 1;
        sleep(service_time); // domyslnie service_time
        printf(GREEN "Fryzjer %ld: zakończyłem strzyżenie klienta %ld (czas usługi: %d s).\n" RESET,
               id, id_obslugiwany_klient, service_time);

        if (fotel)
        {
            sem_v(fotele_semafor, 1);
            fotel = 0;
        }

        if (msg.platnosc == 50)
        {
            sem_p(kasa_semafor, 1);
            kasa = 1;
            while ((banknoty[0] < 2 && banknoty[1] < 1)) // Czekamy, aż w kasie będą dostępne wymagane banknoty (10 zł oraz 20 zł)
            {
                printf(GREEN "Fryzjer %ld: Nie mogę wydać reszty klientowi %ld. Czekam na uzupełnienie\n" RESET, id, id_obslugiwany_klient);
                sem_v(kasa_semafor, 1);
                kasa = 0;
                sleep(3); // domyslnie 3
                sem_p(kasa_semafor, 1);
                kasa = 1;
            }
            if (banknoty[1] >= 1)
            {
                banknoty[1] -= 1;
                printf(GREEN "Fryzjer %ld: wydaję resztę 20 zł klientowi %ld. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
                       id, id_obslugiwany_klient, banknoty[0], banknoty[1], banknoty[2]);
            }
            else
            {
                banknoty[0] -= 2;
                printf(GREEN "Fryzjer %ld: wydaję resztę 2 x 10 zł klientowi %ld. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
                       id, id_obslugiwany_klient, banknoty[0], banknoty[1], banknoty[2]);
            }
            sem_v(kasa_semafor, 1);
            kasa = 0;
        }
        msg.message_type = id_obslugiwany_klient;
        msg.nadawca = id;
        if (wyslalem_reszte != 1)
        {
            wyslij_komunikat_do_kolejki(msg_qid, &msg);
            wyslalem_reszte = 1;
            printf(GREEN "Fryzjer %ld: Wysłałem komunikat o zakończeniu usługi.\n" RESET, id);
        }

        fryzjer_komunikat_poczekalnia = 0;
        czeka_na_zaplate = 0;
        odbiera_zaplate = 0;
        wyslalem_reszte = 0;
    }
    // Po wyjściu z pętli pracy fryzjera, logujemy informację o zakończeniu pracy
    printf(GREEN "Fryzjer %ld: wychodzę z pracy.\n" RESET, id);
}

void sygnal_1(int sig)
{
    long id = getpid();
    printf(RED "Fryzjer %ld: Otrzymałem sygnał 1\n" RESET, id);

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
        zwolnij_zasoby_fryzjer();
        exit(EXIT_SUCCESS);
    }
}

void zwolnij_zasoby_fryzjer()
{
    // Zwolnij semafory
    if (fotel)
    {
        printf(GREEN "Fryzjer %ld: Zwalniam fotel.\n" RESET, id);
        sem_v(fotele_semafor, 1);
    }
    if (kasa)
    {
        printf(GREEN "Fryzjer %ld: Zwalniam kasę.\n" RESET, id);
        sem_v(kasa_semafor, 1);
    }
    // Odłącz pamięć
    odlacz_pamiec_dzielona(banknoty);
}