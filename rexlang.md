# Rexlang
Rexlang is a statically-typed, integer-based programming language optimized for embedded applications with tight memory constraints.

## Contexts
A context in rexlang is a contiguous array of memory divided into 3 sections, useful for executing rexlang programs and immediate instructions:

  1. program memory
  2. data memory
  3. stack memory

Program memory is read-only during execution, data memory is read-write, and stack memory is only accessible by the execution VM.

Any attempt by a program to directly access stack memory via a pointer will trigger an error.

The rexlang VM stores all contexts packed end-to-end contiguously into a statically allocated fixed-size array.

Contexts are allocated explicitly by the `ctx-new` instruction where the sizes of each section are measured in bytes and are provided as arguments. A context allocation will fail if there is no more space available in the fixed-size array.

## Static Typing
All values used in rexlang's data memory and stack memory are statically typed and fixed-sized, similar to C.

Rexlang has no concept of tagged or boxed values.

The following are the defined static types that values in memory may take:

| Type    | Description           | Representation     | Size (bytes) |
| ------- | --------------------- | ------------------ | -----------: |
| `u8`    |  8-bit uint           |                    |            1 |
| `u16`   | 16-bit uint           |                    |            2 |
| `u24`   | 24-bit uint           |                    |            3 |
| `u32`   | 32-bit uint           |                    |            4 |
| `vec`   | vector of bytes       | `len:u16 addr:*u8` |            4 |
| `*T`    | pointer to `T` value  | `u16`              |            2 |
| `label` | points to instruction | `*u8`              |            2 |

## The language

Rexlang programs that are executed by a rexlang VM must be formatted with the binary representation defined below. However, an ASCII representation is also defined to allow easier use by developers. The ASCII representation must be directly translated to its equivalent binary representation in order to be executable.

1. an optional ASCII representation
2. the required binary representation

## ASCII representation
```
statement:
	comment |
	instruction

comment:
	';' comment-text '\n'

instruction:
	'(' instruction-name expression* ')'

expression:
	uint |
	named-constant |
	pointer |
	array |
	instruction
```

| Expression         | RegEx                     | Comment |
| ------------------ | ------------------------- | ------- |
| `uint`             | `[0-9A-F]+`               | uint up to 32-bit |
| `named-constant`   | `#[0-9a-zA-Z_-\/]+`       | compile-time lookup by name and replaced with uint |
| `pointer`          | `&[0-9A-F]+`              | pointer 0..$FFFF : uint |
| `array`            | `\$(_*[0-9A-F][0-9A-F])*` | array of 0..31 bytes |
| `instruction-name` | `[a-z][0-9a-z_-]*`        | name of an instruction |

The rexlang VM implementation does not use regular expressions (RegEx) for parsing the language but the syntax is convenient for documentation purposes.

### Named constants
Named constants are an ASCII-only feature intended to make programs more readable by giving names to raw uint values. When a named-constant is encountered in rexlang ASCII representation, it is looked up by name and replaced with its equivalent uint value.

**These values are subject to change

| Named-constant     | Value |
| ------------------ | ----: |
| `#snes/WRAM`       |     0 |
| `#snes/SRAM`       |     1 |
| `#snes/2C00`       |     2 |

## Binary representation
The rexlang binary representation is a compact and machine-friendly representation for direct inclusion into a rexlang VM context's program memory.

A rexlang program represented in ASCII **must** be transformed into its equivalent binary representation before being submitted to the rexlang VM for execution.

This table describes the binary format of an expression stored in program memory: (`x` and `y` are big-endian with MSB to LSB order across bytes)

| Expression                                      | Description                          |
| ----------------------------------------------- | ------------------------------------ |
| `0xxxxxxx`                                      | uint up to $7F                       |
| `1000xxxx_xxxxxxxx`                             | uint up to $FFF                      |
| `10010000_xxxxxxxx_xxxxxxxx`                    | uint up to $FFFF                     |
| `10010001_xxxxxxxx_xxxxxxxx_xxxxxxxx`           | uint up to $FFFFFF                   |
| `10010010_xxxxxxxx_xxxxxxxx_xxxxxxxx_xxxxxxxx`  | uint up to $FFFFFFFF                 |
| `101xxxxx [x bytes]`                            | array of `x` (0..$F) bytes           |
| `11000xxx_yyyyyyyy [x expressions]`             | instruction `y` (0..$FF) with `x` (0..$7) arguments |

### An example program

Here we demonstrate rexlang in practice with an example program in both its ASCII and equivalent binary representations:

#### ASCII:
```
; declare context 0 with prgm=$40 bytes, stack=$20 bytes, data=$15 bytes:
(ctx-new 0 40 20 15)
; begin entering instructions into program memory of context 0:
(begin)
    ; declare variables in data section:
    (vec    &0 10 $9C002C_6CEAFF)
    (u8    &14 0)
    ; read the first byte from "snes/2C00" chip memory and if it's not zero then return
    (if (ne (st-u8 &14 (chip-read-u8 #snes/2C00 0)) 0) (return))
    ; read $10 bytes from "snes/WRAM" chip memory
    (chip-read #snes/WRAM 10 &0)
    ; write the $10 bytes to the response
    (rsp-write &0)
    ; implied (return) to restart task
; end entering instructions and begin execution
(end)
```

#### Binary:
Parentheses denote expression boundaries.
```
; declare context 0 with prgm=$40 bytes, stack=$20 bytes, data=$15 bytes:
C4 ?? (00) (40) (20) (15) ; 6 bytes
; begin entering instructions into program memory of context 0:
C0 ?? ; 2 bytes
    ; declare variables in data section:
    C3 ?? (00) (10) (A6 9C 00 2C 6C EA FF) ; 11 bytes
    C2 ?? (14) (00) ; 4 bytes
    C2 ?if? (C2 ?ne? (C2 ?st-u8? 14 (C2 ?chip-read-u8? 02 00)) 00) (C0 ?return?) ; 14 bytes
    C3 ?chip-read? (00) (10) (00) ; 5 bytes
    C1 ?rsp-write? (00) ; 3 bytes
    ; internal 37 bytes of program memory
; end entering instructions and begin execution
C0 ?end? ; 2 bytes

; total 47 bytes
```

## Instructions
A rexlang instruction has:
 * a unique instruction-name for ASCII
 * a unique numeric value for binary
 * a single return value
 * 0 to 7 parameters

To describe each instruction and its types, we'll define the concept of an instruction descriptor. An instruction descriptor describes the static types of the parameters and return value for an instruction. Instruction descriptors are used internally in the rexlang VM implementation to enforce static typing.

Let's consider an ASCII representation of an instruction descriptor as such:

    '(' instruction-name
        ( param-name ':' type )*
        ( param-name ':' type '...' )?
    ')' ':' type

The quoted parentheses `(` and `)` separate the instruction-name and its parameter descriptions from the return value description.

Each parameter has a name and a type.

The maximum parameter count is bounded by the maximum argument count allowed by the binary instruction representation (see below).

A variadic parameter is allowed only as the final parameter and it is denoted with a `...` suffix after the type to indicate it accepts zero or more arguments.

The rexlang VM always knows the count of arguments passed to an instruction at runtime since it is encoded in the instruction's binary representation. Arguments passed into a variadic parameter all share the same type so it is safe for the instruction to pop them off the stack.

The final `':' type` after the closing `)` denotes the instruction's return value type.

```
; TODO: integer arithmetic
; TODO: control flow
; TODO: context handling

; define a variable in data memory, allocate space, declare its type, and initialize its value:
(def-u8  ptr:*u8  init:u8 ):*u8
(def-u16 ptr:*u16 init:u16):*u16
(def-u24 ptr:*u24 init:u24):*u24
(def-u32 ptr:*u32 init:u32):*u32
(def-vec ptr:*vec cap:u16 init:array):*vec

; load a value from data or program memory:
(ld-u8  ptr:*u8 ):u8
(ld-u16 ptr:*u16):u16
(ld-u24 ptr:*u24):u24
(ld-u32 ptr:*u32):u32

; store a value in data memory:
(st-u8  ptr:*u8  val:u8 ):u8
(st-u16 ptr:*u16 val:u16):u16
(st-u24 ptr:*u24 val:u24):u24
(st-u32 ptr:*u32 val:u32):u32

; load a vec from data memory:
(ld-vec ptr:*vec):vec

; append a value to a vec in data memory:
(append-u8  dest:*vec src:u8... ):*vec
(append-u16 dest:*vec src:u16...):*vec
(append-u24 dest:*vec src:u24...):*vec
(append-u32 dest:*vec src:u32...):*vec
(append-vec dest:*vec src:vec...):*vec

; read data from chip memory:
(chip-read-u8   chip:u8 offs:u32):u8
(chip-read-u16  chip:u8 offs:u32):u16
(chip-read-u24  chip:u8 offs:u32):u24
(chip-read      chip:u8 offs:u32 len:u16 dest:*vec):*vec

; write data to chip memory:
(chip-write-u8  chip:u8 offs:u32 src:u8):u8
(chip-write-u16 chip:u8 offs:u32 src:u16):u16
(chip-write-u24 chip:u8 offs:u32 src:u24):u24
(chip-write     chip:u8 offs:u32 src:vec):vec
```

## Execution Stack
The execution stack is treated as an untyped array of bytes.

Typed values are pushed onto and popped off of the stack with fixed sizes determined by the static type of the value.

The types of values are _not_ recorded in stack or data memory, nor are values tagged in any way with type information. Values in memory are always represented as primitives.

Literal `uint` values encoded in program memory as instruction arguments have their value coerced to the parameter's static type.

If the return value of an instruction is used as an argument to another instruction then there must be type agreement between the producing instruction's return value and the consuming instruction's parameter to make sure the value is pushed and popped off the stack properly.

Literal `array`s are implicitly converted to `vec`s with the `addr` value pointing at the array bytes in program memory. `vec`s with addrs pointing into program memory are read-only.

By design, all instructions that need to modify `vec`s must take a `*vec`.

TODO: define errors
