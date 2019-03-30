#ifndef _TRIETREE_H_
#define _TRIETREE_H_

#include <inttypes.h>

extern void trie_tree_clear();

extern void declare(char* history);

/* Duża litera, ponieważ funkcja o nazwie remove
 * znajduje się już w bibliotece stdio.h */
extern void Remove(char* history);

extern void valid(char* history);

extern void energy_chk(char* history);

extern void energy_mod(char* history, uint64_t energy);

extern void equal(char* history1, char* history2);

#endif /* _TRIETREE_H_ */
