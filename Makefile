CC = gcc
CFLAGS = -Wall -pthread

# Zaktualizowana lista źródeł - nie ma już folderu 'kasa'
SRC = $(wildcard *.c) 
OBJ = $(SRC:.c=.o)
EXEC = projekt_salon_fryzjerski

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(EXEC)
