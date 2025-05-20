#include "reg_file.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define RAM_SIZE        (1 << 12)
#define DISPLAY_HEIGHT  (32)
#define DISPLAY_WIDTH   (64)
#define STACK_SIZE      (32)
#define KEYPAD_SIZE     (16)
#define REG_TOTAL       (17)
#define PROG_START      (0x200)

#define OK              (0)
#define FILE_READ_ERR   (-1)
#define INST_ABRT       (-2)

/*
 * Prototypes
 */


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

unsigned char *reg_file[REG_TOTAL];

unsigned char d_timer;

unsigned char s_timer;

/*
 * Fetch-execute elements
 */

unsigned short *PC = NULL;

/*
 * Architecture logic
 */

void load_ram() {
    PC = (unsigned short *) &ram[PROG_START];
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
    short opcode = (*PC)++;

    /* Decode */
    unsigned char op = (opcode & 0xF000) >> 12;
    unsigned char Rx = (opcode & 0xF00) >> 8;
    unsigned char Ry = (opcode & 0xF0) >> 4;
    unsigned char N = (opcode & 0xF);
    unsigned char NN = (opcode & 0xFF);
    unsigned short NNN = (opcode & 0xFFF);

    return OK;
}




