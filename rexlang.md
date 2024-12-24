# Rexlang
Rexlang is a statically typed, integer-based programming language optimized for embedded applications with tight memory constraints.

Rexlang is designed to allow applications to generate and upload rexlang programs to an embedded system which executes them with very low latency access to critical functions such as memory I/O or interfacing with hardware. The rexlang program running in the embedded system may then write messages back to the host-side application to report data.

## Binary program format
The rexlang binary program format is a compact and machine-friendly representation of a program as it appears in program memory.

This binary format is a byte-aligned stream so that statements are fully addressable at the byte offset without requiring bit offsets. Great effort has been expended to make sure that this format is as efficient as possible with minimal overhead where necessary, short of turning the entire format into an unaligned bit stream of course.

The tables below describe the legal binary formats allowed for each component of a rexlang program.

Every table row describes one of the possible bit patterns allowed for the component the table describes.

Bits are listed from most-significant bit (MSB) to least-significant bit (LSB) from left to right and are clustered in octets (8-bit bytes).

`0`s and `1`s are literal bits required to be set or cleared for the row's pattern to match.

Alpha characters such as `x` and `y` are treated as groups of bits ordered MSB to LSB that represent an N-bit unsigned integer used for various purposes. The bits that comprise the full integer may cross byte boundaries.

### Instructions
| Type               | Format                                    | Description                                                     |
| ------------------ | ----------------------------------------- | --------------------------------------------------------------- |
| invoke-function    | `01111111_0aaaaaaa`                       | invoke function `a`  (  0..  $7F)                               |
| invoke-function    | `01111111_1aaaaaaa_0aaaaaaa`              | invoke function `a`  ($80..$3FFF)                               |
| opcode             | `11xxxxxx`                                | invoke opcode `x`    (  0..  $3F)                               |
| push-4-arg-values  | `00xxdcba` [x+1 values]                   | push `x+1` ( 1..$4) values of sizes (`a`..`d` `u8`/`u16`)       |
| push-8-arg-values  | `010001xx_hgfedcba` [x+5 values]          | push `x+5` ( 5..$8) values of sizes (`a`..`h` `u8`/`u16`)       |
| push-16-arg-values | `01001xxx_hgfedcba_ponmlkji` [x+9 values] | push `x+9` (9..$10) values of sizes (`a`..`p` `u8`/`u16`)       |
| push-array-ptr     | `10xxxxxx` [x+1 `u8` values]              | push `len:u8`, `ptr:*u8` to array of `x+1` (1..$40) `u8` values |

The `invoke-function` instruction executes the given function.

A push-N-arg-values instruction pushes between 1 and N typed argument values on the stack where each value has its own type specified as `0` for `u8` or `1` for `u16`.

A push-array instruction pushes on the stack a `u8` value with the length of the array (between 1 and $40 bytes), followed by a `*u8` pointer to the array data in program memory that immediately follows.

### Value formats
| Format              | Description                                    |
| ------------------- | ---------------------------------------------- |
| `xxxxxxxx`          | `u8` value                                     |
| `xxxxxxxx_xxxxxxxx` | `u16` value                                    |

### Opcodes
| Format   | Name      | Arg1 | Arg2 | Result |
| -------- | --------- | ---- | ---- | ------ |
| `000000` | load-u8   | ptr  |      | u8     |
| `000001` | load-u16  | ptr  |      | u16    |
| `000010` | store-u8  | ptr  | val  |        |
| `000011` | store-u16 | ptr  | val  |        |
| `000100` | eq        | a    | b    | bool   |
| `000101` | ne        | a    | b    | bool   |
| `000110` | le        | a    | b    | bool   |
| `000111` | gt        | a    | b    | bool   |
| `001000` | lt        | a    | b    | bool   |
| `001001` | ge        | a    | b    | bool   |
| `001010` | and       | a    | b    | u16    |
| `001011` | or        | a    | b    | u16    |
| `001100` | xor       | a    | b    | u16    |
| `001101` | comp      | a    |      | u16    |
| `001110` | shl       | a    | bits | u16    |
| `001111` | shr       | a    | bits | u16    |
| `010000` | add       | a    | b    | u16    |
| `010001` | sub       | a    | b    | u16    |
| `010010` |           |      |      |        |

## Memory layout and Pointers

Memory used for rexlang programs is stored in a context. A context is a contiguous array of bytes stored in RAM. A context allocation will fail with an error if there is no more memory available. Contexts are divided into 3 sections stored in the following order:

  1. data memory
  2. program memory
  3. stack memory

Contexts are allocated explicitly by the `ctx-new` statement where the sizes of each section are measured in bytes and are provided as `u16` typed arguments.

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

## ASCII program format
```
program:
    statement*

statement:
	comment |
	statement-call

comment:
	';' comment-text '\n'

statement-call:
	'(' identifier expression* ')'

function-call:
	'(' identifier expression* ')'

expression:
	uint |
	pointer |
	array |
	named-constant |
	function-call
```

| Token           | RegEx*                    | Comment |
| --------------- | ------------------------- | ------- |
| `identifier`    | `[a-z][0-9a-z_-]*`        | name of a function |
| `uint`          | `[0-9A-F]+`               | `u16` |
| `array`         | `\$(_*[0-9A-F][0-9A-F])+` | array of `u8` values |
| `pointer`       | `&[0-9A-F]+`              | pointer 0..$FFFF |
| `named-constant`| `{[0-9a-zA-Z_-\/]+}`      | compile-time lookup by name |

*The rexlang VM implementation does not use regular expressions (RegEx) for parsing the language; the syntax is convenient for documentation purposes.

Comments are discarded during translation into the binary representation.

Named constants are an ASCII-only feature intended to make programs more readable by giving names to common values. When a named-constant is encountered in rexlang ASCII representation, the compiler looks it up by its name in a table of known constants and replaces it with its equivalent typed value.

## Example program
```
; declare context 0 with program=$40 bytes, stack=$20 bytes, data=$15 bytes
(ctx-new 0 40 20 15)
; begin entering instructions into program memory of context 0
(begin)
    ; read the first byte from "NMI-EXE" chip memory and if it's not zero then return
    (jmp-if 0 (ne 0 (chip-read-u8 {NMI-EXE} 0)))
    (chip-write {NMI-EXE} 0 $9C002C_6CEAFF)
    ; read $10 bytes from "WRAM" chip memory into data memory at 0
    (chip-read {WRAM} 10 &0)
    ; write the $10 bytes from data memory to the response
    (rsp-write 10 &0)
    (jmp 0)
; end entering instructions and begin execution
(end)
```

## Functions
Let's define a function as follows:

    '(' function-name
        ( param-name ':' type )*
        ( param-name ':' type '...' )?
    ')'
    ( ':' '('
        ( ret-name ':' type )+
    ')' )?

A function may have zero, one, or multiple parameters. Each parameter has a name and a type. The name is only for descriptive purposes.

A function may have zero, one, or multiple return values. Each return value has a name and type. The name is only for descriptive purposes.

The last parameter may be defined as variadic, denoted with a `...` suffix after the type, which accepts zero, one, or multiple arguments, all of the same type.

## Static Type Safety
Every function has a well-defined set of typed parameters that it accepts as arguments.

Arguments are passed to parameters by position in the order that parameters are defined. It is not possible to bypass passing a particular parameter. All parameters must be passed an argument of the same type.

The binary program format ensures that every literal value passed to a function as an argument is of a specific type. These argument types must exactly match the function's defined parameter types or else a type mismatch error is raised.

One or more function's arguments may be bound to the return values of a function call. The return value types of the function call must exactly match the parameter types at the positions where the function call is made or else a type mismatch error is raised. The return values are passed as arguments to the parameters in the order the return values are defined in.

## Standard Library
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
;;; List support
; append a value to a vec in data memory:
(append-u8  dest:*vec src:u8... ):*vec
(append-u16 dest:*vec src:u16...):*vec
(append-vec dest:*vec src:vec...):*vec

;;; Control flow:
; jump to offset `n` in program memory
(jmp n:u16)
; jump to offset `n` if `test` != 0
(jmp-if n:u16 test:u8)
; TODO: define more...

; read data from chip memory:
(chip-read-u8   chip:u8 offs:u16):u8
(chip-read-u16  chip:u8 offs:u16):u16
(chip-read      chip:u8 offs:u16 len:u16 dest:*vec):*vec

; write data to chip memory:
(chip-write-u8  chip:u8 offs:u16 src:u8):u8
(chip-write-u16 chip:u8 offs:u16 src:u16):u16
(chip-write     chip:u8 offs:u16 src:u8...)
```

## Named Constants
**These values are subject to change

| Named-constant     | Value |
| ------------------ | ----: |
| `snes/WRAM`        |     0 |
| `snes/SRAM`        |     1 |
| `snes/NMI-EXE`     |     2 |
