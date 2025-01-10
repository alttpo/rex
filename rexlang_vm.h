
#ifndef _REXLANG_VM_H_
#define _REXLANG_VM_H_

#include <stdint.h>
#include <setjmp.h>

struct rexlang_stack {
	uint8_t  s[224];    // items
	uint32_t t[7];      // item type bits
	uint32_t c;         // item count (32 bits is overkill but it rounds the struct up to 256 bytes)
};

_Static_assert(sizeof(struct rexlang_stack) == 256, "stack must be 256 bytes");

struct rexlang_vm;

typedef void (*rexlang_call_f)(struct rexlang_vm* vm, uint_fast16_t fn);

enum rexlang_error {
	REXLANG_ERR_SUCCESS = 0,
	REXLANG_ERR_HALTED,
	REXLANG_ERR_BAD_OPCODE,
	REXLANG_ERR_STACK_EMPTY,
	REXLANG_ERR_STACK_FULL,
	REXLANG_ERR_POP_EXPECTED_U8,
	REXLANG_ERR_POP_EXPECTED_U16,
	REXLANG_ERR_DATA_ADDRESS_OUT_OF_BOUNDS,
};

struct rexlang_vm {
	uint8_t *m;                 // program memory
	uint8_t *d;                 // data memory
	struct rexlang_stack *k;    // stack memory

	size_t m_size;
	size_t d_size;

	uint_fast16_t ip;           // instruction pointer
	uint_fast16_t sp;           // stack pointer to free position

	rexlang_call_f syscall;
	rexlang_call_f extcall;

	// details of last error iff code != REXLANG_ERR_SUCCESS:
	struct {
		enum rexlang_error code;
		const char *file;
		int line;

		jmp_buf j;
	} err;
};

void rexlang_vm_init(
	struct rexlang_vm *vm,
	size_t m_size,
	uint8_t* m,
	size_t d_size,
	uint8_t* d,
	struct rexlang_stack* k,
	rexlang_call_f syscall,
	rexlang_call_f extcall
);

// explicitly reset the VM to initial state:
void rexlang_vm_reset(struct rexlang_vm *vm);

// this must be called after an error occurs to resume execution:
void rexlang_vm_error_ack(struct rexlang_vm *vm);

// execute the given number of instructions or until an error occurs
enum rexlang_error rexlang_vm_exec(struct rexlang_vm *vm, uint_fast16_t instruction_count);

#endif
