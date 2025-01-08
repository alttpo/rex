
#ifndef _REX_VM_H_
#define _REX_VM_H_

#include <stdint.h>

#ifndef REXLANG_MEMSZ_PROG
#  define REXLANG_MEMSZ_PROG 3172
#endif
#ifndef REXLANG_MEMSZ_DATA
#  define REXLANG_MEMSZ_DATA 1024
#endif

#if REXLANG_MEMSZ_PROG + REXLANG_MEMSZ_DATA > 65536
#  error REXLANG_MEMSZ_PROG + REXLANG_MEMSZ_DATA cannot exceed 65336 bytes
#endif

#define REXLANG_MEMOFFS_PRGM REXLANG_MEMSZ_DATA

struct rexlang_vm {
	uint16_t ip;    // program counter as index into m[], starts at REXLANG_MEMOFFS_PRGM
	uint16_t sp;    // stack pointer as index into s[], starts at sizeof(s)

	// data+program memory:
	uint8_t  m[REXLANG_MEMSZ_DATA+REXLANG_MEMSZ_PROG];

	// stack values and their types stored in bitfield (0 = u8, 1 = u16):
	uint8_t  s[64]; // stack
	uint32_t st[2]; // stack type bits
};

void rexlang_init(struct rexlang_vm *vm);
void rexlang_exec(struct rexlang_vm *vm);

#endif
