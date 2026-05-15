# Sub C Compiler

This is my exploration and implementation of a subset-of-C compiler.
It is based on the C89 (ANSI C) standard with a few
notable features removed.

## Features

### Broad
- [x] Lexing and parsing
- [x] Semantic analysis and type checking
- [x] Intermediate representation lowering (mostly done)
- [x] RISC-V target assembly generation (mostly done)
- [ ] Optimization

### Specific

Parsed Features:
- [x] Primitive type declarations
- [x] Function declarations
- [x] Structs declarations
- [x] Pointer declarations
- [x] Array declarations
- [x] Expressions
- [x] For, while, and Do while loops
- [x] If statements
- [x] Forward declarations

Semantic Analysis Features:
- [x] Symbol table type checking
- [x] Expression type checking
- [ ] Complete implementation of type promotion rules

Intermediate Representation Features:
- [x] Three address code based control flow graph (CFG)
- [x] Control flow structures
- [x] Stack dynamic variable allocation
- [x] Pointer references
- [x] Function calls
- [x] Arithmetic operations
- [ ] Struct allocation and handling
- [ ] Array allocation and handling
- [ ] Complete expression lowering

Target Generation Features:
- [x] Stack dynamic register allocation
- [x] Arithmetic and comparison operations
- [x] Function calls
- [x] Recursive functions
- [x] Pointer references
- [x] Control flow structures
- [ ] Liveness based register allocation
- [ ] Syscalls

## Removed Features

While most features of the C89 standard are implemented or planned to be implemented, a few are intentionally removed
to decrease complexity.

- Enums
- Unions
- Type definitions using the `typedef` keyword
- Preprocessor directives
- The `volatile` and `register` qualifiers
- The `static` storage class

## Usage

The project can be built using CMake by running

```shell
cmake -B build
cmake --build build
```

Then in the `build` directory the executable `./c_compiler`
can be run with the following arguments.

`./c_compiler [<args>] <source_file>`

\-o \<filename>

Specifies the destination file for the output of the compiler. 
With no other options, the output will be in RISC-V assembly.
If not specified, output will be printed to terminal.

\-emit-tokens

Stops the compiler after running the Lexer module and prints the list of tokens.

\-emit-ast

Stops the compiler after the parsing and semantic analysis modules and prints the AST as an ascii tree.

\-emit-ir

Stops the compiler after lowering the AST into the intermediate representation and prints the IR.

## Target

Currently, the compiler's only code generation target is the RISC-V assembly ISA. This version of the compiler was built
for usage on the [RARS](https://github.com/thethirdone/rars) simulator.

## Implementation Details and Demo

See the [wiki](https://github.com/flynow10/c-compiler/wiki) on github.