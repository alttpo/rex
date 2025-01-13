
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "rexlang_vm.h"
#include "rexlang_vm_impl.h"

uint32_t chip_addr[0x40];

void syscall(struct rexlang_vm* vm, uint16_t fn) {
    switch (fn) {
    case 0x0000: { // chip-set-addr
        u16 addrhi = pop(vm);
        u16 addrlo = pop(vm);
        u16 chip = pop(vm);
        if (chip >= 0x40) {
            throw_error(vm, REXLANG_ERR_CALL_ARG_OUT_OF_RANGE);
        }
        chip_addr[chip & 0x3F] = ((uint32_t)addrhi<<16) | addrlo;
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
        0b11011010,                     // push 3 values: u8, u16, u16
        0x3F,                           // chip=0x1F        (fxpak)
        0x00, 0x2C,                     // addrlo=0x2C00
        0x00, 0x00,                     // addrhi=0x0000
        0b01011100, 0x00,               // syscall 0 (chip-set-addr)
        0,                              // halt
    };
    uint8_t data[256] = {};

    rexlang_vm_init(&vm, 256, prgm, 256, data, syscall, NULL);

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