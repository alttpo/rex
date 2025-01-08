
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "rexlang_vm.h"

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

// get page of memory:
static inline u8* page(struct rexlang_vm *vm, u16 p)
{
	return vm->m(vm, p >> 8);
}

// read, advance pointer
static inline u8 rdau8(rexlang_page m, u16 *p)
{
	return m[*p++ & 0xFF];
}

// read, advance pointer
static inline u16 rdau16(rexlang_page m, u16 *p)
{
	assert(m);
	u8 lo = m[*p++ & 0xFF];
	u8 hi = m[*p++ & 0xFF];
	return ((u16)(hi)<<8) | (u16)(lo);
}

// read from IP, advance IP
static inline u8 rdipu8(struct rexlang_vm *vm)
{
	u8 *m = page(vm, vm->ip);
	if (!m) {
		throw_error(vm, REXLANG_ERR_PAGE_UNMAPPED);
	}
	return rdau8(m, &vm->ip);
}

// read from IP, advance IP
static inline u16 rdipu16(struct rexlang_vm *vm)
{
	u8 *m = page(vm, vm->ip);
	if (!m) {
		throw_error(vm, REXLANG_ERR_PAGE_UNMAPPED);
	}
	return rdau16(m, &vm->ip);
}

// write, advance pointer
static inline void wrau8(rexlang_page m, u16 *p, u8 v)
{
	assert(m);
	m[*p++ & 0xFF] = v;
}

// write, advance pointer
static inline void wrau16(rexlang_page m, u16 *p, u16 v)
{
	assert(m);
	m[*p++ & 0xFF] = v & 0xFF;
	m[*p++ & 0xFF] = v >> 8;
}

// write, no-advance pointer
static inline void wrnu8(rexlang_page m, u16 *p, u8 v)
{
	assert(m);
	m[(*p+0) & 0xFF] = v;
}

// write, no-advance pointer
static inline void wrnu16(rexlang_page m, u16 *p, u16 v)
{
	assert(m);
	m[(*p+0) & 0xFF] = v & 0xFF;
	m[(*p+1) & 0xFF] = v >> 8;
}

static void push(struct rexlang_vm *vm, u16 v, u8 type)
{
	assert((type <= TY_U16) && "invalid type value");

	if ((vm->sp&0xFF) <= type) {
		throw_error(vm, REXLANG_ERR_STACK_FULL);
		return;
	}

	struct rexlang_stack *k = (struct rexlang_stack *)page(vm, vm->sp);
	if (!k) {
		throw_error(vm, REXLANG_ERR_PAGE_UNMAPPED);
	}

	if (type == TY_U8) {
		// write the value into the stack:
		vm->sp--;
		wrnu8(k->s, &vm->sp, v);
		// update type bits:
		k->t[k->c >> 5] &= ~(k->c & 31);
		k->c++;
	} else if (type == TY_U16) {
		// write the value into the stack:
		vm->sp -= 2;
		wrnu16(k->s, &vm->sp, v);
		// update type bits:
		k->t[k->c >> 5] |= (k->c & 31);
		k->c++;
	}
}

static u16 pop(struct rexlang_vm *vm, u8 *type_out)
{
	if (vm->sp >= 0xFFE0) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
		return 0xFFFF;
	}

	struct rexlang_stack *k = (struct rexlang_stack *)page(vm, vm->sp);

	// read type bits:
	u8 ty = !!(k->t[k->c >> 5] & (k->c & 31));
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
		push(vm, vm->ip, TY_U16);
		vm->ip = a;
		break;
	}

	default:
		break;
	}
}

enum rexlang_error rexlang_exec(struct rexlang_vm *vm)
{
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
		push(vm, o, TY_U8);
	} else if ((o & 0xC0) == 0x40) {
		// push up to 4 mixed values:
		int x = (o & 3) + 1;
		while (x--) {
			if (o & 4) {
				u16 v = rdipu16(vm);
				push(vm, v, TY_U16);
			} else {
				u16 v = rdipu8(vm);
				push(vm, v, TY_U8);
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

void rexlang_init(struct rexlang_vm *vm, rexlang_mem_f m)
{
	assert(vm && "vm cannot be NULL");
	assert(m && "m cannot be NULL");
	vm->m = m;
	vm->ip = 0x8000;
	vm->sp = 0xFF00 + 224;
	vm->err.code = REXLANG_ERR_SUCCESS;
}
