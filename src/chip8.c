#include "port.h"
#include "reg_file.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define RAM_SIZE        (1 << 12)
#define DISPLAY_HEIGHT  (32)
#define DISPLAY_WIDTH   (64)
#define STACK_SIZE      (32)
#define KEYPAD_SIZE     (16)
#define REG_TOTAL       (16)
#define PROG_START      (0x200)
#define FONT_START      (0x050)
#define DISP_IDX(x,y)   ((y) * DISPLAY_WIDTH + (x))

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

unsigned short reg_i;

unsigned char d_timer;

unsigned char s_timer;

unsigned char d_flag;

unsigned char s_flag;

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
    memcpy(&ram[FONT_START], font, (0xF + 1) * 5);
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

    fclose(fp);
    fp = NULL;

    return OK;
}

int inst_cycle() {
    static int counter = 0;
    s_flag = 0;
    d_flag = 0;

    FILE *fp = fopen("logs.txt", "a");
    /* Fetch */
    unsigned char opcode_high = *PC++;
    unsigned char opcode_low = *PC++;

    fprintf(fp, "[%d] 0x%x%x\n", counter++, opcode_high, opcode_low);
    fclose(fp);
    fp = NULL;

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

            break;
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
        case 0xD:
            d_flag = 1;
            reg_file[VF] = 0;
            int x_start = reg_file[Rx] % DISPLAY_WIDTH;
            int y_start = reg_file[Ry] % DISPLAY_HEIGHT;

            for (int row = 0; row < N; row++) {
                if (y_start + row >= DISPLAY_HEIGHT) {
                    break;
                }

                unsigned char sprite_byte = ram[reg_i + row];

                for (int bit = 0; bit < 8; bit++) {
                    if (x_start + bit >= DISPLAY_WIDTH) {
                        break;
                    }

                    if (!(sprite_byte & (0x80 >> bit))) {
                        continue;
                    }

                    int idx = DISP_IDX(x_start + bit, y_start + row);

                    if (display[idx]) {
                        reg_file[VF] = 1;
                    }

                    display[idx] ^= 1;
                }
            }

            break;
        case 0xE:
            switch (NN) {
                case 0x9E:
                    keypad[reg_file[Rx]] ? PC += 2 : 0;

                    break;
                case 0xA1:
                    keypad[reg_file[Rx]] ? 0 : (PC += 2);

                    break;
            }
            break;
        case 0xF:
            switch (NN) {
                case 0x07:
                    reg_file[Rx] = d_timer;

                    break;
                case 0x15:
                    d_timer = reg_file[Rx];

                    break;
                case 0x18:
                    s_timer = reg_file[Rx];

                    break;
                case 0x1E:
                    reg_i += reg_file[Rx];

                    break;
                case 0x0A:
                    for (int i = 0; i < KEYPAD_SIZE; i++) {
                        if (keypad[i]) {
                            break;
                        }
                    }

                    PC -= 2;

                    break;
                case 0x29:
                    reg_i = FONT_START + reg_file[Rx] * 5;

                    break;
                case 0x33:
                    ram[reg_i] = (reg_file[Rx] % 1000) / 100;
                    ram[reg_i + 1] = (reg_file[Rx] % 100) / 10;
                    ram[reg_i + 2] = (reg_file[Rx] % 10);

                    break;
                case 0x55:
                    for (unsigned char i = 0; i <= Rx; i++) {
                        assert(reg_i + i < RAM_SIZE);
                        ram[reg_i + i] = reg_file[i];
                    }

                    break;
                case 0x65:
                    for (unsigned char i = 0; i <= Rx; i++) {
                        assert(reg_i + i < RAM_SIZE);
                        reg_file[i] = ram[reg_i + i];
                    }

                    break;
            }

            break;
        default:
            assert(0 && "Invalid OP read");
            break;
    }

    if (d_timer > 0) {
        d_timer -= 1;
    }

    if (s_timer > 0) {
        s_flag = 1;
        printf(".");
        s_timer -= 1;
    }
    return OK;
}

void push_stack(unsigned short data) {
    *SP = data;
    SP++;
    if (SP >= &stack[STACK_SIZE - 1]) {
        assert(0 && "Stack overflow");
    }
}

unsigned short pop_stack() {
    SP--;
    return *SP;
    if (SP <= &stack[0]) {
        assert(0 && "Stack underflow");
    }
}

