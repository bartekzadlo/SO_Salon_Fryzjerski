CC = gcc
CFLAGS = -pthread

# Pliki źródłowe
SRCS = utils.c
KIEROWNIK_SRCS = kierownik.c $(SRCS)
KLIENT_SRCS = klient.c $(SRCS)
FRYZJER_SRCS = fryzjer.c $(SRCS)

# Pliki nagłówkowe
COMMON_H = common.h

# Pliki wynikowe
TARGETS = kierownik klient fryzjer

all: $(TARGETS)

kierownik: $(KIEROWNIK_SRCS)
	$(CC) $^ -o $@ $(CFLAGS)

klient: $(KLIENT_SRCS)
	$(CC) $^ -o $@ $(CFLAGS)

fryzjer: $(FRYZJER_SRCS)
	$(CC) $^ -o $@ $(CFLAGS)

# Ustal zależności dla każdego programu
kierownik: $(COMMON_H)
klient: $(COMMON_H)
fryzjer: $(COMMON_H)

clean:
	rm -f $(TARGETS)
