CC = gcc
CFLAGS = -I./kasa -I./klient -I./fryzjer -I./komunikaty -I./kierownik

SRC = main.c kasa/kasa.c klient/klient.c fryzjer/fryzjer.c komunikaty/komunikaty.c kierownik/kierownik.c

OBJ = $(SRC:.c=.o)

EXEC = projekt_fryzjer

$(EXEC): $(OBJ)
	$(CC) $(OBJ) -o $(EXEC)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(EXEC)
