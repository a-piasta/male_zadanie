#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "parser.h"
#include "trie_tree.h"
#include "find_union.h"

int main(void) {
	atexit(trie_tree_clear);
	find_union_initialize();
	
	while (true) {
		Command command = read_line();
		
		switch (command.name) {
			case EOF_ERROR:
			fprintf(stderr, "ERROR\n");
			return 0;
			
			case EOF_CORRECT:;
			return 0;
			
			case ERROR:
			fprintf(stderr, "ERROR\n");
			break;
			
			case DECLARE:
			declare(command.arg1);
			break;
			
			case REMOVE:
			Remove(command.arg1);
			break;
			
			case VALID:
			valid(command.arg1);
			break;
			
			case ENERGY_CHK:
			energy_chk(command.arg1);
			break;
			
			case ENERGY_MOD:
			energy_mod(command.arg1, command.arg2_ll);
			break;
			
			case EQUAL:
			equal(command.arg1, command.arg2_s);
			break;
			
			case NONE: ;
		}
	}
}
