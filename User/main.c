#include "gd32f1x0.h"
#include <stdio.h>
#include "systick.h"
uint8_t crc8(uint8_t *data, size_t length);
void com_init0();
void adc_init();
uint8_t receivesize0;
__IO uint16_t rxcount0; 
uint8_t receiver_buffer0[32];
#define TIME_PLUS_MS 1000
void delay(int time)
{
	  int target_time = TIME_PLUS_MS * time;
    while(target_time--);
    
    return;
}
uint8_t hand_count = 10;
#define ADC_IN0_VCC24_MIN				   3217
#define ADC_IN0_VCC24_MAX          3574
#define ADC_IN1_VCC3V3_ULP_MIN     2247 
#define ADC_IN1_VCC3V3_ULP_MAX     2491
#define ADC_IN2_VCC3V3_ULP_MIN     2247 
#define ADC_IN2_VCC3V3_ULP_MAX     2491
#define ADC_IN3_VCC3V3_ULP_MIN     2247 
#define ADC_IN3_VCC3V3_ULP_MAX     2491
#define ADC_IN4_VBAT_MIN           2518
#define ADC_IN4_VBAT_MAX           3438
#define ADC_IN5_VCC12_SYS_MIN      3084
#define ADC_IN5_VCC12_SYS_MAX      3484
#define ADC_IN6_VCC5V0_CORE_MIN    3231
#define ADC_IN6_VCC5V0_CORE_MAX    3588
#define ADC_IN7_VCC5V0_SYS_MIN     3231
#define ADC_IN7_VCC5V0_SYS_MAX     3588
#define ADC_IN8_VCC5V0_SYS_MIN     3231
#define ADC_IN8_VCC5V0_SYS_MAX     3588
#define ADC_IN9_VCC5V0_SYS_MIN     3231
#define ADC_IN9_VCC5V0_SYS_MAX     3588
extern uint16_t adc_value[10];
int main(void)
{		uint8_t *ch = (uint8_t*)adc_value;
		int i = 0;
    rcu_ahb_clock_config(RCU_AHB_CKSYS_DIV2);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_OSPEED_2MHZ, GPIO_PIN_2);
		gpio_bit_reset(GPIOB, GPIO_PIN_2);//关闭蜂鸣器
		com_init0();
		adc_init();
    while(1)
    {
#if 0
			if(0){
				if(((adc_value[0] < ADC_IN0_VCC24_MIN)       || (adc_value[0] > ADC_IN0_VCC24_MAX))       ||
					 ((adc_value[1] < ADC_IN1_VCC3V3_ULP_MIN)  || (adc_value[1] > ADC_IN1_VCC3V3_ULP_MAX))  ||
				   ((adc_value[2] < ADC_IN4_VBAT_MIN)        || (adc_value[2] > ADC_IN4_VBAT_MAX))        ||
				   ((adc_value[3] < ADC_IN5_VCC12_SYS_MIN)   || (adc_value[3] > ADC_IN5_VCC12_SYS_MAX))   ||
				   ((adc_value[4] < ADC_IN6_VCC5V0_CORE_MIN) || (adc_value[4] > ADC_IN6_VCC5V0_CORE_MAX)) ||
				   ((adc_value[5] < ADC_IN7_VCC5V0_SYS_MIN)  || (adc_value[5] > ADC_IN7_VCC5V0_SYS_MAX)))
				{ 
					gpio_bit_set(GPIOB, GPIO_PIN_2);
					//delay(200);
					delay(60);
					gpio_bit_reset(GPIOB, GPIO_PIN_2);
				}
			}
#endif
			//while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
				usart_data_transmit(USART0, 0xAA);
				while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
				usart_data_transmit(USART0, 0xAA);		
				while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));				
			for(i = 0;i < 20; i+=2)
			{    
					 usart_data_transmit(USART0, *(ch+i+1));
					 while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
					 usart_data_transmit(USART0, *(ch+i));
					 while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
			}
			while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
			usart_data_transmit(USART0, crc8(ch, 20));
			while(RESET == usart_flag_get(USART0, USART_FLAG_TBE));
			usart_data_transmit(USART0, 0xff);	
			adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
			delay(1000);
    }
}

int fputc(int ch, FILE *f)
{
    usart_data_transmit(USART1, (uint8_t)ch);
    while(RESET == usart_flag_get(USART1, USART_FLAG_TBE));
    return ch;
}
