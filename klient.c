#include "common.h"

long id_klient;
int msg_qid;
int poczekalnia_semafor;
long id_fryzjera;

volatile sig_atomic_t sygnal_klient = 0;                // obsługa przerwań w przypadku sygnału SIGINT
volatile sig_atomic_t w_poczekalni = 0;                 // czy klient znajduje się w poczekaln
volatile sig_atomic_t klient_komunikat_poczekalnia = 0; // czy klient wysłał już komunikat o swojej obecności w poczekalni
volatile sig_atomic_t pobranie_z_poczekalni = 0;        // czy klient został już pobrany z poczekalni przez fryzjera.
volatile sig_atomic_t zaplacone = 0;                    // czy klient zapłacił za usługę fryzjerską
volatile sig_atomic_t zakonczona_usluga = 0;            // czy klient dostał komunikat o zakończonej obsłudze

int main()
{
    // Inicjalizacja generatora liczb losowych na podstawie czasu systemowego oraz unikalnego identyfikatora procesu (PID).
    srand(time(NULL) + getpid());

    // Rejestracja funkcji obsługi sygnału SIGINT (przerwanie) w celu wywołania funkcji sig_handler_klient przy otrzymaniu sygnału.
    if (signal(SIGINT, sig_handler_klient) == SIG_ERR)
    {
        error_exit("Błąd obsługi sygnalu zabijającego klientów");
    }
    // Deklaracja zmiennej typu Message, która będzie używana do przechowywania wiadomości wysyłanych i odbieranych w komunikacji między procesami
    struct Message msg;
    // Zmienna do przechowywania wartości płatności za usługę fryzjerską, która będzie losowana.
    int platnosc;

    // Deklaracja kluczy
    key_t msg_qkey;
    key_t sem_key_p;

    // Tworzymy klucze
    msg_qkey = ftok(".", 'M');
    sem_key_p = ftok(".", 'P');

    // Tworzymy struktury
    msg_qid = stworz_kolejke_komunikatow(msg_qkey);
    poczekalnia_semafor = stworz_semafor(sem_key_p);

    // Pętla działania klienta
    while (1)
    {
        id_klient = getpid(); // Przypisujemy id klientowi
        if (sygnal_klient)    // Jeśli ustawiona flaga sygnału dla klienta to nie przychodzimy do poczekalni
        {
            break;
        }
        printf(BLUE "Klient %ld: Pracuję.\n" RESET, id_klient);                    // Rozpoczynamy od pracy klienta
        int earning_time = rand() % 10 + 3;                                        // Losowy czas klienta (pomiędzy 3 a 13 sekund)
        sleep(0);                                                                  // domyslnie earning_time
        printf(BLUE "Klient %ld: Próbuję wejść do poczekalni\n" RESET, id_klient); // Klient próbuje wejść do poczekalni

        if (!w_poczekalni) // Sprawdzamy, czy klient nie jest już w poczekalni
        {
            if (sem_try_wait(poczekalnia_semafor, 1) == 0) // Sprawdzamy czy jest wolne miejsce w poczekalni - zwraca 0 = jest wolne miejsce
            {
                w_poczekalni = 1; // Zmieniamy flagę - klient znajduje się w poczekalni
                printf(BLUE "Klient %ld: wchodzę do poczekalni. Liczba wolnych miejsc: %d.\n" RESET, id_klient, sem_getval(poczekalnia_semafor));
                // Klient jest w poczekalni, wyświetlamy ilość pozostałych miejsc
                msg.message_type = 1; // Przygotowujemy komunikat o tym, że klient jest w poczekalni
                msg.nadawca = id_klient;
                wyslij_komunikat_do_kolejki(msg_qid, &msg); // Wysyłamy komunikat do fryzjera o tym, że w poczekalni pojawił się klient
                klient_komunikat_poczekalnia = 1;           // flaga potrzebna nam przy zwalnianiu zasobów - potwierdza ona, że klient jest w trakcie obsługi
                printf(BLUE "Klient %ld: wyslalem komunikat, że jestem w poczekalni.\n" RESET, id_klient);

                if (pobranie_z_poczekalni != 1) // jeśli klient nie został jeszcze pobrany z poczekalni przez żadnego fryzjera
                {
                    printf(BLUE "Klient %ld: zostałem pobrany przez fryzjera.\n" RESET, id_klient);
                    pobierz_komunikat_z_kolejki(msg_qid, &msg, id_klient); // pobieramy komunikat od fryzjera
                    pobranie_z_poczekalni = 1;                             // zaznaczamy flagę o pobraniu z poczekalni
                }

                id_fryzjera = msg.nadawca; // przypisujemy klientowi fryzjera, który go obsługuje

                if (w_poczekalni) // jeśli klient jest w poczekalni
                {
                    printf(BLUE "Klient %ld: opuściłem poczekalnie\n" RESET, id_klient);
                    sem_v(poczekalnia_semafor, 1); // opuszcza poczekalnie (zamysł polega na opusczeniu poczekalni w celu zajęcia fotel - fotel zajmowany przez fryzjera)
                    w_poczekalni = 0;              // klient nie jest już w poczekalni
                }

                platnosc = (rand() % 2 == 0) ? 30 : 50; // losujemy płatnośc, płacimy dokładną kwote 30 złotych lub 50 złotych (nadwyżka 20 zł)

                // przygotowujemy komunikat zawierający kwotę płatności
                msg.message_type = id_fryzjera;
                msg.nadawca = id_klient;
                msg.platnosc = platnosc;

                if (zaplacone != 1) // jeśli klient jeszcze nie zapłacił
                {
                    printf(BLUE "Klient %ld: płacę %d zł.\n" RESET, id_klient, platnosc);
                    wyslij_komunikat_do_kolejki(msg_qid, &msg); // wysyłamy komunikat do fryzjera
                    printf(BLUE "Klient %ld: wysłałem komunikat\n" RESET, id_klient);
                    zaplacone = 1; // zaznaczamy flage o tym, że płatność została dokonana
                }

                if (zakonczona_usluga != 1) // jeśli usługa jeszcze nie została zakończona:
                {
                    pobierz_komunikat_z_kolejki(msg_qid, &msg, id_klient); // pobieramy z kolejki komunikat od fryzjera o tym, że reszta została wydana i usługa zakończona
                    printf(BLUE "Klient %ld: otrzymałem komunikat o zakończeniu obsługi.\n" RESET, id_klient);
                    zakonczona_usluga = 1; // zaznaczamy flage o zakończonej obsłudze
                }
                // resetujemy wszystkie flagi przed ponownym wejściem w pętle
                klient_komunikat_poczekalnia = 0;
                pobranie_z_poczekalni = 0;
                zaplacone = 0;
                zakonczona_usluga = 0;

                printf(BLUE "Klient %ld: zostałem obsłużony i opuszczam salon.\n" RESET, id_klient); // informujemy o tym, że wychodzimy z salonu
            }
            else // jeśli nie udaje nam się wejść do poczekalni
            {
                printf(BLUE "Klient %ld: poczekalnia jest pełna. Wracam do pracy.\n" RESET, id_klient); // informujemy, że się nie udało i wracamy do pracy (początek pętli)
            }
        }
        if (w_poczekalni) // zabezpieczenie na wypadek, gdyby z jakiegoś powodu klient był w poczekalni
        {
            sem_v(poczekalnia_semafor, 1);
        }
    }
    printf(BLUE "Klient %ld: kończę pracę\n" RESET, id_klient); // informacja o zakończeniu pracy klienta (koniec procesu)
}

void sig_handler_klient(int sig) // Funkcja obsługi sygnału SIGINT
{
    printf(RED "Klient %ld: Otrzymałem sygnał końca pracy.\n" RESET, id_klient); // informacja o otrzymaniu sygnału o końcu pracy

    if (klient_komunikat_poczekalnia == 1) // jeśli klient jest już po wysłaniu komunikatu do fryzjera
    {
        sygnal_klient = 1;

        // Sprawdzamy, która część procesu klienta nie została zakończona
        if (pobranie_z_poczekalni != 1)
        {
            pobranie_z_poczekalni = -1;
        }
        else if (zaplacone != 1)
        {
            zaplacone = -1;
        }
        else if (zakonczona_usluga != 1)
        {
            zakonczona_usluga = -1;
        }
    }
    else
    {
        // Jeśli klient jest w poczekalni, zwalniamy miejsce i kończymy pracę
        if (w_poczekalni)
        {
            printf(BLUE "Klient %ld: Zwalniam swoje miejsce w poczekalni.\n" RESET, id_klient);
            sem_v(poczekalnia_semafor, 1);
        }
        printf(BLUE "Klient %ld: kończę pracę\n" RESET, id_klient);
        exit(EXIT_SUCCESS);
    }
}