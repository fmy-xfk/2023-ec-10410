/*
 * Copyright (c) 2022 HiSilicon (Shanghai) Technologies CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lwip/netifapi.h"

#include <hi_io.h>
#include <hi_gpio.h>
#include <hi_task.h>
#include <hi_watchdog.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_config.h"

#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

// #include "pwm16.h"
// PWM16 __servo;
// #define SERVO_BOARD  (&__servo )
// extern unsigned char sendFlag;

#include "iot_uart.h"
#include "oled_ssd1306.h"
#include "wifi_sta_connect.h"

extern unsigned char uartWriteBuff[];
extern char expressBoxNum[];
extern uint8_t index_line;

#define INVAILD_SOCKET          (-1)
#define FREE_CPU_TIME_20MS      (20)
#define INVALID_VALUE           "202.202.202.202"

//#define NATIVE_IP_ADDRESS       "192.168.137.218" 
// 用户查找本地IP后需要进行修改，但是现在已经不需要了
#define WECHAT_MSG_LIGHT_ON     "_light_on"
#define WECHAT_MSG_LIGHT_OFF    "_light_off"
#define DEVICE_MSG_LIGHT_ON     "device_light_on"
#define DEVICE_MSG_LIGHT_OFF    "device_light_off"
#define WECHAT_MSG_UNLOAD_PAGE  "UnoladPage"
#define WECHAT_MSG_STEERING_POSITION    "_change_position"
#define WECHAT_MSG_SPEED_CHANGE    "_change_speed"
#define WECHAT_MSG_REFRESH    "_refresh"
#define RECV_DATA_FLAG_OTHER    (2)
#define HOST_PORT               (5566)
#define DEVICE_PORT             (6655)

#define UDP_RECV_LEN (255)

typedef void (*FnMsgCallBack)(hi_gpio_value val);

typedef struct FunctionCallback {
    hi_bool stop;
    hi_u32 conLost;
    hi_u32 queueID;
    hi_u32 iotTaskID;
    FnMsgCallBack msgCallBack;
}FunctionCallback;
FunctionCallback g_gfnCallback;

int DeviceMsgCallback(FnMsgCallBack msgCallBack)
{
    g_gfnCallback.msgCallBack = msgCallBack;
    return 0;
}

int UdpTransportInit(struct sockaddr_in serAddr, struct sockaddr_in remoteAddr)
{
    int sServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (sServer == INVAILD_SOCKET) {
        printf("[UDP]create server socket failed\r\n");
        close(sServer);
    }

    // 本地主机ip和端口号
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(HOST_PORT);
    serAddr.sin_addr.s_addr = inet_addr(get_local_ip());
    if (bind(sServer, (struct sockaddr*)&serAddr, sizeof(serAddr)) == -1) {
        printf("[UDP]bind socket failed\r\n");
        close(sServer);
        return sServer = INVAILD_SOCKET;
    }

    // 对方ip和端口号
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(DEVICE_PORT);
    serAddr.sin_addr.s_addr = htons(INADDR_ANY);

    return sServer;
}

void UdpServerDemo(void)
{
    struct sockaddr_in serAddr = {0};
    struct sockaddr_in remoteAddr = {0};
    static int recvDataFlag = -1;
    char *sendData = NULL;
    int sServer = 0;

    printf("[UDP]initing...\r\n");
    sServer = UdpTransportInit(serAddr, remoteAddr);
    if(sServer == INVAILD_SOCKET) return;

    int addrLen = sizeof(remoteAddr);
    static char recvData[UDP_RECV_LEN] = {0};

    printf("[UDP]receiving...\r\n");
    while (1) {
        /* 255长度 */
        int recvLen = recvfrom(sServer, recvData, UDP_RECV_LEN, 0, (struct sockaddr*)&remoteAddr, (socklen_t*)&addrLen);
        if (recvLen>0) {
            if (strstr(inet_ntoa(remoteAddr.sin_addr), INVALID_VALUE) == NULL) {
                printf("Datagram package received:%s\r\n", inet_ntoa(remoteAddr.sin_addr));
                printf("Len=%d data=%s\r\n", recvLen, recvData);
            }

            if(strstr(recvData, WECHAT_MSG_LIGHT_OFF) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = HI_FALSE;

                switch (recvData[10])
                {
                case '0':
                    uartWriteBuff[1] = '4';
                    uartWriteBuff[2] = '0';
                    uartWriteBuff[3] = '6';
                    uartWriteBuff[4] = '4';
                    break;
                case '1':
                    uartWriteBuff[1] = '5';
                    uartWriteBuff[2] = '0';
                    uartWriteBuff[3] = '5';
                    uartWriteBuff[4] = '8';
                    break;
                case '2':
                    uartWriteBuff[1] = '6';
                    uartWriteBuff[2] = '0';
                    uartWriteBuff[3] = '5';
                    uartWriteBuff[4] = '4';
                    break;
                default:
                    break;
                }
                // sendFlag = 1;
                int ret = IoTUartWrite(2, (unsigned char *)uartWriteBuff, 5);
                printf("Uart Write data: ret = %d\r\n", ret);

            }else if (strstr(recvData, WECHAT_MSG_LIGHT_ON) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = HI_TRUE;

                switch (recvData[9])
                {
                case '0':
                    uartWriteBuff[1] = '4';
                    uartWriteBuff[2] = '1';
                    uartWriteBuff[3] = '5';
                    uartWriteBuff[4] = '8';
                    break;
                case '1':
                    uartWriteBuff[1] = '5';
                    uartWriteBuff[2] = '1';
                    uartWriteBuff[3] = '5';
                    uartWriteBuff[4] = '0';
                    break;
                case '2':
                    uartWriteBuff[1] = '6';
                    uartWriteBuff[2] = '1';
                    uartWriteBuff[3] = '4';
                    uartWriteBuff[4] = '0';
                    break;
                default:
                    break;
                }
                // sendFlag = 1;
                int ret = IoTUartWrite(2, (unsigned char *)uartWriteBuff, 5);
                printf("Uart Write data: ret = %d\r\n", ret);
                
            }else if (strstr(recvData, WECHAT_MSG_STEERING_POSITION) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = HI_TRUE;

                hi_u8 value_flag = 17;
                hi_u16 pwm_value = 0;
                while (recvData[value_flag] != '_')
                {
                    pwm_value *= 10;
                    pwm_value += (recvData[value_flag++] - 48);
                }
                pwm_value = 20*pwm_value+500;

                uartWriteBuff[2] = pwm_value/1000 +48;
                uartWriteBuff[3] = pwm_value/100%10 +48;
                uartWriteBuff[4] = pwm_value/10%10 +48;
                switch (recvData[16])
                {
                case '0':
                    // PWM16_set_duty(SERVO_BOARD,4,pwm_value);
                    uartWriteBuff[1] = '3';
                    break;
                case '1':
                    // PWM16_set_duty(SERVO_BOARD,5,pwm_value);
                    uartWriteBuff[1] = '2';
                    break;
                case '2':
                    // PWM16_set_duty(SERVO_BOARD,6,pwm_value);
                    uartWriteBuff[1] = '1';
                    break;
                case '3':
                    // PWM16_set_duty(SERVO_BOARD,7,pwm_value);
                    uartWriteBuff[1] = '0';
                    break;
                default:
                    break;
                }
                // sendFlag = 1;
                int ret = IoTUartWrite(2, (unsigned char *)uartWriteBuff, 5);
                printf("Uart Write data: ret = %d\r\n", ret);

            }else if (strstr(recvData, WECHAT_MSG_SPEED_CHANGE) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = HI_TRUE;

                uartWriteBuff[1] = '7';
                switch (recvData[13])
                {
                case '0':
                    uartWriteBuff[2] = '0';
                    uartWriteBuff[3] = '5';
                    uartWriteBuff[4] = '0';
                    break;
                case '1':
                    uartWriteBuff[2] = '1';
                    uartWriteBuff[3] = '0';
                    uartWriteBuff[4] = '6';
                    break;
                case '2':
                    uartWriteBuff[2] = '1';
                    uartWriteBuff[3] = '7';
                    uartWriteBuff[4] = '8';
                    break;
                case '3':
                    uartWriteBuff[2] = '2';
                    uartWriteBuff[3] = '4';
                    uartWriteBuff[4] = '0';
                    break;
                default:
                    break;
                }
                // sendFlag = 1;
                int ret = IoTUartWrite(2, (unsigned char *)uartWriteBuff, 5);
                printf("Uart Write data: ret = %d\r\n", ret);

            }else if (strstr(recvData, WECHAT_MSG_REFRESH) != NULL) {
                printf("Control equipment information received:%s\r\n", recvData);
                recvDataFlag = -1;

                sendData = expressBoxNum;
                sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);

            }else if (strstr(recvData, WECHAT_MSG_UNLOAD_PAGE) != NULL) {
                printf("The applet exits the current interface\r\n");
            }else {
                recvDataFlag = RECV_DATA_FLAG_OTHER;
                index_line = recvData[0];
                OledShowChar(60, 5, recvData[0], 1);
            }

        }

        if (recvDataFlag == HI_TRUE) {
            sendData = DEVICE_MSG_LIGHT_ON;
            sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);
        } else if (recvDataFlag == HI_FALSE) {
            sendData = DEVICE_MSG_LIGHT_OFF;
            sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);
        } else if (recvDataFlag == RECV_DATA_FLAG_OTHER) {
            sendData = "Received a message from the server";
            sendto(sServer, sendData, strlen(sendData), 0, (struct sockaddr*)&remoteAddr, addrLen);
        }
        hi_sleep(FREE_CPU_TIME_20MS);
    }
    close(sServer);
}