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
| Format                       | Name              | a type | b type | c type | result type | computation                                                    |
| ---------------------------- | ----------------- | ------ | ------ | ------ | ----------- | -------------------------------------------------------------- |
| `00000000`                   | halt              |        |        |        |             |                                                                |
| `00000001`                   | discard           | a      |        |        |             | discards `a`                                                   |
| `00000010`                   | dcopy             | *u8    | *u8    | c      | *u8         | copy `c` bytes from `data[b]` to `data[a]`; push `a+c`         |
| `00000011`                   | pcopy             | *u8    | *u8    | c      | *u8         | copy `c` bytes from `prgm[b]` to `data[a]`; push `a+c`         |
| `00000100`                   | swap              | a      | b      |        |             | push a; push b                                                 |
| `00000101`                   | call              | a      |        |        |             | push IP; IP=x                                                  |
| `00000110`                   | return / jump     | a      |        |        |             | IP=a                                                           |
| `00000111`                   | jump-if           | a      | b      |        |             | IP=a if b != 0                                                 |
| `00001000`                   | jump-if-not       | a      | b      |        |             | IP=a if b == 0                                                 |
| `00001001`                   | syscall           | a      |        |        |             | invoke    system function `a`                                  |
| `00001010`                   | extcall           | a      |        |        |             | invoke extension function `a`                                  |
| `00001011`                   | eq                | a      | b      |        | u16         | `a == b`                                                       |
| `00001100`                   | ne                | a      | b      |        | u16         | `a != b`                                                       |
| `00001101`                   | le                | a      | b      |        | u16         | `a <= b`                                                       |
| `00001110`                   | gt                | a      | b      |        | u16         | `a >  b`                                                       |
| `00001111`                   | lt                | a      | b      |        | u16         | `a <  b`                                                       |
| `00010000`                   | ge                | a      | b      |        | u16         | `a >= b`                                                       |
| `00010001`                   | and               | a      | b      |        | u16         | `a &  b`                                                       |
| `00010010`                   | or                | a      | b      |        | u16         | `a \| b`                                                       |
| `00010011`                   | xor               | a      | b      |        | u16         | `a ^  b`                                                       |
| `00010100`                   | add               | a      | b      |        | u16         | `a +  b`                                                       |
| `00010101`                   | sub               | a      | b      |        | u16         | `a -  b`                                                       |
| `00010110`                   | mul               | a      | b      |        | u16         | `a *  b`                                                       |
| `00010111`                   | shl               | a      | b      |        | a           | `a << b`                                                       |
| `00011000`                   | shr               | a      | b      |        | a           | `a >> b`                                                       |
| `00011001`                   | inc               | a      |        |        | a           | `++a`                                                          |
| `00011010`                   | dec               | a      |        |        | a           | `--a`                                                          |
| `00011011`                   | not               | a      |        |        | a           | `!a`                                                           |
| `00011100`                   | neg               | a      |        |        | a           | `-a`                                                           |
| `00011101`                   | ld-u8             | *u8    |        |        | u8          | `*( u8*)(&data[a])`                                            |
| `00011110`                   | ld-u16            | *u16   |        |        | u16         | `*(u16*)(&data[a])`                                            |
| `00011111`                   | ld-u8--offs       | *u8    | u16    |        | u8          | `*( u8*)(&data[a+b])`                                          |
| `00100000`                   | ld-u16-offs       | *u16   | u16    |        | u16         | `*(u16*)(&data[a+b])`                                          |
| `00100001`                   | st-u8             | *u8    | u8     |        | u8          | `*( u8*)(&data[a]) = b`                                        |
| `00100010`                   | st-u16            | *u16   | u16    |        | u16         | `*(u16*)(&data[a]) = b`                                        |
| `00100011`                   | st-u8--offs       | *u8    | u16    | u8     | u8          | `*( u8*)(&data[a+b]) = c`                                      |
| `00100100`                   | st-u16-offs       | *u16   | u16    | u16    | u16         | `*(u16*)(&data[a+b]) = c`                                      |
| `00100101_xxxxxxxx`          | eq--imm8          | a      |        |        | u16         | `a == x`                                                       |
| `00100110_xxxxxxxx`          | ne--imm8          | a      |        |        | u16         | `a != x`                                                       |
| `00100111_xxxxxxxx`          | le--imm8          | a      |        |        | u16         | `a <= x`                                                       |
| `00101000_xxxxxxxx`          | gt--imm8          | a      |        |        | u16         | `a >  x`                                                       |
| `00101001_xxxxxxxx`          | lt--imm8          | a      |        |        | u16         | `a <  x`                                                       |
| `00101010_xxxxxxxx`          | ge--imm8          | a      |        |        | u16         | `a >= x`                                                       |
| `00101011_xxxxxxxx`          | and-imm8          | a      |        |        | u16         | `a &  x`                                                       |
| `00101100_xxxxxxxx`          | or--imm8          | a      |        |        | u16         | `a \| x`                                                       |
| `00101101_xxxxxxxx`          | xor-imm8          | a      |        |        | u16         | `a ^  x`                                                       |
| `00101110_xxxxxxxx`          | add-imm8          | a      |        |        | u16         | `a +  x`                                                       |
| `00101111_xxxxxxxx`          | sub-imm8          | a      |        |        | u16         | `a -  x`                                                       |
| `00110000_xxxxxxxx`          | mul-imm8          | a      |        |        | u16         | `a *  x`                                                       |
| `00110001_0000xxxx`          | shl-imm4          | a      |        |        | u16         | `a << x`                                                       |
| `00110010_0000xxxx`          | shr-imm4          | a      |        |        | u16         | `a >> x`                                                       |
| `00110011_xxxxxxxx`          | ld-u8--imm8       |        |        |        | u8          | `*( u8*)(&data[x])`                                            |
| `00110100_xxxxxxxx`          | ld-u16-imm8       |        |        |        | u16         | `*(u16*)(&data[x])`                                            |
| `00110101_xxxxxxxx`          | ld-u8--offs-imm8  | *u8    |        |        | u8          | `*( u8*)(&data[a+x])`                                          |
| `00110110_xxxxxxxx`          | ld-u16-offs-imm8  | *u16   |        |        | u16         | `*(u16*)(&data[a+x])`                                          |
| `00110111_xxxxxxxx`          | st-u8--imm8       | u8     |        |        | u8          | `*( u8*)(&data[x]) = a`                                        |
| `00111000_xxxxxxxx`          | st-u16-imm8       | u16    |        |        | u16         | `*(u16*)(&data[x]) = a`                                        |
| `00111001_xxxxxxxx`          | st-u8--offs-imm8  | *u8    | u8     |        | u8          | `*( u8*)(&data[a+x]) = b`                                      |
| `00111010_xxxxxxxx`          | st-u16-offs-imm8  | *u16   | u16    |        | u16         | `*(u16*)(&data[a+x]) = b`                                      |
| `00111011_xxxxxxxx`          | call-imm8         |        |        |        |             | push IP; IP=x                                                  |
| `00111100_xxxxxxxx`          | jump-imm8         |        |        |        |             | IP=x                                                           |
| `00111101_xxxxxxxx`          | jump-imm8-if      | a      |        |        |             | IP=x if a != 0                                                 |
| `00111110_xxxxxxxx`          | jump-imm8-if-not  | a      |        |        |             | IP=x if a == 0                                                 |
| `00111111_xxxxxxxx`          | syscall-imm8      |        |        |        |             | invoke    system function `x`                                  |
| `01000000_xxxxxxxx`          | extcall-imm8      |        |        |        |             | invoke extension function `x`                                  |
| `01000001_xxxxxxxx_xxxxxxxx` | eq--imm16         | a      |        |        | u16         | `a == x`                                                       |
| `01000010_xxxxxxxx_xxxxxxxx` | ne--imm16         | a      |        |        | u16         | `a != x`                                                       |
| `01000011_xxxxxxxx_xxxxxxxx` | le--imm16         | a      |        |        | u16         | `a <= x`                                                       |
| `01000100_xxxxxxxx_xxxxxxxx` | gt--imm16         | a      |        |        | u16         | `a >  x`                                                       |
| `01000101_xxxxxxxx_xxxxxxxx` | lt--imm16         | a      |        |        | u16         | `a <  x`                                                       |
| `01000110_xxxxxxxx_xxxxxxxx` | ge--imm16         | a      |        |        | u16         | `a >= x`                                                       |
| `01000111_xxxxxxxx_xxxxxxxx` | and-imm16         | a      |        |        | u16         | `a &  x`                                                       |
| `01001000_xxxxxxxx_xxxxxxxx` | or--imm16         | a      |        |        | u16         | `a \| x`                                                       |
| `01001001_xxxxxxxx_xxxxxxxx` | xor-imm16         | a      |        |        | u16         | `a ^  x`                                                       |
| `01001010_xxxxxxxx_xxxxxxxx` | add-imm16         | a      |        |        | u16         | `a +  x`                                                       |
| `01001011_xxxxxxxx_xxxxxxxx` | sub-imm16         | a      |        |        | u16         | `a -  x`                                                       |
| `01001100_xxxxxxxx_xxxxxxxx` | mul-imm16         | a      |        |        | u16         | `a *  x`                                                       |
| `01001101_xxxxxxxx_xxxxxxxx` | ld-u8--imm16      |        |        |        | u8          | `*( u8*)(&data[x])`                                            |
| `01001110_xxxxxxxx_xxxxxxxx` | ld-u16-imm16      |        |        |        | u16         | `*(u16*)(&data[x])`                                            |
| `01001111_xxxxxxxx_xxxxxxxx` | ld-u8--offs-imm16 | *u8    |        |        | u8          | `*( u8*)(&data[a+x])`                                          |
| `01010000_xxxxxxxx_xxxxxxxx` | ld-u16-offs-imm16 | *u16   |        |        | u16         | `*(u16*)(&data[a+x])`                                          |
| `01010001_xxxxxxxx_xxxxxxxx` | st-u8--imm16      | u8     |        |        | u8          | `*( u8*)(&data[x]) = a`                                        |
| `01010010_xxxxxxxx_xxxxxxxx` | st-u16-imm16      | u16    |        |        | u16         | `*(u16*)(&data[x]) = a`                                        |
| `01010011_xxxxxxxx_xxxxxxxx` | st-u8--offs-imm16 | *u8    | u8     |        | u8          | `*( u8*)(&data[a+x]) = b`                                      |
| `01010100_xxxxxxxx_xxxxxxxx` | st-u16-offs-imm16 | *u16   | u16    |        | u16         | `*(u16*)(&data[a+x]) = b`                                      |
| `01010101_xxxxxxxx_xxxxxxxx` | call-imm16        |        |        |        |             | push IP; IP=x                                                  |
| `01010110_xxxxxxxx_xxxxxxxx` | jump-imm16        |        |        |        |             | IP=x                                                           |
| `01010111_xxxxxxxx_xxxxxxxx` | jump-imm16-if     | a      |        |        |             | IP=x if a != 0                                                 |
| `01011000_xxxxxxxx_xxxxxxxx` | jump-imm16-if-not | a      |        |        |             | IP=x if a == 0                                                 |
| `01011001_xxxxxxxx_xxxxxxxx` | syscall-imm16     |        |        |        |             | invoke    system function `x`                                  |
| `01011010_xxxxxxxx_xxxxxxxx` | extcall-imm16     |        |        |        |             | invoke extension function `x`                                  |
| `01011011`                   |                   |        |        |        |             |                                                                |
| `01011100`                   |                   |        |        |        |             |                                                                |
| `01011101`                   |                   |        |        |        |             |                                                                |
| `01011110`                   |                   |        |        |        |             |                                                                |
| `01011111`                   |                   |        |        |        |             |                                                                |
| `01100000`                   |                   |        |        |        |             |                                                                |
| `01100001`                   |                   |        |        |        |             |                                                                |
| `01100010`                   |                   |        |        |        |             |                                                                |
| `01100011`                   |                   |        |        |        |             |                                                                |
| `01100100`                   |                   |        |        |        |             |                                                                |
| `01100101`                   |                   |        |        |        |             |                                                                |
| `01100110`                   |                   |        |        |        |             |                                                                |
| `01100111`                   |                   |        |        |        |             |                                                                |
| `01101000`                   |                   |        |        |        |             |                                                                |
| `01101001`                   |                   |        |        |        |             |                                                                |
| `01101010`                   |                   |        |        |        |             |                                                                |
| `01101011`                   |                   |        |        |        |             |                                                                |
| `01101100`                   |                   |        |        |        |             |                                                                |
| `01101101`                   |                   |        |        |        |             |                                                                |
| `01101110`                   |                   |        |        |        |             |                                                                |
| `01101111`                   |                   |        |        |        |             |                                                                |
| `01110000`                   |                   |        |        |        |             |                                                                |
| `01110001`                   |                   |        |        |        |             |                                                                |
| `01110010`                   |                   |        |        |        |             |                                                                |
| `01110011`                   |                   |        |        |        |             |                                                                |
| `01110100`                   |                   |        |        |        |             |                                                                |
| `01110101`                   |                   |        |        |        |             |                                                                |
| `01110110`                   |                   |        |        |        |             |                                                                |
| `01110111`                   |                   |        |        |        |             |                                                                |
| `01111000`                   |                   |        |        |        |             |                                                                |
| `01111001`                   |                   |        |        |        |             |                                                                |
| `01111010`                   |                   |        |        |        |             |                                                                |
| `01111011`                   |                   |        |        |        |             |                                                                |
| `011111xx` [x+1 bytes]       | opcode-ext        |        |        |        |             | extended opcodes                                               |
| `10xxxxxx`                   | push-u8           |        |        |        |             | push `x` (0..$3F) value                                        |
| `11dcbaxx` [x+1 values]      | push-values       |        |        |        |             | push `x+1` (1..$4) values of mixed sizes (`a`..`d`=`u8`/`u16`) |

### Value formats
| Format              | Description |
| ------------------- | ----------- |
| `xxxxxxxx`          | `u8` value  |
| `xxxxxxxx_xxxxxxxx` | `u16` value |

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
