
#ifndef _REXLANG_VM_H_
#define _REXLANG_VM_H_

#include <stdint.h>
#include <setjmp.h>

struct rexlang_vm;

typedef void (*rexlang_call_f)(struct rexlang_vm* vm, uint32_t fn);

enum rexlang_error {
	REXLANG_ERR_SUCCESS = 0,
	REXLANG_ERR_HALTED,
	REXLANG_ERR_BAD_OPCODE,
	REXLANG_ERR_STACK_EMPTY,
	REXLANG_ERR_STACK_FULL,
	REXLANG_ERR_DATA_ADDRESS_OUT_OF_BOUNDS,
	REXLANG_ERR_PRGM_ADDRESS_OUT_OF_BOUNDS,
	REXLANG_ERR_BAD_SYSCALL,
	REXLANG_ERR_CALL_ARG_OUT_OF_RANGE,
};

typedef unsigned int rexlang_ip;
typedef unsigned int rexlang_sp;

#define REXLANG_STACKSZ 64

struct rexlang_vm {
	rexlang_ip ip;          // instruction pointer
	rexlang_sp sp;          // stack pointer to free position
	enum rexlang_error err; // enum rexlang_error

	const uint8_t* m;             // program memory
	uint8_t* d;             // data memory

	uint32_t m_size;
	uint32_t d_size;

	// setjmp buffer
	jmp_buf  j;

	rexlang_call_f syscall;

	uint32_t ki[REXLANG_STACKSZ];      // stack items
};

void rexlang_vm_init(
	struct rexlang_vm *vm,
	uint32_t m_size,
	const uint8_t* m,
	uint32_t d_size,
	uint8_t* d,
	rexlang_call_f syscall
);

// explicitly reset the VM to initial state:
void rexlang_vm_reset(struct rexlang_vm *vm);

// this must be called after an error occurs to resume execution:
void rexlang_vm_error_ack(struct rexlang_vm *vm);

// execute the given number of instructions or until an error occurs
enum rexlang_error rexlang_vm_exec(struct rexlang_vm *vm, unsigned int instruction_count);

#endif
