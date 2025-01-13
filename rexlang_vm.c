
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

#define push(v) { \
	if (unlikely(vm->sp == 0)) { \
		vm->err = REXLANG_ERR_STACK_FULL; \
		goto error; \
	} \
 \
	vm->ki[--vm->sp] = v; \
}

#define pop(v) { \
	if (unlikely(vm->sp >= 128)) { \
		vm->err = REXLANG_ERR_STACK_EMPTY; \
		goto error; \
	} \
 \
	v = vm->ki[vm->sp++]; \
}

	u8 o = rdipu8(vm);
	u8 sz = o & 0xC0;
	if (sz == 0x00) {
		// no immediates; stack-only operations:
		switch (o) {
			case 0x00: // 00000000 halt
				vm->err = REXLANG_ERR_HALTED;
				return;
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
				pop(b);
				pop(a);
				op_binary(vm, o, a, b);
				break;
			case 0x0D: // 00001101 ld-u8
				pop(a);
				push(rddu8(vm, a));
				break;
			case 0x0E: // 00001110 ld-u16
				pop(a);
				push(rddu16(vm, a));
				break;
			case 0x0F: // 00001111 ld-u8-offs
				pop(b);
				pop(a);
				push(rddu8(vm, a+b));
				break;
			case 0x10: // 00010000 ld-u16-offs
				pop(b);
				pop(a);
				push(rddu16(vm, a+b));
				break;
			case 0x11: // 00010001 st-u8
				pop(b);
				pop(a);
				wrdu8(vm, a, b);
				push(b&0xFF);
				break;
			case 0x12: // 00010010 st-u16
				pop(b);
				pop(a);
				wrdu16(vm, a, b);
				push(b);
				break;
			case 0x13: // 00010011 st-u8-offs
				pop(c);
				pop(b);
				pop(a);
				wrdu8(vm, a+b, c);
				push(c&0xFF);
				break;
			case 0x14: // 00010100 st-u16-offs
				pop(c);
				pop(b);
				pop(a);
				wrdu16(vm, a+b, c);
				push(c);
				break;
			case 0x15: // 00010101 call
				pop(a);
				push(vm->ip);
				vm->ip = a;
				break;
			case 0x16: // 00010110 return / jump
				pop(a);
				vm->ip = a;
				break;
			case 0x17: // 00010111 jump-if
				pop(b);
				pop(a);
				if (b != 0) {
					vm->ip = a;
				}
				break;
			case 0x18: // 00011000 jump-if-not
				pop(b);
				pop(a);
				if (b == 0) {
					vm->ip = a;
				}
				break;
			case 0x19: // 00011001 jump-rel
				pop(a);
				push(vm->ip);
				vm->ip += (int16_t)a;
				break;
			case 0x1A: // 00011010 jump-rel-if
				pop(b);
				pop(a);
				if (b != 0) {
					vm->ip += (int16_t)a;
				}
				break;
			case 0x1B: // 00011011 jump-rel-if-not
				pop(b);
				pop(a);
				if (b == 0) {
					vm->ip += (int16_t)a;
				}
				break;
			case 0x1C: // 00011100 syscall
				pop(a);
				if (vm->syscall) {
					vm->syscall(vm, a);
				}
				break;
			case 0x1D: // 00011101 extcall
				pop(a);
				if (vm->extcall) {
					vm->extcall(vm, a);
				}
				break;
			case 0x1E: // 00011110 shl
				pop(b);
				pop(a);
				push(a << b);
				break;
			case 0x1F: // 00011111 shr
				pop(b);
				pop(a);
				push(a >> b);
				break;
			case 0x20: // 00100000 inc
				pop(a);
				push(a+1);
				break;
			case 0x21: // 00100001 dec
				pop(a);
				push(a-1);
				break;
			case 0x22: // 00100010 not
				pop(a);
				push(!a);
				break;
			case 0x23: // 00100011 neg
				pop(a);
				push(-a);
				break;
			case 0x24: // 00100100 discard
				pop(a);
				break;
			case 0x25: // 00100101 swap
				pop(b);
				pop(a);
				push(a);
				push(b);
				break;
			case 0x26: // 00100110 dcopy
				pop(c);
				pop(b);
				pop(a);
				bounds_check_data(vm, a+c-1);
				bounds_check_data(vm, b+c-1);
				memcpy(vm->d + a, vm->d + b, c);
				push(a + c);
				break;
			case 0x27: // 00100111 pcopy
				pop(c);
				pop(b);
				pop(a);
				bounds_check_data(vm, a+c-1);
				bounds_check_prgm(vm, b+c-1);
				memcpy(vm->d + a, vm->m + b, c);
				push(a + c);
				break;
			case 0x3B: // 00111011 nop
				break;
			case 0x3F:
				// 3C..3F: extended opcodes with dynamic size 1..4 bytes
				vm->ip += (o & 3) + 1;
				// reserved for future extension.
				break;
			default:
				vm->err = REXLANG_ERR_BAD_OPCODE;
				goto error;
		}
		return;
	}
	if (sz < 0xC0) {
		u16 x;
		if (o < 0x80) {
			// imm8
			x = rdipu8(vm);
		} else {
			// imm16
			x = rdipu16(vm);
		}
		switch (o & 0x3F) {
			case 0x00:
				// push-u8 / push-u16:
				push(x);
				break;
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
				pop(a);
				op_binary(vm, o & 0x1F, a, x);
				break;
			case 0x0D: // __001101_xxxxxxxx ld-u8--imm8/16
				push(rddu8(vm, x));
				break;
			case 0x0E: // __001110_xxxxxxxx ld-u16-imm8/16
				push(rddu16(vm, x));
				break;
			case 0x0F: // __001111_xxxxxxxx ld-u8--offs-imm8/16
				pop(a);
				push(rddu8(vm, x+a));
				break;
			case 0x10: // __010000_xxxxxxxx ld-u16-offs-imm8/16
				pop(a);
				push(rddu16(vm, x+a));
				break;
			case 0x11: // __010001_xxxxxxxx st-u8--imm8/16
				pop(a);
				wrdu8(vm, x, a);
				push(a&0xFF);
				break;
			case 0x12: // __010010_xxxxxxxx st-u16-imm8/16
				pop(a);
				wrdu16(vm, x, a);
				push(a);
				break;
			case 0x13: // __010011_xxxxxxxx st-u8--offs-imm8/16
				pop(b);
				pop(a);
				wrdu8(vm, x+a, b);
				push(b&0xFF);
				break;
			case 0x14: // __010100_xxxxxxxx st-u16-offs-imm8
				pop(b);
				pop(a);
				wrdu16(vm, x+a, b);
				push(b);
				break;
			case 0x15: // __010101_xxxxxxxx call-imm8
				push(vm->ip);
				vm->ip = x;
				break;
			case 0x16: // __010110_xxxxxxxx jump-imm8
				vm->ip = x;
				break;
			case 0x17: // __010111_xxxxxxxx jump-imm8-if
				pop(a);
				if (a != 0) {
					vm->ip = x;
				}
				break;
			case 0x18: // __011000_xxxxxxxx jump-imm8-if-not
				pop(a);
				if (a == 0) {
					vm->ip = x;
				}
				break;
			case 0x19: // __011001_xxxxxxxx jump-rel-imm8/16
				if (sz == 0x40) {
					vm->ip += (int16_t)(int8_t)x;
				} else {
					vm->ip += (int16_t)x;
				}
				break;
			case 0x1A: // __011010_xxxxxxxx jump-rel-imm8/16-if
				pop(a);
				if (a != 0) {
					if (sz == 0x40) {
						vm->ip += (int16_t)(int8_t)x;
					} else {
						vm->ip += (int16_t)x;
					}
				}
				break;
			case 0x1B: // __011011_xxxxxxxx jump-rel-imm8/16-if-not
				pop(a);
				if (a == 0) {
					if (sz == 0x40) {
						vm->ip += (int16_t)(int8_t)x;
					} else {
						vm->ip += (int16_t)x;
					}
				}
				break;
			case 0x1C: // __011100_xxxxxxxx syscall-imm8/16
				if (!vm->syscall) {
					vm->err = REXLANG_ERR_BAD_SYSCALL;
					goto error;
				}
				vm->syscall(vm, x);
				break;
			case 0x1D: // __011101_xxxxxxxx extcall-imm8/16
				if (!vm->extcall) {
					vm->err = REXLANG_ERR_BAD_EXTCALL;
					goto error;
				}
				vm->extcall(vm, x);
				break;
			case 0x1E: // 01011110_0000xxxx shl-imm4
				if (sz == 0x40) {
					pop(a);
					push(a << (x & 0x0F));
					break;
				} // else fallthrough;
			case 0x1F: // 01011111_0000xxxx shr-imm4
				if (sz == 0x40) {
					pop(a);
					push(a >> (x & 0x0F));
					break;
				} // else fallthrough;
			case 0x20: // 01100000_xxxxxxxx push-s8
				if (sz == 0x40) {
					push((int16_t)(int8_t)x);
					break;
				} // else fallthrough;
			default:
				vm->err = REXLANG_ERR_BAD_OPCODE;
				goto error;
		}
		return;
	}
	if (sz == 0xC0) {
		// push up to 4 values each of varying size:
		int x = (o & 3) + 1;
		u16 v;
		while (x--) {
			if (o & 4) {
				v = rdipu16(vm);
				push(v);
			} else {
				v = rdipu8(vm);
				push(v);
			}
			o >>= 1;
		}
		return;
	}

#undef pop
#undef push

	vm->err = REXLANG_ERR_BAD_OPCODE;

error:
	longjmp(vm->j, vm->err);
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
