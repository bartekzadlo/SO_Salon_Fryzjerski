CC=gcc
CFLAGS=-Wall -pthread

SRC=$(wildcard *.c)
OBJ=$(SRC:.c=.o)
EXEC=projekt_salon_fryzjerski

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) -c $<

clean:
	rm -f $(OBJ) $(EXEC)
