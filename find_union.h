#ifndef _FIND_UNION_H_
#define _FIND_UNION_H_

#include <inttypes.h>

extern void find_union_initialize();

extern void find_union_clear();

extern int32_t get_identifier();

extern void remove_identifier(int32_t id);

extern uint64_t get_energy(int32_t id);

extern void set_energy(int32_t id, uint64_t energy);

extern void set_equal(int32_t id1, int32_t id2);

#endif /* _FIND_UNION_H_ */
