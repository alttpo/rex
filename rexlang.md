# Rexlang
Rexlang is a 32-bit integer-only programming language optimized for embedded applications with tight memory constraints.

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

All values in data and stack memory use the host's native endian byte order.

All values in program memory use little endian byte order.

## Stack
All values on the stack must be of size `u32`.

## Binary program format
The rexlang binary program format is a compact and machine-friendly representation of a program as it appears in program memory.

This binary format is a byte-aligned stream so that statements are fully addressable at the byte offset without requiring bit offsets. Great effort has been expended to make sure that this format is as efficient as possible with minimal overhead where necessary, short of turning the entire format into an unaligned bit stream of course.

The tables below describe the legal binary formats allowed for each component of a rexlang program.

Every table row describes one of the possible bit patterns allowed for the component the table describes.

Bits are listed from most-significant bit (MSB) to least-significant bit (LSB) from left to right and are clustered in octets (8-bit bytes).

`0`s and `1`s are literal bits required to be set or cleared for the row's pattern to match.

Alpha characters are treated as bits that represent an N-bit unsigned integer ordered most-significant to least-significant bit from left to right. Bits are grouped in octets, aka bytes, in little-endian order with the least significant byte first.

The three columns A, B, C denote what type the `u32` value popped off the stack is treated as. X denotes what type the immediate value is treated as. A is popped first, followed by B if needed, then C if needed. R1 and R2 denote what type the resulting pushed values are treated as. R1 is pushed first followed by R2 if needed.

Generally, instructions with immediate values replace the no-immediate opcode's use of C.

The type names used are:
| Name  | Description               |
| ----- | ------------------------- |
| `u8`  | 8-bit unsigned integer    |
| `s8`  | 8-bit signed integer      |
| `u16` | 16-bit unsigned integer   |
| `s16` | 16-bit signed integer     |
| `u32` | 32-bit unsigned integer   |
| `s32` | 32-bit signed integer     |
| `ui`  | 32-bit unsigned integer   |
| `si`  | 32-bit signed integer     |
| `ptr` | memory address, aka `u32` |

### Opcodes
| Format                                         | Name                      | C    | B    | A    | X    | R1   | R2  | Operation                             |
| ---------------------------------------------- | ------------------------- | ---- | ---- | ---- | ---- | ---- | --- | ------------------------------------- |
| `00000000`                                     | halt                      |      |      |      |      |      |     |                                       |
| `00000001`                                     | nop                       |      |      |      |      |      |     | no operation                          |
| `00000010`                                     | eq                        |      | ui   | ui   |      | ui   |     | `b == a`                              |
| `00000011`                                     | ne                        |      | ui   | ui   |      | ui   |     | `b != a`                              |
| `00000100`                                     | le-ui                     |      | ui   | ui   |      | ui   |     | `b <= a`                              |
| `00000101`                                     | le-si                     |      | si   | si   |      | ui   |     | `b <= a`                              |
| `00000110`                                     | gt-ui                     |      | ui   | ui   |      | ui   |     | `b >  a`                              |
| `00000111`                                     | gt-si                     |      | si   | si   |      | ui   |     | `b >  a`                              |
| `00001000`                                     | lt-ui                     |      | ui   | ui   |      | ui   |     | `b <  a`                              |
| `00001001`                                     | lt-si                     |      | si   | si   |      | ui   |     | `b <  a`                              |
| `00001010`                                     | ge-ui                     |      | ui   | ui   |      | ui   |     | `b >= a`                              |
| `00001011`                                     | ge-si                     |      | si   | si   |      | ui   |     | `b >= a`                              |
| `00001100`                                     | and                       |      | ui   | ui   |      | ui   |     | `b &  a`                              |
| `00001101`                                     | or                        |      | ui   | ui   |      | ui   |     | `b \| a`                              |
| `00001110`                                     | xor                       |      | ui   | ui   |      | ui   |     | `b ^  a`                              |
| `00001111`                                     | add                       |      | ui   | ui   |      | ui   |     | `b +  a`                              |
| `00010000`                                     | sub                       |      | ui   | ui   |      | ui   |     | `b -  a`                              |
| `00010001`                                     | mul                       |      | ui   | ui   |      | ui   |     | `b *  a`                              |
| `00010010`                                     | ld-u8                     |      |      | dptr |      | u8   |     | `*( u8*)(&data[a])`                   |
| `00010011`                                     | ld-u16                    |      |      | dptr |      | u16  |     | `*(u16*)(&data[a])`                   |
| `00010100`                                     | ld-u32                    |      |      | dptr |      | u32  |     | `*(u32*)(&data[a])`                   |
| `00010101`                                     | ld-u8--offs               |      | dptr | ui   |      | u8   |     | `*( u8*)(&data[b+a])`                 |
| `00010110`                                     | ld-u16-offs               |      | dptr | ui   |      | u16  |     | `*(u16*)(&data[b+a])`                 |
| `00010111`                                     | ld-u32-offs               |      | dptr | ui   |      | u32  |     | `*(u32*)(&data[b+a])`                 |
| `00011000`                                     | ld-s8                     |      |      | dptr |      | s8   |     | `*( s8*)(&data[a])`                   |
| `00011001`                                     | ld-s16                    |      |      | dptr |      | s16  |     | `*(s16*)(&data[a])`                   |
| `00011010`                                     | ld-s8--offs               |      | dptr | ui   |      | s8   |     | `*( s8*)(&data[b+a])`                 |
| `00011011`                                     | ld-s16-offs               |      | dptr | ui   |      | s16  |     | `*(s16*)(&data[b+a])`                 |
| `00011100`                                     | st-u8                     |      | u8   | dptr |      | u8   |     | `*( u8*)(&data[a]) = b`               |
| `00011101`                                     | st-u16                    |      | u16  | dptr |      | u16  |     | `*(u16*)(&data[a]) = b`               |
| `00011110`                                     | st-u32                    |      | u32  | dptr |      | u32  |     | `*(u32*)(&data[a]) = b`               |
| `00011111`                                     | st-u8--offs               | u8   | dptr | ui   |      | u8   |     | `*( u8*)(&data[b+a]) = c`             |
| `00100000`                                     | st-u16-offs               | u16  | dptr | ui   |      | u16  |     | `*(u16*)(&data[b+a]) = c`             |
| `00100001`                                     | st-u32-offs               | u32  | dptr | ui   |      | u32  |     | `*(u32*)(&data[b+a]) = c`             |
| `00100010`                                     | st-u8--discard            |      | u8   | dptr |      |      |     | `*( u8*)(&data[a]) = b`               |
| `00100011`                                     | st-u16-discard            |      | u16  | dptr |      |      |     | `*(u16*)(&data[a]) = b`               |
| `00100100`                                     | st-u32-discard            |      | u32  | dptr |      |      |     | `*(u32*)(&data[a]) = b`               |
| `00100101`                                     | st-u8--offs-discard       | u8   | dptr | ui   |      |      |     | `*( u8*)(&data[b+a]) = c`             |
| `00100110`                                     | st-u16-offs-discard       | u16  | dptr | ui   |      |      |     | `*(u16*)(&data[b+a]) = c`             |
| `00100111`                                     | st-u32-offs-discard       | u32  | dptr | ui   |      |      |     | `*(u32*)(&data[b+a]) = c`             |
| `00101000`                                     | call                      |      |      | mptr |      | mptr |     | push IP; IP=x                         |
| `00101001`                                     | return / jump-abs         |      |      | mptr |      |      |     | IP=a                                  |
| `00101010`                                     | jump-abs-if               |      | ui   | mptr |      |      |     | IP=a if b != 0                        |
| `00101011`                                     | jump-abs-if-not           |      | ui   | mptr |      |      |     | IP=a if b == 0                        |
| `00101100`                                     | jump-rel                  |      |      | si   |      |      |     | IP+=(s32)a                            |
| `00101101`                                     | jump-rel-if               |      | ui   | si   |      |      |     | IP+=(s32)a if b != 0                  |
| `00101110`                                     | jump-rel-if-not           |      | ui   | si   |      |      |     | IP+=(s32)a if b == 0                  |
| `00101111`                                     | syscall                   |      |      | ui   |      |      |     | invoke system function `a`            |
| `00110000`                                     | shl                       |      | ui   | ui   |      | ui   |     | `b << a`                              |
| `00110001`                                     | shr                       |      | ui   | ui   |      | ui   |     | `b >> a`                              |
| `00110010`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00110011`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00110100`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00110101`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00110110`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00110111`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00111000`                                     | **RESERVED**              |      |      |      |      |      |     |                                       |
| `00111001`                                     | not                       |      |      | ui   |      | ui   |     | `!a`                                  |
| `00111010`                                     | neg                       |      |      | si   |      | si   |     | `-a`                                  |
| `00111011`                                     | discard                   |      |      | ui   |      |      |     | discards `a`                          |
| `00111100`                                     | swap                      |      | ui   | ui   |      | ui   | ui  | push a; push b                        |
| `00111101`                                     | dup                       |      |      | ui   |      | ui   | ui  | push a; push a                        |
| `00111110`                                     | dcopy                     | dptr | dptr | ui   |      | ptr  |     | memcpy(data+c, data+b, a); push `c+a` |
| `00111111`                                     | pcopy                     | dptr | mptr | ui   |      | ptr  |     | memcpy(data+c, prgm+b, a); push `c+a` |
| `01000000_xxxxxxxx`                            | push-u8                   |      |      |      | u8   | u8   |     | push (u8)x                            |
| `01000001_xxxxxxxx`                            | push-s8                   |      |      |      | s8   | s8   |     | push (s8)x                            |
| `01000010_xxxxxxxx`                            | eq-imm8                   |      |      | ui   | u8   | ui   |     | `a == x`                              |
| `01000011_xxxxxxxx`                            | ne-imm8                   |      |      | ui   | u8   | ui   |     | `a != x`                              |
| `01000100_xxxxxxxx`                            | le-ui-imm8                |      |      | ui   | u8   | ui   |     | `a <= x`                              |
| `01000101_xxxxxxxx`                            | le-si-imm8                |      |      | si   | s8   | ui   |     | `a <= x`                              |
| `01000110_xxxxxxxx`                            | gt-ui-imm8                |      |      | ui   | u8   | ui   |     | `a >  x`                              |
| `01000111_xxxxxxxx`                            | gt-si-imm8                |      |      | si   | s8   | ui   |     | `a >  x`                              |
| `01001000_xxxxxxxx`                            | lt-ui-imm8                |      |      | ui   | u8   | ui   |     | `a <  x`                              |
| `01001001_xxxxxxxx`                            | lt-si-imm8                |      |      | si   | s8   | ui   |     | `a <  x`                              |
| `01001010_xxxxxxxx`                            | ge-ui-imm8                |      |      | ui   | u8   | ui   |     | `a >= x`                              |
| `01001011_xxxxxxxx`                            | ge-si-imm8                |      |      | si   | s8   | ui   |     | `a >= x`                              |
| `01001100_xxxxxxxx`                            | and-imm8                  |      |      | ui   | u8   | ui   |     | `a &  x`                              |
| `01001101_xxxxxxxx`                            | or--imm8                  |      |      | ui   | u8   | ui   |     | `a \| x`                              |
| `01001110_xxxxxxxx`                            | xor-imm8                  |      |      | ui   | u8   | ui   |     | `a ^  x`                              |
| `01001111_xxxxxxxx`                            | add-imm8                  |      |      | ui   | u8   | ui   |     | `a +  x`                              |
| `01010000_xxxxxxxx`                            | sub-imm8                  |      |      | ui   | u8   | ui   |     | `a -  x`                              |
| `01010001_xxxxxxxx`                            | mul-imm8                  |      |      | ui   | u8   | ui   |     | `a *  x`                              |
| `01010010_xxxxxxxx`                            | ld-u8-imm8                |      |      |      | dptr | u8   |     | `*( u8*)(&data[x])`                   |
| `01010011_xxxxxxxx`                            | ld-u16-imm8               |      |      |      | dptr | u16  |     | `*(u16*)(&data[x])`                   |
| `01010100_xxxxxxxx`                            | ld-u32-imm8               |      |      |      | dptr | u32  |     | `*(u32*)(&data[x])`                   |
| `01010101_xxxxxxxx`                            | ld-u8--offs-imm8          |      |      | ui   | dptr | u8   |     | `*( u8*)(&data[x+a])`                 |
| `01010110_xxxxxxxx`                            | ld-u16-offs-imm8          |      |      | ui   | dptr | u16  |     | `*(u16*)(&data[x+a])`                 |
| `01010111_xxxxxxxx`                            | ld-u32-offs-imm8          |      |      | ui   | dptr | u32  |     | `*(u32*)(&data[x+a])`                 |
| `01011000_xxxxxxxx`                            | ld-s8-imm8                |      |      |      | dptr | s8   |     | `*( s8*)(&data[x])`                   |
| `01011001_xxxxxxxx`                            | ld-s16-imm8               |      |      |      | dptr | s16  |     | `*(s16*)(&data[x])`                   |
| `01011010_xxxxxxxx`                            | ld-s8--offs-imm8          |      |      | ui   | dptr | s8   |     | `*( s8*)(&data[x+a])`                 |
| `01011011_xxxxxxxx`                            | ld-s16-offs-imm8          |      |      | ui   | dptr | s16  |     | `*(s16*)(&data[x+a])`                 |
| `01011100_xxxxxxxx`                            | st-u8-imm8                |      |      | u8   | dptr | u8   |     | `*( u8*)(&data[x]) = a`               |
| `01011101_xxxxxxxx`                            | st-u16-imm8               |      |      | u16  | dptr | u16  |     | `*(u16*)(&data[x]) = a`               |
| `01011110_xxxxxxxx`                            | st-u32-imm8               |      |      | u32  | dptr | u32  |     | `*(u32*)(&data[x]) = a`               |
| `01011111_xxxxxxxx`                            | st-u8--offs-imm8          |      | u8   | ui   | dptr | u8   |     | `*( u8*)(&data[x+a]) = b`             |
| `01100000_xxxxxxxx`                            | st-u16-offs-imm8          |      | u16  | ui   | dptr | u16  |     | `*(u16*)(&data[x+a]) = b`             |
| `01100001_xxxxxxxx`                            | st-u32-offs-imm8          |      | u32  | ui   | dptr | u32  |     | `*(u32*)(&data[x+a]) = b`             |
| `01100010_xxxxxxxx`                            | st-u8--discard-imm8       |      |      | u8   | dptr |      |     | `*( u8*)(&data[x]) = a`               |
| `01100011_xxxxxxxx`                            | st-u16-discard-imm8       |      |      | u16  | dptr |      |     | `*(u16*)(&data[x]) = a`               |
| `01100100_xxxxxxxx`                            | st-u32-discard-imm8       |      |      | u32  | dptr |      |     | `*(u32*)(&data[x]) = a`               |
| `01100101_xxxxxxxx`                            | st-u8--offs-discard-imm8  |      | u8   | ui   | dptr |      |     | `*( u8*)(&data[x+a]) = b`             |
| `01100110_xxxxxxxx`                            | st-u16-offs-discard-imm8  |      | u16  | ui   | dptr |      |     | `*(u16*)(&data[x+a]) = b`             |
| `01100111_xxxxxxxx`                            | st-u32-offs-discard-imm8  |      | u32  | ui   | dptr |      |     | `*(u32*)(&data[x+a]) = b`             |
| `01101000_xxxxxxxx`                            | call-imm8                 |      |      |      | mptr | mptr |     | push IP; IP=x                         |
| `01101001_xxxxxxxx`                            | jump-abs-imm8             |      |      |      | mptr |      |     | IP=x                                  |
| `01101010_xxxxxxxx`                            | jump-abs-if-imm8          |      |      | ui   | mptr |      |     | IP=x if a != 0                        |
| `01101011_xxxxxxxx`                            | jump-abs-if-not-imm8      |      |      | ui   | mptr |      |     | IP=x if a == 0                        |
| `01101100_xxxxxxxx`                            | jump-rel-imm8             |      |      |      | si   |      |     | IP+=(s8)x                             |
| `01101101_xxxxxxxx`                            | jump-rel-if-imm8          |      |      | ui   | si   |      |     | IP+=(s8)x if a != 0                   |
| `01101110_xxxxxxxx`                            | jump-rel-if-not-imm8      |      |      | ui   | si   |      |     | IP+=(s8)x if a == 0                   |
| `01101111_xxxxxxxx`                            | syscall-imm8              |      |      |      | ui   |      |     | invoke system function `x`            |
| `01110000_000xxxxx`                            | shl-imm-u8                |      |      | ui   | u8   | ui   |     | `a << x`                              |
| `01110001_000xxxxx`                            | shr-imm-u8                |      |      | ui   | u8   | ui   |     | `a >> x`                              |
| `01110010_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01110011_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01110100_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01110101_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01110110_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01110111_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111000_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111001_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111010_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111011_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111100_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111101_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111110_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `01111111_xxxxxxxx`                            | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10000000_xxxxxxxx_xxxxxxxx`                   | push-u16                  |      |      |      | u16  | u16  |     | push (u16)x                           |
| `10000001_xxxxxxxx_xxxxxxxx`                   | push-s16                  |      |      |      | s16  | s16  |     | push (s16)x                           |
| `10000010_xxxxxxxx_xxxxxxxx`                   | eq-imm16                  |      |      | ui   | u16  | ui   |     | `a == x`                              |
| `10000011_xxxxxxxx_xxxxxxxx`                   | ne-imm16                  |      |      | ui   | u16  | ui   |     | `a != x`                              |
| `10000100_xxxxxxxx_xxxxxxxx`                   | le-ui-imm16               |      |      | ui   | u16  | ui   |     | `a <= x`                              |
| `10000101_xxxxxxxx_xxxxxxxx`                   | le-si-imm16               |      |      | si   | s16  | ui   |     | `a <= x`                              |
| `10000110_xxxxxxxx_xxxxxxxx`                   | gt-ui-imm16               |      |      | ui   | u16  | ui   |     | `a >  x`                              |
| `10000111_xxxxxxxx_xxxxxxxx`                   | gt-si-imm16               |      |      | si   | s16  | ui   |     | `a >  x`                              |
| `10001000_xxxxxxxx_xxxxxxxx`                   | lt-ui-imm16               |      |      | ui   | u16  | ui   |     | `a <  x`                              |
| `10001001_xxxxxxxx_xxxxxxxx`                   | lt-si-imm16               |      |      | si   | s16  | ui   |     | `a <  x`                              |
| `10001010_xxxxxxxx_xxxxxxxx`                   | ge-ui-imm16               |      |      | ui   | u16  | ui   |     | `a >= x`                              |
| `10001011_xxxxxxxx_xxxxxxxx`                   | ge-si-imm16               |      |      | si   | s16  | ui   |     | `a >= x`                              |
| `10001100_xxxxxxxx_xxxxxxxx`                   | and-imm16                 |      |      | ui   | u16  | ui   |     | `a &  x`                              |
| `10001101_xxxxxxxx_xxxxxxxx`                   | or--imm16                 |      |      | ui   | u16  | ui   |     | `a \| x`                              |
| `10001110_xxxxxxxx_xxxxxxxx`                   | xor-imm16                 |      |      | ui   | u16  | ui   |     | `a ^  x`                              |
| `10001111_xxxxxxxx_xxxxxxxx`                   | add-imm16                 |      |      | ui   | u16  | ui   |     | `a +  x`                              |
| `10010000_xxxxxxxx_xxxxxxxx`                   | sub-imm16                 |      |      | ui   | u16  | ui   |     | `a -  x`                              |
| `10010001_xxxxxxxx_xxxxxxxx`                   | mul-imm16                 |      |      | ui   | u16  | ui   |     | `a *  x`                              |
| `10010010_xxxxxxxx_xxxxxxxx`                   | ld-u8-imm16               |      |      |      | dptr | u8   |     | `*( u8*)(&data[x])`                   |
| `10010011_xxxxxxxx_xxxxxxxx`                   | ld-u16-imm16              |      |      |      | dptr | u16  |     | `*(u16*)(&data[x])`                   |
| `10010100_xxxxxxxx_xxxxxxxx`                   | ld-u32-imm16              |      |      |      | dptr | u32  |     | `*(u32*)(&data[x])`                   |
| `10010101_xxxxxxxx_xxxxxxxx`                   | ld-u8--offs-imm16         |      |      | ui   | dptr | u8   |     | `*( u8*)(&data[x+a])`                 |
| `10010110_xxxxxxxx_xxxxxxxx`                   | ld-u16-offs-imm16         |      |      | ui   | dptr | u16  |     | `*(u16*)(&data[x+a])`                 |
| `10010111_xxxxxxxx_xxxxxxxx`                   | ld-u32-offs-imm16         |      |      | ui   | dptr | u32  |     | `*(u32*)(&data[x+a])`                 |
| `10011000_xxxxxxxx_xxxxxxxx`                   | ld-s8-imm16               |      |      |      | dptr | s8   |     | `*( s8*)(&data[x])`                   |
| `10011001_xxxxxxxx_xxxxxxxx`                   | ld-s16-imm16              |      |      |      | dptr | s16  |     | `*(s16*)(&data[x])`                   |
| `10011010_xxxxxxxx_xxxxxxxx`                   | ld-s8--offs-imm16         |      |      | ui   | dptr | s8   |     | `*( s8*)(&data[x+a])`                 |
| `10011011_xxxxxxxx_xxxxxxxx`                   | ld-s16-offs-imm16         |      |      | ui   | dptr | s16  |     | `*(s16*)(&data[x+a])`                 |
| `10011100_xxxxxxxx_xxxxxxxx`                   | st-u8-imm16               |      |      | u8   | dptr | u8   |     | `*( u8*)(&data[x]) = a`               |
| `10011101_xxxxxxxx_xxxxxxxx`                   | st-u16-imm16              |      |      | u16  | dptr | u16  |     | `*(u16*)(&data[x]) = a`               |
| `10011110_xxxxxxxx_xxxxxxxx`                   | st-u32-imm16              |      |      | u32  | dptr | u32  |     | `*(u32*)(&data[x]) = a`               |
| `10011111_xxxxxxxx_xxxxxxxx`                   | st-u8--offs-imm16         |      | u8   | ui   | dptr | u8   |     | `*( u8*)(&data[x+a]) = b`             |
| `10100000_xxxxxxxx_xxxxxxxx`                   | st-u16-offs-imm16         |      | u16  | ui   | dptr | u16  |     | `*(u16*)(&data[x+a]) = b`             |
| `10100001_xxxxxxxx_xxxxxxxx`                   | st-u32-offs-imm16         |      | u32  | ui   | dptr | u32  |     | `*(u32*)(&data[x+a]) = b`             |
| `10100010_xxxxxxxx_xxxxxxxx`                   | st-u8--discard-imm16      |      |      | u8   | dptr |      |     | `*( u8*)(&data[x]) = a`               |
| `10100011_xxxxxxxx_xxxxxxxx`                   | st-u16-discard-imm16      |      |      | u16  | dptr |      |     | `*(u16*)(&data[x]) = a`               |
| `10100100_xxxxxxxx_xxxxxxxx`                   | st-u32-discard-imm16      |      |      | u32  | dptr |      |     | `*(u32*)(&data[x]) = a`               |
| `10100101_xxxxxxxx_xxxxxxxx`                   | st-u8--offs-discard-imm16 |      | u8   | ui   | dptr |      |     | `*( u8*)(&data[x+a]) = b`             |
| `10100110_xxxxxxxx_xxxxxxxx`                   | st-u16-offs-discard-imm16 |      | u16  | ui   | dptr |      |     | `*(u16*)(&data[x+a]) = b`             |
| `10100111_xxxxxxxx_xxxxxxxx`                   | st-u32-offs-discard-imm16 |      | u32  | ui   | dptr |      |     | `*(u32*)(&data[x+a]) = b`             |
| `10101000_xxxxxxxx_xxxxxxxx`                   | call-imm16                |      |      |      | mptr | mptr |     | push IP; IP=x                         |
| `10101001_xxxxxxxx_xxxxxxxx`                   | jump-abs-imm16            |      |      |      | mptr |      |     | IP=x                                  |
| `10101010_xxxxxxxx_xxxxxxxx`                   | jump-abs-if-imm16         |      |      | ui   | mptr |      |     | IP=x if a != 0                        |
| `10101011_xxxxxxxx_xxxxxxxx`                   | jump-abs-if-not-imm16     |      |      | ui   | mptr |      |     | IP=x if a == 0                        |
| `10101100_xxxxxxxx_xxxxxxxx`                   | jump-rel-imm16            |      |      |      | s16  |      |     | IP+=(s16)x                            |
| `10101101_xxxxxxxx_xxxxxxxx`                   | jump-rel-if-imm16         |      |      | ui   | s16  |      |     | IP+=(s16)x if a != 0                  |
| `10101110_xxxxxxxx_xxxxxxxx`                   | jump-rel-if-not-imm16     |      |      | ui   | s16  |      |     | IP+=(s16)x if a == 0                  |
| `10101111_xxxxxxxx_xxxxxxxx`                   | syscall-imm16             |      |      |      | u16  |      |     | invoke system function `x`            |
| `10110000_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110001_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110010_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110011_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110100_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110101_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110110_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10110111_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111000_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111001_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111010_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111011_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111100_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111101_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111110_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `10111111_xxxxxxxx_xxxxxxxx`                   | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11000000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | push-u32                  |      |      |      | u32  | u32  |     | push (u32)x                           |
| `11000001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | push-s32                  |      |      |      | s32  | s32  |     | push (s32)x                           |
| `11000010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | eq-imm32                  |      |      | ui   | u32  | ui   |     | `a == x`                              |
| `11000011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ne-imm32                  |      |      | ui   | u32  | ui   |     | `a != x`                              |
| `11000100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | le-ui-imm32               |      |      | ui   | u32  | ui   |     | `a <= x`                              |
| `11000101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | le-si-imm32               |      |      | si   | s32  | ui   |     | `a <= x`                              |
| `11000110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | gt-ui-imm32               |      |      | ui   | u32  | ui   |     | `a >  x`                              |
| `11000111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | gt-si-imm32               |      |      | si   | s32  | ui   |     | `a >  x`                              |
| `11001000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | lt-ui-imm32               |      |      | ui   | u32  | ui   |     | `a <  x`                              |
| `11001001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | lt-si-imm32               |      |      | si   | s32  | ui   |     | `a <  x`                              |
| `11001010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ge-ui-imm32               |      |      | ui   | u32  | ui   |     | `a >= x`                              |
| `11001011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ge-si-imm32               |      |      | si   | s32  | ui   |     | `a >= x`                              |
| `11001100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | and-imm32                 |      |      | ui   | u32  | ui   |     | `a &  x`                              |
| `11001101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | or--imm32                 |      |      | ui   | u32  | ui   |     | `a \| x`                              |
| `11001110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | xor-imm32                 |      |      | ui   | u32  | ui   |     | `a ^  x`                              |
| `11001111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | add-imm32                 |      |      | ui   | u32  | ui   |     | `a +  x`                              |
| `11010000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | sub-imm32                 |      |      | ui   | u32  | ui   |     | `a -  x`                              |
| `11010001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | mul-imm32                 |      |      | ui   | u32  | ui   |     | `a *  x`                              |
| `11010010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-u8-imm32               |      |      |      | dptr | u8   |     | `*( u8*)(&data[x])`                   |
| `11010011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-u16-imm32              |      |      |      | dptr | u16  |     | `*(u16*)(&data[x])`                   |
| `11010100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-u32-imm32              |      |      |      | dptr | u32  |     | `*(u32*)(&data[x])`                   |
| `11010101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-u8--offs-imm32         |      |      | ui   | dptr | u8   |     | `*( u8*)(&data[x+a])`                 |
| `11010110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-u16-offs-imm32         |      |      | ui   | dptr | u16  |     | `*(u16*)(&data[x+a])`                 |
| `11010111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-u32-offs-imm32         |      |      | ui   | dptr | u32  |     | `*(u32*)(&data[x+a])`                 |
| `11011000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-s8-imm32               |      |      |      | dptr | s8   |     | `*( s8*)(&data[x])`                   |
| `11011001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-s16-imm32              |      |      |      | dptr | s16  |     | `*(s16*)(&data[x])`                   |
| `11011010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-s8--offs-imm32         |      |      | ui   | dptr | s8   |     | `*( s8*)(&data[x+a])`                 |
| `11011011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | ld-s16-offs-imm32         |      |      | ui   | dptr | s16  |     | `*(s16*)(&data[x+a])`                 |
| `11011100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u8-imm32               |      |      | u8   | dptr | u8   |     | `*( u8*)(&data[x]) = a`               |
| `11011101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u16-imm32              |      |      | u16  | dptr | u16  |     | `*(u16*)(&data[x]) = a`               |
| `11011110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u32-imm32              |      |      | u32  | dptr | u32  |     | `*(u32*)(&data[x]) = a`               |
| `11011111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u8--offs-imm32         |      | u8   | ui   | dptr | u8   |     | `*( u8*)(&data[x+a]) = b`             |
| `11100000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u16-offs-imm32         |      | u16  | ui   | dptr | u16  |     | `*(u16*)(&data[x+a]) = b`             |
| `11100001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u32-offs-imm32         |      | u32  | ui   | dptr | u32  |     | `*(u32*)(&data[x+a]) = b`             |
| `11100010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u8--discard-imm32      |      |      | u8   | dptr |      |     | `*( u8*)(&data[x]) = a`               |
| `11100011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u16-discard-imm32      |      |      | u16  | dptr |      |     | `*(u16*)(&data[x]) = a`               |
| `11100100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u32-discard-imm32      |      |      | u32  | dptr |      |     | `*(u32*)(&data[x]) = a`               |
| `11100101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u8--offs-discard-imm32 |      | u8   | ui   | dptr |      |     | `*( u8*)(&data[x+a]) = b`             |
| `11100110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u16-offs-discard-imm32 |      | u16  | ui   | dptr |      |     | `*(u16*)(&data[x+a]) = b`             |
| `11100111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | st-u32-offs-discard-imm32 |      | u32  | ui   | dptr |      |     | `*(u32*)(&data[x+a]) = b`             |
| `11101000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | call-imm32                |      |      |      | mptr | mptr |     | push IP; IP=x                         |
| `11101001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | jump-abs-imm32            |      |      |      | mptr |      |     | IP=x                                  |
| `11101010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | jump-abs-if-imm32         |      |      | ui   | mptr |      |     | IP=x if a != 0                        |
| `11101011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | jump-abs-if-not-imm32     |      |      | ui   | mptr |      |     | IP=x if a == 0                        |
| `11101100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | jump-rel-imm32            |      |      |      | s32  |      |     | IP+=(s32)x                            |
| `11101101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | jump-rel-if-imm32         |      |      | ui   | s32  |      |     | IP+=(s32)x if a != 0                  |
| `11101110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | jump-rel-if-not-imm32     |      |      | ui   | s32  |      |     | IP+=(s32)x if a == 0                  |
| `11101111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | syscall-imm32             |      |      |      | u32  |      |     | invoke system function `x`            |
| `11110000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11110111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111000_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111001_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111011_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111100_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111101_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111110_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |
| `11111111_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx` | **RESERVED**              |      |      |      |      |      |     |                                       |

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
