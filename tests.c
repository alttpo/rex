
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
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

    int (*check_fn)(struct rexlang_vm *vm);
};

int test_chip_set_addr(struct rexlang_vm *vm) {
    if (chip_addr[0x3F] != 0x2C00) {
        return 2;
    }

    return 0;
}

const struct test_t tests[] = {
    {
        "chip-set-addr",
        {
            0b01000000, 0x3F,                   // push-u8    chip=0x3F        (fxpak)
            0b11000000, 0x00, 0x2C, 0x00, 0x00, // push-u32   addr=0x00002C00
            0b01101111, 0x00,                   // syscall-u8 0 (chip-set-addr)
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        0,
        {
            0,
        },
        test_chip_set_addr,
    },
};

int exec_test(const struct test_t *t) {
    enum rexlang_error err;
    struct rexlang_vm vm;
    uint8_t data[256] = {0};

    rexlang_vm_init(&vm, 64, t->prgm, 256, data, syscall);
    err = rexlang_vm_exec(&vm, 1024);

    if (err != t->check_error) {
        return 1;
    }

    if (t->check_fn) {
        int ret = t->check_fn(&vm);
        if (ret) {
            return ret;
        }
    }

    return 0;
}

int main(void) {
    for (int i = 0; i < sizeof(tests)/sizeof(struct test_t); i++) {
        printf("executing test: %s\n", tests[i].name);
        int ret = exec_test(&tests[i]);
        if (ret) {
            printf("** test FAILED! ret=%d\n", ret);
            return 1;
        }
    }

    return 0;
}