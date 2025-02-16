#include "common.h"

long id_fryzjer;
int msg_qid;
int fotele_semafor;
int kasa_semafor;
int shm_id;
int *kasa;
long id_klienta;
volatile sig_atomic_t sygnal_fryzjer = 0;
volatile sig_atomic_t fryzjer_komunikat_poczekalnia = 0;
volatile sig_atomic_t fotel_zajety = 0;
volatile sig_atomic_t kasa_zajeta = 0;
volatile sig_atomic_t czeka_na_zaplate = 0;
volatile sig_atomic_t odbiera_zaplate = 0;
volatile sig_atomic_t koniec_obslugi = 0;

int main()
{
    // Inicjalizacja generatora liczb losowych na podstawie czasu systemowego
    srand(time(NULL));
    // Rejestracja funkcji obsługi sygnału SIGHUP (przerwanie) w celu wywołania funkcji sygnal_1 przy otrzymaniu sygnału.
    if (signal(SIGHUP, sig_handler_fryzjer) == SIG_ERR)
    {
        error_exit("Błąd obsługi sygnału usunięcia fryzjera");
    }

    // Deklaracja zmiennej typu Message, która będzie używana do przechowywania wiadomości wysyłanych i odbieranych w komunikacji między procesami
    struct Message msg;
    // Zmienna do przechowywania wartości płatności za usługę fryzjerską, jest ona przesłana nam od klienta, gdzie została wylosowana
    int platnosc;

    // deklaracja kluczy do kolejki komunikatów, semaforów foteli i kasy oraz pamięci dzielonej
    key_t msg_qkey;
    key_t sem_key_f;
    key_t sem_key_k;
    key_t shm_key;

    // Tworzymy klucze
    msg_qkey = ftok(".", 'M');
    shm_key = ftok(".", 'S');
    sem_key_f = ftok(".", 'F');
    sem_key_k = ftok(".", 'K');

    // Tworzymy struktury
    msg_qid = stworz_kolejke_komunikatow(msg_qkey);
    shm_id = stworz_pamiec_dzielona(shm_key);
    fotele_semafor = stworz_semafor(sem_key_f);
    kasa_semafor = stworz_semafor(sem_key_k);
    // Kasa nasza pamiec wspoldzielona
    kasa = dolacz_do_pamieci_dzielonej(shm_id);

    while (1)
    {
        id_fryzjer = getpid(); // Pobranie identyfikatora fryzjera
        if (sygnal_fryzjer)    // Jeśli ustawiona flaga sygnału to nie rozpoczynamy ponownej usługi
        {
            printf(GREEN "Fryzjer %ld: otrzymałem sygnał, kończę pracę.\n" RESET, id_fryzjer);
            break;
        }
        printf(GREEN "Fryzjer %ld: czekam na klientów w poczekalni.\n" RESET, id_fryzjer); // Fryzjer na początku pracy domyślnie czeka na klientów
        if (fryzjer_komunikat_poczekalnia != 1)                                            // Jeśli fryzjer nie otrzymał komunikatu o kliencie w poczekalni
        {
            pobierz_komunikat_z_kolejki(msg_qid, &msg, 1); // Pobranie komunikatu o kliencie w poczekalni
            fryzjer_komunikat_poczekalnia = 1;             // Ustawienie flagi o tym, że komunikat został przyjęty
        }

        id_klienta = msg.nadawca;                                                                   // przypisujemy id obsługiwanego klienta przekazane w komunikacie do zmiennej
        printf(GREEN "Fryzjer %ld: zaczynam obsługę klienta %ld.\n" RESET, id_fryzjer, id_klienta); // informacja zaczynamy obsługę

        if (!fotel_zajety) // jeśli fotel nie jest zajęty
        {
            sem_p(fotele_semafor, 1); // zajmujemy semaforem fotel
            fotel_zajety = 1;         // ustawienie flagi o zajęciu fotela
        }
        printf(GREEN "Fryzjer %ld: zajmuję fotel\n" RESET, id_fryzjer); // Informacja o zajęciu fotela

        printf(GREEN "Fryzjer %ld: Proszę klienta %ld o zapłatę za strzyżenie.\n" RESET, id_fryzjer, id_klienta); // Przechodzimy do prośby o płatnosc
        // Przygotowanie komunikatu - wezwania do zapłaty
        msg.message_type = id_klienta;
        msg.nadawca = id_fryzjer;

        if (!czeka_na_zaplate) // Sprawdzenie flagi
        {
            wyslij_komunikat_do_kolejki(msg_qid, &msg); // Wysłanie komunikatu do kolejki o tym, że fryzjer czeka na zapłatę klienta
            czeka_na_zaplate = 1;                       // Ustawienie flagi czeka na zapłatę
        }

        if (!odbiera_zaplate) // Sprawdzenie flagi czy fryzjer już odbierał zapłatę
        {
            pobierz_komunikat_z_kolejki(msg_qid, &msg, id_fryzjer); // Pobieramy komunikat o zapłacie
            printf(GREEN "Fryzjer %ld: otrzymałem komunikat o zapłacie.\n" RESET, id_fryzjer);
            odbiera_zaplate = 1; // Ustawiamy flagę
        }

        platnosc = msg.platnosc; // Przypisujemy płatność przekazaną przez klienta do zmiennej płatność

        zajmij_kase();

        if (platnosc == 30) // jeśli płatność 30 przekazana przez klienta to:
        {
            // Klient płaci 20 i 10 zł – zwiększamy licznik banknotów 20 i 10 zł
            kasa[0] += 1;
            kasa[1] += 1;
            printf(GREEN "Fryzjer %ld: otrzymałem 20 i 10 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
                   id_fryzjer, kasa[0], kasa[1], kasa[2]);
        }
        else if (platnosc == 50) // jeśli płatność klienta wyniosła 50 zł to:
        {
            kasa[2] += 1; // dodajemy do kasy baknoty 50 zł
            printf(GREEN "Fryzjer %ld: otrzymałem 50 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
                   id_fryzjer, kasa[0], kasa[1], kasa[2]);
        }

        zwolnij_kase();

        int service_time = rand() % 3 + 1; // losowanie czasu symulacji strzyżenia
        sleep(0);                          // symulacja strzyżenia - domyslnie service_time
        printf(GREEN "Fryzjer %ld: zakończyłem strzyżenie klienta %ld (czas usługi: %d s).\n" RESET,
               id_fryzjer, id_klienta, service_time);

        if (fotel_zajety) // jeśli fotel zajęty
        {
            sem_v(fotele_semafor, 1); // zwalniamy fotel
            fotel_zajety = 0;         // ustawiamy flagę fotela na 0
        }

        // Wydawanie reszty - tylko gdy płatność = 50 zł
        if (platnosc == 50) // jeśli płatność wynosiłą 50
        {
            wydaj_reszte();
        }
        // Przygotowujemy komunikat dla klienta, że obsługa została zakończona i reszta jeśli wymagana wydana
        msg.message_type = id_klienta;
        msg.nadawca = id_fryzjer;
        if (koniec_obslugi != 1) // sprawdzamy flagę koniec obsługi
        {
            wyslij_komunikat_do_kolejki(msg_qid, &msg); // wysyłamy komunikat do klienta o zakończeniu obsługi
            koniec_obslugi = 1;                         // ustawiamy flagę
            printf(GREEN "Fryzjer %ld: Wysłałem komunikat o zakończeniu usługi.\n" RESET, id_fryzjer);
        }
        // Resetujemy flagi przed kolejną iteracją pracy fryzjera
        fryzjer_komunikat_poczekalnia = 0;
        czeka_na_zaplate = 0;
        odbiera_zaplate = 0;
        koniec_obslugi = 0;
    }
    // Po wyjściu z pętli pracy fryzjera, logujemy informację o zakończeniu pracy
    printf(GREEN "Fryzjer %ld: wychodzę z pracy.\n" RESET, id_fryzjer);
}

void sig_handler_fryzjer(int sig)
{
    printf(RED "Fryzjer %ld: Otrzymałem sygnał końca pracy\n" RESET, id_fryzjer); // informacja o otrzymaniu sygnału końca pracy

    // Ustaw flagi
    if (fryzjer_komunikat_poczekalnia) // jeśli fryzjer jest już po przyjęciu komunikatu - tzn że jest w trakcie obsługi
    {
        sygnal_fryzjer = 1; // ustawiamy sygnał fryzjera
        if (czeka_na_zaplate != 1)
        {
            czeka_na_zaplate = -1;
        }
        else if (odbiera_zaplate != 1)
        {
            odbiera_zaplate = -1;
        }
        else if (koniec_obslugi != 1)
        {
            koniec_obslugi = -1;
        }
    }
    else
    {
        // Jeśli fryzjer nie obsługuje klienta to zwalniamy zasoby i kończymy jego pracę
        fryzjer_exit();
        exit(EXIT_SUCCESS);
    }
}

void fryzjer_exit()
{
    // Zwolnij semafory
    if (fotel_zajety)
    {
        printf(GREEN "Fryzjer %ld: Zwalniam fotel.\n" RESET, id_fryzjer);
        sem_v(fotele_semafor, 1);
    }
    if (kasa_zajeta)
    {
        printf(GREEN "Fryzjer %ld: Zwalniam kasę.\n" RESET, id_fryzjer);
        sem_v(kasa_semafor, 1);
    }
    // Odłącz pamięć
    odlacz_pamiec_dzielona(kasa);
}

void wydaj_reszte()
{
    zajmij_kase();
    while ((kasa[0] < 2 && kasa[1] < 1)) // Jeśli w kasie nie mamy 2 baknkotów 10 złotych lub 1 banknotu 20 złotowego
    {
        printf(GREEN "Fryzjer %ld: Nie mogę wydać reszty klientowi %ld. Czekam na uzupełnienie\n" RESET, id_fryzjer, id_klienta);
        zwolnij_kase();
        sleep(0); // czekamy chwilę, może ktoś w tym czasie uzupełni kasę - domyslnie 3
        zajmij_kase();
    }
    if (kasa[1] >= 1) // jeśli mamy jeden banknot 20 złotych
    {
        kasa[1] -= 1; // wydajemy ten banknot
        printf(GREEN "Fryzjer %ld: wydaję resztę 20 zł klientowi %ld. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
               id_fryzjer, id_klienta, kasa[0], kasa[1], kasa[2]);
    }
    else // w innym przypadku to oznacza, że mamy wystarczająco banknotów 10 złotych
    {
        kasa[0] -= 2; // wydajemy dwa baknoty 10 złotych
        printf(GREEN "Fryzjer %ld: wydaję resztę 2 x 10 zł klientowi %ld. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.\n" RESET,
               id_fryzjer, id_klienta, kasa[0], kasa[1], kasa[2]);
    }
    zwolnij_kase();
}

void zajmij_kase()
{
    sem_p(kasa_semafor, 1); // zajmujemy semafor kasa
    kasa_zajeta = 1;        // ustawiamy flagę zajęcia kasy
}

void zwolnij_kase()
{
    sem_v(kasa_semafor, 1); // zwalniamy kasę
    kasa_zajeta = 0;        // ustawiamy flagę na 0
}