# z80_disassem

Auto disassemble a `.sna` file.


## Building

A C++20 compatible compiler is required.

```shell
git clone https://github.com/BenHanson/lexertl17
git clone https://github.com/BenHanson/parsertl17
git clone https://github.com/BenHanson/z80_assembler
git clone https://github.com/BenHanson/z80_disassem
```

* Use the `Makefile` when building on Linux
* Use the `.sln` file when building with Visual Studio

## Usage

`z80_disassem <pathname (.sna)> <start of code> <entry point>`
