CC = gcc
CFLAGS = -Wall -pthread

# Lista plików źródłowych i obiektowych
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
EXEC = projekt_salon_fryzjerski

# Domyślna reguła
all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o $(EXEC)
