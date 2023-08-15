#ifndef __GATE_H__
#define __GATE_H__

#include <stdio.h>

#include "ohos_init.h"
#include "cmsis_os2.h"

#include "hi_gpio.h"
#include "hi_io.h"
#include "hi_adc.h"
#include "hi_errno.h"

#include "pwm16.h"
#include "queue.h"

#define GATE_BUF_SZ 8
typedef struct{
    hi_io_name pin;             //ADC光电门引脚编号
    hi_adc_channel_index ch;    //ADC通道编号
    int value_record[GATE_BUF_SZ];    //值记录
    int _ptr;                   //值指针
    int diff;                   //差值阈值
    hi_u8 first_entry;          //是否首次进入             
    switcher *sw;               //开关器

    int _stage;                 //当前状态
    int _cnt;                   //当前计数
    queue _q;                   //开关队列
}gate;

void gate_init(gate *g,hi_io_name pin_idx,hi_u8 func,hi_adc_channel_index adc_channel,switcher *sw,int diff);
int gate_detect(gate *g);
unsigned int gate_act(gate *g,unsigned int res);
unsigned int gate_on(gate *g);
unsigned int gate_off(gate *g);

#endif