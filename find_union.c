/* Struktura find and union z operacjami dodania i usuwania elementu.
 * Każdy element od dodania do usunięcia ma swój unikalny identyfikator,
 * który jest nieujemną liczbą całkowitą.
 *
 * Zbiory elementów są przechowywane jako dynamiczne tablice,
 * dodatkowo dla każdego elementu jest przechowywany numer zbioru,
 * w którym się znajduje i jego pozycja w tablicy tego zbioru.
 *
 * Oprócz tego, aby zoptymalizować wykorzystanie pamięci i nie zostawiać
 * pustych miejsc w tablicach, trzymam dwa stosy: na zwolnione identyfikatory
 * i na zwolnione numery zbiorów.
 *
 * Struktura przechowuje również informacje o energii i zrównaniach energii
 * i wyłapuje odpowiednie błędy z tym związane.
 */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include "find_union.h"

#define CREATE(array, size) do { \
	array = malloc(size); \
	if (array == NULL) _Exit(1); \
} while (0)

#define EXTEND(array, size) do { \
	array = realloc(array, size); \
	if (array == NULL) _Exit(1); \
} while (0)

typedef struct {
	int32_t* identifiers;
	size_t count, size;
	uint64_t energy;
} IdentifierList;

// tablica zbiorów elementów
static size_t lists_count = 0, lists_size = 1;
static IdentifierList* lists;

// tablice przechowujące numer zbioru
// i pozycję w zbiorze każdego elementu
static size_t id_count = 0, id_size = 1;
static int32_t* which_list;
static int32_t* position;

// zwolnione numery zbiorów
static size_t free_list_count = 0, free_list_size = 1;
static int32_t* free_lists;

// zwolnione identyfikatory
static size_t free_id_count = 0, free_id_size = 1;
static int32_t* free_ids;

// inicjalizuje odpowiednie tablice
void find_union_initialize(void) {
	CREATE(free_lists, sizeof(int32_t) * free_list_size);
	CREATE(free_ids, sizeof(int32_t) * free_id_size);
	CREATE(which_list, sizeof(int32_t) * id_size);
	CREATE(position, sizeof(int32_t) * id_size);
	CREATE(lists, sizeof(IdentifierList) * lists_size);
}

// Zwalnia całą pamięć zajmowaną przez strukturę.
void find_union_clear(void) {
	free(which_list);
	free(position);
	free(free_lists);
	free(free_ids);
	for (size_t i = 0; i < lists_count; i++) {
		free(lists[i].identifiers);
	}
	free(lists);
}

/* Tworzy nowy element i zwraca jego identyfikator.
 * Nie przypisuje jeszcze tego elementu do żadnego zbioru.
 */
int32_t get_identifier(void) {
	int32_t id;
	
	// Najpierw staramy się sprawdzić, czy jakiś identyfikator nie jest wolny.
	if (free_id_count > 0) {
		free_id_count--;
		id = free_ids[free_id_count];
	}
	else {
		if (id_count == id_size) {
			id_size *= 2;
			EXTEND(which_list, sizeof(int32_t) * id_size);
			EXTEND(position, sizeof(int32_t) * id_size);
		}
		
		id = id_count++;
	}
	
	// Nie przypisujemy jeszcze elementu do żadnego zbioru.
	which_list[id] = -1;
	position[id] = 0;
	
	return id;
}

// Usuwa zbiór o danym numerze.
// Numer zbioru zostaje dodany do stosu wolnych numerów zbiorów.
void remove_list(int32_t list_id) {
	free(lists[list_id].identifiers);
	lists[list_id].identifiers = NULL;
			
	if (free_list_count == free_list_size) {
		free_list_size *= 2;
		EXTEND(free_lists, sizeof(int32_t) * free_list_size);
	}
	
	free_lists[free_list_count++] = list_id;
}

/* Usuwa element o danym identyfikatorze.
 * Jeżeli oznacza to skasowanie wszystkich elementów w zbiorze,
 * to sam zbiór też zostaje usunięty.
 * Identyfikator elementu zostaje dodany do stosu wolnych identyfikatorów.
 */
void remove_identifier(int32_t id) {
	int32_t my_list = which_list[id];
	if (my_list != -1) {
		int last = lists[my_list].count - 1;
		
		if (position[id] < last) {
			position[lists[my_list].identifiers[last]] = position[id];
			lists[my_list].identifiers[position[id]] = lists[my_list].identifiers[last];
		}
		
		lists[my_list].count--;
		
		if (lists[my_list].count == 0) remove_list(my_list);
	}
	
	if (free_id_count == free_id_size) {
		free_id_size *= 2;
		EXTEND(free_ids, sizeof(int32_t) * free_id_size);
	}
	
	free_ids[free_id_count++] = id;
}

// Zwraca energię elementu o zadanym identyfikatorze.
uint64_t get_energy(int32_t id) {
	int32_t my_list = which_list[id];
	return lists[my_list].energy;
}

/* Ustawia energię elementu o zadanym identyfikatorze.
 * Jeżeli element nie miał dotąd przypisanego żadnego zbioru,
 * zostaje utworzony dla niego nowy jednoelementowy zbiór.
 */
void set_energy(int32_t id, uint64_t energy) {
	int32_t my_list = which_list[id];
	
	if (my_list == -1) {
		if (free_list_count > 0) {
			my_list = free_lists[--free_list_count];
		}
		else {
			if (lists_count == lists_size) {
				lists_size *= 2;
				EXTEND(lists, sizeof(IdentifierList) * lists_size);
			}
			
			my_list = lists_count++;
		}
		
		lists[my_list].count = 1;
		lists[my_list].size = 1;
		CREATE(lists[my_list].identifiers, sizeof(int32_t) * lists[my_list].size);
		
		lists[my_list].identifiers[0] = id;
		
		position[id] = 0;
		which_list[id] = my_list;
	}
	
	lists[my_list].energy = energy;
}

/* Funkcja dodaje element id do listy o numerze list_id.
 * Nie usuwa informacji o elemencie w poprzedniej liście.
 */
void add_to_list(int32_t list_id, int32_t id) {
	which_list[id] = list_id;
	IdentifierList* list = &lists[list_id];
	position[id] = list->count;
	
	if (list->count == list->size) {
		list->size *= 2;
		EXTEND(list->identifiers, sizeof(int32_t) * list->size);
	}
	
	list->identifiers[list->count++] = id;
}

void swap(int32_t* a, int32_t* b) {
	int32_t tmp = *a;
	*a = *b;
	*b = tmp;
}

/* Łączy dwa zbiory podane jako numery identyfikatorów pewnych elementów
 * do nich należących. Zakłada, że element id1 ma już przydzielony zbiór
 * (ale id2 może nie mieć - wtedy przyłącza go do zbioru, do którego
 * należy id1).
 * 
 * Funkcja działa na zasadzie przepisywania elementów z mniejszego zbioru
 * do większego, co pozwala zachować dobrą złożoność obliczeniową.
 */
void set_equal(int32_t id1, int32_t id2) {
	if (which_list[id2] == -1) {
		add_to_list(which_list[id1], id2);
	}
	else {
		if (which_list[id1] == which_list[id2]) return;
		
		int32_t list1 = which_list[id1], list2 = which_list[id2];
		if (lists[list1].count < lists[list2].count) {
			swap(&id1, &id2);
			swap(&list1, &list2);
		}
		
		// Liczenie średniej arytmetycznej w taki sposób zapobiega
		// przekręceniu zmiennej przy przekroczeniu zakresu.
		int32_t both_odd = 0;
		if (lists[list1].energy % 2 == 1 && lists[list2].energy % 2 == 1) both_odd = 1;
		lists[list1].energy = lists[list1].energy / 2 + lists[list2].energy / 2 + both_odd;
		
		for (size_t i = 0; i < lists[list2].count; i++) {
			add_to_list(list1, lists[list2].identifiers[i]);
		}
		
		remove_list(list2);
	}
}
