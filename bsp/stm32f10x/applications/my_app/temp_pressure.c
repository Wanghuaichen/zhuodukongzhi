
#include <math.h>
#include "app_config.h"
#include "platform_config.h"
#include "temp_press_hw.h"
#include "temp_pressure.h"
#include "systick.h"
#include "global.h"
#include "deftypes.h"

#define ADC_REF_MV 3300

typedef int16_t q15_t;
 
static uint16_t writeOffsetTemp_1;
static uint16_t writeOffsetTemp_2;
static uint16_t TempADC_1[30];
static uint16_t TempADC_2[30];

extern ChannelInfoDef g_channels[2];

 void arm_circularWrite_q15(
q15_t * circBuffer,
int32_t L,
uint16_t * writeOffset,
int32_t bufferInc,
const q15_t * src,
int32_t srcInc,
uint32_t blockSize);
int8_t getTemp(float32_t omega);

void update_temp_press(void)
{
    uint8_t i;
    static uint8_t cnt;
    uint16_t tempADC_1,tempADC_2,tempCh1ADC,tempCh2ADC;
    uint32_t temp_u32 ; 
    float32_t temp_k;
    float32_t mVol;
    float32_t temp_f32,omega;
    
    if(!CheckTimer(TEMP_PRESS_TIMER))
    {
        return;
    }
    DelayMS_ID(TEMP_PRESS_TIMER,TICKS_PER_SEC / 4);
	if( false == get_temp_press_adc( &tempCh1ADC,&tempCh2ADC) )
		return;
    
    if(global.chSlct == 0)
    {
        if(tempCh1ADC > 300 & tempCh1ADC < 2300)
            tempADC_1 = tempCh1ADC;
        else
            tempADC_1 = 526;
        arm_circularWrite_q15((q15_t *)TempADC_1, 30,&writeOffsetTemp_1,1,(q15_t*)&tempADC_1,0,1);
        temp_u32 = 0;
        for(i = 0; i < 30;i++)
        {
            temp_u32 += TempADC_1[i];
        }
        temp_f32 = (float32_t)temp_u32  / 30.0f;
        
        temp_k = g_channels[0].tempK;
        mVol = temp_f32 * ADC_REF_MV / 4095.0f;    
        omega = (mVol - 300.91f) / temp_k;
        g_channels[0].TempValue = getTemp(omega);
    }
    else
    {    
        if(tempCh2ADC > 300 & tempCh2ADC < 2300)
            tempADC_2 = tempCh2ADC;
        else
            tempADC_2 = 526;
        arm_circularWrite_q15((q15_t *)TempADC_2, 30,&writeOffsetTemp_2,1,(q15_t*)&tempADC_2,0,1);
        temp_u32 = 0;
        for(i = 0; i < 30;i++)
        {
            temp_u32 += TempADC_2[i];
        }
        temp_f32 = (float32_t)temp_u32 / 30.0f;
        
        temp_k = g_channels[1].tempK;        
        mVol = temp_f32 * ADC_REF_MV / 4095.0f;    
        omega = (mVol - 300.91f) / temp_k;
        g_channels[1].TempValue = getTemp(omega);
    }
    
}

const float32_t c_TempOmega[];
int8_t getTemp(float32_t omega)
{
    int i,j = 45;
    for(i = 0; i < 120; i++)
    {
        if(omega > c_TempOmega[i])
        {
           j = i;
           break;
        }
    }
    return (j - 20);
}


 void arm_circularWrite_q15(
q15_t * circBuffer,
int32_t L,
uint16_t * writeOffset,
int32_t bufferInc,
const q15_t * src,
int32_t srcInc,
uint32_t blockSize)
{
uint32_t i = 0u;
int32_t wOffset;

/* Copy the value of Index pointer that points
 * to the current location where the input samples to be copied */
wOffset = *writeOffset;

/* Loop over the blockSize */
i = blockSize;

while(i > 0u)
{
  /* copy the input sample to the circular buffer */
  circBuffer[wOffset] = *src;

  /* Update the input pointer */
  src += srcInc;

  /* Circularly update wOffset.  Watch out for positive and negative value */
  wOffset += bufferInc;
  if(wOffset >= L)
    wOffset -= L;

  /* Decrement the loop counter */
  i--;
}

/* Update the index pointer */
*writeOffset = wOffset;
}


//-20'C ~ 100'C
const float32_t c_TempOmega[] = 
{
35.693 ,
33.985 ,
32.371 ,
30.843 ,
29.395 ,
28.023 ,
26.721 ,
25.485 ,
24.311 ,
23.195 ,
22.135 ,
21.126 ,
20.167 ,
19.255 ,
18.386 ,
17.56  ,
16.774 ,
16.025 ,
15.313 ,
14.634 ,
13.989 ,
13.439 ,
12.891 ,
12.352 ,
11.825 ,
11.314 ,
10.823 ,
10.352 ,
9.9023 ,
9.4744 ,
9.0685 ,
8.684  ,
8.3203 ,
7.9765 ,
7.6517 ,
7.3447 ,
7.0544 ,
6.7797 ,
6.5192 ,
6.2718 ,
6.0363 ,
5.8117 ,
5.5967 ,
5.3905 ,
5.1919 ,
5      ,
4.8141 ,
4.6332 ,
4.4568 ,
4.284  ,
4.1144 ,
3.9612 ,
3.8149 ,
3.6753 ,
3.5419 ,
3.4146 ,
3.2929 ,
3.1765 ,
3.0652 ,
2.9586 ,
2.8564 ,
2.7586 ,
2.6647 ,
2.5746 ,
2.4881 ,
2.405  ,
2.325  ,
2.2481 ,
2.174  ,
2.1026 ,
2.0337 ,
1.9673 ,
1.9032 ,
1.8411 ,
1.7812 ,
1.723  ,
1.6668 ,
1.6124 ,
1.5596 ,
1.5084 ,
1.4587 ,
1.4121 ,
1.3675 ,
1.3247 ,
1.2837 ,
1.2443 ,
1.2066 ,
1.1703 ,
1.1354 ,
1.1018 ,
1.0696 ,
1.0385 ,
1.0086 ,
0.9796 ,
0.9518 ,
0.9248 ,
0.8989 ,
0.8737 ,
0.8493 ,
0.8257 ,
0.8028 ,
0.7807 ,
0.7591 ,
0.7381 ,
0.7178 ,
0.6979 ,
0.6782 ,
0.6592 ,
0.641  ,
0.6234 ,
0.6064 ,
0.5901 ,
0.5742 ,
0.5589 ,
0.5441 ,
0.5298 ,
0.5159 ,
0.5024 ,
0.4894 ,
0.4767 ,
0.4645 ,


};

