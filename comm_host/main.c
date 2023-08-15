#include <stdio.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "hi_errno.h"
#include "iot_gpio.h"
#include "iot_uart.h"

#include "oled_ssd1306.h"
#include "udp_server.h"
#include "wifi_sta_connect.h"

#define STACK_SIZE (4096)


/****************************
         Keyboard
****************************/
uint8_t index_line = '0';

static void KeyboardTask(void)
{
    hi_u16 value;
    uint8_t flag = 0;
    while (1)
    {
        if (hi_adc_read(HI_ADC_CHANNEL_2, &value, HI_ADC_EQU_MODEL_2, HI_ADC_CUR_BAIS_DEFAULT, 0) != HI_ERR_SUCCESS)
        {
            printf("ADC read error!\n");
        }
        else
        {   
            // printf("value:%d\n",value);
            if(value<400){
                if(index_line>'0') --index_line;
                OledShowChar(60, 5, index_line, FONT6_X8);
            }else if(value<600){
                if(index_line<'9') ++index_line;
                OledShowChar(60, 5, index_line, FONT6_X8);
            }
        }
        usleep(200000);
    }
}

/****************************
           UART
****************************/
#define IOT_UART_IDX_2 (2)
#define STACK_SIZE (1024)
#define DELAY_US (100000)
#define IOT_GPIO_11 (11)
#define IOT_GPIO_12 (12)

unsigned char uartWriteBuff[] = {0xff, 0x30, 0x31, 0x35, 0x30};
unsigned char uartReadBuff[2048] = {0};
char expressBoxNum[] = {'0', '0', '0'};

int usr_uart_config(void)
{
    // 初始化UART配置，115200，数据bit为8, 停止位1，奇偶校验为NONE，流控为NONE
    IotUartAttribute g_uart_cfg = {115200, 8, 1, IOT_UART_PARITY_NONE, 500, 500, 0};
    int ret = IoTUartInit(IOT_UART_IDX_2, &g_uart_cfg);
    if (ret != 0)
    {
        printf("uart init fail\r\n");
    }

    return ret;
}

static void UartTask(void)
{
    while (1)
    {
        unsigned int len = IoTUartRead(IOT_UART_IDX_2, uartReadBuff, sizeof(uartReadBuff));
        if (len > 0)
        {
            // printf("Uart read data:%s\r\n", uartReadBuff);
            printf("%s\r\n", uartReadBuff);
            switch (uartReadBuff[0])
            {
            case '0':
                expressBoxNum[0] += 1;
                break;
            case '1':
                expressBoxNum[1] += 1;
                break;
            case '2':
                expressBoxNum[2] += 1;
                break;
            default:
                break;
            }
        }
        usleep(DELAY_US);
    }
    return NULL;
}


/****************************
           Main
****************************/
static void MainEntry(void)
{
    printf("System start...\r\n");
    hi_gpio_init();
    hi_io_set_func(HI_GPIO_IDX_10, HI_IO_FUNC_GPIO_10_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_10, HI_GPIO_DIR_IN);
    
    printf("OLED init...\r\n");
    OledInit();
    OledFillScreen(0);
    IoTI2cInit(0, 400000);

    printf("UART init...\r\n");
    IoTGpioInit(IOT_GPIO_11);
    hi_io_set_func(IOT_GPIO_11, HI_IO_FUNC_GPIO_11_UART2_TXD);
    IoTGpioInit(IOT_GPIO_12);
    hi_io_set_func(IOT_GPIO_12, HI_IO_FUNC_GPIO_12_UART2_RXD);
    usr_uart_config();

    printf("OLED show...\r\n");
    OledShowString(5, 2, "Current control", FONT6_X8); /* 屏幕第20列3行显示1行 */
    OledShowString(30, 3, "sequence", FONT6_X8);
    OledShowString(60, 5, "0", FONT6_X8);
    OledShowString(5, 7, " -           +", FONT6_X8);

    printf("Task Set 1 start...\r\n");
    osThreadAttr_t attr,attr2,attr3;
    
    attr.name = "KeyboardTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)KeyboardTask, NULL, &attr) == NULL){
        printf("[Keyboard] Falied to create KeyboardTask!\n");
    }
    
    attr2=attr;
    attr2.name="UartTask";

    if (osThreadNew((osThreadFunc_t)UartTask, NULL, &attr2) == NULL){
        printf("[UartTask] Falied to create UartTask!\n");
    }

    WifiStaModule();

    attr3=attr;
    attr3.name="NetTask";
    attr3.stack_size=0x1000;
    attr3.priority=osPriorityNormal3;

    if (osThreadNew(UdpServerDemo, NULL, &attr3) == NULL) {
        printf("[NetTask] Falied to create NetTask!\n");
    }
}

SYS_RUN(MainEntry);