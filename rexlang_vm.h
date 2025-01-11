
#ifndef _REXLANG_VM_H_
#define _REXLANG_VM_H_

#include <stdint.h>
#include <setjmp.h>

struct rexlang_vm;

typedef void (*rexlang_call_f)(struct rexlang_vm* vm, uint16_t fn);

enum rexlang_error {
	REXLANG_ERR_SUCCESS = 0,
	REXLANG_ERR_HALTED,
	REXLANG_ERR_BAD_OPCODE,
	REXLANG_ERR_STACK_EMPTY,
	REXLANG_ERR_STACK_FULL,
	REXLANG_ERR_POP_EXPECTED_U8,
	REXLANG_ERR_POP_EXPECTED_U16,
	REXLANG_ERR_DATA_ADDRESS_OUT_OF_BOUNDS,
	REXLANG_ERR_PRGM_ADDRESS_OUT_OF_BOUNDS,
};

typedef uint_fast16_t rexlang_ip;
typedef uint_fast16_t rexlang_sp;

struct rexlang_vm {
	rexlang_ip ip;          // instruction pointer
	rexlang_sp sp;          // stack pointer to free position
	enum rexlang_error err; // enum rexlang_error

	uint8_t* m;             // program memory
	uint8_t* d;             // data memory

	uint16_t m_size;
	uint16_t d_size;

	// setjmp buffer
	jmp_buf  j;

	rexlang_call_f syscall;
	rexlang_call_f extcall;

#ifndef NDEBUG
	// VM source location of last error iff err != REXLANG_ERR_SUCCESS:
	const char *file;
	int line;
#endif

	uint16_t  ki[128];      // stack items
};

void rexlang_vm_init(
	struct rexlang_vm *vm,
	uint16_t m_size,
	uint8_t* m,
	uint16_t d_size,
	uint8_t* d,
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
