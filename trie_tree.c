/* Struktura przechowująca zbiór słów w postaci drzewa trie
 * i obsługująca komendy z wejścia (po przekazaniu ich w odpowiedniej formie
 * jako wywołania funkcji).
 *
 * Przekazywane argumenty do funkcji powinny być poprawne składniowo
 * (tzn. historia powinna składać się z odpowiednich znaków i kończyć \0,
 * a energia powinna być dodatnią liczbą z odpowiedniego przedziału).
 *
 * Moduł obsługuje natomiast takie błędy, jak próba przypisania energii
 * do historii, która nie jest dopuszczona.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie_tree.h"
#include "find_union.h"

#define ALPHABET_SIZE 4

#define CALL_ERROR do { \
	fprintf(stderr, "ERROR\n"); \
	return; \
} while (0)

/* Pojedynczy wierzchołek w drzewie.
 * Przechowuje informacje o energii dla historii takiej,
 * jaka jest ścieżka od korzenia drzewa do niego.
 *
 * son[] -- tablica wskaźników na synów: son[0] odpowiada
 * za historię przedłużoną o '0' itd.
 *
 * id -- identyfikator wierzchołka w find and union.
 * Jeżeli wierzchołek nie ma przypisanej energii,
 * to jego identyfikator jest równy -1.
 * W przeciwnym wypadku jest on pewną liczbą nieujemną.
 */
struct Node {
	struct Node* son[ALPHABET_SIZE];
	int32_t id;
};

typedef struct Node Node;

// korzeń drzewa
static Node root = { { NULL, NULL, NULL, NULL }, -1 };

/* Zwraca wierzchołek w drzewie na głębokości len,
 * idąc po krawędziach odpowiadających historii (argument history).
 * Jeżeli nie ma takiego wierzchołka w drzewie, zwraca NULL.
 * Zakłada, że len <= strlen(history).
 */
Node* find_node(const char* history, int len) {
	Node* current = &root;
	
	for (int i = 0; i < len; i++) {
		int state = history[i] - '0';
		if (current->son[state] == NULL) return NULL;
		current = current->son[state];
	}
	
	return current;
}

/* Usuwa odpowiedni wierzchołek w drzewie i rekurencyjnie całe jego poddrzewo.
 * Jeżeli historia odpowiadająca wierzchołkowi miała przypisaną energię,
 * to odpowiednie informacje w strukturze find and union
 * również zostają usunięte.
 */
void erase(Node* to_erase) {
	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (to_erase->son[i] != NULL) erase(to_erase->son[i]);
	}
	
	if (to_erase->id != -1) {
		remove_identifier(to_erase->id);
	}
	
	free(to_erase);
}

/* Usuwa drzewo trie i zwraca zajmowaną przez nie pamięć.
 * Dodatkowo, wywołuje funkcję czyszczącą find and union.
 * Funkcja ta jest wywoływana na koniec programu.
 */
void trie_tree_clear(void) {
	for (int i = 0; i < ALPHABET_SIZE; i++) {
		if (root.son[i] != NULL) erase(root.son[i]);
	}
	find_union_clear();
}

// Obsługuje polecenie DECLARE.
void declare(char* history) {
	int len = strlen(history);
	Node* current = &root;
	
	for (int i = 0; i < len; i++) {
		int state = history[i] - '0';
		if (current->son[state] == NULL) {
			current->son[state] = malloc(sizeof(Node));
			if (current->son[state] == NULL) _Exit(1);
			
			current->son[state]->id = -1;
			memset(current->son[state]->son, 0, sizeof(Node*) * ALPHABET_SIZE);
		}
		current = current->son[state];
	}
	
	free(history);
	puts("OK");
}

/* Obsługuje polecenie REMOVE.
 * Duża litera w nazwie funkcji, ponieważ mała pokrywałaby się z pewną funkcją z biblioteki stdio.h.
 */
void Remove(char* history) {
	int len = strlen(history);
	Node* parent_of_erased = find_node(history, len - 1);
	int last_state = history[len - 1] - '0';
	
	free(history);
	
	if (parent_of_erased != NULL && parent_of_erased->son[last_state] != NULL) {
		erase(parent_of_erased->son[last_state]);
		parent_of_erased->son[last_state] = NULL;
	}
	
	puts("OK");
}

// Obsługuje polecenie VALID.
void valid(char* history) {
	Node* node = find_node(history, strlen(history));
	free(history);
	
	if (node == NULL) puts("NO");
	else puts("YES");	
}

// Obsługuje jednoparametrowe polecenie ENERGY.
void energy_chk(char* history) {
	Node* node = find_node(history, strlen(history));
	free(history);
	
	if (node == NULL || node->id == -1) CALL_ERROR;
	
	uint64_t energy = get_energy(node->id);
	
	if (energy > 0) printf("%"PRIu64"\n", energy);
	else CALL_ERROR;
}

// Obsługuje dwuparametrowe polecenie ENERGY.
void energy_mod(char* history, uint64_t new_energy) {
	Node* node = find_node(history, strlen(history));
	free(history);
	if (node == NULL) CALL_ERROR;
	if (node->id == -1) node->id = get_identifier();
	
	set_energy(node->id, new_energy);
	
	puts("OK");
}

// Obsługuje polecenie EQUAL.
void equal(char* history1, char* history2) {
	Node* node1 = find_node(history1, strlen(history1));
	Node* node2 = find_node(history2, strlen(history2));
	free(history1);
	free(history2);
	
	if (node1 == NULL || node2 == NULL) CALL_ERROR;
	
	if (node1 != node2) {
		if (node1->id == -1) {
			Node* helper = node1;
			node1 = node2;
			node2 = helper;
		}

		if (node1->id == -1) CALL_ERROR;
		if (node2->id == -1) node2->id = get_identifier();
		
		set_equal(node1->id, node2->id);
	}
	
	puts("OK");
}
