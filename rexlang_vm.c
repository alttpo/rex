
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "rexlang_vm.h"

#define   likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

// using uint_fastX_t makes a big code-size difference for ARM thumb
typedef uint_fast8_t  u8;
typedef uint_fast16_t u16;

#define TY_U8  0
#define TY_U16 1

// OR the two type bits together to find the bigger size (any 1 wins)
#define tmax(a,b) ((a) | (b))

#define throw_error(vm, e) { \
	vm->err.file = __FILE__; \
	vm->err.line = __LINE__; \
	vm->err.code = e; \
	longjmp(vm->err.j, vm->err.code); \
}

#ifdef REXLANG_NO_BOUNDS_CHECK
#  define bounds_check_data(vm, p)
#else
#  define bounds_check_data(vm, p) \
	if (unlikely(p >= vm->d_size)) throw_error(vm, REXLANG_ERR_DATA_ADDRESS_OUT_OF_BOUNDS)
#endif

// read from data
static inline u8 rddu8(struct rexlang_vm* vm, u16 p)
{
	bounds_check_data(vm, p);
	return vm->d[p];
}

// read from data
static inline u16 rddu16(struct rexlang_vm* vm, u16 p)
{
	bounds_check_data(vm, p);
	u16 lo = vm->d[p+0];
	u16 hi = vm->d[p+1];
	u16 val = ((hi)<<8) | (lo);
	return val;
}

// write to data
static inline void wrdu8(struct rexlang_vm* vm, u16 p, u8 v)
{
	bounds_check_data(vm, p);
	vm->d[p] = v;
}

// write to data
static inline void wrdu16(struct rexlang_vm* vm, u16 p, u16 v)
{
	bounds_check_data(vm, p);
	vm->d[p+0] = v;
	vm->d[p+1] = v >> 8;
}

// read, advance pointer
static inline u8 rdau8(uint8_t* m, u16 *p)
{
	return m[*p++];
}

// read, advance pointer
static inline u16 rdau16(uint8_t* m, u16 *p)
{
	assert(m);
	u16 a = *p;
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
	m[p] = v;
}

// write, no-advance pointer
static inline void wrnu16(uint8_t* m, u16 p, u16 v)
{
	assert(m);
	m[p+0] = v;
	m[p+1] = v >> 8;
}

static inline void push_u8(struct rexlang_vm *vm, u8 v)
{
	if (unlikely(vm->sp == 0)) {
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
	if (unlikely(vm->sp <= 1)) {
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

static inline void push(struct rexlang_vm *vm, u16 v, u8 ty)
{
	if (ty == TY_U8) {
		push_u8(vm, v);
	} else {
		push_u16(vm, v);
	}
}

static inline u8 pop_u8(struct rexlang_vm *vm)
{
	if (unlikely(vm->sp >= 0xE0)) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
		return 0xFF;
	}

	struct rexlang_stack *k = vm->k;

	// read type bits:
	if (unlikely((k->t[k->c >> 5] & (1UL<<(k->c & 31))) != 0)) {
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
	if (unlikely((k->t[k->c >> 5] & (1UL<<(k->c & 31))) == 0)) {
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
	u16 a;
	u16 b;
	u16 c;
	u8 ta;
	u8 tb;
	u8 tc;

	if ((x & 0xF0) == 0x20) { // 010xxxx shlx
		a = pop(vm, &ta);
		push(vm, a << (x & 0x0F), ta);
		return;
	} else if ((x & 0xF0) == 0x30) { // 011xxxx shrx
		a = pop(vm, &ta);
		push(vm, a >> (x & 0x0F), ta);
		return;
	} else if ((x & 0xF8) == 0x40) { // 1000xxx ld-u8-offs
		a = pop_u16(vm) + (u16)(x & 0x07);
		push_u8(vm, rddu8(vm, a));
		return;
	} else if ((x & 0xF8) == 0x48) { // 1001xxx ld-u16-offs
		a = pop_u16(vm) + (u16)(x & 0x07);
		push_u16(vm, rddu16(vm, a));
		return;
	} else if ((x & 0xF8) == 0x50) { // 1010xxx st-u8-offs
		b = pop_u8(vm);
		a = pop_u16(vm) + (u16)(x & 0x07);
		wrdu8(vm, a, b);
		push_u8(vm, b);
		return;
	} else if ((x & 0xF8) == 0x58) { // 1011xxx st-u16-offs
		b = pop_u16(vm);
		a = pop_u16(vm) + (u16)(x & 0x07);
		wrdu16(vm, a, b);
		push_u16(vm, b);
		return;
	} else switch (x) {
	case 0x02: { // 0000010 call
		a = pop(vm, &ta);
		push_u16(vm, vm->ip);
		vm->ip = a;
		break;
	}
	case 0x03: { // 0000011 jump / return
		a = pop(vm, &ta);
		vm->ip = a;
		break;
	}
	case 0x04: { // 0000100 jump-if
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		if (b != 0) {
			vm->ip = a;
		}
		break;
	}
	case 0x05: { // 0000101 jump-if-not
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		if (b == 0) {
			vm->ip = a;
		}
		break;
	}
	case 0x06: { // 0000110 swap
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a, ta);
		push(vm, b, tb);
		break;
	}
	case 0x07: { // 0000111 discard
		pop(vm, &ta);
		break;
	}
	case 0x08: { // 0001000 to-u8
		a = pop(vm, &ta);
		push_u8(vm, a);
		break;
	}
	case 0x09: { // 0001001 to-u16
		a = pop(vm, &ta);
		push_u16(vm, a);
		break;
	}
	case 0x0A: { // 0001010 eq
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push_u8(vm, a == b);
		break;
	}
	case 0x0B: { // 0001011 ne
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push_u8(vm, a != b);
		break;
	}
	case 0x0C: { // 0001100 le
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push_u8(vm, a <= b);
		break;
	}
	case 0x0D: { // 0001101 gt
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push_u8(vm, a >  b);
		break;
	}
	case 0x0E: { // 0001110 lt
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push_u8(vm, a <  b);
		break;
	}
	case 0x0F: { // 0001111 ge
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push_u8(vm, a >= b);
		break;
	}
	case 0x10: { // 0010000 and
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a & b, tmax(ta, tb));
		break;
	}
	case 0x11: { // 0010001 or
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a | b, tmax(ta, tb));
		break;
	}
	case 0x12: { // 0010010 xor
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a ^ b, tmax(ta, tb));
		break;
	}
	case 0x13: { // 0010011 not
		a = pop(vm, &ta);
		push(vm, !a, ta);
		break;
	}
	case 0x14: { // 0010100 neg
		a = pop(vm, &ta);
		push(vm, -a, ta);
		break;
	}
	case 0x15: { // 0010101 add
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a + b, tmax(ta, tb));
		break;
	}
	case 0x16: { // 0010110 sub
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a - b, tmax(ta, tb));
		break;
	}
	case 0x17: { // 0010111 mul
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a * b, tmax(ta, tb));
		break;
	}
	case 0x18: { // 0011000 inc
		a = pop(vm, &ta);
		push(vm, a+1, tmax(ta, tb));
		break;
	}
	case 0x19: { // 0011001 dec
		a = pop(vm, &ta);
		push(vm, a-1, tmax(ta, tb));
		break;
	}
	case 0x1A: { // 0011010 ld-u8
		a = pop_u16(vm);
		push_u8(vm, rddu8(vm, a));
		break;
	}
	case 0x1B: { // 0011011 ld-u16
		a = pop_u16(vm);
		push_u16(vm, rddu16(vm, a));
		break;
	}
	case 0x1C: { // 0011100 st-u8
		b = pop_u8(vm);
		a = pop_u16(vm);
		wrdu8(vm, a, b);
		push_u8(vm, b);
		break;
	}
	case 0x1D: { // 0011101 st-u16
		b = pop_u16(vm);
		a = pop_u16(vm);
		wrdu8(vm, a, b);
		push_u16(vm, b);
		break;
	}
	case 0x1E: { // 0011110 shl
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a << b, ta);
		break;
	}
	case 0x1F: { // 0011111 shr
		b = pop(vm, &tb);
		a = pop(vm, &ta);
		push(vm, a >> b, ta);
		break;
	}
	// NOTE: range from 0x20 to 0x5F handled by if statements above
	case 0x60: { // 1100000 copy
		c = pop(vm, &tc);
		b = pop_u16(vm);
		a = pop_u16(vm);
		bounds_check_data(vm, a+c-1);
		bounds_check_data(vm, b+c-1);
		memcpy(vm->d + a, vm->d + b, c);
		push_u16(vm, a + c);
		break;
	}

	default:
		throw_error(vm, REXLANG_ERR_BAD_OPCODE);
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
	vm->m_size = m_size;
	vm->d = d;
	vm->d_size = d_size;
	vm->k = k;

	vm->ip = 0;
	vm->sp = 224;
	vm->err.code = REXLANG_ERR_SUCCESS;
}
