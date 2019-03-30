#ifndef _PARSER_H_
#define _PARSER_H_

#include <inttypes.h>

typedef enum {
	EOF_CORRECT,
	EOF_ERROR,
	ERROR,
	NONE,
	DECLARE,
	REMOVE,
	VALID,
	EQUAL,
	ENERGY_MOD,
	ENERGY_CHK
} CommandType;

typedef struct { 
	CommandType name;
	char* arg1;
	union {
		char* arg2_s;
		uint64_t arg2_ll;
	};
} Command;

extern Command read_line();

#endif /* _PARSER_H_ */
