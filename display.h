#ifndef DISPLAY_H
#define DISPLAY_H

void init_display();
void draw(unsigned char* display);
void sdl_ehandler(unsigned char* keypad);
void stop_display();

extern int should_quit;

#endif
