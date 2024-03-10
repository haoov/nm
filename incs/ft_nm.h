#ifndef FT_NM
#define FT_NM

#include <elf.h>
#include <stdio.h>
#include "nm_error.h"

#define OPTS	"agnurp"

#define OALL	0x01 //-a display all symbol table entries
#define OGLOB	0x02 //-g display only global symbols
#define ONUM	0x04 //-n sort numerically
#define OUND	0x08 //-u display only undefined symbols
#define OREV	0x10 //-r display in reverse order
#define ONSRT	0x20 //-p dont sort

typedef struct data_s {
	uint8_t opt;
} data_t;

#endif // FT_NM