# Rexlang
Rexlang is a statically typed, integer-based programming language optimized for embedded applications with tight memory constraints.

## Contexts
A context in rexlang is a contiguous array of bytes stored in RAM and divided into 3 sections used for executing rexlang programs and immediate statements:

  1. program memory
  2. data memory
  3. stack memory

Statements and functions may only directly access data memory. An out of bounds memory access is treated as an error and the program is halted.

The rexlang VM stores all contexts packed end-to-end contiguously into a statically allocated fixed-size array.

Contexts are allocated explicitly by the `ctx-new` statement where the sizes of each section are measured in bytes and are provided as `u16` arguments. A context allocation will fail with an error if there is no more space available in the fixed-size array.

## Binary program format
The rexlang binary program format is a compact and machine-friendly representation of a program as it appears in the program memory of a rexlang VM context.

This binary format is a byte-aligned stream so that statements are fully addressable at the byte offset without requiring bit offsets. Great effort has been expended to make sure that this format is as efficient as possible with minimal overhead where necessary, short of turning the entire format into an unaligned bit stream of course.

The tables below describe the legal binary formats allowed for each component of a rexlang program.

Every table row describes one of the possible bit patterns allowed for the component the table describes.

Bits are listed from most-significant bit (MSB) to least-significant bit (LSB) from left to right and are clustered in octets (8-bit bytes).

`0`s and `1`s are literal bits required to be set or cleared for the row's pattern to match.

Alpha characters such as `x` and `y` are treated as groups of bits ordered MSB to LSB that represent an N-bit unsigned integer used for various purposes. The bits the comprise the full integer may cross byte boundaries.

A rexlang program is simply an ordered sequence of `statement-call`s.

Statement-calls specify a mix of literal typed argument values and function-calls whose return values are used as arguments.

### statement-call formats
| Format                              | Description                                              |
| ----------------------------------- | -------------------------------------------------------- |
| `111xxxxx_0yyyyyyy` [x arg-groups]  | call statement `y` (0..$7F) with `x` (0..$1F) arg-groups |

### arg-group formats
| Format                                    | Description                                                                          |
| ----------------------------------------- | ------------------------------------------------------------------------------------ |
| `110xxxxx_0yyyyyyy` [x+1 arg-groups]      | call function `y` (0..$7F) with `x+1` (1..$20) arg-groups, and push return values    |
| `010xbbaa` [x+1 values]                   | push `x+1`  (1..$2) values of type `a`, `b` (in order)                               |
| `011000xx_ddccbbaa` [x+1 values]          | push `x+1`  (1..$4) values of type `a`, `b`, `c`, `d` (in order)                     |
| `01100xxx_ddccbbaa_hhggffee` [x+1 values] | push `x+1`  (1..$8) values of type `a`, `b`, `c`, `d`, `e`, `f`, `g`, `h` (in order) |
| `00ttxxxx` [x+1 values]                   | push `x+1` (1..$10) values, all of type `t`                                          |

### value formats
| Format              | Description                                    |
| ------------------- | ---------------------------------------------- |
| `xxxxxxxx`          | `u8` value                                     |
| `xxxxxxxx_xxxxxxxx` | `u16` value                                    |
| `xxxxxxxx_xxxxxxxx` | `*u8` pointer in data memory as a `u16` value  |
| `xxxxxxxx_xxxxxxxx` | `*u16` pointer in data memory as a `u16` value |

### Types
A type number is a `u2`, a 2-bit unsigned integer, and represents one of four possible types as defined by this table:

| Type  | Name    | Description      | Size (bytes) |
| ----: | ------- | ---------------- | -----------: |
|   `0` | `u8`    |  8-bit uint      |            1 |
|   `1` | `u16`   | 16-bit uint      |            2 |
|   `2` | `*u8`   | pointer to `u8`  |            2 |
|   `3` | `*u16`  | pointer to `u16` |            2 |

NOTE: pointers are absolute addresses in data memory and share the `u16` format.

### Arguments
Statement-calls and function-calls specify the number `arg-group`s (_not_ the number of arguments) that follow in order to delimit the end of the statement-call/function-call.

An arg-group represents a way to pass one or more typed values as arguments to the statement/function being called.

A function-call is an arg-group where the return values of the function are passed as arguments to the outer statement/function being called.

Otherwise, there are 4 arg-groups which allow for different ways of passing literal typed values as arguments, primarily differing in the number of typed arguments and how the types are specified and/or grouped.

The total count of arguments passed to a statement/function is the sum of all its arg-groups' argument counts.

## Static Type Safety
Every statement/function has a well-defined set of typed parameters that it accepts as arguments.

Arguments are passed to parameters by position in the order that parameters are defined. It is not possible to bypass passing a particular parameter. All parameters must be passed an argument of the same type.

The binary program format ensures that every literal value passed to a statement/function as an argument is of a specific type. These argument types must exactly match the statement/function's defined parameter types or else a type mismatch error is raised.

One or more statement/function's arguments may be bound to the return values of a function call. The return value types of the function call must exactly match the parameter types at the positions where the function call is made or else a type mismatch error is raised. The return values are passed as arguments to the parameters in the order the return values are defined in.

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
	named-constant |
	function-call
```

| Token           | RegEx*                    | Comment |
| --------------- | ------------------------- | ------- |
| `identifier`    | `[a-z][0-9a-z_-]*`        | name of an statement or function |
| `uint`          | `[0-9A-F]+[uU]?`          | `u8` or `u16` |
| `pointer`       | `[0-9A-F]+\*[uU]?`        | pointer 0..$FFFF |
| `named-constant`| `{[0-9a-zA-Z_-\/]+}[uU]?` | compile-time lookup by name |

*The rexlang VM implementation does not use regular expressions (RegEx) for parsing the language; the syntax is convenient for documentation purposes.

Comments are discarded during translation into the binary representation.

The suffixes denote which type the value is. A missing suffix and a `u` suffix defaults to a `u8`, a `U` suffix is a `u16`.

Named constants are an ASCII-only feature intended to make programs more readable by giving names to common values. When a named-constant is encountered in rexlang ASCII representation, the compiler looks it up by its name in a table of known constants and replaces it with its equivalent typed value.

## Example program
```
; declare context 0 with program=$40 bytes, stack=$20 bytes, data=$15 bytes
(ctx-new 0u 40U 20U 15U)
; begin entering instructions into program memory of context 0
(begin)
    ; read the first byte from "nmi-exe" chip memory and if it's not zero then return
    (jmp-if 0*u (ne 0u (chip-read-u8 {nmi-exe}u 0U)))
    ; read $10 bytes from "WRAM" chip memory into data memory at 0
    (chip-read {WRAM}u 10U 0*u)
    ; write the $10 bytes from data memory to the response
    (rsp-write 10U 0*u)
    ; implied (return) to restart task
; end entering instructions and begin execution
(end)
```

## Statements
Statements are the building blocks of rexlang programs and may also be used in immediate mode.

Statements return no values and are primarily intended to produce side effects, trigger external behavior, or to affect control flow of the rexlang program.

Since statements do not return values, they cannot be used as arguments to other statements or function calls.

    '(' statement-name
        ( param-name ':' type )*
        ( param-name ':' type '...' )?
    ')'

```
; jump to offset `n` in program memory (must be a statement)
(jmp n:u16)
; jump to offset `n` if `test` != 0
(jmp-if n:u16 test:u8)
; TODO: define more...
```

## Function Descriptors
To describe each function, we'll define the concept of a function descriptor.

A function descriptor describes the static types of a function's parameters and return values. Function descriptors are used internally in the rexlang VM implementation to enforce static typing.

Let's consider an ASCII representation of a function descriptor as such:

    '(' function-name
        ( param-name ':' type )*
        ( param-name ':' type '...' )?
    ')' ':' '(' (ret-name ':' type)+ ')'

Each parameter has a name and a type.

Each return value has a name and type. There may be multiple return values.

The maximum parameter count is bounded by the maximum argument count allowed by the program binary representation.

A variadic parameter is allowed only as the final parameter and it is denoted with a `...` suffix after the type to indicate it accepts zero or more arguments.

The final `':'` after the closing `)` denotes the function's return values.

```
; compare `a` and `b`
(eq a:u8  b:u8):u8
(eq a:u16 b:u16):u8

(ne a:u8  b:u8):u8
(ne a:u16 b:u16):u8

(lt a:u8  b:u8):u8
(lt a:u16 b:u16):u8

(le a:u8  b:u8):u8
(le a:u16 b:u16):u8

(gt a:u8  b:u8):u8
(gt a:u16 b:u16):u8

(ge a:u8  b:u8):u8
(ge a:u16 b:u16):u8

; TODO: integer arithmetic

; load a value from data or program memory:
(ld-u8  ptr:*u8 ):u8
(ld-u16 ptr:*u16):u16

; store a value in data memory:
(st-u8  ptr:*u8  val:u8 ):u8
(st-u16 ptr:*u16 val:u16):u16

; load a vec from data memory:
(ld-vec ptr:*vec):vec

; append a value to a vec in data memory:
(append-u8  dest:*vec src:u8... ):*vec
(append-u16 dest:*vec src:u16...):*vec
(append-vec dest:*vec src:vec...):*vec

; read data from chip memory:
(chip-read-u8   chip:u8 offs:u16):u8
(chip-read-u16  chip:u8 offs:u16):u16
(chip-read      chip:u8 offs:u16 len:u16 dest:*vec):*vec

; write data to chip memory:
(chip-write-u8  chip:u8 offs:u16 src:u8):u8
(chip-write-u16 chip:u8 offs:u16 src:u16):u16
(chip-write     chip:u8 offs:u16 src:vec):vec
```

## Named Constants
**These values are subject to change

| Named-constant     | Value |
| ------------------ | ----: |
| `snes/WRAM`        |     0 |
| `snes/SRAM`        |     1 |
| `snes/NMI-EXE`     |     2 |
