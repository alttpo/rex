
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
        0b10111111,                     // push 0x1F   (chip=fxpak)
        0b01100000, 0x00, 0x2C,         // push 0x2C00 (addrlo)
        0b10000000,                     // push 0x0000 (addrhi)
        0b01011001, 0x00,               // syscall 0 (chip-set-addr)
        0,                              // halt
    };
    uint8_t data[256] = {};

    rexlang_vm_init(&vm, 256, prgm, 256, data, syscall, NULL);

    err = rexlang_vm_exec(&vm, 256);
    printf("err: %d\n", err);

    return 0;
}