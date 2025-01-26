
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rexlang_vm.h"
#include "rexlang_vm_impl.h"

uint32_t chip_addr[0x40];

void syscall(struct rexlang_vm* vm, uint32_t fn) {
    switch (fn) {
    case 0x0000: { // chip-set-addr
        u32 addr = pop(vm);
        u32 chip = pop(vm);
        if (chip >= 0x40) {
            throw_error(vm, REXLANG_ERR_CALL_ARG_OUT_OF_RANGE);
        }
        chip_addr[chip & 0x3F] = addr;
        break;
    }

    default:
        throw_error(vm, REXLANG_ERR_BAD_SYSCALL);
        break;
    }
}

struct test_t {
    const char *name;   // name of test
    uint8_t prgm[64];   // test rexlang program

    enum rexlang_error check_error;
    int         check_stack_count;      // number of stack items to check
    uint32_t    check_stack_values[4];  // values of stack items to check

    int (*check_fn)(struct rexlang_vm* vm, char* msg);
};

#define VALUE_TO_STRING(x) #x
#define VALUE(x) VALUE_TO_STRING(x)
#define expect(expected_expr, actual_expr, msg_out) { \
    uint32_t expected = expected_expr; \
    uint32_t actual = actual_expr; \
    if (actual != expected) { \
        sprintf((msg_out), " expected %08X, got %08X", expected, actual); \
        return 1; \
    } \
}

#include "test_cases.h"

int exec_test(const struct test_t *t, char* msg) {
    enum rexlang_error err;
    struct rexlang_vm vm;
    uint8_t data[256] = {0};

    rexlang_vm_init(&vm, 64, t->prgm, 256, data, syscall);
    err = rexlang_vm_exec(&vm, 1024);

    if (err != t->check_error) {
        sprintf(msg, "error expected %d, got %d", t->check_error, err);
        return 1;
    }

    for (int i = 0; i < t->check_stack_count; i++) {
        int n = sprintf(msg, "stack[%d]", i);
        expect(t->check_stack_values[i], vm.ki[REXLANG_DATA_STACKSZ - i - 1], msg+n);
    }

    if (t->check_fn) {
        int ret = t->check_fn(&vm, msg);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

typedef uint32_t (*rexlang_eval_fn)(uint8_t opcode, uint32_t b, uint32_t a);

void push_ui(uint8_t** p, uint32_t a) {
    if (a <= UINT8_MAX) {
        *(*p)++ = 0b01000000; // push-u8
        *(*p)++ = (uint8_t)a;
    } else if (a <= UINT16_MAX) {
        *(*p)++ = 0b10000000; // push-u16
        *(*p)++ = (uint8_t)a;
        *(*p)++ = (uint8_t)(a >> 8);
    } else {
        *(*p)++ = 0b11000000; // push-u32
        *(*p)++ = (uint8_t)a;
        *(*p)++ = (uint8_t)(a >> 8);
        *(*p)++ = (uint8_t)(a >> 16);
        *(*p)++ = (uint8_t)(a >> 24);
    }
}

void push_si(uint8_t** p, int32_t a) {
    if (a >= INT8_MIN && a <= INT8_MAX) {
        *(*p)++ = 0b01000001; // push-s8
        *(*p)++ = (uint8_t)(uint32_t)a;
    } else if (a >= INT16_MIN && a <= INT16_MAX) {
        *(*p)++ = 0b10000001; // push-s16
        *(*p)++ = (uint8_t)(uint32_t)a;
        *(*p)++ = (uint8_t)((uint32_t)a >> 8);
    } else {
        *(*p)++ = 0b11000001; // push-s32
        *(*p)++ = (uint8_t)(uint32_t)a;
        *(*p)++ = (uint8_t)((uint32_t)a >> 8);
        *(*p)++ = (uint8_t)((uint32_t)a >> 16);
        *(*p)++ = (uint8_t)((uint32_t)a >> 24);
    }
}

int automate_test_ui(
    const char *name,
    uint8_t opcode,
    uint32_t a_min,
    uint32_t a_max,
    uint32_t b_min,
    uint32_t b_max
) {
    char msg[256] = {0};

    uint32_t a = a_min;
    do {
        uint32_t b = b_min;
        do {
            struct test_t t = {0};
            uint8_t* p;
            uint32_t result = rexlang_pure_eval(opcode, a, b);

            t.check_stack_values[t.check_stack_count++] = result;
            t.check_error = REXLANG_ERR_HALTED;

            {
                // stack-only opcode test:
                p = &t.prgm[0];
                push_ui(&p, a);
                push_ui(&p, b);
                *p++ = opcode;
                *p++ = 0; // halt
                printf("executing test: (%10u %5s %10u) == %10u // stack\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }

            if (b <= UINT8_MAX) {
                // imm8 opcode test:
                p = &t.prgm[0];
                push_ui(&p, a);
                *p++ = opcode + 0x40;
                *p++ = (uint8_t)b;
                *p++ = 0; // halt
                printf("executing test: (%10u %5s %10u) == %10u // imm8\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }

            if (b <= UINT16_MAX) {
                // imm16 opcode test:
                p = &t.prgm[0];
                push_ui(&p, a);
                *p++ = opcode + 0x80;
                *p++ = (uint8_t)(uint32_t)b;
                *p++ = (uint8_t)((uint32_t)b >> 8);
                *p++ = 0; // halt
                printf("executing test: (%10u %5s %10u) == %10u // imm16\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }

            {
                // imm32 opcode test:
                p = &t.prgm[0];
                push_ui(&p, a);
                *p++ = opcode + 0xC0;
                *p++ = (uint8_t)(uint32_t)b;
                *p++ = (uint8_t)((uint32_t)b >> 8);
                *p++ = (uint8_t)((uint32_t)b >> 16);
                *p++ = (uint8_t)((uint32_t)b >> 24);
                *p++ = 0; // halt
                printf("executing test: (%10u %5s %10u) == %10u // imm32\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }
        } while (b++ != b_max);
    } while (a++ != a_max);

    return 0;
}

int automate_test_si(
    const char *name,
    uint8_t opcode,
    int32_t a_min,
    int32_t a_max,
    int32_t b_min,
    int32_t b_max
) {
    char msg[256] = {0};

    int32_t a = a_min;
    do {
        int32_t b = b_min;
        do {
            struct test_t t = {0};
            uint8_t *p;
            uint32_t result = rexlang_pure_eval(opcode, (uint32_t)a, (uint32_t)b);

            t.check_stack_values[t.check_stack_count++] = result;
            t.check_error = REXLANG_ERR_HALTED;

            {
                // stack-only opcode test:
                p = &t.prgm[0];
                push_si(&p, a);
                push_si(&p, b);
                *p++ = opcode;
                // end with HALT:
                *p++ = 0;
                printf("executing test: (%10d %5s %10d) == %10d // stack\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }

            if (b >= INT8_MIN && b <= INT8_MAX) {
                // imm8 opcode test:
                p = &t.prgm[0];
                push_si(&p, a);
                *p++ = opcode + 0x40;
                *p++ = (uint8_t)(uint32_t)b;
                // end with HALT:
                *p++ = 0;
                printf("executing test: (%10d %5s %10d) == %10d // imm8\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }

            if (b >= INT16_MIN && b <= INT16_MAX) {
                // imm16 opcode test:
                p = &t.prgm[0];
                push_si(&p, a);
                *p++ = opcode + 0x80;
                *p++ = (uint8_t)(uint32_t)b;
                *p++ = (uint8_t)((uint32_t)b >> 8);
                // end with HALT:
                *p++ = 0;
                printf("executing test: (%10d %5s %10d) == %10d // imm16\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }

            {
                // imm32 opcode test:
                p = &t.prgm[0];
                push_si(&p, a);
                *p++ = opcode + 0xC0;
                *p++ = (uint8_t)(uint32_t)b;
                *p++ = (uint8_t)((uint32_t)b >> 8);
                *p++ = (uint8_t)((uint32_t)b >> 16);
                *p++ = (uint8_t)((uint32_t)b >> 24);
                // end with HALT:
                *p++ = 0;
                printf("executing test: (%10d %5s %10d) == %10d // imm32\n", a, name, b, result);
                int ret = exec_test(&t, msg);
                if (ret) {
                    printf("** test FAILED! (%d); %s\n", ret, msg);
                    return ret;
                }
            }
        } while (b++ != b_max);
    } while (a++ != a_max);

    return 0;
}

struct named_op { const char *name; uint8_t op; };

int main(void) {
    int ret = 0;
    char msg[256] = {0};
    uint32_t ranges_ui[][2] = {
        {           0U,            2U},
        {  INT8_MAX-2U,   INT8_MAX+2U},
        { UINT8_MAX-2U,  UINT8_MAX+2U},
        { INT16_MAX-2U,  INT16_MAX+2U},
        {UINT16_MAX-2U, UINT16_MAX+2U},
        { INT32_MAX-2U,  INT32_MAX+2U},
        {UINT32_MAX-2U, UINT32_MAX},
    };
    int32_t ranges_si[][2] = {
        {           -2,             2},
        {   INT8_MIN-2,    INT8_MIN+2},
        {   INT8_MAX-2,    INT8_MAX+2},
        {  INT16_MIN-2,   INT16_MIN+2},
        {  INT16_MAX-2,   INT16_MAX+2},
        { -UINT8_MAX-2,  -UINT8_MAX+2},
        {  UINT8_MAX-2,   UINT8_MAX+2},
        {-UINT16_MAX-2, -UINT16_MAX+2},
        { UINT16_MAX-2,  UINT16_MAX+2},
        {  INT32_MIN,     INT32_MIN+2},
        {  INT32_MAX-2,   INT32_MAX},
    };

    struct named_op opcodes_ui[] = {
        {"eq",      0x02},
        {"ne",      0x03},
        {"le-ui",   0x04},
        {"gt-ui",   0x06},
        {"lt-ui",   0x08},
        {"ge-ui",   0x0A},
        {"and",     0x0C},
        {"or",      0x0D},
        {"xor",     0x0E},
        {"add",     0x0F},
        {"sub",     0x10},
        {"mul",     0x11},
        {NULL, 0},
    };
    struct named_op opcodes_si[] = {
        {"le-si", 0x05},
        {"gt-si", 0x07},
        {"lt-si", 0x09},
        {"ge-si", 0x0B},
        {NULL, 0},
    };
    struct named_op* p;

    // signed tests:
    for (p = opcodes_si; p->name; p++) {
        for (int i = 0; i < 11; i++) {
            int32_t lo = ranges_si[i][0];
            int32_t hi = ranges_si[i][1];
            if ((ret = automate_test_si(p->name, p->op, lo, hi, lo, hi)) != 0) {
                return ret;
            }
        }
    }

    // unsigned tests:
    for (p = opcodes_ui; p->name; p++) {
        for (int i = 0; i < 7; i++) {
            uint32_t lo = ranges_ui[i][0];
            uint32_t hi = ranges_ui[i][1];
            if ((ret = automate_test_ui(p->name, p->op, lo, hi, lo, hi)) != 0) {
                return ret;
            }
        }
    }

#if 1
    for (int i = 0; i < sizeof(tests)/sizeof(struct test_t); i++) {
        printf("executing test: %s\n", tests[i].name);
        int ret = exec_test(&tests[i], msg);
        if (ret) {
            printf("** test FAILED! (%d); %s\n", ret, msg);
            return ret;
        }
    }
#endif

    return 0;
}
