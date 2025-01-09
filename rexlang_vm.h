
#ifndef _REXLANG_VM_H_
#define _REXLANG_VM_H_

#include <stdint.h>
#include <setjmp.h>

#define REXLANG_PAGESZ 256

typedef uint8_t rexlang_page[REXLANG_PAGESZ];

struct rexlang_vm;

struct rexlang_stack {
	uint8_t  s[224];    // items
	uint32_t t[7];      // item type bits
	uint32_t c;         // item count
};

_Static_assert(sizeof(struct rexlang_stack) == REXLANG_PAGESZ, "stack must be same size as a page");

// returns a pointer to 256 bytes of memory mapped for the given page or NULL
typedef uint8_t* (*rexlang_mem_f)(struct rexlang_vm *vm, uint8_t page);

enum rexlang_error {
	REXLANG_ERR_SUCCESS = 0,
	REXLANG_ERR_STACK_EMPTY,
	REXLANG_ERR_STACK_FULL,
	REXLANG_ERR_PAGE_UNMAPPED
};

struct rexlang_vm {
	uint16_t ip;                // instruction pointer
	uint16_t sp;                // stack pointer to free position
	struct rexlang_stack *k;    // stack memory page
	rexlang_mem_f m;            // memory mapper
	struct {
		const char *file;
		int line;
		enum rexlang_error code;
		jmp_buf j;
	} err;
};

void rexlang_init(struct rexlang_vm *vm, rexlang_mem_f m);
enum rexlang_error rexlang_exec(struct rexlang_vm *vm);

#endif
