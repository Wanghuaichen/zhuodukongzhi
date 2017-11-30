#include "platform_config.h"
#include <stdbool.h>

bool get_temp_press_adc(uint16_t *ptemp,uint16_t *ppress)
{
	volatile int i;
	uint16_t temp,press;
	ADC_SoftwareStartConvCmd(ADC2,ENABLE);
	for( i = 0; i < 1000; i++)
	{
		if(RESET == ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC) )
			continue;
		ADC_ClearFlag(ADC2,ADC_FLAG_EOC);
		press = ADC_GetConversionValue(ADC2);
		break;
	}
	if(i == 1000)
		return false;

	ADC_SoftwareStartConvCmd(ADC2,ENABLE);
	for( i = 0; i < 1000; i++)
	{ 
		if(RESET == ADC_GetFlagStatus(ADC2,ADC_FLAG_EOC) )
			continue;
		ADC_ClearFlag(ADC2,ADC_FLAG_EOC);
		temp = ADC_GetConversionValue(ADC2);
		break;
	}
	if(i == 1000)
		return false;
	*ptemp = temp;
	*ppress = press;
	return true;
}

void adc_temp_press_init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
    ADC_InitTypeDef ADC_InitStruct;
	
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA ,ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA,&GPIO_InitStruct);
	
	ADC_DeInit(ADC2);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC2,ENABLE);
	ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_NbrOfChannel = 2;
	ADC_Init(ADC2,&ADC_InitStruct);	
	ADC_RegularChannelConfig(ADC2,ADC_Channel_6,1,ADC_SampleTime_239Cycles5);
	ADC_RegularChannelConfig(ADC2,ADC_Channel_7,2,ADC_SampleTime_239Cycles5);
	ADC_ExternalTrigConvCmd(ADC2, ENABLE);
    ADC_DiscModeChannelCountConfig(ADC2,1);
	ADC_DiscModeCmd(ADC2,ENABLE);
	ADC_Cmd(ADC2, ENABLE);
    
	ADC_ResetCalibration(ADC2);
	while(ADC_GetResetCalibrationStatus(ADC2));
	ADC_StartCalibration(ADC2);
	while(ADC_GetCalibrationStatus(ADC2));
}
