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
        "eq true",
        {
            0b01000000, 0,                      // push-u8
            0b01000000, 0,                      // push-u8
            0x02,                               // eq
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "eq false",
        {
            0b01000000, 1,                      // push-u8
            0b01000000, 0,                      // push-u8
            0x02,                               // eq
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "eq-imm8 true",
        {
            0b01000000, 0,                      // push-u8
            0x42, 0,                            // eq-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "eq-imm8 false",
        {
            0b01000000, 1,                      // push-u8
            0x42, 0,                            // eq-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "eq-imm16 true",
        {
            0b01000000, 0,                      // push-u8
            0x82, 0, 0,                         // eq-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "eq-imm16 false",
        {
            0b01000000, 1,                      // push-u8
            0x82, 0, 0,                         // eq-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "eq-imm32 true",
        {
            0b01000000, 0,                      // push-u8
            0xC2, 0, 0, 0, 0,                   // eq-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "eq-imm32 false",
        {
            0b01000000, 1,                      // push-u8
            0xC2, 0, 0, 0, 0,                   // eq-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "ne false",
        {
            0b01000000, 1,                      // push-u8
            0b01000000, 1,                      // push-u8
            0x03,                               // ne
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "ne true",
        {
            0b01000000, 0,                      // push-u8
            0b01000000, 1,                      // push-u8
            0x03,                               // ne
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "ne-imm8 false",
        {
            0b01000000, 1,                      // push-u8
            0x43, 1,                            // ne-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "ne-imm8 true",
        {
            0b01000000, 0,                      // push-u8
            0x43, 1,                            // ne-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "ne-imm16 false",
        {
            0b01000000, 1,                      // push-u8
            0x83, 1, 0,                         // ne-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "ne-imm16 true",
        {
            0b01000000, 0,                      // push-u8
            0x83, 1, 0,                         // ne-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "ne-imm32 false",
        {
            0b01000000, 1,                      // push-u8
            0xC3, 1, 0, 0, 0,                   // ne-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "ne-imm32 true",
        {
            0b01000000, 0,                      // push-u8
            0xC3, 1, 0, 0, 0,                   // ne-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-ui false",
        {
            0b01000000, 128,                    // push-u8
            0b01000000, 127,                    // push-u8
            0x04,                               // le-ui
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-ui true",
        {
            0b01000000, 128,                    // push-u8
            0b01000000, 129,                    // push-u8
            0x04,                               // le-ui
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-ui-imm8 false",
        {
            0b01000000, 128,                    // push-u8
            0x44, 127,                          // le-ui-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-ui-imm8 true",
        {
            0b01000000, 128,                    // push-u8
            0x44, 129,                          // le-ui-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-ui-imm16 false",
        {
            0b01000000, 128,                    // push-u8
            0x84, 127, 0,                       // le-ui-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-ui-imm16 true",
        {
            0b01000000, 128,                    // push-u8
            0x84, 129, 0,                       // le-ui-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-ui-imm32 false",
        {
            0b01000000, 128,                    // push-u8
            0xC4, 127, 0, 0, 0,                 // le-ui-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-ui-imm32 true",
        {
            0b01000000, 128,                    // push-u8
            0xC4, 129, 0, 0, 0,                 // le-ui-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-si false",
        {
            0b01000001, (s8)127,                // push-s8
            0b01000001, (s8)-128,               // push-s8
            0x05,                               // le-si
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-si true",
        {
            0b01000001, (s8)-128,               // push-s8
            0b01000001, (s8)127,                // push-s8
            0x05,                               // le-si
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-si-imm8 false",
        {
            0b01000001, (s8)127,                // push-s8
            0x45, (s8)-128,                     // le-si-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-si-imm8 true",
        {
            0b01000001, (s8)-128,               // push-s8
            0x45, (s8)127,                      // le-ui-imm8
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-si-imm16 false",
        {
            0b10000001, (s8)127, 0,             // push-s16
            0x85, (s8)-128, 0xFF,               // le-ui-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-si-imm16 true",
        {
            0b10000001, (s8)-128, 0xFF,         // push-s16
            0x85, 129, 0,                       // le-ui-imm16
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
    {
        "le-si-imm32 false",
        {
            0b11000001, (s8)127, 0, 0, 0,       // push-s32
            0xC5, (s8)-128, 0xFF, 0xFF, 0xFF,   // le-si-imm32
            0,                                  // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "le-si-imm32 true",
        {
            0b11000001, (s8)-128, 0xFF, 0xFF, 0xFF, // push-s32
            0xC5, (s8)127, 0, 0, 0,                 // le-si-imm32
            0,                                      // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
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
        "gt-ui-imm32 false",
        {
            0b11000001, 0xFE, 0xFF, 0xFF, 0xFF,     // push-s32
            0xC6, 0xFF, 0xFF, 0xFF, 0xFF,           // gt-ui-imm32
            0,                                      // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            0
        },
        NULL,
    },
    {
        "gt-ui-imm32 true",
        {
            0b11000001, 0xFF, 0xFF, 0xFF, 0xFF,     // push-s32
            0xC6, 0xFE, 0xFF, 0xFF, 0xFF,           // gt-ui-imm32
            0,                                      // halt
        },
        REXLANG_ERR_HALTED,
        1,
        {
            1
        },
        NULL,
    },
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
