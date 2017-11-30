
#include <rtthread.h>
#include "platform_config.h"
#include "global.h"
#include "mcp41xxx.h"

uint16_t g_ADC_Buffer[2000];
uint16_t g_adc_process[1050];

static void dma_config(uint16_t* dmaBuff,uint16_t dmaLen);
void pwm_init(float freq);

static void dma_interrupt_config(void)
{   
    NVIC_InitTypeDef NVIC_InitStructure;
   
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void adc_signal_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_InitTypeDef ADC_InitStruct;
	
    RCC_ADCCLKConfig(RCC_PCLK2_Div4);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1  ,ENABLE);
    
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_14;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
    
	ADC_DeInit(ADC1);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1,ENABLE);
	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_ScanConvMode = ENABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;	//ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;		//note: here is right
	ADC_InitStruct.ADC_NbrOfChannel = 1;
	ADC_Init(ADC1,&ADC_InitStruct);	
	ADC_RegularChannelConfig(ADC1,ADC_Channel_3,1,ADC_SampleTime_1Cycles5);
	
    ADC_ExternalTrigConvCmd(ADC1, ENABLE);    
	
	ADC_Cmd(ADC1, ENABLE);
    ADC_ResetCalibration(ADC1);
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
    
    dma_config(g_ADC_Buffer,2000);
    dma_interrupt_config();
	//ADC_Cmd(ADC1, ENABLE);
    
	pwm_init(166666.6f);
    
}
 
#define ADC1_DR_Address    ((uint32_t)0x4001244C)


static void dma_config(uint16_t* dmaBuff,uint16_t dmaLen)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
	
	ADC_ITConfig(ADC1,ADC_IT_EOC,DISABLE);
	DMA_DeInit(DMA1_Channel1);
	DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)dmaBuff;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_BufferSize = dmaLen;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_ITConfig(DMA1_Channel1,DMA_IT_HT | DMA_IT_TC,ENABLE);
	ADC_DMACmd(ADC1, ENABLE);
	/* Enable DMA1 channel1 */
	DMA_Cmd(DMA1_Channel1, ENABLE);
}

void DMA1_Channel1_IRQHandler(void)
{    
    static uint16_t cnt,cnt_send;
	
    /* enter interrupt */
    rt_interrupt_enter();
    
	cnt++;
	cnt_send = cnt;
	rt_mb_send(&signal_mb,(rt_uint32_t)&cnt_send);
	if(cnt == 14)
	{
		cnt = 0;
		ADC_Cmd(ADC1, DISABLE);
	}
    
	
	if(DMA_GetITStatus(DMA1_IT_HT1))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL1 | DMA1_IT_HT1 | DMA1_IT_TE1);        
    }
    else if(DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_GL1 | DMA1_IT_TC1 | DMA1_IT_TE1);       
    }
    else
    {
        DMA_ClearITPendingBit( DMA1_IT_TE1);
    }
    
    /* leave interrupt */
    rt_interrupt_leave();
}


#define TIM_PULSE   TIM5
void pwm_init(float freq)
{
    uint16_t prescaler;
    uint16_t period;
	
    GPIO_InitTypeDef GPIO_InitStructure; 
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA , ENABLE);
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
	TIM_DeInit(TIM5); 
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	
	float tmp;
    
    tmp = ( SystemCoreClock / freq);
    if(tmp > 30000.0f)
    {
        prescaler = (uint16_t)(tmp / 30000.0f - 1);
    }
    else
    {
        prescaler = 0;
    }
    period = (uint16_t)( (tmp / (prescaler + 1)) - 1 );
    TIM_PULSE->PSC = prescaler;
	TIM_PULSE->ARR = period;
	TIM_PULSE->CCR1 = period >> 1;
	
	TIM_PULSE->CR1 &= ~TIM_CR1_DIR;
	
	TIM_PULSE->CCMR1 &= ~(7 << 4);
	TIM_PULSE->CCMR1 |= (6 << 4);
	TIM_PULSE->CCER |= TIM_CCER_CC1E;
	TIM_PULSE->CCR1 = period >> 1;
	//TIM_PULSE->CR1 |= TIM_CR1_ARPE;
	//TIM_PULSE->CR1 |= TIM_CR1_CEN;
    
}

void pwm_start(void)
{
	TIM_PULSE->CR1 |= TIM_CR1_CEN;
}

void pwm_stop(void)
{
	TIM_PULSE->CR1 &= ~TIM_CR1_CEN;
}

void ch_switch_init(void);
void pulse_send_init(float freq,float us)
{
    float tmp;
    uint16_t prescaler;
    uint16_t period;
    NVIC_InitTypeDef NVIC_InitStructure;
    
	//ch_switch_init();
    pwm_init(freq);
    
    TIM_DeInit(TIM6);
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM6, ENABLE);
    
    freq = 1000000.0f / us;
    tmp = ( SystemCoreClock / freq);
    if(tmp > 30000.0f)
    {
        prescaler = (uint16_t)(tmp / 30000.0f - 1);
    }
    else
    {
        prescaler = 0;
    }
    period = (uint16_t)( (tmp / (prescaler + 1)) - 1 );
    TIM6->PSC = prescaler;
	TIM6->ARR = period;
    TIM6->DIER |= TIM_DIER_UIE;
    TIM6->CR1 |= TIM_CR1_OPM;
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    mcp41xxx_set_pot(1,0,256 - global.gain_2);
    mcp41xxx_set_pot(2,0,256 - global.gain_2);
}

void pulse_send(void)
{
    TIM6->CR1 |= TIM_CR1_CEN;
    TIM_PULSE->CCMR1 &= ~(7 << 4);
	TIM_PULSE->CCMR1 |= (6 << 4);
    TIM_PULSE->CR1 |= TIM_CR1_CEN;
}

void TIM6_IRQHandler(void)
{
	/* enter interrupt */
    rt_interrupt_enter();
    TIM_PULSE->CR1 &= ~TIM_CR1_CEN;
    TIM_PULSE->CCMR1 &= ~(7 << 4);
	TIM_PULSE->CCMR1 |= (5 << 4);
    TIM_PULSE->CNT = 0;
    TIM6->SR &= ~TIM_SR_UIF;
    // ADC Start
    ADC_Cmd(ADC1, ENABLE);
    /* leave interrupt */
    rt_interrupt_leave();
}

void pulse_release(void)
{
	TIM5->CCMR1 &= ~(7 << 4);
	TIM5->CCMR1 |= (4 << 4);
	TIM5->CCMR1 |= (6 << 4);
}

void ch_switch_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure; 
	
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB , ENABLE);
  
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOB, &GPIO_InitStructure);
	switch_ch1();
}
