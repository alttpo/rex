
#ifndef _REXLANG_VM_H_
#define _REXLANG_VM_H_

#include <stdint.h>

#define REXLANG_PAGESZ 256

typedef uint8_t rexlang_page[REXLANG_PAGESZ];

struct rexlang_vm;

struct rexlang_stack {
	uint8_t s[224];
	uint32_t st[8];
};

_Static_assert(sizeof(struct rexlang_stack) == REXLANG_PAGESZ, "stack must be same size as a page");

// returns a pointer to 256 bytes of memory mapped for the given page or NULL
typedef uint8_t* (*rexlang_mem_f)(struct rexlang_vm *vm, uint8_t page);

struct rexlang_vm {
	uint16_t ip;        // instruction pointer
	uint16_t sp;        // stack pointer to free position
	rexlang_mem_f m;    // memory mapper
};

void rexlang_init(struct rexlang_vm *vm, rexlang_mem_f m);
void rexlang_exec(struct rexlang_vm *vm);

#endif
