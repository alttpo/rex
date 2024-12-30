# Rexlang
Rexlang is a statically typed, integer-based programming language optimized for embedded applications with tight memory constraints.

Rexlang is designed to allow applications to generate and upload rexlang programs to an embedded system which executes them with very low latency access to critical functions such as memory I/O or interfacing with hardware. The rexlang program running in the embedded system may then write messages back to the host-side application to report data.

## Memory layout

Memory used for rexlang programs is stored in a context. A context is a contiguous array of bytes stored in RAM. A context allocation will fail with an error if there is no more memory available. Contexts are divided into 3 sections stored in the following order:

  1. data memory
  2. program memory
  3. stack memory

These memory sections are subject to the following rules:

  1. Data memory is readable and writable by programs.
  2. Program memory is only readable by programs.
  3. Stack memory may not be directly read from nor written to by programs. It is used by the VM for execution purposes only.

Any violation of the above rules will raise an error and the program will be halted.

Pointers point to memory addresses within the current context and are represented as `u16` values.

Pointers may point to addresses in either data memory or program memory.

An out of bounds memory access (via pointer) will raise an error and the program will be halted.

## Stack behavior
The stack contains only `u16` values.

## Binary program format
The rexlang binary program format is a compact and machine-friendly representation of a program as it appears in program memory.

This binary format is a byte-aligned stream so that statements are fully addressable at the byte offset without requiring bit offsets. Great effort has been expended to make sure that this format is as efficient as possible with minimal overhead where necessary, short of turning the entire format into an unaligned bit stream of course.

The tables below describe the legal binary formats allowed for each component of a rexlang program.

Every table row describes one of the possible bit patterns allowed for the component the table describes.

Bits are listed from most-significant bit (MSB) to least-significant bit (LSB) from left to right and are clustered in octets (8-bit bytes).

`0`s and `1`s are literal bits required to be set or cleared for the row's pattern to match.

Alpha characters are treated as bits that represent an N-bit unsigned integer ordered most-significant to least-significant bit from left to right. Bits are grouped in octets, aka bytes, in little-endian order with the least significant byte first.

### Instructions
| Type                 | Format                                    | Description                                                     |
| -------------------- | ----------------------------------------- | --------------------------------------------------------------- |
| push-4-mixed-values  | `00dcbaxx` [x+1 values]                   | push `x+1` ( 1..$4) values of mixed sizes (`a`..`d`=`u8`/`u16`) |
| push-8-mixed-values  | `010001xx_hgfedcba` [x+5 values]          | push `x+5` ( 5..$8) values of mixed sizes (`a`..`h`=`u8`/`u16`) |
| push-16-mixed-values | `01001xxx_hgfedcba_ponmlkji` [x+9 values] | push `x+9` (9..$10) values of mixed sizes (`a`..`p`=`u8`/`u16`) |
| push-16-u8-values    | `0110xxxx` [x+1 values]                   | push `x+1` (1..$10) u8 values                                   |
| push-16-u16-values   | `0111xxxx` [x+1 values]                   | push `x+1` (1..$10) u16 values                                  |
| push-array           | `10xxxxxx` [x+1 `u8` values]              | push `len`, `ptr` to array of `x+1` (1..$40) `u8` values        |
| invoke-function      | `11111110_0aaaaaaa`                       | invoke function `a`   (  0..  $7F)                              |
| invoke-function-ext  | `11111110_1aaaaaaa_0aaaaaaa`              | invoke function `a`   ($80..$3FFF)                              |
| opcode               | `11xxxxxx`                                | invoke opcode `x`     (  0..  $3F)                              |
| opcode-ext           | `11111111_0xxxxxxx`                       | invoke opcode `x+$40` ($40..  $BF)                              |

The `invoke-function` instruction executes the given function.

A push-N-mixed-values instruction pushes between 1 and N mixed sized argument values on the stack where each value has its own sized specified as `0` for `u8` or `1` for `u16`. All values are converted to `u16` on the stack.

A push-array instruction pushes on the stack a `u16` value with the length of the array (between 1 and $40 bytes), followed by a `u16` pointer to the array data in program memory that immediately follows.

### Value formats
| Format              | Description |
| ------------------- | ----------- |
| `xxxxxxxx`          | `u8` value  |
| `xxxxxxxx_xxxxxxxx` | `u16` value |

### Opcodes
| Format   | Name      | Arg1 | Arg2 | Result                      |
| -------- | --------- | ---- | ---- | --------------------------- |
| `000000` | eq        | a    | b    | `a == b`                    |
| `000001` | ne        | a    | b    | `a != b`                    |
| `000010` | le        | a    | b    | `a <= b`                    |
| `000011` | gt        | a    | b    | `a  > b`                    |
| `000100` | lt        | a    | b    | `a <  b`                    |
| `000101` | ge        | a    | b    | `a >= b`                    |
| `000110` | and       | a    | b    | `a & b`                     |
| `000111` | or        | a    | b    | `a \| b`                    |
| `001000` | xor       | a    | b    | `a ^ b`                     |
| `001001` | shl       | a    | bits | `a << b`                    |
| `001010` | shr       | a    | bits | `a >> b`                    |
| `001011` | not       | a    |      | `!a`                        |
| `001100` | add       | a    | b    | `a + b`                     |
| `001101` | sub       | a    | b    | `a - b`                     |
| `001110` |           |      |      |                             |
| `001111` |           |      |      |                             |
| `010000` |           |      |      |                             |
| `010001` |           |      |      |                             |
| `010010` |           |      |      |                             |
| `010011` |           |      |      |                             |
| `010100` |           |      |      |                             |
| `010101` |           |      |      |                             |
| `010110` |           |      |      |                             |
| `010111` |           |      |      |                             |
| `011000` |           |      |      |                             |
| `011001` |           |      |      |                             |
| `011010` |           |      |      |                             |
| `011011` |           |      |      |                             |
| `011100` |           |      |      |                             |
| `011101` |           |      |      |                             |
| `011110` |           |      |      |                             |
| `011111` |           |      |      |                             |
| `100000` | load-u8   | ptr  |      | u8                          |
| `100001` | load-u16  | ptr  |      | u16                         |
| `100010` | store-u8  | ptr  | val  |                             |
| `100011` | store-u16 | ptr  | val  |                             |
| `100100` |           |      |      |                             |
| `100101` |           |      |      |                             |
| `100110` |           |      |      |                             |
| `100111` |           |      |      |                             |
| `101000` |           |      |      |                             |
| `101001` |           |      |      |                             |
| `101010` |           |      |      |                             |
| `101011` |           |      |      |                             |
| `101100` |           |      |      |                             |
| `101101` |           |      |      |                             |
| `101110` |           |      |      |                             |
| `101111` |           |      |      |                             |
| `110000` |           |      |      |                             |
| `110001` |           |      |      |                             |
| `110010` |           |      |      |                             |
| `110011` |           |      |      |                             |
| `110100` |           |      |      |                             |
| `110101` |           |      |      |                             |
| `110110` |           |      |      |                             |
| `110111` |           |      |      |                             |
| `111000` |           |      |      |                             |
| `111001` |           |      |      |                             |
| `111010` |           |      |      |                             |
| `111011` |           |      |      |                             |
| `111100` | jmp       | ptr  |      | jump to ptr                 |
| `111101` | jmp-if    | ptr  | test | jump to ptr if test is true |
| `111110` | function  |      |      |                             |
| `111111` | extended  |      |      |                             |

## Standard Function Library
| Code | Definition                                               | Description                                                          |
| ---: | :------------------------------------------------------- | -------------------------------------------------------------------- |
| `00` | `()`                                                     |                                                                      |
| `01` | `()`                                                     |                                                                      |
| `02` | `()`                                                     |                                                                      |
| `03` | `()`                                                     |                                                                      |
| `04` | `()`                                                     |                                                                      |
| `05` | `()`                                                     |                                                                      |
| `06` | `()`                                                     |                                                                      |
| `07` | `()`                                                     |                                                                      |
| `08` | `()`                                                     |                                                                      |
| `09` | `()`                                                     |                                                                      |
| `0A` | `()`                                                     |                                                                      |
| `0B` | `()`                                                     |                                                                      |
| `0C` | `()`                                                     |                                                                      |
| `0D` | `()`                                                     |                                                                      |
| `0E` | `()`                                                     |                                                                      |
| `0F` | `()`                                                     |                                                                      |
| `10` | `()`                                                     |                                                                      |
| `11` | `()`                                                     |                                                                      |
| `12` | `()`                                                     |                                                                      |
| `13` | `()`                                                     |                                                                      |
| `14` | `()`                                                     |                                                                      |
| `15` | `()`                                                     |                                                                      |
| `16` | `()`                                                     |                                                                      |
| `17` | `()`                                                     |                                                                      |
| `18` | `()`                                                     |                                                                      |
| `19` | `()`                                                     |                                                                      |
| `1A` | `()`                                                     |                                                                      |
| `1B` | `()`                                                     |                                                                      |
| `1C` | `()`                                                     |                                                                      |
| `1D` | `()`                                                     |                                                                      |
| `1E` | `()`                                                     |                                                                      |
| `1F` | `()`                                                     |                                                                      |
| `20` | `()`                                                     |                                                                      |
| `21` | `()`                                                     |                                                                      |
| `22` | `()`                                                     |                                                                      |
| `23` | `()`                                                     |                                                                      |
| `24` | `()`                                                     |                                                                      |
| `25` | `()`                                                     |                                                                      |
| `26` | `()`                                                     |                                                                      |
| `27` | `()`                                                     |                                                                      |
| `28` | `()`                                                     |                                                                      |
| `29` | `()`                                                     |                                                                      |
| `2A` | `()`                                                     |                                                                      |
| `2B` | `()`                                                     |                                                                      |
| `2C` | `()`                                                     |                                                                      |
| `2D` | `()`                                                     |                                                                      |
| `2E` | `()`                                                     |                                                                      |
| `2F` | `()`                                                     |                                                                      |

```
; read data from chip memory:
(chip-read-u8   chip:u8 offs:u16):u8
(chip-read-u16  chip:u8 offs:u16):u16
(chip-read      chip:u8 offs:u16 len:u16 dest:*vec):*vec

; write data to chip memory:
(chip-write-u8  chip:u8 offs:u16 src:u8):u8
(chip-write-u16 chip:u8 offs:u16 src:u16):u16
(chip-write     chip:u8 offs:u16 src:u8...)
```
