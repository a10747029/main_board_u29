#include "gd32f1x0_usart.h"
uint16_t adc_value[10];
void com_init0()    // for PA9，PA10
{
    /* enable GPIO clock */
    rcu_periph_clock_enable(RCU_GPIOA);

    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);

    /* connect port to USARTx_Tx */
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_9);

    /* connect port to USARTx_Rx */
    gpio_af_set(GPIOA, GPIO_AF_1, GPIO_PIN_10);

    /* configure USART Tx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_9);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ,GPIO_PIN_9);

    /* configure USART Rx as alternate function push-pull */
    gpio_mode_set(GPIOA, GPIO_MODE_AF, GPIO_PUPD_PULLUP,GPIO_PIN_10);
    gpio_output_options_set(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_10MHZ,GPIO_PIN_10);

    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0,115200U);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
		usart_interrupt_enable(USART0, USART_INT_RBNE);
    usart_enable(USART0);
}

static void rcu_config(void)
{
	    /* enable GPIOC clock */
    rcu_periph_clock_enable(RCU_GPIOC);
    /* enable ADC clock */
    rcu_periph_clock_enable(RCU_ADC);
    /* enable DMA clock */
    rcu_periph_clock_enable(RCU_DMA);
    /* config ADC clock */
    rcu_adc_clock_config(RCU_ADCCK_APB2_DIV6);
}
/*!
    \brief      configure the gpio peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void gpio_config(void)
{
    /* config the GPIO as analog mode */
    gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_0);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_1);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_2);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_3);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_4);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_5);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_6);
		gpio_mode_set(GPIOA,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_7);
		gpio_mode_set(GPIOB,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_0);
		gpio_mode_set(GPIOB,GPIO_MODE_ANALOG,GPIO_PUPD_NONE,GPIO_PIN_1);
}

/*!
    \brief      configure the dma peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void dma_config(void)
{
    /* ADC DMA_channel configuration */
    dma_deinit(DMA_CH0);
    dma_periph_address_config(DMA_CH0,(uint32_t)(&ADC_RDATA));
    dma_memory_address_config(DMA_CH0,(uint32_t)(adc_value));
    dma_transfer_direction_config(DMA_CH0,DMA_PERIPHERAL_TO_MEMORY);
    dma_memory_width_config(DMA_CH0,DMA_MEMORY_WIDTH_16BIT);
    dma_periph_width_config(DMA_CH0,DMA_PERIPHERAL_WIDTH_16BIT);
    dma_priority_config(DMA_CH0,DMA_PRIORITY_HIGH);
    dma_transfer_number_config(DMA_CH0,10);
    dma_periph_increase_disable(DMA_CH0);
    dma_memory_increase_enable(DMA_CH0);
    dma_circulation_enable(DMA_CH0);
    dma_channel_enable(DMA_CH0);
}

/*!
    \brief      configure the adc peripheral
    \param[in]  none
    \param[out] none
    \retval     none
*/
static void adc_config(void)
{
    /* ADC channel length config */
    adc_channel_length_config(ADC_REGULAR_CHANNEL,10);

    /* ADC regular channel config */
    adc_regular_channel_config(0,ADC_CHANNEL_0,ADC_SAMPLETIME_28POINT5);
    adc_regular_channel_config(1,ADC_CHANNEL_1,ADC_SAMPLETIME_28POINT5);
    adc_regular_channel_config(2,ADC_CHANNEL_2,ADC_SAMPLETIME_28POINT5);
    adc_regular_channel_config(3,ADC_CHANNEL_3,ADC_SAMPLETIME_28POINT5);
    adc_regular_channel_config(4,ADC_CHANNEL_4,ADC_SAMPLETIME_28POINT5);
    adc_regular_channel_config(5,ADC_CHANNEL_5,ADC_SAMPLETIME_28POINT5);
		adc_regular_channel_config(6,ADC_CHANNEL_6,ADC_SAMPLETIME_28POINT5);
		adc_regular_channel_config(7,ADC_CHANNEL_7,ADC_SAMPLETIME_28POINT5);
		adc_regular_channel_config(8,ADC_CHANNEL_8,ADC_SAMPLETIME_28POINT5);
		adc_regular_channel_config(9,ADC_CHANNEL_9,ADC_SAMPLETIME_28POINT5);

    /* ADC external trigger enable */
    adc_external_trigger_config(ADC_REGULAR_CHANNEL,ENABLE);
    /* ADC external trigger source config */
    adc_external_trigger_source_config(ADC_REGULAR_CHANNEL,ADC_EXTTRIG_REGULAR_SWRCST);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC_DATAALIGN_RIGHT);
    /* ADC discontinuous mode */
    adc_discontinuous_mode_config(ADC_REGULAR_CHANNEL,4);
    /* enable ADC interface */
    adc_enable();
    /* ADC calibration and reset calibration */
    adc_calibration_enable();
    /* ADC DMA function enable */
    adc_dma_mode_enable();

    /* ADC software trigger enable */
    adc_software_trigger_enable(ADC_REGULAR_CHANNEL);
}
void adc_init()
{
	  /* system clocks configuration */
    rcu_config();
    /* GPIO configuration */
    gpio_config();
    /* DMA configuration */
    dma_config();
    /* ADC configuration */
    adc_config();
}

uint8_t crc8(uint8_t *data, int length) {
    uint8_t crc = 0x00; // 初始值
    uint8_t poly = 0x07; // 生成多项式 x^8 + x^2 + x + 1
		int i,j;
    for (i = 0; i < length; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            if (crc & 0x80) { // 如果最高位是1
                crc = (crc << 1) ^ poly; // 左移并异或生成多项式
            } else {
                crc = crc << 1; // 左移
            }
        }
    }
    return crc; // 返回最终的CRC8校验码
}
