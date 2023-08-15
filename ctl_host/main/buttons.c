#include "buttons.h"

//ADC按键初始化
void buttons_init(){
    hi_io_set_func(HI_GPIO_IDX_10, HI_IO_FUNC_GPIO_10_GPIO);
    hi_gpio_set_dir(HI_GPIO_IDX_10, HI_GPIO_DIR_IN);
}

//按钮检测，0=无，1=USER按键，2=左侧按键，3=右侧按键
unsigned char button_detect(){
    static hi_u16 value;
    if (hi_adc_read(HI_ADC_CHANNEL_2, &value,
        HI_ADC_EQU_MODEL_2, HI_ADC_CUR_BAIS_DEFAULT, 0) != HI_ERR_SUCCESS) {
        return 0;
    } else {
        //printf("value:%d\n",value);
        if(value<200){
            return 1;
        }else if(value<400){
            return 2;
        }else if(value<600){
            return 3;
        }else{
            return 0;
        }
    }
}