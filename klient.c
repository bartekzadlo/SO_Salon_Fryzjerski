#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include "common.h"

/* Funkcja klienta */

void *client_thread(void *arg)
{
    int id = *((int *)arg);
    free(arg);