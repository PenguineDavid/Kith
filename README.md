# Kith Language

Kith is a statically typed, compiled programming language designed for clarity and explicitness. It features a clean syntax, word-based logical operators, and multiple string literal formats. This repository contains the reference compiler (transpiler to C) for Kith.

---

## Building the Compiler

### Prerequisites

- A C++ compiler supporting C++17 (e.g., `g++`, `clang++`)
- `gcc` (to compile the generated C code)

### Compile the Kith compiler

From the root of the repository(the directory containing src), run:

```bash
g++ src/*.cpp -o kith -std=c++17
```

This produces an executable, if you wish to be able to run the language from any file drop the "kith.exe" file
into your bin folder 

to compile a .kith file run the kith exe with 2 arguments for PS(power shell)

```powershell
./kith <yourfile.kith> <desired output name.exe>
```

to compile on CMD

```CMD
kith <yourfile.kith> <desired output name.exe>
```

this will produce an exe and an output.c file the output.c file is the code in C and the exe is the compiled code you can run for terminal based applications instead of just double clicking the exe file run it form the terminal

--- 

## writing your first kith program

if you are looking for a detailed explanaition here is not the place i wont be explaining how the code thats the same as other languages worksi suggjest learning js or C++ first

how it  differ from languages like C++

it has special functions similar to event scripting languages such as JS

init, setup, start, loop, shutdown(shut down is currently literaly useless WIP)

they do exactly what they sound like

### functions

you define a function like this

func name(args) {

}

it is staticly typed but i dont like languages where you have to declare the function diffrently depending on what it returns, like java for inputs you must specifi what tye of var they are like this

func add_nums(int numA, int numB) {
     return int(numA + numB)
}

as you can see you do still need to specifi what it returns 

## Operators

### boolean

to do logical operations you use key words

and,
xor,
or,
not,

and for bitwise operations

&
^
|
~

### math

standard

+-/* =

DISCLAIMER this is in very heavy WIP there will be bugs there will be missing features there will be features ive listed as complete and forgoten to add

---

i recommend instlling my extension pack for the language as well https://github.com/PenguineDavid/Kith-extension
