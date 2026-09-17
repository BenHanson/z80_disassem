# z80_disassem

Auto disassemble a <a href="https://sinclair.wiki.zxnet.co.uk/wiki/SNA_format">.sna</a> file.

The following OP codes are recognised:

* `CALL`
* `CALL C`
* `CALL M`
* `CALL NC`
* `CALL NZ`
* `CALL P`
* `CALL PE`
* `CALL PO`
* `CALL Z`
* `DJNZ`
* `RETI`
* `RETN`
* `JP`
* `JP C`
* `JP M`
* `JP NC`
* `JP NZ`
* `JP P`
* `JP PE`
* `JP PO`
* `JP Z`
* `JR`
* `JR C`
* `JR NC`
* `JR NZ`
* `JR Z`
* `RET`
* `RST`

As supporting `JP (HL)`, `JP (IX)`, `JP (IY)` would requires simulating the opcodes, these are currently not supported.

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
