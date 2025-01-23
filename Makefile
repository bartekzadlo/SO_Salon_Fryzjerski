CC = gcc
CFLAGS = -I./fryzjer -I./klient -I./komunikaty -I./kierownik

SRC = main.c fryzjer/fryzjer.c klient/klient.c komunikaty/komunikaty.c kierownik/kierownik.c

OBJ = $(SRC:.c=.o)

EXEC = projekt_fryzjer

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
