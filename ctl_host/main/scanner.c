#include "scanner.h"

#define SCANNER_UART_NUM 2

unsigned char scanner_data[1024] = {0};

unsigned int scanner_init(){
    IoTGpioInit(11);  // 使用GPIO，都需要调用该接口
    hi_io_set_func(11,HI_IO_FUNC_GPIO_11_UART2_TXD);
    IoTGpioInit(12);  // 使用GPIO，都需要调用该接口
    hi_io_set_func(12,HI_IO_FUNC_GPIO_12_UART2_RXD);
    // 初始化UART配置，115200，数据bit为8, 停止位1，奇偶校验为NONE，流控为NONE
    IotUartAttribute g_uart_cfg = {115200, 8, 1, IOT_UART_PARITY_NONE, 
            IOT_UART_BLOCK_STATE_BLOCK, IOT_UART_BLOCK_STATE_BLOCK, 0};
    return IoTUartInit(SCANNER_UART_NUM, &g_uart_cfg);
}

unsigned int scanner_wakeup(){
    static unsigned char cmd[]={0x00};
    int ret=IoTUartWrite(SCANNER_UART_NUM,cmd,sizeof(cmd));
    usleep(50000);
    return ret;
}

unsigned int scanner_start(){
    static unsigned char cmd[]={0x04,0xE4,0x04,0x00,0xFF,0x14};
    return IoTUartWrite(SCANNER_UART_NUM,cmd,sizeof(cmd));
}

unsigned int scanner_stop(){
    static unsigned char cmd[]={0x04,0xE4,0x04,0x00,0xFF,0x13};
    return IoTUartWrite(SCANNER_UART_NUM,cmd,sizeof(cmd));
}

unsigned int scanner_read(){
    return IoTUartRead(SCANNER_UART_NUM, scanner_data, sizeof(scanner_data));
}
