
#include <stdio.h>

void PWM_ma_4_20_init(void)
{
    uint16_t prescaler_value;
    uint16_t period;

    GPIO_InitTypeDef GPIO_InitStructure;    
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    
    RCC_APB2PeriphClockCmd( PWM_4_20_GPIO_CLK , ENABLE);
    GPIO_InitStructure.GPIO_Pin = PWM_4_20_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(PWM_4_20_GPIO_PORT, &GPIO_InitStructure);
  
    prescaler_value = (uint16_t) (SystemCoreClock / 36E6) - 1;
    period = (36E6 / 1000) - 1;
    TIM_DeInit(TIM_4_20MA);
    RCC_APB2PeriphClockCmd(RCC_TIM_4_20MA, ENABLE);
    
    TIM_TimeBaseStructure.TIM_Period = period;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler_value;
    TIM_TimeBaseStructure.TIM_ClockDivision = 0;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0x0;
    TIM_TimeBaseInit(TIM_4_20MA, &TIM_TimeBaseStructure);
  
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Disable;
    TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_Pulse = period * 0.13f;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCNIdleState_Set;
    
    TIM_OC3Init(TIM_4_20MA, &TIM_OCInitStructure);
    TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    TIM_Cmd(TIM_4_20MA, ENABLE);
    TIM_CtrlPWMOutputs(TIM_4_20MA, ENABLE);    
}

void PWM_Percent_4_20(float32_t percent)
{
    uint16_t period;
    
    TIM_4_20MA->CR1 |= TIM_CR1_CEN;
	period = TIM_4_20MA -> ARR;
    period = (uint16_t)(period * percent);
    TIM_SetCompare3(TIM_4_20MA, period);
}
