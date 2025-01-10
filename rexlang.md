# Rexlang
Rexlang is a statically typed, integer-based programming language optimized for embedded applications with tight memory constraints.

Rexlang is designed to allow applications to generate and upload rexlang programs to an embedded system which executes them with very low latency access to critical functions such as memory I/O or interfacing with hardware. The rexlang program running in the embedded system may then write messages back to the host-side application to report data.

## Virtual machine
The virtual machine uses a minimal set of native state variables to keep track of execution state:

  1. `IP`: Instruction Pointer
  2. `SP`: Stack Pointer

These state variables are not directly accessible by rexlang programs and are implementation details of the virtual machine.

## Memory layout
Memory available for rexlang programs is divided into 3 sections:

  1. data memory
  2. program memory
  3. stack memory

These memory sections are subject to the following rules:

  1. Data memory is readable and writable by programs.
  2. Program memory is only readable by programs.
  3. Stack memory may not be directly read from nor written to by programs. It is used by the VM for execution purposes only.

Any violation of the above rules will raise an error and the program will be halted.

An out of bounds memory access will raise an error and the program will be halted.

## Data Types
Rexlang uses only `u8` and `u16` types for primitive values.

Pointer types `*T` are only defined for documentation purposes; pointers are simply `u16` memory addresses.

## Stack
The stack grows downward from higher memory addresses to lower memory addresses.

All pushes to the stack must record the primitive type of the value pushed. When a value is popped off the stack, its type must be checked against the parameter type it is used in. For example, it is not possible to push a `u16` value to the stack and pop it as a `u8` value or vice versa. If this were allowed it would cause the next stack pop operation to be unaligned and could lead to corrupted results or unpredictable execution behavior.

## Binary program format
The rexlang binary program format is a compact and machine-friendly representation of a program as it appears in program memory.

This binary format is a byte-aligned stream so that statements are fully addressable at the byte offset without requiring bit offsets. Great effort has been expended to make sure that this format is as efficient as possible with minimal overhead where necessary, short of turning the entire format into an unaligned bit stream of course.

The tables below describe the legal binary formats allowed for each component of a rexlang program.

Every table row describes one of the possible bit patterns allowed for the component the table describes.

Bits are listed from most-significant bit (MSB) to least-significant bit (LSB) from left to right and are clustered in octets (8-bit bytes).

`0`s and `1`s are literal bits required to be set or cleared for the row's pattern to match.

Alpha characters are treated as bits that represent an N-bit unsigned integer ordered most-significant to least-significant bit from left to right. Bits are grouped in octets, aka bytes, in little-endian order with the least significant byte first.

### Instructions
| Type                | Format                       | Description                                                    |
| ------------------- | ---------------------------- | -------------------------------------------------------------- |
| push-u8             | `00xxxxxx`                   | push `x` (0..$3F) as a `u8` value                              |
| push-4-mixed-values | `01dcbaxx` [x+1 values]      | push `x+1` (1..$4) values of mixed sizes (`a`..`d`=`u8`/`u16`) |
| prgm-enter          | `11111101_aaaaaaaa_aaaaaaaa` | start writing to program memory at `a`                         |
| prgm-end            | `11111110`                   | stop writing to program memory                                 |
| syscall             | `10000000_xxxxxxxx`          | invoke system function `x` (0..$FF)                            |
| extcall             | `10000001_xxxxxxxx_xxxxxxxx` | invoke extension function `x` (0..$FFFF)                       |
| opcode              | `1xxxxxxx`                   | invoke opcode     `x` (  0.. $7C)                              |
| opcode-ext          | `11111111_xxxxxxxx`          | invoke opcode `x+$80` ($80..$17F)                              |

The `push-u8` instruction pushes the literal value (0..$3F) as a `u8` onto the stack. For larger values use the `push-4-mixed-values` instruction.

The `push-4-mixed-values` instruction pushes `x+1` (between 1 and 4) mixed sized values onto the stack. Each value has its own sized specified with `0` for `u8` and `1` for `u16`. `a` specifies the first value's size, `b` specifies the second value's size, and so on. Only `x+1` number of values are consumed.

The `prgm-enter` instruction starts writing the subsequent instructions into program memory at address `a` until the `prgm-end` instruction is encountered.

Otherwise the opcode `x` is executed.

### Value formats
| Format              | Description |
| ------------------- | ----------- |
| `xxxxxxxx`          | `u8` value  |
| `xxxxxxxx_xxxxxxxx` | `u16` value |

### Opcodes
| Format    | Name          | a type | b type | c type | result type | computation                                |
| --------- | ------------- | ------ | ------ | ------ | ----------- | ------------------------------------------ |
| `0000000` | RESERVED      |        |        |        |             |                                            |
| `0000001` | RESERVED      |        |        |        |             |                                            |
| `0000010` | call          | a      |        |        |             | push IP; IP=a                              |
| `0000011` | jump / return | a      |        |        |             | IP=a                                       |
| `0000100` | jump-if       | a      | b      |        |             | IP=a if b != 0                             |
| `0000101` | jump-if-not   | a      | b      |        |             | IP=a if b == 0                             |
| `0000110` | swap          | a      | b      |        |             | push a; push b                             |
| `0000111` | discard       | a      |        |        |             | discards popped value                      |
| `0001000` | to-u8         | a      |        |        | u8          | `(u8)(a & 0xFF)`                           |
| `0001001` | to-u16        | a      |        |        | u16         | `(u16)a`                                   |
| `0001010` | eq            | a      | b      |        | u8          | `a == b`                                   |
| `0001011` | ne            | a      | b      |        | u8          | `a != b`                                   |
| `0001100` | le            | a      | b      |        | u8          | `a <= b`                                   |
| `0001101` | gt            | a      | b      |        | u8          | `a >  b`                                   |
| `0001110` | lt            | a      | b      |        | u8          | `a <  b`                                   |
| `0001111` | ge            | a      | b      |        | u8          | `a >= b`                                   |
| `0010000` | and           | a      | b      |        | max         | `a &  b`                                   |
| `0010001` | or            | a      | b      |        | max         | `a \| b`                                   |
| `0010010` | xor           | a      | b      |        | max         | `a ^  b`                                   |
| `0010011` | not           | a      |        |        | a           | `!a`                                       |
| `0010100` | neg           | a      |        |        | a           | `-a`                                       |
| `0010101` | add           | a      | b      |        | max         | `a +  b`                                   |
| `0010110` | sub           | a      | b      |        | max         | `a -  b`                                   |
| `0010111` | mul           | a      | b      |        | max         | `a *  b`                                   |
| `0011000` | inc           | a      |        |        | a           | `++a`                                      |
| `0011001` | dec           | a      |        |        | a           | `--a`                                      |
| `0011010` | ld-u8         | *u8    |        |        | u8          | `*( u8*)(&M[a])`                           |
| `0011011` | ld-u16        | *u16   |        |        | u16         | `*(u16*)(&M[a])`                           |
| `0011100` | st-u8         | *u8    | u8     |        | u8          | `*( u8*)(&M[a]) = b`                       |
| `0011101` | st-u16        | *u16   | u16    |        | u16         | `*(u16*)(&M[a]) = b`                       |
| `0011110` | shl           | a      | b      |        | a           | `a << b`                                   |
| `0011111` | shr           | a      | b      |        | a           | `a >> b`                                   |
| `010xxxx` | shlx          | a      |        |        | a           | `a << x`                                   |
| `011xxxx` | shrx          | a      |        |        | a           | `a >> x`                                   |
| `1000xxx` | ld-u8-offs    | *u8    |        |        | u8          | `*( u8*)(&M[a+x+1])`                       |
| `1001xxx` | ld-u16-offs   | *u16   |        |        | u16         | `*(u16*)(&M[a+x+1])`                       |
| `1010xxx` | st-u8-offs    | *u8    | u8     |        | u8          | `*( u8*)(&M[a+x+1]) = b`                   |
| `1011xxx` | st-u16-offs   | *u16   | u16    |        | u16         | `*(u16*)(&M[a+x+1]) = b`                   |
| `1100000` | copy          | *u8    | *u8    | c      | *u8         | copy `c` bytes from `b` to `a`; push `a+c` |
| `1100001` |               |        |        |        |             |                                            |
| `1100010` |               |        |        |        |             |                                            |
| `1100011` |               |        |        |        |             |                                            |
| `1100100` |               |        |        |        |             |                                            |
| `1100101` |               |        |        |        |             |                                            |
| `1100110` |               |        |        |        |             |                                            |
| `1100111` |               |        |        |        |             |                                            |
| `1101000` |               |        |        |        |             |                                            |
| `1101001` |               |        |        |        |             |                                            |
| `1101010` |               |        |        |        |             |                                            |
| `1101011` |               |        |        |        |             |                                            |
| `1101100` |               |        |        |        |             |                                            |
| `1101101` |               |        |        |        |             |                                            |
| `1101110` |               |        |        |        |             |                                            |
| `1101111` |               |        |        |        |             |                                            |
| `1110000` |               |        |        |        |             |                                            |
| `1110001` |               |        |        |        |             |                                            |
| `1110010` |               |        |        |        |             |                                            |
| `1110011` |               |        |        |        |             |                                            |
| `1110100` |               |        |        |        |             |                                            |
| `1110101` |               |        |        |        |             |                                            |
| `1110110` |               |        |        |        |             |                                            |
| `1110111` |               |        |        |        |             |                                            |
| `1111000` |               |        |        |        |             |                                            |
| `1111001` |               |        |        |        |             |                                            |
| `1111010` |               |        |        |        |             |                                            |
| `1111011` |               |        |        |        |             |                                            |
| `1111100` |               |        |        |        |             |                                            |
| `1111101` | RESERVED      |        |        |        |             |                                            |
| `1111110` | RESERVED      |        |        |        |             |                                            |
| `1111111` | RESERVED      |        |        |        |             |                                            |

## Standard Function Library
| Code | Definition | Description |
| ---: | :--------- | ----------- |
| `00` | `(exit)`   |             |
| `01` | `()`       |             |
| `02` | `()`       |             |
| `03` | `()`       |             |
| `04` | `()`       |             |
| `05` | `()`       |             |
| `06` | `()`       |             |
| `07` | `()`       |             |
| `08` | `()`       |             |
| `09` | `()`       |             |
| `0A` | `()`       |             |
| `0B` | `()`       |             |
| `0C` | `()`       |             |
| `0D` | `()`       |             |
| `0E` | `()`       |             |
| `0F` | `()`       |             |

TODO: input/output via USB
considered raw stdin/stdout treatment but that's too unstructured to allow for multiplexing
consider allowing in/out socket pairs per client?

## Extension Function Library
|   Code | Definition                                | Description                  |
| -----: | :---------------------------------------- | ---------------------------- |
| `0000` | `(chip-set-addr  chip:u8 lo:u16 hi:u16)`  | set address for chip         |
| `0001` | `(chip-rdn-u8    chip:u8):u8`             | read, no advance address     |
| `0002` | `(chip-rdn-u16   chip:u8):u16`            | read, no advance address     |
| `0003` | `(chip-rda-u8    chip:u8):u8`             | read, advance address        |
| `0004` | `(chip-rda-u16   chip:u8):u16`            | read, advance address        |
| `0005` | `(chip-wrn-ui    chip:u8 src:a)`          | write, no advance address    |
| `0006` | `(chip-wra-ui    chip:u8 src:a)`          | write, advance address       |
| `0007` | `(chip-rda-blk   chip:u8 len:u8 dst:*u8)` | read block, advance address  |
| `0008` | `(chip-wra-blk   chip:u8 len:u8 src:*u8)` | write block, advance address |
