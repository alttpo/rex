
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "rexlang_vm_impl.h"

static void opcode(struct rexlang_vm *vm, u16 x)
{
	u16 a;
	u16 b;
	u16 c;
	u8 ta;
	u8 tb;
	u8 tc;

	if (x == 0) { // halt
		vm->err = REXLANG_ERR_HALTED;
		return;
	}

	if ((x & 0xF8) == 0x40) { // 1000xxx ld-u8-offs
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
	} else if ((x & 0xF0) == 0x20) { // 010xxxx shlx
		a = pop(vm, &ta);
		push(vm, a << (x & 0x0F), ta);
		return;
	} else if ((x & 0xF0) == 0x30) { // 011xxxx shrx
		a = pop(vm, &ta);
		push(vm, a >> (x & 0x0F), ta);
		return;
	} else switch (x) {
	case 0x01: // nop
		break;
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

enum rexlang_error rexlang_vm_exec(struct rexlang_vm *vm, uint_fast16_t instruction_count)
{
	assert(vm->m);
	assert(vm->d);

	// require an explicit error acknowledgement:
	if (vm->err != REXLANG_ERR_SUCCESS) {
		return vm->err;
	}

	// mark longjmp destination for error handling:
	if (setjmp(vm->j)) {
		// we get here only if throw_error() (aka longjmp) is called
		// return error code; additional details found in vm->err struct:
		return vm->err;
	}

	while ((vm->err == REXLANG_ERR_SUCCESS) && instruction_count--) {
		// read instruction:
		u8 o = rdipu8(vm);
		if ((o & 0xC0) == 0x80) {
			// push u8 value:
			push_u8(vm, o);
		} else if ((o & 0xC0) == 0xC0) {
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
		} else if (o == 0x7D) {
			// syscall:
			u16 x = rdipu8(vm);
			if (x & 0x80) {
				// extended call number:
				x = ((u16)x & 0x7F) | (((u16)rdipu8(vm))<<8);
			} else {
				x &= 0x7F;
			}
			if (vm->syscall) {
				vm->syscall(vm, x);
			}
		} else if (o == 0x7E) {
			// extcall:
			u16 x = rdipu8(vm);
			if (x & 0x80) {
				// extended call number:
				x = ((u16)x & 0x7F) | (((u16)rdipu8(vm))<<8);
			} else {
				x &= 0x7F;
			}
			if (vm->extcall) {
				vm->extcall(vm, x);
			}
		} else if (o == 0x7F) {
			// opcode-ext:
			u16 x = (u16)rdipu8(vm) + 0x80;
			opcode(vm, x);
		} else {
			// opcode:
			u16 x = (o & 0x7F);
			opcode(vm, x);
		}
	}

	return vm->err;
}

void rexlang_vm_error_ack(struct rexlang_vm *vm)
{
	// reset error state:
	vm->err = REXLANG_ERR_SUCCESS;
#ifndef NDEBUG
	vm->file = NULL;
	vm->line = 0;
#endif
}

void rexlang_vm_reset(struct rexlang_vm *vm)
{
	// reset IP and SP:
	vm->ip = 0;
	vm->sp = 224;
	// we do not clear program memory nor data memory.
	// clear stack:
	memset(vm->ki, 0, 224);
	// clear error status:
	rexlang_vm_error_ack(vm);
}

void rexlang_vm_init(
	struct rexlang_vm *vm,
	uint16_t m_size,
	uint8_t* m,
	uint16_t d_size,
	uint8_t* d,
	rexlang_call_f syscall,
	rexlang_call_f extcall
) {
	assert(vm && "vm cannot be NULL");
	assert(m && "m cannot be NULL");
	assert(d && "d cannot be NULL");

	vm->m = m;
	vm->m_size = m_size;
	vm->d = d;
	vm->d_size = d_size;

	vm->syscall = syscall;
	vm->extcall = extcall;

	rexlang_vm_reset(vm);
}
