CC = gcc
CFLAGS = -pthread -Wall
OBJ = kasa.o kolejka_klientow.o fotele.o fryzjer.o main.o

all: main

main: $(OBJ)
	$(CC) $(OBJ) -o main

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o main
