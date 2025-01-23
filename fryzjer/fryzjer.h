#define FRYZJER_H

void przyjmij_zaplate(int klient_id, int kwota, int zaplacona_kwota); //przenioslem w to miejsce obliczenie reszty
//ma to na celu zapobiec problem gdy ostatni klient bedzie tym, ktoremu nie jestesmy w stanie wydac reszty
//samo wydanie reszty jest wykonywane po zakonczeniu strzyzenia, ale wczesniej rezerwujemy kwote

void wydaj_reszte(int klient_id);
