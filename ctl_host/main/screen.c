#include "screen.h"

//#define MY_DEBUG

static int CONSOLE_CUR_X,CONSOLE_CUR_Y;

static void __console_char(char c){
    if(c=='\n'){
        ++CONSOLE_CUR_Y;
        CONSOLE_CUR_X=0;
    }else if(c=='\r'){
        CONSOLE_CUR_X=0;
    }else if(c=='\b'){
        if(CONSOLE_CUR_X>0) --CONSOLE_CUR_X;
    }else{
        ssd1306_SetCursor(CONSOLE_CUR_X*6,CONSOLE_CUR_Y*8);
        ssd1306_DrawChar(c,Font_6x8,White);
        ++CONSOLE_CUR_X;
        if(CONSOLE_CUR_X>=MAX_COL) {
            CONSOLE_CUR_X=0;++CONSOLE_CUR_Y;
        }
    }
    if(CONSOLE_CUR_Y>=MAX_ROW) {
        CONSOLE_CUR_Y=0;
        ssd1306_Fill(Black);
    }
}

//屏幕初始化
void console_init(){
    hi_io_set_func(14,HI_IO_FUNC_GPIO_14_I2C0_SCL);
    hi_io_set_func(13,HI_IO_FUNC_GPIO_13_I2C0_SDA);
    CONSOLE_CUR_X=0;
    CONSOLE_CUR_Y=0;
    ssd1306_Init();
}

//设置光标位置，是字符位置而不是像素位置
void console_cursor(int x,int y){
    if(x>=0 && x<MAX_COL) CONSOLE_CUR_X=x;
    if(y>=0 && y<MAX_ROW)  CONSOLE_CUR_Y=y;
}

//清空屏幕
void console_clr(){
    CONSOLE_CUR_X=0;
    CONSOLE_CUR_Y=0;
    ssd1306_Fill(Black);
    ssd1306_UpdateScreen();
}

//输出一个字符
void console_char(char c){
    __console_char(c);
    ssd1306_UpdateScreen();
}

//在指定位置输出一个字符
void console_char_at(int x,int y,char c){
    console_cursor(x,y);
    console_char(c);
}

//输出一个字符串
void console_str(const char *s){
#ifdef MY_DEBUG
    printf("[CONS]%s\n",s);
#endif
    char *p=s;
    while(*p!='\0') __console_char(*(p++));
    ssd1306_UpdateScreen();
}

//在指定位置输出一个字符串
void console_str_at(int x,int y,const char *s){
    console_cursor(x,y);
    console_str(s);
}

static char sbuf[128];

//printf式的输出一个字符串
void console_printf(const char *format, ...)
{
	va_list args;
	__builtin_va_start(args, format);
	vsprintf(sbuf,format, args);
	__builtin_va_end(args);
	console_str(sbuf);		
}

//在指定位置printf式的输出一个字符串
void console_printf_at(int x,int y,const char *format, ...)
{
	va_list args;
	__builtin_va_start(args, format);
	vsprintf(sbuf,format, args);
	__builtin_va_end(args);
	console_str_at(x,y,sbuf);
}
