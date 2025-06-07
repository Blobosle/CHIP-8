#include "port.h"

#include <stdio.h>

#define ROM ("test.bin")

int main() {
    load_ram();
    load_rom(ROM);
    inst_cycle();
    inst_cycle();
}
