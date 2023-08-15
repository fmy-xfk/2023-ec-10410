#ifndef __PWM16_H__
#define __PWM16_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_errno.h"
#include "iot_i2c.h"
#include "hi_gpio.h"
#include "hi_io.h"
#include "hi_i2c.h"
#include "hi_adc.h"

typedef struct{
    hi_i2c_idx id;
    hi_u8 addr;
    int __freq,__T_us;
}PWM16;

unsigned int PWM16_start(PWM16 *sp);
unsigned int PWM16_stop(PWM16 *sp);
unsigned int PWM16_init(PWM16 *sp,unsigned char id,unsigned char addr,int freq_hz);
unsigned int PWM16_set_freq(PWM16 *sp,int freq_hz);
unsigned int PWM16_set_duty(PWM16 *sp,unsigned char out,int duty_us);

typedef struct{
    PWM16 *pw;
    unsigned char out;
    int max_duty,min_duty,delta_duty;
    int max_degree,min_degree,delta_degree;
}steering;

unsigned int steering_init(steering *st,PWM16 *p,unsigned char out,int max_duty,int min_duty,int max_degree,int min_degree);
unsigned int steering_set(steering *st,int degree);

typedef struct{
    PWM16 *pw;
    unsigned char out;
    int duties[2];
}switcher;

void switcher_init(switcher *sw,PWM16 *p,unsigned char out,int pos1_duty,int pos2_duty);
unsigned int switcher_set(switcher *sw,unsigned char pos);

typedef struct{
    PWM16 *pw;
    unsigned char out[4];
    int servo_duties[2][3];
    int stamp_duties[4];
}robot;

void robot_init(robot *rb,PWM16 *p,unsigned char o[4],int dt[2][3],int stamp_dt[4]);
void robot_up(robot *rb);
void robot_down(robot *rb);
unsigned int robot_stamp(robot *rb,int stamp_pose);

typedef struct{
    PWM16 *pw;
    unsigned char out;
    int duties[4],cur_lvl,last_lvl;
}spdset;

typedef enum {
    SPD_LVL_STOP=0,
    SPD_LVL_LOW,
    SPD_LVL_MID,
    SPD_LVL_HIGH
}spd_lvl_enum;

void spdset_init(spdset* ss,PWM16 *p,unsigned char o,int duties[4]);
void spdset_level(spdset* ss,spd_lvl_enum lvl);
void spdset_resume(spdset* ss);
void spdset_duty(spdset*ss,int duty);

#endif  // PWM16_H