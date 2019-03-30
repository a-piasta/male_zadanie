/* Parser wejścia. Czyta po jednej linii wejścia, a następnie przekazuje
 * ją jako odpowiedni typ struktury (zdefiniowanej w pliku parser.h)
 * do dalszego przetworzenia przez program.
 */

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>

#include "parser.h"

// Pomocnicze makra.

#define CALL_ERROR(type) return make_command_s((type), NULL, NULL)

#define CHECK_ENDL(type) \
do { \
	bool is_endl = read_until_endl(); \
	if (is_endl) CALL_ERROR(type); \
	else CALL_ERROR(EOF_ERROR); \
} while(0)

#define CHECK_HISTORY_ERROR(arg) \
do { \
	if ((arg) == NULL) _Exit(1); \
	if ((arg)[0] == 'F') { \
		free(arg); \
		CALL_ERROR(EOF_ERROR); \
	} \
	if ((arg)[0] == 'E') { \
		free(arg); \
		CHECK_ENDL(ERROR); \
	} \
} while(0)

#define COMMANDS_COUNT 5

static const char* commands[COMMANDS_COUNT] = {
	"DECLARE",
	"REMOVE",
	"VALID",
	"EQUAL",
	"ENERGY"
};

static const size_t command_length[COMMANDS_COUNT] = {7, 6, 5, 5, 6};

/* Konstruktor dla polecenia (typ Command przekazywany dalej do obsłużenia w programie).
 * Jeżeli polecenie potrzebuje mniej niż dwóch argumentów,
 * pozostałe należy ustawić jako NULL.
 * Tyczy się to również poleceń oznaczających błąd, puste polecenie
 * lub zakończenie programu (ERROR, EOF_ERROR, EOF_CORRECT, NONE)
 */
static Command make_command_s(CommandType command_name, char* arg1, char* arg2) {
	Command tmp;
	tmp.name = command_name;
	tmp.arg1 = arg1;
	tmp.arg2_s = arg2;
	return tmp;
}

// Konstruktor używany tylko dla dwuparametrowego polecenia ENERGY.
static Command make_command_ll(CommandType command_name, char* arg1, uint64_t arg2) {
	Command tmp;
	tmp.name = command_name;
	tmp.arg1 = arg1;
	tmp.arg2_ll = arg2;
	return tmp;
}

/* Funkcja czytająca znaki do końca linii i zwracająca,
 * czy linia kończy się znakiem \n (czy też końcem pliku).
 */
static bool read_until_endl(void) {
	int ch;
	do {
		ch = getchar();
		if (ch == '\n') return true;
	} while (ch != EOF);
	return false;
}

/* Funkcja czytająca z wejścia jedną komendę i zwracająca jej typ.
 * W przypadku błędów (np. zły rodzaj komendy, brak spacji po nazwie itd.)
 * funkcja zwraca ERROR (lub EOF_ERROR, jeżeli nastąpi koniec pliku)
 * jako typ komendy.
 */
static CommandType read_command_name(void) {
	size_t current_command = 0, pos = 0;
	while (true) {
		int ch = getchar();
		
		if (ch == EOF) {
			if (pos == 0) return EOF_CORRECT;
			else return EOF_ERROR;
		}
		
		if (pos == 0 && ch == '\n') return NONE;
		if (pos == 0 && ch == '#') {
			bool is_endl = read_until_endl();
			if (is_endl) return NONE;
			return EOF_ERROR;
		}

		if (ch == ' ') {
			if (pos == command_length[current_command]) {
				// konwersja int -> CommandType
				return current_command + 4;
			}
			else {
				return ERROR;
			}
		}
		
		while (current_command < COMMANDS_COUNT && (pos >= command_length[current_command]
				|| ch != commands[current_command][pos])) current_command++;
		
		if (current_command == COMMANDS_COUNT) {
			if (ch == '\n') ungetc(ch, stdin);
			return ERROR;
		}
		
		pos++;
	}
}

/* Funkcja czyta historię - ciąg składający się ze znaków '0' .. '3'
 * i zwraca ją jako łańcuch znaków (zakończony znakiem \0).
 * Argumentem funkcji jest oczekiwany znak za końcem historii.
 * Może to być ' ' lub '\n'. Jeżeli natomiast oczekiwany jest dowolny
 * z tych znaków, argumentem powinien być znak o kodzie 0. W tej sytuacji
 * znak ten zostanie również wstawiony na koniec historii.
 * 
 * Sygnalizowane błędy;
 * 	- zły typ znaku -> zostaje zwrócona historia zaczynająca się od 'E';
 * 		napotkany wtedy błędny znak zostaje zwrócony do strumienia,
 * 	- pusta historia -> tak samo, jak przy złym typie znaku
 *  - napotkany koniec pliku -> zostaje zwrócona historia zaczynająca się od 'F'
 */
static char* read_history(char end) {
	char end1, end2;

	if (end == 0) end1 = ' ', end2 = '\n';
	else end1 = end2 = end;

	size_t buff_size = 4, pos = 0;
	char* buff = malloc(buff_size);
	if (buff == NULL) _Exit(1);

	int ch;

	do {
		ch = getchar();
		if (ch == EOF) {
			buff[0] = 'F';
			return buff;
		}

		if ((ch < '0' || ch > '3') && ch != end1 && ch != end2) {
			buff[0] = 'E';
			ungetc(ch, stdin);
			return buff;
		}

		if (pos + 1 == buff_size) {
			buff_size *= 2;
			buff = realloc(buff, buff_size);
			if (buff == NULL) _Exit(1);
		}

		buff[pos++] = ch;
	} while (ch != end1 && ch != end2);
	
	if (pos == 1) {
		ungetc(buff[0], stdin);
		buff[0] = 'E';
		return buff;
	}
	
	if (end != 0) buff[pos-1] = 0;
	else buff[pos] = 0;
	
	return buff;
}

/* Funkcja czyta z wejścia liczbę całkowitą z zakresu [1, 2^64 - 1].
 * Funkcja oczekuje, że po ciągu cyfr nastąpi znak '\n'.
 * 
 * Ewentualne błędy są sygnalizowane poprzez zwrócenie wartości 0
 * (i ewentualne zwrócenie '\n' z powrotem do strumienia, jeżeli
 * zostało napotkane w niewłaściwym momencie).
 * 
 * Jedynymi dopuszczalnymi odstępstwami od standardowego zapisu
 * dziesiętnego są zera wiodące.
 */
static uint64_t read_ull(void) {
	
	// Wartość równa 2^64 / 10. Jest to największa liczba, jaka może
	// być przechowywana w zmiennej value przed wczytaniem ostatniej cyfry.
	const uint64_t MAX_VALUE = 1844674407370955161ULL;
	uint64_t value = 0;
	
	// Jeżeli value == MAX_VALUE, to następna cyfra może być równa
	// co najwyżej last_digit. W przeciwnym przypadku może być dowolna.
	const int last_digit = '5';
	int ch;
	do {
		ch = getchar();
		if (!isdigit(ch)) {
			if (ch == '\n') {
				if (value == 0) ungetc(ch, stdin);
				return value;
			}
			return 0;
		}

		if (value < MAX_VALUE || (value == MAX_VALUE && ch <= last_digit)) {
			value = 10 * value + (ch - '0');
		}
		else {
			return 0;
		}
	} while (true);
}

/* Funkcja czytająca jedną linię wejścia i zwracająca ją jako polecenie
 * do dalszego przetworzenia.
 * 
 * Możliwe błędy są sygnalizowane jako odpowiedni typ polecenia
 * w zwracanej strukturze.
 */
Command read_line(void) {
	CommandType type = read_command_name();
	
	if (type == EOF_ERROR || type == EOF_CORRECT) CALL_ERROR(type);
	if (type == ERROR) CHECK_ENDL(type);
	if (type == NONE) CALL_ERROR(NONE);

	char* arg1;
	
	// Polecenie EQUAL.
	if (type == EQUAL) {
		arg1 = read_history(' ');
		CHECK_HISTORY_ERROR(arg1);

		char* arg2 = read_history('\n');
		if (arg2 == NULL || arg2[0] == 'E' || arg2[0] == 'F') free(arg1);
		CHECK_HISTORY_ERROR(arg2);

		return make_command_s(type, arg1, arg2);
	}
	/* Ten przypadek obejmuje zarówno jedno-, jak i dwuparametrowe
	 * polecenie ENERGY. Zostają one rozróżnione później, na podstawie
	 * rodzaju białego znaku po wczytaniu pierwszego argumentu.
	 */
	else if (type == ENERGY_MOD) {
		arg1 = read_history(0);
		CHECK_HISTORY_ERROR(arg1);

		int len = strlen(arg1);

		if (arg1[len-1] == ' ') {
			uint64_t arg2 = read_ull();
			
			if (arg2 == 0) {
				free(arg1);
				CHECK_ENDL(ERROR);
			}

			arg1[len-1] = 0;
			return make_command_ll(ENERGY_MOD, arg1, arg2);
		}
		else {
			arg1[len-1] = 0;
			return make_command_s(ENERGY_CHK, arg1, NULL);
		}	
	}
	// Wszystkie pozostałe rodzaje prawidłowych poleceń.
	else {
		arg1 = read_history('\n');
		CHECK_HISTORY_ERROR(arg1);

		return make_command_s(type, arg1, NULL);
	}
}
