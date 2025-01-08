
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include "rexlang_vm.h"

typedef uint8_t  u8;
typedef uint16_t u16;

#define TY_U8  0
#define TY_U16 1

void rexlang_init(struct rexlang_vm *vm)
{
	vm->ip = 0;
	vm->sp = sizeof(vm->s);
	memset(vm->m,  0, sizeof(vm->m));
	memset(vm->s,  0, sizeof(vm->s));
	memset(vm->st, 0, sizeof(vm->st));
}

static u8 mu8(struct rexlang_vm *vm)
{
	return vm->m[REXLANG_MEMOFFS_PRGM + vm->ip++];
}

static u16 mu16(struct rexlang_vm *vm)
{
	u8 lo = vm->m[REXLANG_MEMOFFS_PRGM + vm->ip++];
	u8 hi = vm->m[REXLANG_MEMOFFS_PRGM + vm->ip++];
	return ((u16)(hi)<<8) | (u16)(lo);
}

static void push(struct rexlang_vm *vm, u16 v, u8 type)
{
	assert((type <= TY_U16) && "invalid type value");

	if (vm->sp <= type) {
		// TODO: indicate out of stack space error
		return;
	}

	if (type == TY_U8) {
		vm->sp--;
		vm->s[vm->sp] = v;
	} else if (type == TY_U16) {
		vm->sp -= 2;
		vm->s[vm->sp+0] = v & 0xFF;
		vm->s[vm->sp+1] = (v>>8) & 0xFF;
	}

	// shift-left the entire type stack by one bit:
	vm->st[1] = (vm->st[1] << 1) | (vm->st[0] >> 31);
	// and push in the size bit:
	vm->st[0] = (vm->st[0] << 1) | (type);
}

static u16 pop(struct rexlang_vm *vm, u8 *type_out)
{
	if (vm->sp == sizeof(vm->s)) {
		// TODO: stack empty error
		return 0xFFFF;
	}

	u8 ty = vm->st[0] & 1;
	*type_out = ty;

	// shift-right the entire type stack by one bit:
	vm->st[0] = (vm->st[0] >> 1) | (vm->st[1] << 31);
	vm->st[1] = (vm->st[1] >> 1);

	// read the value from the stack and move the sp:
	if (ty == TY_U8) {
		return vm->s[vm->sp++];
	} else {
		u8 lo = vm->s[vm->sp++];
		u8 hi = vm->s[vm->sp++];
		return ((u16)(hi)<<8) | (u16)(lo);
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

void rexlang_exec(struct rexlang_vm *vm)
{
	u8 o = mu8(vm);
	if ((o & 0xC0) == 0) {
		// push u8 value:
		push(vm, o, TY_U8);
	} else if ((o & 0xC0) == 0x40) {
		// push up to 4 mixed values:
		int x = (o & 3);
		while (x--) {
			if (o & 4) {
				u16 v = mu16(vm);
				push(vm, v, TY_U16);
			} else {
				u16 v = mu8(vm);
				push(vm, v, TY_U8);
			}
			o >>= 1;
		}
	} else if (o == 0x80) {
		// syscall:
		u8 x = mu8(vm);
	} else if (o == 0x81) {
		// extcall:
		u16 x = mu16(vm);
	} else if (o == 0xFD) {
		// prgm-enter:
		u16 x = mu16(vm);
	} else if (o == 0xFE) {
		// prgm-end:
	} else if (o == 0xFF) {
		// opcode-ext:
		u16 x = mu8(vm) + 0x80;
		opcode(vm, x);
	} else {
		// opcode:
		u16 x = (o & 0x7F);
		opcode(vm, x);
	}
}
