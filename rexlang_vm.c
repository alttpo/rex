
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <setjmp.h>
#include "rexlang_vm_impl.h"

static void opcode(struct rexlang_vm *vm)
{
	u32 a;
	u32 b;
	u32 c;
	s32 sa;

	bounds_check_prgm(vm, vm->ip);

// the `goto error` pattern significantly reduces redundant branch targets
// compared with inlined push()/pop() calls, when using arm-none-eabi-gcc v13.3.1
#define push(v) { \
	if (unlikely(vm->sp == 0)) { \
		goto error_stack_full; \
	} \
 \
	vm->ki[--vm->sp] = v; \
}

#define pop(v) { \
	if (unlikely(vm->sp >= REXLANG_STACKSZ)) { \
		goto error_stack_empty; \
	} \
 \
	v = vm->ki[vm->sp++]; \
}

	u8 o = rdipu8(vm);
	switch (o) {
		// no immediates; stack-only operations:
		case 0x00: // halt
			vm->err = REXLANG_ERR_HALTED;
			break;
		case 0x01: // nop
			break;

		case 0x02: // eq
			pop(a);
			pop(b);
		impl_eq:
			push(b == a);
			break;
		case 0x03: // ne
			pop(a);
			pop(b);
		impl_ne:
			push(b != a);
			break;
		case 0x04: // le-ui
			pop(a);
			pop(b);
		impl_le_ui:
			push(b <= a);
			break;
		case 0x05: // le-si
			pop(sa);
			pop(b);
		impl_le_si:
			push((s32)b <= sa);
			break;
		case 0x06: // gt-ui
			pop(a);
			pop(b);
		impl_gt_ui:
			push(b > a);
			break;
		case 0x07: // gt-si
			pop(sa);
			pop(b);
		impl_gt_si:
			push((s32)b > sa);
			break;
		case 0x08: // lt-ui
			pop(a);
			pop(b);
		impl_lt_ui:
			push(b < a);
			break;
		case 0x09: // lt-si
			pop(sa);
			pop(b);
		impl_lt_si:
			push((s32)b < sa);
			break;
		case 0x0A: // ge-ui
			pop(a);
			pop(b);
		impl_ge_ui:
			push(b >= a);
			break;
		case 0x0B: // ge-si
			pop(sa);
			pop(b);
		impl_ge_si:
			push((s32)b >= sa);
			break;
		case 0x0C: // and
			pop(a);
			pop(b);
		impl_and:
			push(b & a);
			break;
		case 0x0D: // or
			pop(a);
			pop(b);
		impl_or:
			push(b | a);
			break;
		case 0x0E: // xor
			pop(a);
			pop(b);
		impl_xor:
			push(b ^ a);
			break;
		case 0x0F: // add
			pop(a);
			pop(b);
		impl_add:
			push(b + a);
			break;
		case 0x10: // sub
			pop(a);
			pop(b);
		impl_sub:
			push(b - a);
			break;
		case 0x11: // mul
			pop(a);
			pop(b);
		impl_mul:
			push(b * a);
			break;

		case 0x12: // ld-u8
			pop(a);
		impl_ld_u8:
			push(rddu8(vm, a));
			break;
		case 0x13: // ld-u16
			pop(a);
		impl_ld_u16:
			push(rddu16(vm, a));
			break;
		case 0x14: // ld-u32
			pop(a);
		impl_u32:
			push(rddu32(vm, a));
			break;
		case 0x15: // ld-u8-offs
			pop(a);
			pop(b);
		impl_ld_u8_offs:
			push(rddu8(vm, b+a));
			break;
		case 0x16: // ld-u16-offs
			pop(a);
			pop(b);
		impl_ld_u16_offs:
			push(rddu16(vm, b+a));
			break;
		case 0x17: // ld-u32-offs
			pop(a);
			pop(b);
		impl_ld_u32_offs:
			push(rddu32(vm, b+a));
			break;
		case 0x18: // ld-s8
			pop(a);
		impl_ld_s8:
			push((s8)rddu8(vm, a));
			break;
		case 0x19: // ld-s16
			pop(a);
		impl_ld_s16:
			push((s16)rddu16(vm, a));
			break;
		case 0x1A: // ld-s8-offs
			pop(a);
			pop(b);
		impl_ld_s8_offs:
			push((s8)rddu8(vm, b+a));
			break;
		case 0x1B: // ld-s16-offs
			pop(a);
			pop(b);
		impl_ld_s16_offs:
			push((s16)rddu16(vm, b+a));
			break;
		case 0x1C: // st-u8
			pop(a);
			pop(b);
		impl_st_u8:
			wrdu8(vm, a, b);
			push(b);
			break;
		case 0x1D: // st-u16
			pop(a);
			pop(b);
		impl_st_u16:
			wrdu16(vm, a, b);
			push(b);
			break;
		case 0x1E: // st-u32
			pop(a);
			pop(b);
		impl_st_u32:
			wrdu32(vm, a, b);
			push(b);
			break;
		case 0x1F: // st-u8-offs
			pop(a);
			pop(b);
			pop(c);
		impl_st_u8_offs:
			wrdu8(vm, b+a, c);
			push(c);
			break;
		case 0x20: // st-u16-offs
			pop(a);
			pop(b);
			pop(c);
		impl_st_u16_offs:
			wrdu16(vm, b+a, c);
			push(c);
			break;
		case 0x21: // st-u32-offs
			pop(a);
			pop(b);
			pop(c);
		impl_st_u32_offs:
			wrdu32(vm, b+a, c);
			push(c);
			break;
		case 0x22: // st-u8--discard
			pop(a);
			pop(b);
		impl_st_u8_discard:
			wrdu8(vm, a, b);
			break;
		case 0x23: // st-u16-discard
			pop(a);
			pop(b);
		impl_st_u16_discard:
			wrdu16(vm, a, b);
			break;
		case 0x24: // st-u32-discard
			pop(a);
			pop(b);
		impl_st_u32_discard:
			wrdu32(vm, a, b);
			break;
		case 0x25: // st-u8-offs-discard
			pop(a);
			pop(b);
			pop(c);
		impl_st_u8_offs_discard:
			wrdu8(vm, b+a, c);
			break;
		case 0x26: // st-u16-offs-discard
			pop(a);
			pop(b);
			pop(c);
		impl_st_u16_offs_discard:
			wrdu16(vm, b+a, c);
			break;
		case 0x27: // st-u32-offs-discard
			pop(a);
			pop(b);
			pop(c);
		impl_st_u32_offs_discard:
			wrdu32(vm, b+a, c);
			break;
		case 0x28: // call
			pop(a);
		impl_call:
			push(vm->ip);
			vm->ip = a;
			break;
		case 0x29: // return / jump-abs
			pop(a);
		impl_jump_abs:
			vm->ip = a;
			break;
		case 0x2A: // jump-abs-if
			pop(a);
			pop(b);
		impl_jump_abs_if:
			if (b != 0) {
				vm->ip = a;
			}
			break;
		case 0x2B: // jump-abs-if-not
			pop(a);
			pop(b);
		impl_jump_abs_if_not:
			if (b == 0) {
				vm->ip = a;
			}
			break;
		case 0x2C: // jump-rel
			pop(sa);
		impl_jump_rel:
			push(vm->ip);
			vm->ip += sa;
			break;
		case 0x2D: // jump-rel-if
			pop(sa);
			pop(b);
		impl_jump_rel_if:
			if (b != 0) {
				vm->ip += sa;
			}
			break;
		case 0x2E: // jump-rel-if-not
			pop(sa);
			pop(b);
		impl_jump_rel_if_not:
			if (b == 0) {
				vm->ip += sa;
			}
			break;
		case 0x2F: // syscall
			pop(a);
		impl_syscall:
			if (!vm->syscall) {
				vm->err = REXLANG_ERR_BAD_SYSCALL;
				goto error;
			}
			vm->syscall(vm, a);
			break;

		case 0x30: // shl
			pop(a);
			pop(b);
		impl_shl:
			push(b << a);
			break;
		case 0x31: // shr
			pop(a);
			pop(b);
		impl_shr:
			push(b >> a);
			break;

		case 0x39: // not
			pop(a);
			push(!a);
			break;
		case 0x3A: // neg
			pop(a);
			push(-a);
			break;
		case 0x3B: // discard
			pop(a);
			break;
		case 0x3C: // swap
			pop(a);
			pop(b);
			push(a);
			push(b);
			break;
		case 0x3D: // dup
			pop(a);
			push(a);
			push(a);
			break;
		case 0x3E: // dcopy
			pop(a);
			pop(b);
			pop(c);
			bounds_check_data(vm, c+a-1);
			bounds_check_data(vm, b+a-1);
			memcpy(vm->d + c, vm->d + b, a);
			push(c + a);
			break;
		case 0x3F: // pcopy
			pop(a);
			pop(b);
			pop(c);
			bounds_check_data(vm, c+a-1);
			bounds_check_prgm(vm, b+a-1);
			memcpy(vm->d + c, vm->m + b, a);
			push(c + a);
			break;

		// 0x40..0x7F:
		case 0x40: // push-u8
			push(rdipu8(vm));
			break;
		case 0x41: // push-s8
			push((s8)rdipu8(vm));
			break;

		case 0x42: // eq
			a = rdipu8(vm);
			pop(b);
			goto impl_eq;
		case 0x43: // ne
			a = rdipu8(vm);
			pop(b);
			goto impl_ne;
		case 0x44: // le-ui
			a = rdipu8(vm);
			pop(b);
			goto impl_le_ui;
		case 0x45: // le-si
			sa = (s8)rdipu8(vm);
			pop(b);
			goto impl_le_si;
		case 0x46: // gt-ui
			a = rdipu8(vm);
			pop(b);
			goto impl_gt_ui;
		case 0x47: // gt-si
			sa = (s8)rdipu8(vm);
			pop(b);
			goto impl_gt_si;
		case 0x48: // lt-ui
			a = rdipu8(vm);
			pop(b);
			goto impl_lt_ui;
		case 0x49: // lt-si
			sa = (s8)rdipu8(vm);
			pop(b);
			goto impl_lt_si;
		case 0x4A: // ge-ui
			a = rdipu8(vm);
			pop(b);
			goto impl_ge_ui;
		case 0x4B: // ge-si
			sa = (s8)rdipu8(vm);
			pop(b);
			goto impl_ge_si;
		case 0x4C: // and
			a = rdipu8(vm);
			pop(b);
			goto impl_and;
		case 0x4D: // or
			a = rdipu8(vm);
			pop(b);
			goto impl_or;
		case 0x4E: // xor
			a = rdipu8(vm);
			pop(b);
			goto impl_xor;
		case 0x4F: // add
			a = rdipu8(vm);
			pop(b);
			goto impl_add;
		case 0x50: // sub
			a = rdipu8(vm);
			pop(b);
			goto impl_sub;
		case 0x51: // mul
			a = rdipu8(vm);
			pop(b);
			goto impl_mul;

		case 0x52: // ld-u8
			a = rdipu8(vm);
			goto impl_ld_u8;
		case 0x53: // ld-u16
			a = rdipu8(vm);
			goto impl_ld_u16;
		case 0x54: // ld-u32
			a = rdipu8(vm);
			goto impl_u32;
		case 0x55: // ld-u8-offs
			pop(a);
			b = rdipu8(vm);
			goto impl_ld_u8_offs;
		case 0x56: // ld-u16-offs
			pop(a);
			b = rdipu8(vm);
			goto impl_ld_u16_offs;
		case 0x57: // ld-u32-offs
			pop(a);
			b = rdipu8(vm);
			goto impl_ld_u32_offs;
		case 0x58: // ld-s8
			a = rdipu8(vm);
			goto impl_ld_s8;
		case 0x59: // ld-s16
			a = rdipu8(vm);
			goto impl_ld_s16;
		case 0x5A: // ld-s8-offs
			pop(a);
			b = rdipu8(vm);
			goto impl_ld_s8_offs;
		case 0x5B: // ld-s16-offs
			pop(a);
			b = rdipu8(vm);
			goto impl_ld_s16_offs;
		case 0x5C: // st-u8
			a = rdipu8(vm);
			pop(b);
			goto impl_st_u8;
		case 0x5D: // st-u16
			a = rdipu8(vm);
			pop(b);
			goto impl_st_u16;
		case 0x5E: // st-u32
			a = rdipu8(vm);
			pop(b);
			goto impl_st_u32;
		case 0x5F: // st-u8-offs
			pop(a);
			b = rdipu8(vm);
			pop(c);
			goto impl_st_u8_offs;
		case 0x60: // st-u16-offs
			pop(a);
			b = rdipu8(vm);
			pop(c);
			goto impl_st_u16_offs;
		case 0x61: // st-u32-offs
			pop(a);
			b = rdipu8(vm);
			pop(c);
			goto impl_st_u32_offs;
		case 0x62: // st-u8--discard
			a = rdipu8(vm);
			pop(b);
			goto impl_st_u8_discard;
		case 0x63: // st-u16-discard
			a = rdipu8(vm);
			pop(b);
			goto impl_st_u16_discard;
		case 0x64: // st-u32-discard
			a = rdipu8(vm);
			pop(b);
			goto impl_st_u32_discard;
		case 0x65: // st-u8-offs-discard
			pop(a);
			b = rdipu8(vm);
			pop(c);
			goto impl_st_u8_offs_discard;
		case 0x66: // st-u16-offs-discard
			pop(a);
			b = rdipu8(vm);
			pop(c);
			goto impl_st_u16_offs_discard;
		case 0x67: // st-u32-offs-discard
			pop(a);
			b = rdipu8(vm);
			pop(c);
			goto impl_st_u32_offs_discard;
		case 0x68: // call
			a = rdipu8(vm);
			goto impl_call;
		case 0x69: // return / jump-abs
			a = rdipu8(vm);
			goto impl_jump_abs;
		case 0x6A: // jump-abs-if
			a = rdipu8(vm);
			pop(b);
			goto impl_jump_abs_if;
		case 0x6B: // jump-abs-if-not
			a = rdipu8(vm);
			pop(b);
			goto impl_jump_abs_if_not;
		case 0x6C: // jump-rel
			sa = (s8)rdipu8(vm);
			goto impl_jump_rel;
		case 0x6D: // jump-rel-if
			sa = (s8)rdipu8(vm);
			pop(b);
			goto impl_jump_rel_if;
		case 0x6E: // jump-rel-if-not
			sa = (s8)rdipu8(vm);
			pop(b);
			goto impl_jump_rel_if_not;
		case 0x6F: // syscall
			a = rdipu8(vm);
			goto impl_syscall;
		case 0x70: // shl
			a = rdipu8(vm);
			pop(b);
			push(b << a);
			break;
		case 0x71: // shr
			a = rdipu8(vm);
			pop(b);
			push(b >> a);
			break;

		// 0x80..0xBF:
		case 0x80: // push-u16
			push(rdipu16(vm));
			break;
		case 0x81: // push-s16
			push((s16)rdipu16(vm));
			break;

		case 0x82: // eq
			a = rdipu16(vm);
			pop(b);
			goto impl_eq;
		case 0x83: // ne
			a = rdipu16(vm);
			pop(b);
			goto impl_ne;
		case 0x84: // le-ui
			a = rdipu16(vm);
			pop(b);
			goto impl_le_ui;
		case 0x85: // le-si
			sa = (s16)rdipu16(vm);
			pop(b);
			goto impl_le_si;
		case 0x86: // gt-ui
			a = rdipu16(vm);
			pop(b);
			goto impl_gt_ui;
		case 0x87: // gt-si
			sa = (s16)rdipu16(vm);
			pop(b);
			goto impl_gt_si;
		case 0x88: // lt-ui
			a = rdipu16(vm);
			pop(b);
			goto impl_lt_ui;
		case 0x89: // lt-si
			sa = (s16)rdipu16(vm);
			pop(b);
			goto impl_lt_si;
		case 0x8A: // ge-ui
			a = rdipu16(vm);
			pop(b);
			goto impl_ge_ui;
		case 0x8B: // ge-si
			sa = (s16)rdipu16(vm);
			pop(b);
			goto impl_ge_si;
		case 0x8C: // and
			a = rdipu16(vm);
			pop(b);
			goto impl_and;
		case 0x8D: // or
			a = rdipu16(vm);
			pop(b);
			goto impl_or;
		case 0x8E: // xor
			a = rdipu16(vm);
			pop(b);
			goto impl_xor;
		case 0x8F: // add
			a = rdipu16(vm);
			pop(b);
			goto impl_add;
		case 0x90: // sub
			a = rdipu16(vm);
			pop(b);
			goto impl_sub;
		case 0x91: // mul
			a = rdipu16(vm);
			pop(b);
			goto impl_mul;

		case 0x92: // ld-u8
			a = rdipu16(vm);
			goto impl_ld_u8;
		case 0x93: // ld-u16
			a = rdipu16(vm);
			goto impl_ld_u16;
		case 0x94: // ld-u32
			a = rdipu16(vm);
			goto impl_u32;
		case 0x95: // ld-u8-offs
			pop(a);
			b = rdipu16(vm);
			goto impl_ld_u8_offs;
		case 0x96: // ld-u16-offs
			pop(a);
			b = rdipu16(vm);
			goto impl_ld_u16_offs;
		case 0x97: // ld-u32-offs
			pop(a);
			b = rdipu16(vm);
			goto impl_ld_u32_offs;
		case 0x98: // ld-s8
			a = rdipu16(vm);
			goto impl_ld_s8;
		case 0x99: // ld-s16
			a = rdipu16(vm);
			goto impl_ld_s16;
		case 0x9A: // ld-s8-offs
			pop(a);
			b = rdipu16(vm);
			goto impl_ld_s8_offs;
		case 0x9B: // ld-s16-offs
			pop(a);
			b = rdipu16(vm);
			goto impl_ld_s16_offs;
		case 0x9C: // st-u8
			a = rdipu16(vm);
			pop(b);
			goto impl_st_u8;
		case 0x9D: // st-u16
			a = rdipu16(vm);
			pop(b);
			goto impl_st_u16;
		case 0x9E: // st-u32
			a = rdipu16(vm);
			pop(b);
			goto impl_st_u32;
		case 0x9F: // st-u8-offs
			pop(a);
			b = rdipu16(vm);
			pop(c);
			goto impl_st_u8_offs;
		case 0xA0: // st-u16-offs
			pop(a);
			b = rdipu16(vm);
			pop(c);
			goto impl_st_u16_offs;
		case 0xA1: // st-u32-offs
			pop(a);
			b = rdipu16(vm);
			pop(c);
			goto impl_st_u32_offs;
		case 0xA2: // st-u8--discard
			a = rdipu16(vm);
			pop(b);
			goto impl_st_u8_discard;
		case 0xA3: // st-u16-discard
			a = rdipu16(vm);
			pop(b);
			goto impl_st_u16_discard;
		case 0xA4: // st-u32-discard
			a = rdipu16(vm);
			pop(b);
			goto impl_st_u32_discard;
		case 0xA5: // st-u8-offs-discard
			pop(a);
			b = rdipu16(vm);
			pop(c);
			goto impl_st_u8_offs_discard;
		case 0xA6: // st-u16-offs-discard
			pop(a);
			b = rdipu16(vm);
			pop(c);
			goto impl_st_u16_offs_discard;
		case 0xA7: // st-u32-offs-discard
			pop(a);
			b = rdipu16(vm);
			pop(c);
			goto impl_st_u32_offs_discard;
		case 0xA8: // call
			a = rdipu16(vm);
			goto impl_call;
		case 0xA9: // return / jump-abs
			a = rdipu16(vm);
			goto impl_jump_abs;
		case 0xAA: // jump-abs-if
			a = rdipu16(vm);
			pop(b);
			goto impl_jump_abs_if;
		case 0xAB: // jump-abs-if-not
			a = rdipu16(vm);
			pop(b);
			goto impl_jump_abs_if_not;
		case 0xAC: // jump-rel
			sa = (s16)rdipu16(vm);
			goto impl_jump_rel;
		case 0xAD: // jump-rel-if
			sa = (s16)rdipu16(vm);
			pop(b);
			goto impl_jump_rel_if;
		case 0xAE: // jump-rel-if-not
			sa = (s16)rdipu16(vm);
			pop(b);
			goto impl_jump_rel_if_not;
		case 0xAF: // syscall
			a = rdipu16(vm);
			goto impl_syscall;

		// 0xC0..0xFF:
		case 0xC0: // push-u32
			push(rdipu32(vm));
			break;
		case 0xC1: // push-s32
			push((s32)rdipu32(vm));
			break;

		case 0xC2: // eq
			a = rdipu32(vm);
			pop(b);
			goto impl_eq;
		case 0xC3: // ne
			a = rdipu32(vm);
			pop(b);
			goto impl_ne;
		case 0xC4: // le-ui
			a = rdipu32(vm);
			pop(b);
			goto impl_le_ui;
		case 0xC5: // le-si
			sa = (s32)rdipu32(vm);
			pop(b);
			goto impl_le_si;
		case 0xC6: // gt-ui
			a = rdipu32(vm);
			pop(b);
			goto impl_gt_ui;
		case 0xC7: // gt-si
			sa = (s32)rdipu32(vm);
			pop(b);
			goto impl_gt_si;
		case 0xC8: // lt-ui
			a = rdipu32(vm);
			pop(b);
			goto impl_lt_ui;
		case 0xC9: // lt-si
			sa = (s32)rdipu32(vm);
			pop(b);
			goto impl_lt_si;
		case 0xCA: // ge-ui
			a = rdipu32(vm);
			pop(b);
			goto impl_ge_ui;
		case 0xCB: // ge-si
			sa = (s32)rdipu32(vm);
			pop(b);
			goto impl_ge_si;
		case 0xCC: // and
			a = rdipu32(vm);
			pop(b);
			goto impl_and;
		case 0xCD: // or
			a = rdipu32(vm);
			pop(b);
			goto impl_or;
		case 0xCE: // xor
			a = rdipu32(vm);
			pop(b);
			goto impl_xor;
		case 0xCF: // add
			a = rdipu32(vm);
			pop(b);
			goto impl_add;
		case 0xD0: // sub
			a = rdipu32(vm);
			pop(b);
			goto impl_sub;
		case 0xD1: // mul
			a = rdipu32(vm);
			pop(b);
			goto impl_mul;

		case 0xD2: // ld-u8
			a = rdipu32(vm);
			goto impl_ld_u8;
		case 0xD3: // ld-u16
			a = rdipu32(vm);
			goto impl_ld_u16;
		case 0xD4: // ld-u32
			a = rdipu32(vm);
			goto impl_u32;
		case 0xD5: // ld-u8-offs
			pop(a);
			b = rdipu32(vm);
			goto impl_ld_u8_offs;
		case 0xD6: // ld-u16-offs
			pop(a);
			b = rdipu32(vm);
			goto impl_ld_u16_offs;
		case 0xD7: // ld-u32-offs
			pop(a);
			b = rdipu32(vm);
			goto impl_ld_u32_offs;
		case 0xD8: // ld-s8
			a = rdipu32(vm);
			goto impl_ld_s8;
		case 0xD9: // ld-s16
			a = rdipu32(vm);
			goto impl_ld_s16;
		case 0xDA: // ld-s8-offs
			pop(a);
			b = rdipu32(vm);
			goto impl_ld_s8_offs;
		case 0xDB: // ld-s16-offs
			pop(a);
			b = rdipu32(vm);
			goto impl_ld_s16_offs;
		case 0xDC: // st-u8
			a = rdipu32(vm);
			pop(b);
			goto impl_st_u8;
		case 0xDD: // st-u16
			a = rdipu32(vm);
			pop(b);
			goto impl_st_u16;
		case 0xDE: // st-u32
			a = rdipu32(vm);
			pop(b);
			goto impl_st_u32;
		case 0xDF: // st-u8-offs
			pop(a);
			b = rdipu32(vm);
			pop(c);
			goto impl_st_u8_offs;
		case 0xE0: // st-u16-offs
			pop(a);
			b = rdipu32(vm);
			pop(c);
			goto impl_st_u16_offs;
		case 0xE1: // st-u32-offs
			pop(a);
			b = rdipu32(vm);
			pop(c);
			goto impl_st_u32_offs;
		case 0xE2: // st-u8--discard
			a = rdipu32(vm);
			pop(b);
			goto impl_st_u8_discard;
		case 0xE3: // st-u16-discard
			a = rdipu32(vm);
			pop(b);
			goto impl_st_u16_discard;
		case 0xE4: // st-u32-discard
			a = rdipu32(vm);
			pop(b);
			goto impl_st_u32_discard;
		case 0xE5: // st-u8-offs-discard
			pop(a);
			b = rdipu32(vm);
			pop(c);
			goto impl_st_u8_offs_discard;
		case 0xE6: // st-u16-offs-discard
			pop(a);
			b = rdipu32(vm);
			pop(c);
			goto impl_st_u16_offs_discard;
		case 0xE7: // st-u32-offs-discard
			pop(a);
			b = rdipu32(vm);
			pop(c);
			goto impl_st_u32_offs_discard;
		case 0xE8: // call
			a = rdipu32(vm);
			goto impl_call;
		case 0xE9: // return / jump-abs
			a = rdipu32(vm);
			goto impl_jump_abs;
		case 0xEA: // jump-abs-if
			a = rdipu32(vm);
			pop(b);
			goto impl_jump_abs_if;
		case 0xEB: // jump-abs-if-not
			a = rdipu32(vm);
			pop(b);
			goto impl_jump_abs_if_not;
		case 0xEC: // jump-rel
			sa = (s32)rdipu32(vm);
			goto impl_jump_rel;
		case 0xED: // jump-rel-if
			sa = (s32)rdipu32(vm);
			pop(b);
			goto impl_jump_rel_if;
		case 0xEE: // jump-rel-if-not
			sa = (s32)rdipu32(vm);
			pop(b);
			goto impl_jump_rel_if_not;
		case 0xEF: // syscall
			a = rdipu32(vm);
			goto impl_syscall;

		default:
			vm->err = REXLANG_ERR_BAD_OPCODE;
			goto error;
	}
	return;

#undef pop
#undef push


error_stack_empty:
	vm->err = REXLANG_ERR_STACK_EMPTY;
	goto error;
error_stack_full:
	vm->err = REXLANG_ERR_STACK_FULL;

error:
	longjmp(vm->j, vm->err);
}

enum rexlang_error rexlang_vm_exec(struct rexlang_vm *vm, unsigned int instruction_count)
{
	assert(vm->m);

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
	vm->sp = REXLANG_STACKSZ;
	// we do not clear program memory nor data memory.
	// clear stack:
	memset(vm->ki, 0, sizeof(uint32_t)*REXLANG_STACKSZ);
	// clear error status:
	rexlang_vm_error_ack(vm);
}

void rexlang_vm_init(
	struct rexlang_vm *vm,
	uint32_t m_size,
	const uint8_t* m,
	uint32_t d_size,
	uint8_t* d,
	rexlang_call_f syscall
) {
	assert(vm && "vm cannot be NULL");
	assert(m && "m cannot be NULL");
	assert(d && "d cannot be NULL");

	vm->m = m;
	vm->m_size = m_size;
	vm->d = d;
	vm->d_size = d_size;

	vm->syscall = syscall;

	rexlang_vm_reset(vm);
}
