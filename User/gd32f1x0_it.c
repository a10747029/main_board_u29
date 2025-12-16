/*!
    \file  gd32f1x0_it.c
    \brief interrupt service routines
*/

/*
    Copyright (C) 2017 GigaDevice

    2014-12-26, V1.0.0, platform GD32F1x0(x=3,5)
    2016-01-15, V2.0.0, platform GD32F1x0(x=3,5,7,9)
    2016-04-30, V3.0.0, firmware update for GD32F1x0(x=3,5,7,9)
    2017-06-19, V3.1.0, firmware update for GD32F1x0(x=3,5,7,9)
*/

#include "gd32f1x0_it.h"
#include "systick.h"
#include "stdio.h"

/*!
    \brief      this function handles NMI exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void NMI_Handler(void)
{
}

/*!
    \brief      this function handles HardFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void HardFault_Handler(void)
{
    /* if Hard Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles MemManage exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void MemManage_Handler(void)
{
    /* if Memory Manage exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles BusFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void BusFault_Handler(void)
{
    /* if Bus Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles UsageFault exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void UsageFault_Handler(void)
{
    /* if Usage Fault exception occurs, go to infinite loop */
    while(1){
    }
}

/*!
    \brief      this function handles SVC exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SVC_Handler(void)
{
}

/*!
    \brief      this function handles DebugMon exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void DebugMon_Handler(void)
{
}

/*!
    \brief      this function handles PendSV exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void PendSV_Handler(void)
{
}

/*!
    \brief      this function handles SysTick exception
    \param[in]  none
    \param[out] none
    \retval     none
*/
void SysTick_Handler(void)
{
}
/*!
    \brief      this function handles USART RBNE interrupt request and TBE interrupt request
    \param[in]  none
    \param[out] none
    \retval     none
*/
extern uint8_t receivesize0;
extern __IO uint16_t rxcount0; 
extern uint8_t receiver_buffer0[32];
extern int16_t adc_value[12];
void USART1_IRQHandler(void)
{
    if(RESET != usart_interrupt_flag_get(USART1, USART_INT_FLAG_RBNE)){
        /* receive data */
        receiver_buffer0[rxcount0++] = usart_data_receive(USART1);
        if(rxcount0 == receivesize0){
            usart_interrupt_disable(USART1, USART_INT_RBNE);
					//将adc的数据传输出去并且enable
						printf("%x%x%x%x%x%x,%x%x%x%x%x%x\r\n",adc_value[0],adc_value[1],adc_value[2],adc_value[3],adc_value[4],adc_value[5]
																	,adc_value[6],adc_value[7],adc_value[8],adc_value[9],adc_value[10],adc_value[11]);
						usart_interrupt_enable(USART1, USART_INT_RBNE);
        }
    }
}
