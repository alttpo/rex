
#include <stdint.h>
#include "rexlang_vm.h"

#define   likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef int8_t      s8;
typedef int16_t     s16;
typedef int32_t     s32;
typedef unsigned int ui;

#define throw_error(vm, e) { \
	vm->err = e; \
	longjmp(vm->j, vm->err); \
}

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

// read u8 from data
static inline u8 rddu8(struct rexlang_vm* vm, ui p)
{
	bounds_check_data(vm, p);
	return vm->d[p];
}

// read u16 from data
static inline u16 rddu16(struct rexlang_vm* vm, ui p)
{
	bounds_check_data(vm, p);
	return *(u16*)(&vm->d[p]);
}

// read u16 from data
static inline u32 rddu32(struct rexlang_vm* vm, ui p)
{
	bounds_check_data(vm, p);
	return *(u32*)(&vm->d[p]);
}

// write u8 to data
static inline void wrdu8(struct rexlang_vm* vm, ui p, u8 v)
{
	bounds_check_data(vm, p);
	vm->d[p] = v;
}

// write u16 to data
static inline void wrdu16(struct rexlang_vm* vm, ui p, u16 v)
{
	bounds_check_data(vm, p);
	*(u16*)(&vm->d[p]) = v;
}

// write u32 to data
static inline void wrdu32(struct rexlang_vm* vm, ui p, u32 v)
{
	bounds_check_data(vm, p);
	*(u32*)(&vm->d[p]) = v;
}

// read u8 from IP, advance IP
static inline u8 rdipu8(struct rexlang_vm *vm)
{
	return vm->m[vm->ip++];
}

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
// read u16 from IP, advance IP
static inline u16 rdipu16(struct rexlang_vm *vm)
{
	rexlang_ip ip = vm->ip;
	vm->ip += sizeof(u16);
	return *(u16*)(&vm->m[ip]);
}

// read u32 from IP, advance IP
static inline u32 rdipu32(struct rexlang_vm *vm)
{
	rexlang_ip ip = vm->ip;
	vm->ip += sizeof(u32);
	return *(u32*)(&vm->m[ip]);
}
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
// read u16 from IP, advance IP
static inline u16 rdipu16(struct rexlang_vm *vm)
{
	u16 lo = vm->m[vm->ip++];
	u16 hi = vm->m[vm->ip++];
	u16 val = (hi<<8) | (lo);
	return val;
}

// read u32 from IP, advance IP
static inline u32 rdipu32(struct rexlang_vm *vm)
{
	u32 b0 = vm->m[vm->ip++];
	u32 b1 = vm->m[vm->ip++];
	u32 b2 = vm->m[vm->ip++];
	u32 b3 = vm->m[vm->ip++];
	u32 val = (b3<<24) | (b2<<16) | (b1<<8) | (b0);
	return val;
}
#else
#  define VALUE_TO_STRING(x) #x
#  define VALUE(x) VALUE_TO_STRING(x)
#  pragma error("unknown __BYTE_ORDER__: " VALUE(__BYTE_ORDER__))
#endif

static inline void push(struct rexlang_vm *vm, u32 v)
{
	if (unlikely(vm->sp == 0)) {
		throw_error(vm, REXLANG_ERR_STACK_FULL);
	}

	// write the value into the stack:
	vm->ki[--vm->sp] = v;
}

static inline u32 pop(struct rexlang_vm *vm)
{
	if (unlikely(vm->sp >= REXLANG_STACKSZ)) {
		throw_error(vm, REXLANG_ERR_STACK_EMPTY);
	}

	// read the value from the stack and move the sp:
	return vm->ki[vm->sp++];
}
