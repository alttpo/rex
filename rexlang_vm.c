
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "rexlang_vm.h"

#define   likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

typedef uint8_t  u8;
typedef uint16_t u16;

#define TY_U8  0
#define TY_U16 1

#define throw_error(vm, e) { \
	vm->err.file = __FILE__; \
	vm->err.line = __LINE__; \
	vm->err.code = e; \
	longjmp(vm->err.j, vm->err.code); \
}

// read, advance pointer
static inline u8 rdau8(uint8_t* m, u16 *p)
{
	return m[*p++ & 0xFF];
}

// read, advance pointer
static inline u16 rdau16(uint8_t* m, u16 *p)
{
	assert(m);
	u16 a = *p & 0xFF;
	u16 lo = m[a+0];
	u16 hi = m[a+1];
	u16 val = ((hi)<<8) | (lo);
	*p += 2;
	return val;
}

// read from IP, advance IP
static inline u8 rdipu8(struct rexlang_vm *vm)
{
	return rdau8(vm->m, &vm->ip);
}

// read from IP, advance IP
static inline u16 rdipu16(struct rexlang_vm *vm)
{
	return rdau16(vm->m, &vm->ip);
}

// write, no-advance pointer
static inline void wrnu8(uint8_t* m, u16 p, u8 v)
{
	assert(m);
	m[(p & 0xFF)] = v;
}

// write, no-advance pointer
static inline void wrnu16(uint8_t* m, u16 p, u16 v)
{
	assert(m);
	unsigned int a = p & 0xFF;
	m[a+0] = v;
	m[a+1] = v >> 8;
}

static inline void push_u8(struct rexlang_vm *vm, u8 v)
{
	if (unlikely((vm->sp&0xFF) <= 0)) {
		throw_error(vm, REXLANG_ERR_STACK_FULL);
		return;
	}

	struct rexlang_stack *k = vm->k;

	// write the value into the stack:
	vm->sp--;
	wrnu8(k->s, vm->sp, v);
	// update type bits:
	k->t[k->c >> 5] &= ~(1UL<<(k->c & 31));
	k->c++;
}

static inline void push_u16(struct rexlang_vm *vm, u16 v)
{
	if (unlikely((vm->sp&0xFF) <= 1)) {
		throw_error(vm, REXLANG_ERR_STACK_FULL);
		return;
	}

	struct rexlang_stack *k = vm->k;

	// write the value into the stack:
	vm->sp -= 2;
	wrnu16(k->s, vm->sp, v);
	// update type bits:
	k->t[k->c >> 5] |= 1UL<<(k->c & 31);
	k->c++;
}

static inline u8 pop_u8(struct rexlang_vm *vm)
{
	if (unlikely(vm->sp >= 0xE0)) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
		return 0xFF;
	}

	struct rexlang_stack *k = vm->k;

	// read type bits:
	if ((k->t[k->c >> 5] & (1UL<<(k->c & 31))) != 0) {
		throw_error(vm, REXLANG_ERR_POP_EXPECTED_U8);
		return 0xFF;
	};
	k->c--;

	// read the value from the stack and move the sp:
	return rdau8(k->s, &vm->sp);
}

static inline u16 pop_u16(struct rexlang_vm *vm)
{
	if (unlikely(vm->sp >= 0xE0)) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
		return 0xFF;
	}

	struct rexlang_stack *k = vm->k;

	// read type bits:
	if ((k->t[k->c >> 5] & (1UL<<(k->c & 31))) == 0) {
		throw_error(vm, REXLANG_ERR_POP_EXPECTED_U16);
		return 0xFF;
	};
	k->c--;

	// read the value from the stack and move the sp:
	return rdau16(k->s, &vm->sp);
}

static inline u16 pop(struct rexlang_vm *vm, u8 *type_out)
{
	if (unlikely(vm->sp >= 0xE0)) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
		return 0xFF;
	}

	struct rexlang_stack *k = vm->k;

	// read type bits:
	u8 ty = (k->t[k->c >> 5] & (1UL<<(k->c & 31))) != 0;
	k->c--;

	*type_out = ty;

	// read the value from the stack and move the sp:
	if (ty == TY_U8) {
		return rdau8(k->s, &vm->sp);
	} else {
		return rdau16(k->s, &vm->sp);
	}
}

static void opcode(struct rexlang_vm *vm, u16 x)
{
	switch (x) {
	case 0x02: { // call
		u8 ty;
		u16 a = pop(vm, &ty);
		push_u16(vm, vm->ip);
		vm->ip = a;
		break;
	}

	default:
		break;
	}
}

enum rexlang_error rexlang_vm_exec(struct rexlang_vm *vm)
{
	assert(vm->m);
	assert(vm->d);
	assert(vm->k);

	// reset error state:
	vm->err.code = REXLANG_ERR_SUCCESS;
	vm->err.file = NULL;
	vm->err.line = 0;
	// mark longjmp destination for error handling:
	if (setjmp(vm->err.j)) {
		// we get here only if throw_error() (aka longjmp) is called
		// return error code; additional details found in vm->err struct:
		return vm->err.code;
	}

	// read instruction:
	u8 o = rdipu8(vm);
	if ((o & 0xC0) == 0) {
		// push u8 value:
		push_u8(vm, o);
	} else if ((o & 0xC0) == 0x40) {
		// push up to 4 mixed values:
		int x = (o & 3) + 1;
		while (x--) {
			if (o & 4) {
				u16 v = rdipu16(vm);
				push_u16(vm, v);
			} else {
				u16 v = rdipu8(vm);
				push_u8(vm, v);
			}
			o >>= 1;
		}
	} else if (o == 0x80) {
		// syscall:
		u8 x = rdipu8(vm);
	} else if (o == 0x81) {
		// extcall:
		u16 x = rdipu16(vm);
	} else if (o == 0xFD) {
		// prgm-enter:
		u16 x = rdipu16(vm);
	} else if (o == 0xFE) {
		// prgm-end:
	} else if (o == 0xFF) {
		// opcode-ext:
		u16 x = rdipu8(vm) + 0x80;
		opcode(vm, x);
	} else {
		// opcode:
		u16 x = (o & 0x7F);
		opcode(vm, x);
	}

	return vm->err.code;
}

void rexlang_vm_init(
	struct rexlang_vm *vm,
	size_t m_size,
	uint8_t* m,
	size_t d_size,
	uint8_t* d,
	struct rexlang_stack* k
) {
	assert(vm && "vm cannot be NULL");
	assert(m && "m cannot be NULL");
	assert(d && "d cannot be NULL");
	assert(k && "k cannot be NULL");
	vm->m = m;
	vm->d = d;
	vm->k = k;
	vm->ip = 0;
	vm->sp = 224;
	vm->err.code = REXLANG_ERR_SUCCESS;
}
