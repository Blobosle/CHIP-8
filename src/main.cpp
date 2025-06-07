#include "port.h"
#include "display.h"

#include <stdio.h>
#include <unistd.h>

#define RED         "\x1B[31m"
#define RESET       "\x1B[0m"
#define ERR(x)      printf(RED "[ERROR] " x RESET)

int main(int argc, char **argv) {
    if (argc != 2) {
        ERR("Missing arguments: expected ROM source path\n");
        return -1;
    }

    printf("[START] Loading CHIP-8 arch\n");
    load_ram();

    printf("[START] Loading ROM: %s\n", argv[1]);
    if (load_rom(argv[1]) == FILE_READ_ERR) {
        ERR("ROM was not loaded appropriately\n");
    }

    init_display();

    while (!should_quit) {
        inst_cycle();
        sdl_ehandler(keypad);

        if (d_flag) {
            draw(display);
        }

        usleep(1500);
    }

    stop_display();
}
