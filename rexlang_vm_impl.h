
#include <stdint.h>
#include "rexlang_vm.h"

#define   likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

typedef uint_fast8_t  u8;
typedef uint_fast16_t u16;

#ifdef NDEBUG
#  define throw_error(vm, e) { \
	vm->err = e; \
	longjmp(vm->j, vm->err); \
}
#else
#  define throw_error(vm, e) { \
	vm->file = __FILE__; \
	vm->line = __LINE__; \
	vm->err = e; \
	longjmp(vm->j, vm->err); \
}
#endif

#ifdef REXLANG_NO_BOUNDS_CHECK
#  define bounds_check_data(vm, p)
#  define bounds_check_prgm(vm, p)
#else
#  define bounds_check_data(vm, p) \
	if (unlikely(p >= vm->d_size)) \
		throw_error(vm, REXLANG_ERR_DATA_ADDRESS_OUT_OF_BOUNDS)
#  define bounds_check_prgm(vm, p) \
	if (unlikely(p >= vm->m_size)) \
		throw_error(vm, REXLANG_ERR_PRGM_ADDRESS_OUT_OF_BOUNDS)
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

static inline void push(struct rexlang_vm *vm, u16 v)
{
	if (unlikely(vm->sp == 0)) {
		throw_error(vm, REXLANG_ERR_STACK_FULL);
	}

	// write the value into the stack:
	vm->ki[vm->sp--] = v;
}

static inline u16 pop(struct rexlang_vm *vm)
{
	if (unlikely(vm->sp >= 128)) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
	}

	// read the value from the stack and move the sp:
	return vm->ki[vm->sp++];
}
