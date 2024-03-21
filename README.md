# Assembler for DUO64 architecture

## Description
This assembler is a custom assembler designed to translate assembly language into machine code.
It supports various directives, instructions, and features tailored for the 64DUO architecture.
It is currently (to my knowledge) the only opensource assembler targeting this architecture.
The syntax used is the same as the syntax proposed by the proprietary assemblers.

## Features
- As the assembler is still under development, it does not yet support the translation of instructions itself.
- It nevertheless supports the following directives:
  - BYTE, WORD, LONG, QUAD
  - TEXT, DATA, BSS
  - SECTION
  - GLOBAL
  - ASCII, ASCIZ
  - ORG
  - INCLUDE
  - ALIGN
  - SPACE, RESERVE

## Getting Started

### Prerequisites
- A C compiler, such as GCC, Clang or MSVC.
- CMake (or other software but the MakeFile is not provided)

### Installation
1. Clone the repository:
   ```
   git clone https://github.com/angerenage/assembler.git
   ```
2. Navigate to the project directory:
   ```
   cd assembler
   ```
3. Compile the source code with CMake:
   ```
   cmake
   cmake --build
   ```

## Usage
The assembler is run by the command:
```
./assembler input.asm
```
The output file can be renamed with the following option:
```
./assembler input.asm -o output.obj
```

### Command Line Options
- `-o <file>`: Specify the output file name.
