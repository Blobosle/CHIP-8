#include "port.h"
#include "reg_file.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define RAM_SIZE        (1 << 12)
#define DISPLAY_HEIGHT  (32)
#define DISPLAY_WIDTH   (64)
#define STACK_SIZE      (32)
#define KEYPAD_SIZE     (16)
#define REG_TOTAL       (16)
#define PROG_START      (0x200)

#define OK              (0)
#define FILE_READ_ERR   (-1)
#define INST_ABRT       (-2)

/*
 * Prototypes
 */

void push_stack(unsigned short);
unsigned short pop_stack();

/*
 * Memory
 */

unsigned char ram[RAM_SIZE];

unsigned char font[0xF + 1][5] = {
    {0xF0, 0x90, 0x90, 0x90, 0xF0},
    {0x20, 0x60, 0x20, 0x20, 0x70},
    {0xF0, 0x10, 0xF0, 0x80, 0xF0},
    {0xF0, 0x10, 0xF0, 0x10, 0xF0},
    {0x90, 0x90, 0xF0, 0x10, 0x10},
    {0xF0, 0x80, 0xF0, 0x10, 0xF0},
    {0xF0, 0x80, 0xF0, 0x90, 0xF0},
    {0xF0, 0x10, 0x20, 0x40, 0x40},
    {0xF0, 0x90, 0xF0, 0x90, 0xF0},
    {0xF0, 0x90, 0xF0, 0x10, 0xF0},
    {0xF0, 0x90, 0xF0, 0x90, 0x90},
    {0xE0, 0x90, 0xE0, 0x90, 0xE0},
    {0xF0, 0x80, 0x80, 0x80, 0xF0},
    {0xE0, 0x90, 0x90, 0x90, 0xE0},
    {0xF0, 0x80, 0xF0, 0x80, 0xF0},
    {0xF0, 0x80, 0xF0, 0x80, 0x80},
};

unsigned char display[DISPLAY_HEIGHT * DISPLAY_WIDTH];

/* Outside of RAM for emulation */
unsigned short stack[STACK_SIZE];

unsigned char keypad[KEYPAD_SIZE];

unsigned char reg_file[REG_TOTAL];

unsigned char reg_i;

unsigned char d_timer;

unsigned char s_timer;

/*
 * Fetch-execute elements
 */

unsigned char *PC = NULL;

unsigned short *SP = NULL;

/*
 * Architecture logic
 */

void load_ram() {
    PC = &ram[PROG_START];
    SP = &stack[0];
    srandom(time(0));
    memcpy(&ram[0x050], font, (0xF + 1) * 5);
}

int load_rom(char *file_name) {
    assert(file_name);

    FILE *fp = fopen(file_name, "rb");

    if (!fp) {
        return FILE_READ_ERR;
    }

    fseek(fp, 0, SEEK_END);
    int file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int read = fread(&ram[PROG_START], file_size, 1, fp);
    assert(read == 1);

    return OK;
}

int inst_cycle() {
    /* Fetch */
    unsigned char opcode_high = *PC++;
    unsigned char opcode_low = *PC++;

    /* Decode */
    unsigned char op = (opcode_high & 0xF0) >> 4;
    unsigned char Rx = (opcode_high & 0xF);
    unsigned char Ry = (opcode_low & 0xF0) >> 4;
    unsigned char N = (opcode_low & 0xF);
    unsigned char NN = (opcode_low & 0xFF);
    unsigned short NNN = (opcode_high & 0xF) << 8 | (opcode_low & 0xFF);

    /* Execute */
    switch (op) {
        case 0x0:
            switch (NNN) {
                case 0x0E0:
                    for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) {
                        display[i] = 0;
                    }

                    break;
                case 0x0EE:
                    PC = &ram[pop_stack()];

                    break;
            }

            break;
        case 0x1:
            PC = &ram[NNN];
            break;
        case 0x2:
            push_stack((*PC) << 8 | PC[1]);
            PC = &ram[NNN];

            break;
        case 0x3:
            if (reg_file[Rx] == NN) {
                PC += 2;
            }

            break;
        case 0x4:
            if (reg_file[Rx] != NN) {
                PC += 2;
            }

            break;
        case 0x5:
            if (reg_file[Rx] == reg_file[Ry]) {
                PC += 2;
            }

            break;
        case 0x6:
            reg_file[Rx] = NN;

            break;
        case 0x7:
            reg_file[Rx] += NN;

            break;
        case 0x8:
            switch (N) {
                case 0x0:
                    reg_file[Rx] = reg_file[Ry];

                    break;
                case 0x1:
                    reg_file[Rx] |= reg_file[Ry];

                    break;
                case 0x2:
                    reg_file[Rx] &= reg_file[Ry];

                    break;
                case 0x3:
                    reg_file[Rx] ^= reg_file[Ry];

                    break;
                case 0x4:
                    reg_file[VF] = reg_file[Rx] + reg_file[Ry] > 0xFF ? 1 : 0;
                    reg_file[Rx] += reg_file[Ry];

                    break;
                case 0x5:
                    reg_file[VF] = reg_file[Rx] < reg_file[Ry] ? 0 : 1;
                    reg_file[Rx] -= reg_file[Ry];

                    break;
                /* Using post 1990s change of instruction execution for 0x6 and 0xE */
                case 0x6:
                    reg_file[VF] = reg_file[Rx] & 0x1;
                    reg_file[Rx] >>= 1;

                    break;
                case 0x7:
                    reg_file[VF] = reg_file[Rx] < reg_file[Ry] ? 0 : 1;
                    reg_file[Rx] = reg_file[Ry] - reg_file[Rx];

                    break;
                case 0xE:
                    reg_file[VF] = (reg_file[Rx] >> 7) & 0x1;
                    reg_file[Rx] <<= 1;

                    break;
            }
        case 0x9:
            if (reg_file[Rx] != reg_file[Ry]) {
                PC += 2;
            }

            break;
        case 0xA:
            reg_i = NNN;

            break;
        case 0xB:
            /* Emulator prioritizes BNNN over BXNN instruction set */
            PC = &ram[NNN + reg_file[V0]];

            break;
        case 0xC:
            reg_file[Rx] = ((unsigned char)random() & 0xFF) & NN;

            break;
        default:
            assert(0 && "Invalid OP read");
            break;
    }

    return OK;
}

void push_stack(unsigned short data) {
    *SP = data;
    SP++;
    if (SP < &stack[STACK_SIZE - 1]) {
        assert(0 && "Stack overflow");
    }
}

unsigned short pop_stack() {
    SP--;
    return *SP;
    if (SP > &stack[0]) {
        assert(0 && "Stack underflow");
    }
}





