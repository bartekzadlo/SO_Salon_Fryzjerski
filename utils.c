#include "common.h"

int stworz_kolejke_komunikatow(key_t msg_qkey)
{
    int msg_qid;
    msg_qid = msgget(msg_qkey, IPC_CREAT | 0600);
    if (msg_qid == -1)
    {
        error_exit("Błąd przy tworzeniu kolejki komunikatów");
    }
    return msg_qid;
}

void usun_kolejke_komunikatow(int msg_qid)
{
    if (msgctl(msg_qid, IPC_RMID, NULL) == -1)
    {
        error_exit("Nie udało się usunąć kolejki komunikatów");
    }
}

void wyslij_komunikat_do_kolejki(int msg_qid, struct Message *msg)
{
    int status = msgsnd(msg_qid, (struct msgbuf *)msg, sizeof(struct Message) - sizeof(long), 0);
    if (status == -1)
    {
        if (errno == EINTR)
        {
            wyslij_komunikat_do_kolejki(msg_qid, msg); // ponowienie próby
        }
        else
        {
            error_exit("Błąd wysyłania komunikatu");
        }
    }
}

void pobierz_komunikat_z_kolejki(int msg_qid, struct Message *msg, long odbiorca_id)
{
    int status = msgrcv(msg_qid, (struct msgbuf *)msg, sizeof(struct Message) - sizeof(long), odbiorca_id, 0);
    if (status == -1)
    {
        if (errno == EINTR)
        {
            pobierz_komunikat_z_kolejki(msg_qid, msg, odbiorca_id); // ponowienie próby
        }
        else
        {
            error_exit("Błąd odbierania komunikatu");
        }
    }
}

int stworz_pamiec_dzielona(key_t shm_key)
{
    int shm_id = shmget(shm_key, sizeof(int), 0600 | IPC_CREAT);
    if (shm_id == -1)
    {
        error_exit("Błąd tworzenia pamięci dzielonej");
    }
    return shm_id;
}

int *dolacz_do_pamieci_dzielonej(int shm_id)
{
    int *adres = shmat(shm_id, 0, 0);
    if (adres == (int *)-1)
    {
        error_exit("Błąd dołączania do pamięci dzielonej");
    }
    return adres;
}

void odlacz_pamiec_dzielona(int *shm_ptr)
{
    int result = shmdt(shm_ptr);
    if (result == -1)
    {
        error_exit("shmdt");
    }
}

void usun_pamiec_dzielona(int shm_id)
{
    int result = shmctl(shm_id, IPC_RMID, 0);
    if (result == -1)
    {
        error_exit("shmctl IPC_RMID");
    }
}

int stworz_semafor(key_t sem_key)
{
    int sem_id = semget(sem_key, 1, 0600 | IPC_CREAT);
    if (sem_id == -1)
    {
        error_exit("semget failed");
    }
    return sem_id;
}

void usun_semafor(int sem_id)
{
    int result = semctl(sem_id, 0, IPC_RMID);
    if (result == -1)
    {
        error_exit("semctl IPC_RMID failed");
    }
}

void sem_setval(int sem_id, int value)
{
    int res = semctl(sem_id, 0, SETVAL, value);
    if (res == -1)
    {
        perror("semctl SETVAL");
        exit(EXIT_FAILURE);
    }
}

int sem_try_wait(int sem_id, int n)
{
    struct sembuf sem_buff = {.sem_num = 0, .sem_op = -n, .sem_flg = IPC_NOWAIT};

    while (1)
    {
        int result = semop(sem_id, &sem_buff, 1);
        if (result == 0)
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
            error_exit("semop - nie udało się wykonać operacji try wait");
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

// zmniejszenie wartości semafora
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
            error_exit("semop p");
        }
    }
}

// zwiększenie wartości semafora
void sem_v(int id, int n)
{
    struct sembuf sem_buff;
    sem_buff.sem_num = 0;
    sem_buff.sem_op = n;
    sem_buff.sem_flg = 0;
    int res = semop(id, &sem_buff, 1);
    if (res == -1)
    {
        if (errno == EINTR)
        {
            sem_v(id, n);
        }
        else
        {
            perror("semop v");
            exit(EXIT_FAILURE);
        }
    }
}

void error_exit(const char *msg)
{
    fprintf(stderr, "%s", RED); // Kolorowanie tekstu
    perror(msg);
    fprintf(stderr, "%s", RESET); // Przywrócenie domyślnego koloru
    exit(EXIT_FAILURE);
}

void set_process_limit()
{
    struct rlimit rl;

    if (getrlimit(RLIMIT_NPROC, &rl) != 0)
    {
        error_exit("getrlimit");
    }

    if (rl.rlim_cur < F + P)
    {
        error_exit("Przekroczono możliwą liczbę procesów!");
    }
}