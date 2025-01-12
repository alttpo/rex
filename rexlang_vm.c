
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "rexlang_vm_impl.h"

static void op_binary(struct rexlang_vm *vm, u8 op, u16 a, u16 b)
{
	switch (op) {
		case 0x01: // 0001 eq
			push(vm, a == b);
			break;
		case 0x02: // 0010 ne
			push(vm, a != b);
			break;
		case 0x03: // 0011 le
			push(vm, a <= b);
			break;
		case 0x04: // 0100 gt
			push(vm, a > b);
			break;
		case 0x05: // 0101 lt
			push(vm, a < b);
			break;
		case 0x06: // 0110 ge
			push(vm, a >= b);
			break;
		case 0x07: // 0111 and
			push(vm, a & b);
			break;
		case 0x08: // 1000 or
			push(vm, a | b);
			break;
		case 0x09: // 1001 xor
			push(vm, a ^ b);
			break;
		case 0x0A: // 1010 add
			push(vm, a + b);
			break;
		case 0x0B: // 1011 sub
			push(vm, a - b);
			break;
		case 0x0C: // 1100 mul
			push(vm, a * b);
			break;
		default:
			assert("bad developer!");
	}
}

static void opcode(struct rexlang_vm *vm)
{
	u16 a;
	u16 b;
	u16 c;

	bounds_check_prgm(vm, vm->ip);

	u8 o = rdipu8(vm);
	if (o == 0) { // 00000000 halt
		vm->err = REXLANG_ERR_HALTED;
		return;
	} else if (o < 0x40) {
		// no immediates; stack-only operations:
		if (o >= 0x01 && o <= 0x0C) {
			b = pop(vm);
			a = pop(vm);
			op_binary(vm, o, a, b);
		} else switch (o) {
			case 0x0D: // 00001101 ld-u8
				a = pop(vm);
				push(vm, rddu8(vm, a));
				break;
			case 0x0E: // 00001110 ld-u16
				a = pop(vm);
				push(vm, rddu16(vm, a));
				break;
			case 0x0F: // 00001111 ld-u8-offs
				b = pop(vm);
				a = pop(vm);
				push(vm, rddu8(vm, a+b));
				break;
			case 0x10: // 00010000 ld-u16-offs
				b = pop(vm);
				a = pop(vm);
				push(vm, rddu16(vm, a+b));
				break;
			case 0x11: // 00010001 st-u8
				b = pop(vm);
				a = pop(vm);
				wrdu8(vm, a, b);
				push(vm, b&0xFF);
				break;
			case 0x12: // 00010010 st-u16
				b = pop(vm);
				a = pop(vm);
				wrdu16(vm, a, b);
				push(vm, b);
				break;
			case 0x13: // 00010011 st-u8-offs
				c = pop(vm);
				b = pop(vm);
				a = pop(vm);
				wrdu8(vm, a+b, c);
				push(vm, c&0xFF);
				break;
			case 0x14: // 00010100 st-u16-offs
				c = pop(vm);
				b = pop(vm);
				a = pop(vm);
				wrdu16(vm, a+b, c);
				push(vm, c);
				break;
			case 0x15: // 00010101 call
				a = pop(vm);
				push(vm, vm->ip);
				vm->ip = a;
				break;
			case 0x16: // 00010110 return / jump
				a = pop(vm);
				vm->ip = a;
				break;
			case 0x17: // 00010111 jump-if
				b = pop(vm);
				a = pop(vm);
				if (b != 0) {
					vm->ip = a;
				}
				break;
			case 0x18: // 00011000 jump-if-not
				b = pop(vm);
				a = pop(vm);
				if (b == 0) {
					vm->ip = a;
				}
				break;
			case 0x19: // 00011001 syscall
				a = pop(vm);
				if (vm->syscall) {
					vm->syscall(vm, a);
				}
				break;
			case 0x1A: // 00011010 extcall
				a = pop(vm);
				if (vm->extcall) {
					vm->extcall(vm, a);
				}
				break;
			case 0x1B: // 00011011 shl
				b = pop(vm);
				a = pop(vm);
				push(vm, a << b);
				break;
			case 0x1C: // 00011100 shr
				b = pop(vm);
				a = pop(vm);
				push(vm, a >> b);
				break;
			case 0x1D: // 00011101 inc
				a = pop(vm);
				push(vm, a+1);
				break;
			case 0x1E: // 00011110 dec
				a = pop(vm);
				push(vm, a-1);
				break;
			case 0x1F: // 00011111 not
				a = pop(vm);
				push(vm, !a);
				break;
			case 0x20: // 00100000 neg
				a = pop(vm);
				push(vm, -a);
				break;
			case 0x21: // 00100001 discard
				a = pop(vm);
				break;
			case 0x22: // 00100010 swap
				b = pop(vm);
				a = pop(vm);
				push(vm, a);
				push(vm, b);
				break;
			case 0x23: // 00100011 dcopy
				c = pop(vm);
				b = pop(vm);
				a = pop(vm);
				bounds_check_data(vm, a+c-1);
				bounds_check_data(vm, b+c-1);
				memcpy(vm->d + a, vm->d + b, c);
				push(vm, a + c);
				break;
			case 0x24: // 00100100 pcopy
				c = pop(vm);
				b = pop(vm);
				a = pop(vm);
				bounds_check_data(vm, a+c-1);
				bounds_check_prgm(vm, b+c-1);
				memcpy(vm->d + a, vm->m + b, c);
				push(vm, a + c);
				break;
			case 0x3F: // 00111111 nop
				break;
			default:
				throw_error(vm, REXLANG_ERR_BAD_OPCODE);
		}
	} else if (o < 0x7C) {
		u16 x;
		if (o < 0x60) {
			// imm8
			x = rdipu8(vm);
		} else {
			// imm16
			x = rdipu16(vm);
		}
		switch (o & 0x1F) {
			case 0x01:
			case 0x02:
			case 0x03:
			case 0x04:
			case 0x05:
			case 0x06:
			case 0x07:
			case 0x08:
			case 0x09:
			case 0x0A:
			case 0x0B:
			case 0x0C:
				a = pop(vm);
				op_binary(vm, o & 0x1F, a, x);
				break;
			case 0x0D: // 01001101_xxxxxxxx ld-u8--imm8
				push(vm, rddu8(vm, x));
				break;
			case 0x0E: // 01001110_xxxxxxxx ld-u16-imm8
				push(vm, rddu16(vm, x));
				break;
			case 0x0F: // 01001111_xxxxxxxx ld-u8--offs-imm8
				a = pop(vm);
				push(vm, rddu8(vm, x+a));
				break;
			case 0x10: // 01010000_xxxxxxxx ld-u16-offs-imm8
				a = pop(vm);
				push(vm, rddu16(vm, x+a));
				break;
			case 0x11: // 01010001_xxxxxxxx st-u8--imm8
				a = pop(vm);
				wrdu8(vm, x, a);
				push(vm, a&0xFF);
				break;
			case 0x12: // 01010010_xxxxxxxx st-u16-imm8
				a = pop(vm);
				wrdu16(vm, x, a);
				push(vm, a);
				break;
			case 0x13: // 01010011_xxxxxxxx st-u8--offs-imm8
				b = pop(vm);
				a = pop(vm);
				wrdu8(vm, x+a, b);
				push(vm, b&0xFF);
				break;
			case 0x14: // 01010100_xxxxxxxx st-u16-offs-imm8
				b = pop(vm);
				a = pop(vm);
				wrdu16(vm, x+a, b);
				push(vm, b);
				break;
			case 0x15: // 01010101_xxxxxxxx call-imm8
				push(vm, vm->ip);
				vm->ip = x;
				break;
			case 0x16: // 01010110_xxxxxxxx jump-imm8
				vm->ip = x;
				break;
			case 0x17: // 01010111_xxxxxxxx jump-imm8-if
				a = pop(vm);
				if (a != 0) {
					vm->ip = x;
				}
				break;
			case 0x18: // 01011000_xxxxxxxx jump-imm8-if-not
				a = pop(vm);
				if (a == 0) {
					vm->ip = x;
				}
				break;
			case 0x19: // 01011001_xxxxxxxx syscall-imm8
				if (!vm->syscall) {
					throw_error(vm, REXLANG_ERR_BAD_SYSCALL);
				}
				vm->syscall(vm, x);
				break;
			case 0x1A: // 01011010_xxxxxxxx extcall-imm8
				if (!vm->extcall) {
					throw_error(vm, REXLANG_ERR_BAD_EXTCALL);
				}
				vm->extcall(vm, x);
				break;
			case 0x1B: // 01011011_0000xxxx shl-imm4
				if (o < 0x60) {
					a = pop(vm);
					push(vm, a << (x & 0x0F));
					break;
				} // else fallthrough;
			case 0x1C: // 01011100_0000xxxx shr-imm4
				if (o < 0x60) {
					a = pop(vm);
					push(vm, a >> (x & 0x0F));
					break;
				} // else fallthrough;
			default:
				throw_error(vm, REXLANG_ERR_BAD_OPCODE);
		}
	} else if (o < 0x80) {
		// 7C..7F: extended opcodes with dynamic size 1..4 bytes
		vm->ip += (o & 3) + 1;
		// reserved for future extension.
	} else if ((o & 0xC0) == 0x80) {
		// push u8 value:
		push(vm, o & 0x3F);
	} else if ((o & 0xC0) == 0xC0) {
		// push up to 4 values each of varying size:
		int x = (o & 3) + 1;
		u16 v;
		while (x--) {
			if (o & 4) {
				v = rdipu16(vm);
				push(vm, v);
			} else {
				v = rdipu8(vm);
				push(vm, v);
			}
			o >>= 1;
		}
	} else {
		throw_error(vm, REXLANG_ERR_BAD_OPCODE);
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
		// decode and execute the next opcode:
		opcode(vm);
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
	vm->sp = 128;
	// we do not clear program memory nor data memory.
	// clear stack:
	memset(vm->ki, 0, sizeof(uint16_t)*128);
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
