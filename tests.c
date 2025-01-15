
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

int main(void) {
    enum rexlang_error err;
    struct rexlang_vm vm;
    uint8_t prgm[256] = {
        0b01000000, 0x3F,                   // push-u8    chip=0x3F        (fxpak)
        0b11000000, 0x00, 0x2C, 0x00, 0x00, // push-u32   addr=0x00002C00
        0b01101111, 0x00,                   // syscall-u8 0 (chip-set-addr)
        0,                                  // halt
    };
    uint8_t data[256] = {};

    rexlang_vm_init(&vm, 256, prgm, 256, data, syscall);

    err = rexlang_vm_exec(&vm, 256);
    printf("err: %d\n", err);

    if (err != REXLANG_ERR_HALTED) {
        return 1;
    }
    if (chip_addr[0x3F] != 0x2C00) {
        return 2;
    }

    return 0;
}