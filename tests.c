
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include "rexlang_vm.h"

void syscall(struct rexlang_vm* vm, uint16_t fn) {
    return;
}

int main(void) {
    enum rexlang_error err;
    struct rexlang_vm vm;
    uint8_t prgm[256] = {
        0b10000000,             // push 0
        0b10000000,             // push 0
        0b10000000,             // push 0
        0b01011001, 0b00000000, // syscall 0
        0,                      // halt
    };
    uint8_t data[256] = {};

    rexlang_vm_init(&vm, 256, prgm, 256, data, syscall, NULL);
    err = rexlang_vm_exec(&vm, 256);
    printf("err: %d\n", err);

    return 0;
}