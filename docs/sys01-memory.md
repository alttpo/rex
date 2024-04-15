SUBSYSTEM 01: Memory Chip Access
---

Memory Chip IDs (u16):
--
    0000: (read-only)  SNES console WRAM mirror
    0001: (read-only)  SNES console VRAM mirror
    0002: (read-only)  SNES console CGRAM mirror
    0003: (read-only)  SNES console OAM mirror
    0004: (read-only)  SNES console APURAM mirror
    0005..001F reserved for more SNES console memory chips

    0020: (read-write) SNES cart ROM
    0021: (read-write) SNES cart SRAM (battery-backed save RAM)
    0022: (read-write) SNES cart SA-1 IRAM
    0023: (read-write) SNES cart SA-1 BW-RAM
    0024..003F reserved for more SNES cart memory chips

    0040: (read-write) SNES FX PAK cart NMI EXE buffer [mapped $2C00..2DFF]
    0041..004F reserved for more FX PAK memory chips

Comparison Functions:
--
3-bit identifier (0..7), type `bool cmpfn(u8 a, u8 b)`

    000: a == b, equal
    001: a != b, not equal
    010: a <  b, less than
    011: a <= b, less than or equal
    100: a >  b, greater than
    101: a >= b, greater than or equal
    110: undefined, always returns true
    111: undefined, always returns true

Error Codes:
--
    FE: timed out
    FD: program: one or more commands are invalid
    01: unknown/unsupported memory chip
    02: memory chip not readable
    03: memory chip not writable
    04: address out of range
    05: address+size out of range

Opcodes:
--
    FD: enumerate memory chips
        in: {
        }
        out: {
            u8   chipCount;
            struct {
                u16  chip;                  // chip id
                u32  size;                  // memory size in bytes
                u8   flags;                 // [------wr]; r=readable, w=writable
                str  chipName;              // name of chip
            } chips[chipCount];
        }

    FC: enter program
        input payload is a sequence of [PRGM]-denoted commands with their opcodes
        every command's inputs are validated before execution begins
        in: {
            struct {
                u8      opcode;             // opcode of [PRGM] command
                u8      input[...];         // command input data (length depends on command)
            } commands[...];                // program contents, limit 4096 bytes
        }
        out: {
            struct {
                u8      opcode;             // opcode of command accepted
                u8      error;              // error code (check for non-zero)
            } errors[...];
        }

    FB: execute last entered program
        output payload is the sequence of all executed command output responses
        first command with an execution error stops the program
            failing command's error code is set in response.header
            all command responses up to and including the errored command are included in the response
        in: {
        }
        out: {
            struct {
                u8      opcode;             // opcode of command
                u8      out[...];           // command output data (length depends on command)
            } responses[...];
        }

    FA: [PRGM] WRITE:   write data to memory chip at address
        in: {
            u16  chip;
            u32  addr;
            n8   size;
            u8   data[size];
        }
        out: {
        }

    F9: [PRGM] READ:    read data from memory chip at address
        in: {
            u16  chip;
            u32  addr;
            n8   size;
        }
        out: {
            u16  chip;
            u32  addr;
            n8   size;
            u8   data[size];
        }

    F8: [PRGM] TIMEOUT: set loop timeout in SNES master clock cycle count
        in: {
            u32  cycles;                    // 1364 cycles per scanline, 261/311 scanlines in NTSC/PAL
        }
        out: {
        }

    F7: [PRGM] UNTIL:   loop until timeout or until `compare_u8((read_u8(chip, addr) >> shr) & mask, cmpValue) == true`
        in: {
            u16  chip;
            u32  addr;
            u8   cmpFnShr;                  // [00sssccc]; s = shift-right factor (0..7), c = compare function
            u8   mask;
            u8   cmpValue;
        }
        out: {
            u8   readValue;
        }
        if timed out then set error = FE

    F6: [PRGM] WHILEZ:  loop until timeout or until `(read_u8(chip, addr) != 0)`
        in: {
            u16  chip;
            u32  addr;
        }
        out: {
            u8   readValue;
        }
        if timed out then set error = FE
