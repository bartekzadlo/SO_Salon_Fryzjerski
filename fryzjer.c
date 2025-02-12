#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include "common.h"

long id;
int kolejka;
key_t klucz;
int fotele_semafor;
int shm_id;
int pamiec;
long id_obslugiwany_klient;
volatile sig_atomic_t salon_open;
volatile sig_atomic_t close_all_clients;
volatile sig_atomic_t fryzjer_komunikat_poczekalnia = 0;
volatile sig_atomic_t fotel = 0;
volatile sig_atomic_t czeka_na_zaplate = 0;
volatile sig_atomic_t odebiera_zaplate = 0;

int main()
{
    long id = getpid();        // Pobranie identyfikatora fryzjera
    char log_buffer[MSG_SIZE]; // Bufor do przechowywania komunikatów logujących
    struct komunikat kom;

    klucz = ftok(".", "M");
    kolejka = utworz_kolejke(klucz);
    klucz = ftok(".", "F");
    fotele_semafor = utworz_semafor(klucz);
    klucz = ftok(".", "S");
    shm_id = utworz_pamiec_dzielona(klucz);
    pamiec = dolacz_pamiec_dzielona(shm_id);

    while (1) // Główna pętla pracy fryzjera
    {
        /* Sprawdzenie, czy dla tego fryzjera wysłano sygnał zakończenia pracy.
         * Jeśli flaga barber_stop dla danego fryzjera jest ustawiona, kończymy pracę.
         */
        if (barber_stop[id])
        {
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem sygnał 1, kończę pracę.", id);
            send_message(log_buffer); // Wysłanie komunikatu do loggera
            break;
        }

        // Czeka na klienta
        if (fryzjer_komunikat_poczekalnia != 1)
        {
            odbierz_komunikat(kolejka, &kom, 1);
            fryzjer_komunikat_poczekalnia = 1;
        }

        id_obslugiwany_klient = kom.nadawca;
        if (!fotel)
        {
            sem_p(fotele_semafor, 1); // Zajmujemy fotel
            fotel = 1;
        }
        snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: rozpoczynam obsługę klienta %d zajmując fotel", id, id_obslugiwany_klient);
        send_message(log_buffer);

        kom.mtype = id_obslugiwany_klient;
        kom.nadawca = id;
        if (czeka_na_zaplate != 1)
        {
            wyslij_komunikat(kolejka, &kom);
            czeka_na_zaplate = 1;
        }

        if (odbiera_zaplate != 1)
        {
            // printf("\033[0;34m[FRYZJER %ld]: odbieram komunikat zaplata\033[0m\n", ja);
            odbierz_komunikat(kolejka, &kom, ja);
            // printf("\033[0;34m[FRYZJER %ld]: odebralem komunikat zaplata\033[0m\n", ja);
            odebiera_zaplate = 1;
        }

        pthread_mutex_lock(&kasa.mutex_kasa);
        if (klient->payment == 30)
        {
            // Klient płaci 20 i 10 zł – zwiększamy licznik banknotów 20 i 10 zł
            kasa.banknot_10++;
            kasa.banknot_20++;
            pthread_cond_signal(&kasa.uzupelnienie);
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem 20 i 10 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
        }
        else if (klient->payment == 50)
        {
            kasa.banknot_50++; // Klient płaci 50 zł – zwiększamy licznik banknotów 50 zł
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: otrzymałem 50 zł. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
        }
        pthread_mutex_unlock(&kasa.mutex_kasa);

        /* Realizacja usługi – symulacja czasu strzyżenia.
         * Losujemy czas trwania usługi (od 1 do 3 sekund) i "usypiamy" wątek.
         */
        int service_time = rand() % 3 + 1;
        int elapsed = 0;
        // Pętla symulująca czas trwania strzyżenia klienta
        while (elapsed < service_time)
        {
            if (close_all_clients) // Jeśli salon jest zamknięty lub mamy sygnał zakończenia obsługi, przerywamy pętlę
                break;
            sleep(0);  // Symulujemy upływ czasu
            elapsed++; // Zwiększamy licznik upływającego czasu
        }
        if (close_all_clients) // Sprawdzamy, czy salon został zamknięty przed zakończeniem strzyżenia
        {
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: przerwałem obsługę klienta %d z powodu zamknięcia salonu.",
                     id, klient->id); // Logujemy przerwanie usługi z powodu zamknięcia salonu
            send_message(log_buffer); // Wysyłamy komunikat
        }
        else
        {
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: zakończyłem strzyżenie klienta %d (czas usługi: %d s).",
                     id, klient->id, service_time); // Logujemy zakończenie usługi strzyżenia i czas trwania
            send_message(log_buffer);               // Wysyłamy komunikat
        }

        /* Zwalnianie fotela – klient opuszcza fotel po zakończeniu usługi.
         * Uwalniamy semafor, co umożliwia zajęcie fotela kolejnemu klientowi.
         */
        sem_post(&fotele_semafor);

        /* Wydawanie reszty klientowi, jeśli zapłacił 50 zł.
         * Reszta wynosi 30 zł, składająca się z banknotów 10 zł i 20 zł.
         */
        if (klient->payment == 50 && !close_all_clients)
        {
            pthread_mutex_lock(&kasa.mutex_kasa);
            while ((kasa.banknot_10 < 1 || kasa.banknot_20 < 1) && !close_all_clients) // Czekamy, aż w kasie będą dostępne wymagane banknoty (10 zł oraz 20 zł)
            {
                pthread_cond_wait(&kasa.uzupelnienie, &kasa.mutex_kasa);
            }
            if (close_all_clients) // Jeśli salon się zamyka, przerywamy operację wydawania reszty
            {
                pthread_mutex_unlock(&kasa.mutex_kasa);
                snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: salon zamknięty, nie wydaję reszty klientowi %d.", id, klient->id);
                send_message(log_buffer);
                sem_post(&klient->served); // Sygnalizujemy klientowi, że został obsłużony (choć reszta nie została wydana)
                continue;
            }
            // Wydajemy resztę – zmniejszamy liczbę banknotów w kasie

            kasa.banknot_20--;
            snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: wydaję resztę 20 zł klientowi %d. Kasa: 10zł=%d, 20zł=%d, 50zł=%d.",
                     id, klient->id, kasa.banknot_10, kasa.banknot_20, kasa.banknot_50);
            send_message(log_buffer);
            pthread_mutex_unlock(&kasa.mutex_kasa);
        }

        /* Powiadomienie klienta o zakończeniu obsługi.
         * Funkcja sem_post sygnalizuje, że klient może kontynuować działanie (np. odebrać informację).
         */
        sem_post(&klient->served);
    }

    // Po wyjściu z pętli pracy fryzjera, logujemy informację o zakończeniu pracy
    snprintf(log_buffer, MSG_SIZE, "Fryzjer %d: wychodzę z pracy.", id);
    send_message(log_buffer);
    return NULL;
}