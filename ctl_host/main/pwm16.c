#include "pwm16.h"

#define I2C_ADDR_WRITE(addr) ((addr)<<1)
#define I2C_ADDR_READ(addr) (((addr)<<1&)0x01)

/****************************
 * PWM16
 ***************************/

//启动16路PWM输出板
unsigned int PWM16_start(PWM16 *sp){
    static unsigned char snd[]={0x00,0x21};
    return IoTI2cWrite(sp->id,I2C_ADDR_WRITE(sp->addr),snd,2);
}

//停止16路PWM输出板
unsigned int PWM16_stop(PWM16 *sp){
    static unsigned char snd[]={0x00,0x31};
    return IoTI2cWrite(sp->id,I2C_ADDR_WRITE(sp->addr),snd,2);
}

//设置16路PWM输出板的输出频率
unsigned int PWM16_set_freq(PWM16 *sp,int freq_hz){
    sp->__freq=freq_hz;sp->__T_us=1000000/freq_hz;
    unsigned char snd1[]={0xfe,25000000/(4096*freq_hz*0.915)-1};
    unsigned int ret=PWM16_stop(sp);
    if(ret!=HI_ERR_SUCCESS) return ret;
    usleep(500);
    ret=IoTI2cWrite(sp->id,I2C_ADDR_WRITE(sp->addr),snd1,2);
    if(ret!=HI_ERR_SUCCESS) return ret;
    return PWM16_start(sp);
}

//初始化16路PWM输出板：sp=结构体，id=引脚，addr=I2C地址，freq_hz=频率
unsigned int PWM16_init(PWM16 *sp,unsigned char id,unsigned char addr,int freq_hz){
    sp->id=id; sp->addr=addr;
    unsigned int ret=IoTI2cInit(id,400000);
    if(ret) return ret;
    return PWM16_set_freq(sp,freq_hz);
}

//设置16路PWM板某一路的占空比：sp=结构体，out=输出路，从左到右0~15，duty_us=高电平时间，舵机建议500~2500
unsigned int PWM16_set_duty(PWM16 *sp,unsigned char out,int duty_us){
    int off = (unsigned int)((double)duty_us/sp->__T_us*4096);
    unsigned char snd[]={0x06+(out<<2),0,0,off,off>>8};   
    return IoTI2cWrite(sp->id,I2C_ADDR_WRITE(sp->addr),snd,5); 
}

/****************************
 * Steering
 ***************************/

//将某一路PWM封装成舵机角度
unsigned int steering_init(steering *st,PWM16 *p,unsigned char out,
                int max_duty,int min_duty,int max_degree,int min_degree){
    st->pw=p;st->out=out;
    st->max_duty=max_duty;st->max_degree=max_degree;
    st->min_duty=min_duty;st->min_degree=min_degree;
    st->delta_duty=max_duty-min_duty;st->delta_degree=max_degree-min_degree;
    return PWM16_set_duty(p,out,(min_duty+max_duty)>>1);
}

//设置舵机角度
unsigned int steering_set(steering *st,int degree){
    int to_set=st->delta_duty*(degree-st->min_degree)/(float)(st->delta_degree)+st->min_duty;
    return PWM16_set_duty(st->pw,st->out,to_set);
}

/****************************
 * Switcher
 ***************************/

//将某一路PWM驱动的舵机封装成开关，out=输出通道，pos0_duty=位置0的占空比，pos1_duty=位置1的占空比
void switcher_init(switcher *sw,PWM16 *p,unsigned char out,int pos0_duty,int pos1_duty){
    sw->pw=p;sw->out=out;
    sw->duties[0]=pos0_duty;
    sw->duties[1]=pos1_duty;
}

//设置开关，pos=0/1
unsigned int switcher_set(switcher *sw,unsigned char pos){
    return PWM16_set_duty(sw->pw,sw->out,sw->duties[pos]);
}

/****************************
 * Robot
 ***************************/

void robot_init(robot *rb,PWM16 *p,unsigned char o[4],int arm_dt[2][3],int stamp_dt[4]){
    rb->pw=p;
    memcpy(rb->out,o,sizeof(rb->out));
    memcpy(rb->servo_duties,arm_dt,sizeof(rb->servo_duties));
    memcpy(rb->stamp_duties,stamp_dt,sizeof(rb->stamp_duties));
}

void robot_up(robot *rb){
    PWM16_set_duty(rb->pw,rb->out[0],rb->servo_duties[0][0]);
    PWM16_set_duty(rb->pw,rb->out[1],rb->servo_duties[0][1]);
    PWM16_set_duty(rb->pw,rb->out[2],rb->servo_duties[0][2]);
}
void robot_down(robot *rb){
    PWM16_set_duty(rb->pw,rb->out[0],rb->servo_duties[1][0]);
    PWM16_set_duty(rb->pw,rb->out[1],rb->servo_duties[1][1]);
    PWM16_set_duty(rb->pw,rb->out[2],rb->servo_duties[1][2]);
}

unsigned int robot_stamp(robot *rb,int stamp_pose){
    return PWM16_set_duty(rb->pw,rb->out[3],rb->stamp_duties[stamp_pose]);
}

/****************************
 * Speed Setter
 ***************************/

void spdset_init(spdset* ss,PWM16 *p,unsigned char o,int duties[4]){
    ss->cur_lvl=0;ss->last_lvl=0;
    ss->out=o;ss->pw=p;
    memcpy(ss->duties,duties,4*sizeof(int));
}

void spdset_level(spdset* ss,spd_lvl_enum lvl){
    ss->last_lvl=ss->cur_lvl;ss->cur_lvl=lvl;
    PWM16_set_duty(ss->pw,ss->out,ss->duties[lvl]);
}

void spdset_resume(spdset* ss){
    ss->cur_lvl=ss->last_lvl;
    PWM16_set_duty(ss->pw,ss->out,ss->duties[ss->cur_lvl]);
}

void spdset_duty(spdset*ss,int duty){
    PWM16_set_duty(ss->pw,ss->out,duty);
}