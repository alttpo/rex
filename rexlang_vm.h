
#ifndef _REX_VM_H_
#define _REX_VM_H_

#include <stdint.h>

#ifndef REXLANG_MEMSZ
#  define REXLANG_MEMSZ 1024
#endif

#if REXLANG_MEMSZ > 4096
#  error REXLANG_MEMSZ cannot be more than 4096 bytes
#endif

struct rexlang_vm {
	uint16_t   p;               // program counter as index into m[], starts at 0
	uint16_t   s;               // stack pointer as index into m[], starts at len(m)-1
	uint16_t   d;               // data allocation pointer

	// TODO: store multiple contexts in m[] and denote split points

	uint8_t m[REXLANG_MEMSZ];    // read-write memory
};

void rexlang_exec(struct rexlang_vm *vm);

enum rexlang_op {
	REXLANG_OP_RETURN = 0,
	REXLANG_OP_GOTO,
	REXLANG_OP_IF,

	// TBD...

	REXLANG_OP_NOT,
	REXLANG_OP_AND,
	REXLANG_OP_OR,
	REXLANG_OP_XOR,
	REXLANG_OP_SHL,
	REXLANG_OP_SHR,
	REXLANG_OP_EQ,
	REXLANG_OP_NE,

	REXLANG_OP_ADD,
	REXLANG_OP_SUB,

	REXLANG_OP_GT_SIGNED,
	REXLANG_OP_LT_SIGNED,
	REXLANG_OP_GE_SIGNED,
	REXLANG_OP_LE_SIGNED,

	REXLANG_OP_GT_UNSIGNED,
	REXLANG_OP_LT_UNSIGNED,
	REXLANG_OP_GE_UNSIGNED,
	REXLANG_OP_LE_UNSIGNED,
};

#endif
