// this file is directly #included in tests.c

int test_chip_set_addr(struct rexlang_vm* vm, char* msg) {
    int i = sprintf(msg, "chip_addr[0x3F]");
    expect(0x2C00, chip_addr[0x3F], msg+i);

    return 0;
}

const struct test_t tests[] = {
    {
        "halt",
        {
            0,                                  // halt
            0b01000000, 0x3F                    // push-u8
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0   // expect the push-u8 to not be executed after the halt
        },
        NULL,
    },
    {
        "nop",
        {
            1,                                  // nop
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        0,
        {
            0
        },
        NULL,
    },
    {
        "eq err1",
        {
            2,                                  // eq
            0,                                  // halt
        },
        REXLANG_ERR_DATA_STACK_EMPTY,
        0,
        {
            0
        },
        NULL,
    },
    {
        "eq err2",
        {
            0b01000000, 0,                      // push-u8
            0x02,                               // eq
            0,                                  // halt
        },
        REXLANG_ERR_DATA_STACK_EMPTY,
        0,
        {
            0
        },
        NULL,
    },

    {
        "le-si-imm8 fails for u8 values",
        {
            0b01000000, (s8)-128,               // push-u8
            0x45, (s8)127,                      // le-ui-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0 // expect to fail because -128 was not sign-extended from the push-u8
        },
        NULL,
    },

    {
        "chip-set-addr",
        {
            0b01000000, 0x3F,                   // push-u8    chip=0x3F        (fxpak)
            0b10000000, 0x00, 0x2C,             // push-u16   addr=0x2C00
            0b01101111, 0x00,                   // syscall-u8 0 (chip-set-addr)
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        0,
        { 0 },
        test_chip_set_addr,
    },
    {
        "chip-rdn-u8",
        {
            0b01000000, 0x3F,                   // push-u8    chip=0x3F        (fxpak)
            0b10000000, 0x00, 0x2C,             // push-u16   addr=0x2C00
            0b01101111, 0x00,                   // syscall-u8 0 (chip-set-addr)

            0b01000000, 0x3F,                   // push-u8    chip=0x3F        (fxpak)
            0b01101111, 0x01,                   // syscall-u8 1 (chip-rdn-u8)
            0b01101010, 7,                      // jump-abs-if-imm8 7

            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        0,
        { 0 },
        test_chip_set_addr,
    },
};
