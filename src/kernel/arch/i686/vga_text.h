#ifndef VGA_TEXT_H
#define VGA_TEXT_H

#include <stdint.h>

void VGA_putcolor(int x, int y, uint8_t color);
void VGA_putc(char c);
void VGA_setcursor(int x, int y);
int VGA_getcursor_x();
int VGA_getcursor_y();
void VGA_clrscr();

#endif