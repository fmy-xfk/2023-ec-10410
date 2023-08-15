#include "gate.h"

void gate_init(gate *g,hi_io_name pin_idx,hi_u8 func,hi_adc_channel_index adc_channel,
               switcher *sw, int diff){
    hi_io_set_func(pin_idx, func);
    hi_gpio_set_dir(pin_idx, HI_GPIO_DIR_IN);
    g->pin=pin_idx;g->ch=adc_channel;
    g->first_entry=1;g->sw=sw;g->diff=diff;
    qClear(&(g->_q));g->_stage=0;g->_cnt=0;g->_ptr=0;
}

int gate_detect(gate *g){
    hi_u16 value;
    if (hi_adc_read(g->ch, &value, HI_ADC_EQU_MODEL_4, HI_ADC_CUR_BAIS_DEFAULT, 0) != HI_ERR_SUCCESS){
        printf("[GATE]ADC read error!\n");
    }else{
        //if (!g->first_entry) g->first_entry=0,g->value_record[1]=(hi_s16)value;
        //g->value_record[0]=g->value_record[1];
        //g->value_record[1]=(hi_s16)value;
        //int diff = (int)g->value_record[1]-g->value_record[0];
        //if(g->pin==9)printf("[GATE%d]%d\n",g->pin,(int)value);
        for(int i=0;i<GATE_BUF_SZ-1;++i) g->value_record[i]=g->value_record[i+1];
        g->value_record[GATE_BUF_SZ-1]=(int)value;
        int cnt=0,half=GATE_BUF_SZ>>1,fs=0,rs=0;
        for(int i=0;i<half;++i){fs+=g->value_record[i];}fs/=half;
        for(int i=half;i<GATE_BUF_SZ;++i){rs+=g->value_record[i];}rs/=half;
        if (fs>700 && rs<700){
            return 1;
        }
        // printf("ADC_VALUE = %u\n", (unsigned int)value);
    }
    return 0;
}

unsigned int gate_on (gate *g){return switcher_set(g->sw,1);}
unsigned int gate_off(gate *g){return switcher_set(g->sw,0);}

//返回值表示stage和/或缓冲队列有无发生改变
unsigned int gate_act(gate *g,unsigned int res){
    unsigned int f,ret=0;
    if(qFront(&(g->_q),&f)){
        if(g->_stage==0 && res){
            ++g->_stage;g->_cnt=1;ret=1;
        }
        if(g->_stage==1){
            --g->_cnt;
            if(!g->_cnt){
                if(f)gate_on(g);
                ++g->_stage;g->_cnt=50;ret=1;
            }
        }
        if(g->_stage==2){
            --g->_cnt;
            if(!g->_cnt){
                if(f)gate_off(g);
                g->_stage=0;ret=1;
                qPop(&(g->_q));
            }
        }
    }
    return ret;
}
