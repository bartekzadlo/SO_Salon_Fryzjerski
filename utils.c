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