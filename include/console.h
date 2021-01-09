#ifndef __CONSOLE_H
#define __CONSOLE_H

#include <types.h>

void kprintf(const char *fmt,...);
void init_vconsole();
void vcrlf();
void kputchar(char ch);
void update_cursor(int row,int col);

#endif

