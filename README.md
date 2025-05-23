# CHIP-8

CHIP-8 emulator/interpreter made in C and SDL2.

<img width="1701" alt="Screenshot 2025-05-22 at 5 01 28 PM" src="https://github.com/user-attachments/assets/09091198-d510-4503-b5e5-0f5de6d320fe" />

## Emulator/interpreter execution

Makefile specified for ARM based macOS with SDL2.

> Inspect include directives in ```display.c``` for SDL2 implementation.

Supports usage of ```bytes.hex``` to convert hex digits into a machine instruction program ```tests/test.bin```, using ```make test```.

## Architecture design

Specifications of emulation, in which some vary from the original CHIP-8/SUPER-CHIP design components.

- Direct program access to 4 kilobytes of contiguous RAM.
- Display is 64 x 32 pixels (adjustable) monochrome.
- 16 8-bit adjacent general purpose registers & 1 16-bit register for indexing RAM.
- 128 bytes of upwards growing Stack on a separate memory space from RAM (practically implies "unlimited" stack)
- Two 8-bit timers (delay & sound).
- Fonts loaded from ```0x050``` on RAM.
- Programs loaded from ```0x200``` on RAM.
- Post 1990s instruction set execution for ```0x6``` and ```0xE``` op half-bytes.
- Prioritization of ```BNNN``` over ```BXNN``` instruction formats.
- Big-endian opcodes.
- Single 16-bit instruction cycle.

## CHIP-8 specific resources

An [article](http://devernay.free.fr/hacks/chip8/C8TECH10.HTM) for understanding the instruction set for the CHIP-8.

Sufficient background information about the architecture can be found on [Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)
