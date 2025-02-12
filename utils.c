#include "common.h"

int utworz_kolejke(key_t klucz)
{
    int kolejka;
    if ((kolejka = msgget(klucz, IPC_CREAT | 0600)) == -1)
    {
        error_exit("Nie udalo sie stworzyc kolejki komunikatów");
    }
    return kolejka;
}

void usun_kolejke(int kolejka)
{
    if (msgctl(kolejka, IPC_RMID, NULL) == -1)
    {
        error_exit("Nie udalo sie usunac kolejki komunikatów");
    }
}

int utworz_pamiec_dzielona(key_t klucz)
{
    int shm_id = shmget(klucz, sizeof(int), 0600 | IPC_CREAT);
    if (shm_id == -1)
    {
        error_exit("shmget");
    }
    return shm_id;
}

int dolacz_pamiec_dzielona(int shm_id)
{
    int *ptr = shmat(shm_id, 0, 0);
    if (*ptr == -1)
    {
        error_exit("shmat");
    }
    return ptr;
}

int utworz_semafor(key_t klucz)
{
    int id = semget(klucz, 1, 0600 | IPC_CREAT);
    if (id == -1)
    {
        exit_error("semget");
    }
    return id;
}