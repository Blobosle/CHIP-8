#ifndef PORT_H
#define PORT_H

#define OK              (0)
#define FILE_READ_ERR   (-1)
#define INST_ABRT       (-2)
#define KEYPAD_SIZE     (16)
#define DISPLAY_HEIGHT  (32)
#define DISPLAY_WIDTH   (64)

void load_ram();
int load_rom(char *);
int inst_cycle();

extern unsigned char keypad[KEYPAD_SIZE];
extern unsigned char d_flag;
extern unsigned char s_flag;
extern unsigned char display[DISPLAY_HEIGHT * DISPLAY_WIDTH];

#endif
