#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>

#include "ssd1306.h"
#include "hi_io.h"

#define MAX_COL 21
#define MAX_ROW 8

void console_init();
void console_cursor(int x,int y);
void console_clr();
void console_char(char c);
void console_char_at(int x,int y,char c);
void console_str(const char *s);
void console_str_at(int x,int y,const char *s);
void console_printf(const char *format, ...);
void console_printf_at(int x,int y,const char *format, ...);

#endif