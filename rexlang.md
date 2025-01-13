# Rexlang
Rexlang is a 16-bit integer-only programming language optimized for embedded applications with tight memory constraints.

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

Pointer types `*T` are defined for documentation purposes; pointers are simply `u16` memory addresses.

## Stack
All values on the stack must be of size `u16`. This keeps the implementation simple and efficient.

## Binary program format
The rexlang binary program format is a compact and machine-friendly representation of a program as it appears in program memory.

This binary format is a byte-aligned stream so that statements are fully addressable at the byte offset without requiring bit offsets. Great effort has been expended to make sure that this format is as efficient as possible with minimal overhead where necessary, short of turning the entire format into an unaligned bit stream of course.

The tables below describe the legal binary formats allowed for each component of a rexlang program.

Every table row describes one of the possible bit patterns allowed for the component the table describes.

Bits are listed from most-significant bit (MSB) to least-significant bit (LSB) from left to right and are clustered in octets (8-bit bytes).

`0`s and `1`s are literal bits required to be set or cleared for the row's pattern to match.

Alpha characters are treated as bits that represent an N-bit unsigned integer ordered most-significant to least-significant bit from left to right. Bits are grouped in octets, aka bytes, in little-endian order with the least significant byte first.

### Opcodes
| Format                       | Name                  | a    | b   | c   | result1 | result2 | computation                                              |
| ---------------------------- | --------------------- | ---- | --- | --- | ------- | ------- | -------------------------------------------------------- |
| `00000000`                   | halt                  |      |     |     |         |         |                                                          |
| `00000001`                   | eq                    | a    | b   |     | u16     |         | `a == b`                                                 |
| `00000010`                   | ne                    | a    | b   |     | u16     |         | `a != b`                                                 |
| `00000011`                   | le                    | a    | b   |     | u16     |         | `a <= b`                                                 |
| `00000100`                   | gt                    | a    | b   |     | u16     |         | `a >  b`                                                 |
| `00000101`                   | lt                    | a    | b   |     | u16     |         | `a <  b`                                                 |
| `00000110`                   | ge                    | a    | b   |     | u16     |         | `a >= b`                                                 |
| `00000111`                   | and                   | a    | b   |     | u16     |         | `a &  b`                                                 |
| `00001000`                   | or                    | a    | b   |     | u16     |         | `a \| b`                                                 |
| `00001001`                   | xor                   | a    | b   |     | u16     |         | `a ^  b`                                                 |
| `00001010`                   | add                   | a    | b   |     | u16     |         | `a +  b`                                                 |
| `00001011`                   | sub                   | a    | b   |     | u16     |         | `a -  b`                                                 |
| `00001100`                   | mul                   | a    | b   |     | u16     |         | `a *  b`                                                 |
| `00001101`                   | ld-u8                 | *u8  |     |     | u8      |         | `*( u8*)(&data[a])`                                      |
| `00001110`                   | ld-u16                | *u16 |     |     | u16     |         | `*(u16*)(&data[a])`                                      |
| `00001111`                   | ld-u8--offs           | *u8  | u16 |     | u8      |         | `*( u8*)(&data[a+b])`                                    |
| `00010000`                   | ld-u16-offs           | *u16 | u16 |     | u16     |         | `*(u16*)(&data[a+b])`                                    |
| `00010001`                   | st-u8                 | *u8  | u8  |     | u8      |         | `*( u8*)(&data[a]) = b`                                  |
| `00010010`                   | st-u16                | *u16 | u16 |     | u16     |         | `*(u16*)(&data[a]) = b`                                  |
| `00010011`                   | st-u8--offs           | *u8  | u16 | u8  | u8      |         | `*( u8*)(&data[a+b]) = c`                                |
| `00010100`                   | st-u16-offs           | *u16 | u16 | u16 | u16     |         | `*(u16*)(&data[a+b]) = c`                                |
| `00010101`                   | call                  | a    |     |     | IP      |         | push IP; IP=x                                            |
| `00010110`                   | return / jump-abs     | a    |     |     |         |         | IP=a                                                     |
| `00010111`                   | jump-abs-if           | a    | b   |     |         |         | IP=a if b != 0                                           |
| `00011000`                   | jump-abs-if-not       | a    | b   |     |         |         | IP=a if b == 0                                           |
| `00011001`                   | jump-rel              | a    |     |     |         |         | IP+=(s16)a                                               |
| `00011010`                   | jump-rel-if           | a    | b   |     |         |         | IP+=(s16)a if b != 0                                     |
| `00011011`                   | jump-rel-if-not       | a    | b   |     |         |         | IP+=(s16)a if b == 0                                     |
| `00011100`                   | syscall               | a    |     |     |         |         | invoke    system function `a`                            |
| `00011101`                   | extcall               | a    |     |     |         |         | invoke extension function `a`                            |
| `00011110`                   | shl                   | a    | b   |     | a       |         | `a << b`                                                 |
| `00011111`                   | shr                   | a    | b   |     | a       |         | `a >> b`                                                 |
| `00100000`                   | inc                   | a    |     |     | a       |         | `++a`                                                    |
| `00100001`                   | dec                   | a    |     |     | a       |         | `--a`                                                    |
| `00100010`                   | not                   | a    |     |     | a       |         | `!a`                                                     |
| `00100011`                   | neg                   | a    |     |     | a       |         | `-a`                                                     |
| `00100100`                   | discard               | a    |     |     |         |         | discards `a`                                             |
| `00100101`                   | swap                  | a    | b   |     | a       | b       | push a; push b                                           |
| `00100110`                   | dcopy                 | *u8  | *u8 | c   | *u8     |         | copy `c` bytes from `data[b]` to `data[a]`; push `a+c`   |
| `00100111`                   | pcopy                 | *u8  | *u8 | c   | *u8     |         | copy `c` bytes from `prgm[b]` to `data[a]`; push `a+c`   |
| `00101000`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101001`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101010`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101011`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101100`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101101`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101110`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00101111`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110000`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110001`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110010`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110011`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110100`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110101`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110110`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00110111`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00111000`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00111001`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00111010`                   | **RESERVED**          |      |     |     |         |         |                                                          |
| `00111011`                   | nop                   |      |     |     |         |         | no operation                                             |
| `001111xx` [x+1 bytes]       | opcode-ext            |      |     |     |         |         | extended opcodes                                         |
| `01000000_xxxxxxxx`          | push-u8               |      |     |     | x       |         | push x                                                   |
| `01000001_xxxxxxxx`          | eq--imm8              | a    |     |     | u16     |         | `a == x`                                                 |
| `01000010_xxxxxxxx`          | ne--imm8              | a    |     |     | u16     |         | `a != x`                                                 |
| `01000011_xxxxxxxx`          | le--imm8              | a    |     |     | u16     |         | `a <= x`                                                 |
| `01000100_xxxxxxxx`          | gt--imm8              | a    |     |     | u16     |         | `a >  x`                                                 |
| `01000101_xxxxxxxx`          | lt--imm8              | a    |     |     | u16     |         | `a <  x`                                                 |
| `01000110_xxxxxxxx`          | ge--imm8              | a    |     |     | u16     |         | `a >= x`                                                 |
| `01000111_xxxxxxxx`          | and-imm8              | a    |     |     | u16     |         | `a &  x`                                                 |
| `01001000_xxxxxxxx`          | or--imm8              | a    |     |     | u16     |         | `a \| x`                                                 |
| `01001001_xxxxxxxx`          | xor-imm8              | a    |     |     | u16     |         | `a ^  x`                                                 |
| `01001010_xxxxxxxx`          | add-imm8              | a    |     |     | u16     |         | `a +  x`                                                 |
| `01001011_xxxxxxxx`          | sub-imm8              | a    |     |     | u16     |         | `a -  x`                                                 |
| `01001100_xxxxxxxx`          | mul-imm8              | a    |     |     | u16     |         | `a *  x`                                                 |
| `01001101_xxxxxxxx`          | ld-u8--imm8           |      |     |     | u8      |         | `*( u8*)(&data[x])`                                      |
| `01001110_xxxxxxxx`          | ld-u16-imm8           |      |     |     | u16     |         | `*(u16*)(&data[x])`                                      |
| `01001111_xxxxxxxx`          | ld-u8--offs-imm8      | *u8  |     |     | u8      |         | `*( u8*)(&data[a+x])`                                    |
| `01010000_xxxxxxxx`          | ld-u16-offs-imm8      | *u16 |     |     | u16     |         | `*(u16*)(&data[a+x])`                                    |
| `01010001_xxxxxxxx`          | st-u8--imm8           | u8   |     |     | u8      |         | `*( u8*)(&data[x]) = a`                                  |
| `01010010_xxxxxxxx`          | st-u16-imm8           | u16  |     |     | u16     |         | `*(u16*)(&data[x]) = a`                                  |
| `01010011_xxxxxxxx`          | st-u8--offs-imm8      | *u8  | u8  |     | u8      |         | `*( u8*)(&data[a+x]) = b`                                |
| `01010100_xxxxxxxx`          | st-u16-offs-imm8      | *u16 | u16 |     | u16     |         | `*(u16*)(&data[a+x]) = b`                                |
| `01010101_xxxxxxxx`          | call-imm8             |      |     |     |         |         | push IP; IP=x                                            |
| `01010110_xxxxxxxx`          | jump-abs-imm8         |      |     |     |         |         | IP=x                                                     |
| `01010111_xxxxxxxx`          | jump-abs-imm8-if      | a    |     |     |         |         | IP=x if a != 0                                           |
| `01011000_xxxxxxxx`          | jump-abs-imm8-if-not  | a    |     |     |         |         | IP=x if a == 0                                           |
| `01011001_xxxxxxxx`          | jump-rel-imm8         |      |     |     |         |         | IP+=(s8)x                                                |
| `01011010_xxxxxxxx`          | jump-rel-imm8-if      | a    |     |     |         |         | IP+=(s8)x if a != 0                                      |
| `01011011_xxxxxxxx`          | jump-rel-imm8-if-not  | a    |     |     |         |         | IP+=(s8)x if a == 0                                      |
| `01011100_xxxxxxxx`          | syscall-imm8          |      |     |     |         |         | invoke    system function `x`                            |
| `01011101_xxxxxxxx`          | extcall-imm8          |      |     |     |         |         | invoke extension function `x`                            |
| `01011110_0000xxxx`          | shl-imm4              | a    |     |     | u16     |         | `a << x`                                                 |
| `01011111_0000xxxx`          | shr-imm4              | a    |     |     | u16     |         | `a >> x`                                                 |
| `01100000_xxxxxxxx`          | push-s8               |      |     |     | x       |         | push (s16)x                                              |
| `01100001_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01100010_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01100011_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01100100_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01100101_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01100110_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01100111_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101000_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101001_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101010_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101011_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101100_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101101_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101110_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01101111_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110000_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110001_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110010_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110011_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110100_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110101_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110110_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01110111_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111000_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111001_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111010_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111011_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111100_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111101_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111110_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `01111111_xxxxxxxx`          | **RESERVED**          |      |     |     |         |         |                                                          |
| `10000000_xxxxxxxx_xxxxxxxx` | push-imm16            |      |     |     | x       |         | push x                                                   |
| `10000001_xxxxxxxx_xxxxxxxx` | eq--imm16             | a    |     |     | u16     |         | `a == x`                                                 |
| `10000010_xxxxxxxx_xxxxxxxx` | ne--imm16             | a    |     |     | u16     |         | `a != x`                                                 |
| `10000011_xxxxxxxx_xxxxxxxx` | le--imm16             | a    |     |     | u16     |         | `a <= x`                                                 |
| `10000100_xxxxxxxx_xxxxxxxx` | gt--imm16             | a    |     |     | u16     |         | `a >  x`                                                 |
| `10000101_xxxxxxxx_xxxxxxxx` | lt--imm16             | a    |     |     | u16     |         | `a <  x`                                                 |
| `10000110_xxxxxxxx_xxxxxxxx` | ge--imm16             | a    |     |     | u16     |         | `a >= x`                                                 |
| `10000111_xxxxxxxx_xxxxxxxx` | and-imm16             | a    |     |     | u16     |         | `a &  x`                                                 |
| `10001000_xxxxxxxx_xxxxxxxx` | or--imm16             | a    |     |     | u16     |         | `a \| x`                                                 |
| `10001001_xxxxxxxx_xxxxxxxx` | xor-imm16             | a    |     |     | u16     |         | `a ^  x`                                                 |
| `10001010_xxxxxxxx_xxxxxxxx` | add-imm16             | a    |     |     | u16     |         | `a +  x`                                                 |
| `10001011_xxxxxxxx_xxxxxxxx` | sub-imm16             | a    |     |     | u16     |         | `a -  x`                                                 |
| `10001100_xxxxxxxx_xxxxxxxx` | mul-imm16             | a    |     |     | u16     |         | `a *  x`                                                 |
| `10001101_xxxxxxxx_xxxxxxxx` | ld-u8--imm16          |      |     |     | u8      |         | `*( u8*)(&data[x])`                                      |
| `10001110_xxxxxxxx_xxxxxxxx` | ld-u16-imm16          |      |     |     | u16     |         | `*(u16*)(&data[x])`                                      |
| `10001111_xxxxxxxx_xxxxxxxx` | ld-u8--offs-imm16     | *u8  |     |     | u8      |         | `*( u8*)(&data[a+x])`                                    |
| `10010000_xxxxxxxx_xxxxxxxx` | ld-u16-offs-imm16     | *u16 |     |     | u16     |         | `*(u16*)(&data[a+x])`                                    |
| `10010001_xxxxxxxx_xxxxxxxx` | st-u8--imm16          | u8   |     |     | u8      |         | `*( u8*)(&data[x]) = a`                                  |
| `10010010_xxxxxxxx_xxxxxxxx` | st-u16-imm16          | u16  |     |     | u16     |         | `*(u16*)(&data[x]) = a`                                  |
| `10010011_xxxxxxxx_xxxxxxxx` | st-u8--offs-imm16     | *u8  | u8  |     | u8      |         | `*( u8*)(&data[a+x]) = b`                                |
| `10010100_xxxxxxxx_xxxxxxxx` | st-u16-offs-imm16     | *u16 | u16 |     | u16     |         | `*(u16*)(&data[a+x]) = b`                                |
| `10010101_xxxxxxxx_xxxxxxxx` | call-imm16            |      |     |     |         |         | push IP; IP=x                                            |
| `10010110_xxxxxxxx_xxxxxxxx` | jump-abs-imm16        |      |     |     |         |         | IP=x                                                     |
| `10010111_xxxxxxxx_xxxxxxxx` | jump-abs-imm16-if     | a    |     |     |         |         | IP=x if a != 0                                           |
| `10011000_xxxxxxxx_xxxxxxxx` | jump-abs-imm16-if-not | a    |     |     |         |         | IP=x if a == 0                                           |
| `10011001_xxxxxxxx_xxxxxxxx` | jump-rel-imm16        |      |     |     |         |         | IP+=(s16)x                                               |
| `10011010_xxxxxxxx_xxxxxxxx` | jump-rel-imm16-if     | a    |     |     |         |         | IP+=(s16)x if a != 0                                     |
| `10011011_xxxxxxxx_xxxxxxxx` | jump-rel-imm16-if-not | a    |     |     |         |         | IP+=(s16)x if a == 0                                     |
| `10011100_xxxxxxxx_xxxxxxxx` | syscall-imm16         |      |     |     |         |         | invoke    system function `x`                            |
| `10011101_xxxxxxxx_xxxxxxxx` | extcall-imm16         |      |     |     |         |         | invoke extension function `x`                            |
| `10011110_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10011111_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100000_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100001_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100010_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100011_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100100_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100101_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100110_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10100111_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101000_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101001_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101010_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101011_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101100_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101101_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101110_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10101111_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110000_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110001_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110010_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110011_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110100_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110101_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110110_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10110111_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111000_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111001_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111010_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111011_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111100_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111101_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111110_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `10111111_xxxxxxxx_xxxxxxxx` | **RESERVED**          |      |     |     |         |         |                                                          |
| `11dcbaxx` [x+1 values]      | push-values           |      |     |     |         |         | push `x+1` (1..$4) values of sizes (`a`..`d`=`u8`/`u16`) |

### Value formats
| Format              | Description |
| ------------------- | ----------- |
| `xxxxxxxx`          | `u8` value  |
| `xxxxxxxx_xxxxxxxx` | `u16` value |

## Standard Function Library
|   Code | Name          | Arg1 | Arg2   | Arg3   | Result    | Description                                 |
| -----: | :------------ | ---- | ------ | ------ | --------- | ------------------------------------------- |
| `0000` | chip-set-addr | chip | addrlo | addrhi |           | set chip address (32-bit)                   |
| `0001` | chip-rdn-u8   | chip |        |        | u8        | read `u8`, do not advance chip address      |
| `0002` | chip-wrn-u8   | chip | u8     |        |           | write `u8`, do not advance chip address     |
| `0003` | chip-rda-u8   | chip |        |        | u8        | read `u8`, auto-advance chip address by 1   |
| `0004` | chip-rda-u16  | chip |        |        | u16       | read `u16`, auto-advance chip address by 2  |
| `0005` | chip-wra-u8   | chip | u8     |        |           | write `u8`, auto-advance chip address by 1  |
| `0006` | chip-wra-u16  | chip | u16    |        |           | write `u16`, auto-advance chip address by 2 |
| `0007` | chip-rda-blk  | chip | len    | *dest  | *dest+len | read block of `len` bytes into `dest`       |
| `0008` | chip-wra-blk  | chip | len    | *src   |           | write block of `len` bytes from `src`       |
| `0009` |               |      |        |        |           |                                             |
| `000A` |               |      |        |        |           |                                             |
| `000B` |               |      |        |        |           |                                             |
| `000C` |               |      |        |        |           |                                             |
| `000D` |               |      |        |        |           |                                             |
| `000E` |               |      |        |        |           |                                             |
| `000F` |               |      |        |        |           |                                             |

TODO: input/output via USB
considered raw stdin/stdout treatment but that's too unstructured to allow for multiplexing
consider allowing in/out socket pairs per client?
