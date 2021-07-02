M16Py
===============

A minimal Python implementation to run on an ATMega16 (or comparable Microcontrollers)

Language features
-------------------
* Datatype: 16-Bit signed integers
* Control flow: while/if
* Functions
* Direct access to the registers of the Microcontroller
* Arithmetic operators (by order of evaluation)
    * `()`
    * `**` (power)
    * `-` (unary)
    * `*`, `/`, `%`
    * `+`, `-`
    * `<<`, `>>`
    * `==`, `!=`, `>`, `>=`, `<`, `<=`
    * `not` (unary)
    * `and`
    * `or`


Build (UC)
----------------
The `uc` subfolder contains the source code to compile and flash the interpreter directly to an ATMega16 microcontroller on an old development board of TU Vienna (MCLab). The installation requires `at16prog` for flashing which can be found here: https://ti.tuwien.ac.at/ecs/teaching/courses/mclu_2014/old-versions/old_directories/software-packages/src.

The `cli.py` file at the top-level of the repository provides a serial interface to interact with the interactive terminal running on the Microcontroller.


Build (Local)
------------------
The `program` subfolder contains source code to directly compile the interpreter as an executable (`m16py`) which runs locally (providing space for more nodes and symbols than the ATMega16 version).

Usage
----------------
`cli.py` and `m16py` provide the same interface. Code can be provided line by line and will be interpreted by the program. Any statements that are not function definitions will be immediately evaluated as soon as they are finished.

In both versions a `load` command can be used to load the content of a given file and send it to the interpreter line by line. The `load` command is not part of the interpreter itself, it is only a convenience function.

The language provides some builtin functions:
* `print(val)`: Prints `val` as a decimal and then a newline character
* `put(c, ...)`: Prints character `c` (Only the lower 8 bits of `c` are considered). Any number of characters can be passed as indivial arguments which will all be printed.
* `putval(val)`: Prints `val` as a decimal without a newline character
* `get(reg)`: Gets the value of the register at `reg` (given as integer). The address of the register needs to be known and can be looked up at the datasheet of the microcontroller.
* `set(reg, val)`: Sets The register `reg` to `val`. The address again needs to be known
* `debug`: Anytime the `debug` string is detected, the interpreter will print out some debug information and an overview of all defined nodes. The `debug` command has no effect on the rest of the language (i.e. it can be inserted at any point without affecting the functionality).
* `yeet`: Removes all nodes and clears the symbol table.

The `get` and `set` commands will cause segmentation faults when run in the local version as they try to read/write memory not belonging to the program.

Example usage:
```
./m16py
>load ../tests/gcd.py
>gcd(15048, 22572)
GCD=7524
>
```