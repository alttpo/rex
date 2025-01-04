# Rexlang
Rexlang is a statically typed, integer-based programming language optimized for embedded applications with tight memory constraints.

Rexlang is designed to allow applications to generate and upload rexlang programs to an embedded system which executes them with very low latency access to critical functions such as memory I/O or interfacing with hardware. The rexlang program running in the embedded system may then write messages back to the host-side application to report data.

## Virtual machine
The virtual machine uses a minimal set of native state variables to keep track of execution state:

  1. `IP`: Instruction Pointer
  2. `SP`: Stack Pointer

These state variables are not directly accessible by rexlang programs and are implementation details of the virtual machine.

## Data Types
Rexlang uses only `u8` and `u16` types for primitive values.

Pointer types `*T` are only defined for documentation purposes; the VM only deals with `u16` raw pointer values.

`vec` type is `len:u8, data:*u8`.

All pushes to the stack must record the primitive type of the value pushed. When a value is popped off the stack, its type must be checked against the parameter type it is used in. For example, it is not possible to push a `u16` value to the stack and pop it as a `u8` value or vice versa. If this were allowed it would cause the next stack pop operation to be unaligned and could lead to corrupted results or unpredictable execution behavior.

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

An out of bounds memory access (via pointer) will raise an error and the program will be halted.

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
| invoke-function      | `00111110_0aaaaaaa`                       | invoke function `a`   (  0..  $7F)                              |
| invoke-function-ext  | `00111110_1aaaaaaa_0aaaaaaa`              | invoke function `a`   ($80..$3FFF)                              |
| opcode-ext           | `00111111_0xxxxxxx`                       | invoke opcode `x+$40` ($40..  $BF)                              |
| opcode               | `00xxxxxx`                                | invoke opcode `x`     (  0..  $3D)                              |
| push-array           | `01xxxxxx` [x+1 `u8` values]              | push `len`, `ptr` to array of `x+1` (1..$40) `u8` values        |
| push-4-mixed-values  | `10dcbaxx` [x+1 values]                   | push `x+1` ( 1..$4) values of mixed sizes (`a`..`d`=`u8`/`u16`) |
| push-8-mixed-values  | `110001xx_hgfedcba` [x+5 values]          | push `x+5` ( 5..$8) values of mixed sizes (`a`..`h`=`u8`/`u16`) |
| push-16-mixed-values | `11001xxx_hgfedcba_ponmlkji` [x+9 values] | push `x+9` (9..$10) values of mixed sizes (`a`..`p`=`u8`/`u16`) |
| push-16-u8-values    | `1110xxxx` [x+1 `u8` values]              | push `x+1` (1..$10) `u8` values                                 |
| push-16-u16-values   | `1111xxxx` [x+1 `u16` values]             | push `x+1` (1..$10) `u16` values                                |

The `invoke-function` instruction executes the given function.

A push-N-mixed-values instruction pushes between 1 and N mixed sized argument values on the stack where each value has its own sized specified as `0` for `u8` or `1` for `u16`.

A push-array instruction pushes on the stack a `u8` value with the length of the array (between 1 and $40 bytes), followed by a `*u8` pointer to the array data in program memory that immediately follows.

### Value formats
| Format              | Description |
| ------------------- | ----------- |
| `xxxxxxxx`          | `u8` value  |
| `xxxxxxxx_xxxxxxxx` | `u16` value |

### Opcodes
| Format   | Name           | a type | b type | c type | result type | result value           |
| -------- | -------------- | ------ | ------ | ------ | ----------- | ---------------------- |
| `000000` | return         |        |        |        |             | pop IP                 |
| `000001` | call           | u16    |        |        |             | push IP; IP=a          |
| `000010` | jump           | u16    |        |        |             | IP=a                   |
| `000011` | jump-if        | u16    | u8     |        |             | IP=a if b != 0         |
| `000100` | discard        | a      |        |        |             | discards popped value  |
| `000101` |                |        |        |        |             |                        |
| `000110` |                |        |        |        |             |                        |
| `000111` |                |        |        |        |             |                        |
| `001000` | eq             | a      | b      |        | u8          | `a == b`               |
| `001001` | ne             | a      | b      |        | u8          | `a != b`               |
| `001010` | le             | a      | b      |        | u8          | `a <= b`               |
| `001011` | gt             | a      | b      |        | u8          | `a >  b`               |
| `001100` | lt             | a      | b      |        | u8          | `a <  b`               |
| `001101` | ge             | a      | b      |        | u8          | `a >= b`               |
| `001110` | and            | a      | b      |        | max         | `a &  b`               |
| `001111` | or             | a      | b      |        | max         | `a \| b`               |
| `010000` | xor            | a      | b      |        | max         | `a ^  b`               |
| `010001` | not            | a      | -      |        | a           | `!a`                   |
| `010010` | shl            | a      | u8     |        | a           | `a << b`               |
| `010011` | shr            | a      | u8     |        | a           | `a >> b`               |
| `010100` | add            | a      | b      |        | max         | `a +  b`               |
| `010101` | sub            | a      | b      |        | max         | `a -  b`               |
| `010110` | mul            | a      | b      |        | max         | `a *  b`               |
| `010111` |                |        |        |        |             |                        |
| `011000` | inc            | a      | -      |        | a           | `a++`                  |
| `011001` | dec            | a      | -      |        | a           | `a--`                  |
| `011010` | inc-u8         | *u8    | -      |        | u8          | `*( u8*)(&D[a])++`     |
| `011011` | inc-u16        | *u16   | -      |        | u16         | `*(u16*)(&D[a])++`     |
| `011100` | dec-u8         | *u8    | -      |        | u8          | `*( u8*)(&D[a])--`     |
| `011101` | dec-u16        | *u16   | -      |        | u16         | `*(u16*)(&D[a])--`     |
| `011110` | conv-u8        | u16    | -      |        | u8          | `(u8)(a & 0xFF)`       |
| `011111` | conv-u16       | u8     | -      |        | u16         | `(u16)a`               |
| `100000` | load-u8        | *u8    | -      |        | u8          | `*( u8*)(&D[a])`       |
| `100001` | load-u16       | *u16   | -      |        | u16         | `*(u16*)(&D[a])`       |
| `100010` | load-u8-offs   | *u8    | u16    |        | u8          | `*( u8*)(&D[a+b])`     |
| `100011` | load-u16-offs  | *u16   | u16    |        | u16         | `*(u16*)(&D[a+b])`     |
| `100100` | store-u8       | *u8    | u8     |        | u8          | `*( u8*)(&D[a]) = b`   |
| `100101` | store-u16      | *u16   | u16    |        | u16         | `*(u16*)(&D[a]) = b`   |
| `100110` | store-u8-offs  | *u8    | u16    | u8     | u8          | `*( u8*)(&D[a+b]) = c` |
| `100111` | store-u16-offs | *u16   | u16    | u16    | u16         | `*(u16*)(&D[a+b]) = c` |
| `101000` |                |        |        |        |             |                        |
| `101001` |                |        |        |        |             |                        |
| `101010` |                |        |        |        |             |                        |
| `101011` |                |        |        |        |             |                        |
| `101100` |                |        |        |        |             |                        |
| `101101` |                |        |        |        |             |                        |
| `101110` |                |        |        |        |             |                        |
| `101111` |                |        |        |        |             |                        |
| `110000` |                |        |        |        |             |                        |
| `110001` |                |        |        |        |             |                        |
| `110010` |                |        |        |        |             |                        |
| `110011` |                |        |        |        |             |                        |
| `110100` |                |        |        |        |             |                        |
| `110101` |                |        |        |        |             |                        |
| `110110` |                |        |        |        |             |                        |
| `110111` |                |        |        |        |             |                        |
| `111000` |                |        |        |        |             |                        |
| `111001` |                |        |        |        |             |                        |
| `111010` |                |        |        |        |             |                        |
| `111011` |                |        |        |        |             |                        |
| `111100` |                |        |        |        |             |                        |
| `111101` |                |        |        |        |             |                        |
| `111110` | function       |        |        |        |             |                        |
| `111111` | extended       |        |        |        |             |                        |

## Standard Function Library
| Code | Definition                      | Description                            |
| ---: | :------------------------------ | -------------------------------------- |
| `00` | `(exit)`                        |                                        |
| `01` | `(vec-len        *vec):u16`     |                                        |
| `02` | `(vec-reset      *vec)`         |                                        |
| `03` | `(vec-append-vec *vec vec)`     |                                        |
| `04` | `(vec-append-u8  *vec  u8):u8 ` | `*( u8*)(&D[a+1+D[a]]) = b; D[a] += 1` |
| `05` | `(vec-append-u16 *vec u16):u16` | `*(u16*)(&D[a+1+D[a]]) = b; D[a] += 2` |
| `06` | `()`                            |                                        |
| `07` | `()`                            |                                        |
| `08` | `()`                            |                                        |
| `09` | `()`                            |                                        |
| `0A` | `()`                            |                                        |
| `0B` | `()`                            |                                        |
| `0C` | `()`                            |                                        |
| `0D` | `()`                            |                                        |
| `0E` | `()`                            |                                        |
| `0F` | `()`                            |                                        |


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
