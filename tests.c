
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

int main(void) {
    char msg[256] = {0};

    for (int i = 0; i < sizeof(tests)/sizeof(struct test_t); i++) {
        printf("executing test: %s\n", tests[i].name);
        int ret = exec_test(&tests[i], msg);
        if (ret) {
            printf("** test FAILED! (%d); %s\n", ret, msg);
            return 1;
        }
    }

    return 0;
}