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

void wyslij_komunikat(int kolejka, struct komunikat *kom)
{
    int res = msgsnd(kolejka, (struct msgbuf *)kom, sizeof(struct komunikat) - sizeof(long), 0);
    if (res == -1)
    {
        if (errno == EINTR)
        {
            wyslij_komunikat(kolejka, kom);
        }
        else
        {
            error_exit("msgsnd");
        }
    }
}

void odbierz_komunikat(int kolejka, struct komunikat *kom, long odbiorca)
{
    int res = msgrcv(kolejka, (struct msgbuf *)kom, sizeof(struct komunikat) - sizeof(long), odbiorca, 0);
    if (res == -1)
    {
        if (errno == EINTR)
        {
            odbierz_komunikat(kolejka, kom, odbiorca);
        }
        else
        {
            error_exit("msgrcv");
        }
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
        error_exit("semget");
    }
    return id;
}

int sem_try_wait(int id, int n)
{
    struct sembuf sem_buff = {.sem_num = 0, .sem_op = -n, .sem_flg = IPC_NOWAIT};

    while (1)
    {
        int res = semop(id, &sem_buff, 1);
        if (res == 0)
        {
            return 0; // Udało się zmniejszyć semafor
        }

        if (errno == EAGAIN)
        {
            return 1; // Semafor był zajęty, ale nie blokujemy
        }
        else if (errno == EINTR)
        {
            continue; // Ponowienie operacji po przerwaniu
        }
        else
        {
            error_exit("sem op try wait");
        }
    }
}

int sem_getval(int id)
{
    int retval = semctl(id, 0, GETVAL);
    if (retval == -1)
    {
        error_exit("sem_getval");
    }
    return retval;
}

void sem_p(int id, int n)
{
    struct sembuf sem_buff;
    sem_buff.sem_num = 0;
    sem_buff.sem_op = -n;
    sem_buff.sem_flg = 0;
    int res = semop(id, &sem_buff, 1);
    if (res == -1)
    {
        if (errno == EINTR)
        {
            sem_p(id, n);
        }
        else
        {
            perror("semop p");
            exit(EXIT_FAILURE);
        }
    }
}