#include <stdio.h>tcp
#include <string.h>
#include <unistd.h>

#include "pwm16.h"
#include "screen.h"
#include "buttons.h"
#include "scanner.h"
#include "gate.h"
#include "queue.h"
#include "wifi_connecter.h"
#include "tcp.h"

#define BASE_CHANNEL    0
#define MID_CHANNEL     1
#define LVL_CHANNEL     8
#define STAMP_CHANNEL   3
#define GATE0_CHANNEL   4
#define GATE1_CHANNEL   5
#define BLOCKER_CHANNEL 6
#define SPD_CHANNEL     7

#define STACK_SIZE (8192)

static PWM16 _servo;
static robot _rb;
static gate _gate[2];
static switcher _sw_gate[3];
static spdset _ss;
static int obj_cnt[3];
static tcp_client mt;
#define TCP_CLIENT (&mt)

#define TRANS(x) (500+20*(x))
#define SERVO_BOARD (&_servo)
#define ROBOT (&_rb)
#define GATE(x) (_gate+(x))
#define GSW(x) (_sw_gate+(x))
#define BLOCKER (_sw_gate+2)
#define SPD_SET (&_ss)

#define MTX_TIMEOUT 100000
static osMutexId_t LCK_SCREEN,LCK_PWM,LCK_NETSEND;
static osThreadAttr_t mtx_attr_scr={0},mtx_attr_gate={0},mtx_attr_netsend={0};
static osThreadId_t __threads[6];
#define TH_MAIN __threads[0]
#define TH_SCAN __threads[1]
#define TH_SW   __threads[2]
#define TH_MCTL __threads[3]
#define TH_NET  __threads[4]
#define TH_NETRECV  __threads[5]

#define CONNECT_UART_NUM 1

static unsigned char manual_ctlmode=0;

#define PWM_WRAP(x) if(osMutexAcquire(LCK_PWM,MTX_TIMEOUT)==osOK){x;}osMutexRelease(LCK_PWM)
#define SCREEN_WRAP(x,y,data) if(osMutexAcquire(LCK_SCREEN,MTX_TIMEOUT)==osOK){ \
        console_str_at((x),(y),(data)); \
    }osMutexRelease(LCK_SCREEN)

#define SCREEN_WIFI(connected) SCREEN_WRAP(3,0,(connected)?"W+":"W.")
#define SCREEN_SERVER() SCREEN_WRAP(6,0,TCP_CLIENT->connected?"S+":"S.")

static void filp_manctl(void){
    manual_ctlmode=!manual_ctlmode;
    SCREEN_WRAP(13,0,manual_ctlmode?"Manual":"Auto  ");
}

static int net_send(const char* s);
static void print_counter(void);

static void clear_cache(void){
    if(osMutexAcquire(LCK_PWM,MTX_TIMEOUT)==osOK){
        memset(obj_cnt,0,sizeof(obj_cnt));
        print_counter();
    }osMutexRelease(LCK_PWM);
    net_send("C000\nC010\nC020");
}

int cmd_proc(unsigned char op,unsigned int dat){
    if(op>='0' && op<='7' && (dat<500 || dat>2500)){
        printf("[CMD]Bad PWM duty:%d\n",dat);
        return;
    }
    if(manual_ctlmode){
        switch(op){
            case '0':PWM16_set_duty(SERVO_BOARD,BASE_CHANNEL,dat);return 1;
            case 'X':ROBOT->servo_duties[0][0]=TRANS(dat/10);return 1;
            case 'U':ROBOT->servo_duties[1][0]=TRANS(dat/10);return 1;
            case '1':PWM16_set_duty(SERVO_BOARD,MID_CHANNEL,dat);return 1;
            case 'Y':ROBOT->servo_duties[0][1]=TRANS(dat/10);return 1;
            case 'V':ROBOT->servo_duties[1][1]=TRANS(dat/10);return 1;
            case '2':PWM16_set_duty(SERVO_BOARD,LVL_CHANNEL,dat);return 1;
            case 'Z':ROBOT->servo_duties[0][2]=TRANS(dat/10);return 1;
            case 'W':ROBOT->servo_duties[1][2]=TRANS(dat/10);return 1;
            case '3':PWM16_set_duty(SERVO_BOARD,STAMP_CHANNEL,dat);return 1;
            case '4':PWM16_set_duty(SERVO_BOARD,BLOCKER_CHANNEL,dat);return 1;
            case '5':PWM16_set_duty(SERVO_BOARD,GATE0_CHANNEL,dat);return 1;
            case '6':PWM16_set_duty(SERVO_BOARD,GATE1_CHANNEL,dat);return 1;
            case 'A':gate_on(GATE(0));return 1;
            case 'B':gate_off(GATE(0));return 1;
            case 'C':gate_on(GATE(1));return 1;
            case 'D':gate_off(GATE(1));return 1;
            case 'E':switcher_set(BLOCKER,0);return 1;
            case 'F':switcher_set(BLOCKER,1);return 1;
            case 'G':robot_down(ROBOT);return 1;
            case 'H':robot_up(ROBOT);return 1;
            case 'R':robot_stamp(ROBOT,0);return 1;
            case 'S':robot_stamp(ROBOT,1);return 1;
            case 'T':robot_stamp(ROBOT,2);return 1;
        }
    }
    switch(op){
        case '7':spdset_duty(SPD_SET,dat);return 1;
        case 'I':spdset_level(SPD_SET,SPD_LVL_STOP);return 1;
        case 'J':spdset_level(SPD_SET,SPD_LVL_LOW);return 1;
        case 'K':spdset_level(SPD_SET,SPD_LVL_MID);return 1;
        case 'L':spdset_level(SPD_SET,SPD_LVL_HIGH);;return 1;
        case 'M':filp_manctl();return 1;
        case 'N':clear_cache();return 1;
        default:printf("[CMD]Bad command: %c,%d\n",op,dat);return 0;
    }
    return 0;
}

#define PARAM_HOTSPOT_SSID "xxx"   // your AP SSID
#define PARAM_HOTSPOT_PSK  "xxx"  // your AP PSK
#define PARAM_SERVER_ADDR "x.x.x.x"
#define PARAM_SERVER_PORT 8888

void onTCPClientConnectionChanged(int connected){
    SCREEN_SERVER();
}

static int net_send(const char* s){
    if(!isHotspotConnected() || !TCP_CLIENT->connected)return -1;
    if(osMutexAcquire(LCK_NETSEND,MTX_TIMEOUT)==osOK){
        int ret=tcp_client_send(TCP_CLIENT,s,strlen(s));
        osMutexRelease(LCK_NETSEND);
        return ret;
    }osMutexRelease(LCK_NETSEND);
    return -1;
}

static void net_recv_thread(void){
    static unsigned char buf[128],sendlen=0;
    while(isHotspotConnected() && TCP_CLIENT->connected){
        memset(buf,0,sizeof(buf));
        if(tcp_client_recv(TCP_CLIENT,buf,sizeof(buf))>0){
            unsigned char op=buf[1];
            unsigned int dat=10*((buf[2]-'0')*100+(buf[3]-'0')*10+(buf[4]-'0'));
            printf("[WIFIComm]%c %d\n",op,dat);
            if(net_send(cmd_proc(op,dat)?"@ok":"@err")<0){
                printf("[TCP]Fail to response.\n");
            }
            usleep(100000);
        }else{
            printf("[TCP]Connection reset.\n"); 
        }
    }
}

static void net_thread(void) {
    WifiDeviceConfig config = {0};
    strcpy_s(config.ssid, WIFI_MAX_SSID_LEN, PARAM_HOTSPOT_SSID);
    strcpy_s(config.preSharedKey, WIFI_MAX_KEY_LEN, PARAM_HOTSPOT_PSK);
    config.securityType = WIFI_SEC_TYPE_PSK;
    usleep(10000);

    while(1){
        //连接WiFi
        int netId = ConnectToHotspot(&config);
        SCREEN_WIFI(1);
        usleep(10000);

        //连接服务器
        int ret;
        while(ret=tcp_client_connect(TCP_CLIENT)){
            printf("[TCP]Fail to start TCP client: errno=%d.\n",ret);
        }

        //开启接收线程
        static osThreadAttr_t attr={
            .name="NetRecv",
            .attr_bits=0U,
            .cb_mem=NULL,
            .cb_size=0U,
            .stack_mem=NULL,
            .stack_size=STACK_SIZE,
            .priority=osPriorityNormal
        };

        if ((TH_NETRECV=osThreadNew((osThreadFunc_t)net_recv_thread, NULL, &attr))==NULL) {
            printf("[NETRECV]Failed!\n");
            return;
        }

        while(isHotspotConnected() && TCP_CLIENT->connected){
            net_send("@alive");
            for(int i=0;i<5;++i){
                if(!isHotspotConnected() || !TCP_CLIENT->connected) break;
                sleep(1);
            }
        }
        osThreadTerminate(TH_NETRECV);
        usleep(100000);
        SCREEN_SERVER();

        DisconnectWithHotspot(netId);
        SCREEN_WIFI(0);
    }
}

//串口控制线程
static void ctl_thread(void){
    static unsigned char ctl_buf[128];
    int len,rem=0;
    while(1){
        if((len=IoTUartRead(CONNECT_UART_NUM, ctl_buf+rem, sizeof(ctl_buf)-rem))>0){
            len+=rem;
            int i=0;
            while(i<len){
                while(ctl_buf[i]!=0xff && i<len)++i;
                if(i>=len) {rem=0;break;}
                if(len-i<5) {rem=len-i;memcpy(ctl_buf+i,ctl_buf,rem);break;}
                ++i;
                unsigned char op=ctl_buf[i++];
                unsigned int dat=10*((ctl_buf[i]-'0')*100+(ctl_buf[i+1]-'0')*10+(ctl_buf[i+2]-'0'));
                printf("[UARTComm]%c %d\n",op,dat);
                cmd_proc(op,dat);
                usleep(1000);
            }
        }else{
            usleep(1000);
        }
    }
}

//条码扫描器线程
#define MAX_ID 2

static int get_id_from_scanner_data(){ //从条码数据生成ID
    int id=(scanner_data[9]-'0')*10+(scanner_data[10]-'0')-28;
    if(id<0 || id>MAX_ID) {
        printf("[Scanner]Bad ID.\n");
        id=-1;
    }
    return id;
}

static void scanner_thread(void){
    switcher_set(BLOCKER,0);
    while(1){
        memset(scanner_data,0,sizeof(scanner_data));
        if(scanner_read()){
            SCREEN_WRAP(0,1,scanner_data);     //屏幕打印ID
            int id=get_id_from_scanner_data(); //获取ID
            if(id<0) {usleep(10000);continue;} //非法ID，跳过
            PWM_WRAP(spdset_level(SPD_SET,SPD_LVL_STOP));
            usleep(500000);
            if(osMutexAcquire(LCK_PWM,MTX_TIMEOUT)==osOK){
                robot_stamp(ROBOT,id);
                for(int i=0;i<id;++i){
                    qPush(&(GATE(i)->_q),0);
                }
                if(id!=2) qPush(&(GATE(id)->_q),1);
                obj_cnt[id]++;
                static char msg[30];
                sprintf(msg,"C00%d\nC01%d\nC02%d",obj_cnt[0],obj_cnt[1],obj_cnt[2]);
                net_send(msg);
                msg[0]=id+'0';
                IoTUartWrite(CONNECT_UART_NUM,msg,1);
                print_counter();
            }osMutexRelease(LCK_PWM);
            usleep(500000);
            PWM_WRAP(robot_down(ROBOT));
            usleep(500000);
            PWM_WRAP(robot_up(ROBOT));
            usleep(500000);
            PWM_WRAP(spdset_resume(SPD_SET));
            usleep(500000);
            PWM_WRAP(switcher_set(BLOCKER,1));
            switch(SPD_SET->cur_lvl){
                case 1:usleep(1250000);break; //LOW
                case 2:usleep(720000);break;  //MID
                case 3:usleep(500000);break;  //HIGH
                default:break;
            }
            PWM_WRAP(switcher_set(BLOCKER,0));
        }
        usleep(10000);
    }
}

void print_sw_unsafe(int id){
    console_printf_at(0,3+id,"G%d %5d %5d",id,GATE(id)->_stage,qLength(&(GATE(id)->_q)));
}

void print_sw(int id){
    if(osMutexAcquire(LCK_SCREEN,MTX_TIMEOUT)==osOK){
        print_sw_unsafe(id);
    }osMutexRelease(LCK_SCREEN);
}

static void print_counter(void){
    if(osMutexAcquire(LCK_SCREEN,MTX_TIMEOUT)==osOK){
        print_sw_unsafe(0);print_sw_unsafe(1);
        console_printf_at(0,5,"J0=%d,J1=%d,J2=%d   ",obj_cnt[0],obj_cnt[1],obj_cnt[2]);
    }osMutexRelease(LCK_SCREEN);
}

static void sw_thread(void) {
    while(1) {
        if(osMutexAcquire(LCK_PWM,MTX_TIMEOUT)==osOK){
            if(gate_act(GATE(0),gate_detect(GATE(0)))) print_sw(0);
            if(gate_act(GATE(1),gate_detect(GATE(1)))) print_sw(1);
        }osMutexRelease(LCK_PWM);
        usleep(10000);
    }
}

//主线程
static void main_thread(void){
    while(1){
        unsigned char key=button_detect();
        switch(key){
            case 2:clear_cache();break;
            case 3:filp_manctl();break;
        }
        usleep(200000);
    }
}

static void app_main(void)
{
    osThreadAttr_t attr,attr2,attr3,attr4,attr5;
    //屏幕
    console_init();

    int failure_cnt=0;
    //UART1控制
    IoTGpioInit(0);  // 使用GPIO，都需要调用该接口
    hi_io_set_func(0,HI_IO_FUNC_GPIO_0_UART1_TXD);
    IoTGpioInit(1);  // 使用GPIO，都需要调用该接口
    hi_io_set_func(1,HI_IO_FUNC_GPIO_1_UART1_RXD);
    // 初始化UART配置，115200，数据bit为8, 停止位1，奇偶校验为NONE，流控为NONE
    IotUartAttribute g_uart_cfg = {115200, 8, 1, IOT_UART_PARITY_NONE, 
            IOT_UART_BLOCK_STATE_BLOCK, IOT_UART_BLOCK_STATE_BLOCK, 0};
    if(IoTUartInit(CONNECT_UART_NUM, &g_uart_cfg)){
        console_str("[x]UART\n");
        ++failure_cnt;
    }else{
        console_str("[+]UART\n");
    }  
    
    //ADC光电门
    switcher_init(GSW(0),SERVO_BOARD,GATE0_CHANNEL,580,1500);
    gate_init(GATE(0),HI_GPIO_IDX_9,HI_IO_FUNC_GPIO_9_GPIO,HI_ADC_CHANNEL_4,GSW(0),700);
    switcher_init(GSW(1),SERVO_BOARD,GATE1_CHANNEL,540,1400);
    gate_init(GATE(1),HI_GPIO_IDX_7,HI_IO_FUNC_GPIO_7_GPIO,HI_ADC_CHANNEL_3,GSW(1),700);
    switcher_init(BLOCKER,SERVO_BOARD,BLOCKER_CHANNEL,680,1580);
    console_str("[+]Gate\n");

    //ADC按键
    buttons_init();
    console_str("[+]Button\n");

    //扫描器
    if(scanner_init()){
        console_str("[x]Scanner\n");
        ++failure_cnt;
    }else{
        console_str("[+]Scanner\n");
    }
    
    //PWM输出板/舵机
    if(PWM16_init(SERVO_BOARD,HI_I2C_IDX_0,0x40,50)){
        console_str("[x]PWM\n");
        ++failure_cnt;
    }else{
        console_str("[+]PWM\n");
    }
    
    //机械臂
    static unsigned char rb_out[]={BASE_CHANNEL,MID_CHANNEL,LVL_CHANNEL,STAMP_CHANNEL};
    static int rb_duties[2][3]={
        {TRANS(48),TRANS(20),TRANS(33)}, //UP
        {TRANS(48),TRANS( 5),TRANS(33)}  //DOWN
    };
    static int rb_stamp_duties[]={TRANS(20),TRANS(50),TRANS(81),500};
    robot_init(ROBOT,SERVO_BOARD,rb_out,rb_duties,rb_stamp_duties);
    robot_up(ROBOT);
    robot_stamp(ROBOT,0);

    //调速系统
    static int ss_duties[4]={TRANS(0),TRANS(28),TRANS(64),TRANS(98)};
    spdset_init(SPD_SET,SERVO_BOARD,SPD_CHANNEL,ss_duties);
    spdset_level(SPD_SET,SPD_LVL_HIGH);

    //弹出器和拦截器
    switcher_set(GSW(0),0);
    switcher_set(GSW(1),0);
    switcher_set(BLOCKER,0);
    console_str("[+]Robot\n");

    //服务器参数初始化
    tcp_client_init(TCP_CLIENT,PARAM_SERVER_ADDR,PARAM_SERVER_PORT,onTCPClientConnectionChanged);

    if(failure_cnt){
        console_printf("%d err(s). Continue?",failure_cnt);
        while(button_detect()==0) usleep(200000);
    }
    console_clr();
    console_str("   W. S.     Auto\n\n   Stage Pends\nG0\nG1");
    console_str_at(0,7," ReCache   Man/Auto");
    
    LCK_SCREEN=osMutexNew(&mtx_attr_scr);
    LCK_PWM=osMutexNew(&mtx_attr_gate);
    LCK_NETSEND=osMutexNew(&mtx_attr_netsend);

    attr.name = "Main";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = STACK_SIZE;
    attr.priority = osPriorityNormal;

    if ((TH_MAIN=osThreadNew((osThreadFunc_t)main_thread, NULL, &attr))==NULL) {
        printf("[MAIN]Failed!\n");
        return;
    }

    attr2=attr; attr2.name="Scanner";
    if ((TH_SCAN=osThreadNew((osThreadFunc_t)scanner_thread, NULL, &attr2))==NULL) {
        printf("[SCAN]Failed!\n"); return;
    }

    attr3=attr; attr3.name="SW";
    if ((TH_SW=osThreadNew((osThreadFunc_t)sw_thread, NULL, &attr3))==NULL) {
        printf("[SW]Failed!\n"); return;
    }

    attr4=attr; attr4.name="MCTL";
    if ((TH_MCTL=osThreadNew((osThreadFunc_t)ctl_thread, NULL, &attr4))==NULL) {
        printf("[MCTL]Failed!\n"); return;
    }

    attr5=attr; attr5.name="NET";
    if ((TH_NET=osThreadNew((osThreadFunc_t)net_thread, NULL, &attr5))==NULL) {
        printf("[NET]Failed!\n"); return;
    }
}

SYS_RUN(app_main);
