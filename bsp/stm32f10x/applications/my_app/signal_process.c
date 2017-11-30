#include <stdint.h>
#include <string.h>
#include "global.h"
#include "app_config.h"
#include "sort_misc.h"

ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t signal_stack[ 1024 ];
static struct rt_thread signal_thread;

#define abs(x) ((x)>0?(x):-(x))

uint16_t sum_abs_buff[200];
uint16_t s_sum_buf_len;

void sum_7_buf(uint16_t* in,uint16_t len,uint16_t* out)
{
	int i;
	s_sum_buf_len = 0;
	for(i = 0; i < len;)
	{
		if(i+7 < len)
		{
			out[s_sum_buf_len++] = (abs(in[i]-2046) +abs(in[i+1]-2046) +abs(in[i+2]-2046) +\
			abs(in[i+3]-2046) + abs(in[i+4]-2046) +abs(in[i+5]-2046) +abs(in[i+6]-2046)) / 7; 
		}
		
		i += 7;
	}
}

static int mag_calc[30];
int get_signals(signal_analysis_t* sig_analysis,signal_cond_t* sig_cond ,signals_t *signals)
{
	int i,j,start,end,tmp;
    
    start = (sig_analysis->buf_index-1) * 1000; end = sig_analysis->buf_index * 1000; 
    
    if(start >= sig_cond->end_index || end < sig_cond->start_index)
    {
        sig_analysis->stat = 0;
        return -1;
    }
	
    sum_7_buf(sig_analysis->buf_in,sig_analysis->len,sum_abs_buff);
	
	for(i = 0; i < s_sum_buf_len; i++)
	{
		if(i*7 < (sig_cond->start_index % 1000) )
			continue;
		switch(sig_analysis->stat)
		{
			case 0:
	            if( sum_abs_buff[i] > sig_cond->start_mag)
                {
                    sig_analysis->stat = 1; 
					sig_analysis->signal.index = (sig_analysis->buf_index-1)*1000 + i*7;
					if(global.signal_wave[global.chSlct].index < 30)
					{
						global.signal_wave[global.chSlct].wave[global.signal_wave[global.chSlct].index++] = sum_abs_buff[i];
					}
                }
				break;
			case 1:
				if(global.signal_wave[global.chSlct].index < 30)
				{
					global.signal_wave[global.chSlct].wave[global.signal_wave[global.chSlct].index++] = sum_abs_buff[i];
				}
				if( sum_abs_buff[i] < sig_cond->end_mag  )
				{ 
					
					sig_analysis->signal.width = (sig_analysis->buf_index-1)*1000 + i*7 - sig_analysis->signal.index;
					
					tmp = global.signal_wave[global.chSlct].index;
					global.signal_wave[global.chSlct].index = 0;
					for(j = 0; j < tmp; j++)
					{
						mag_calc[j] = global.signal_wave[global.chSlct].wave[j];
					}
					MergeSortIteration(mag_calc,tmp);
					
					sig_analysis->signal.mag = (mag_calc[tmp-1] + mag_calc[tmp-2] + mag_calc[tmp-3]) / 3;
					
					sig_analysis->stat = 0;
					if(sig_analysis->signal.width > sig_cond->width_min && sig_analysis->signal.width < sig_cond->width_max)
					{
						signals->signal[signals->index++] = sig_analysis->signal;
					}
				}           
				break;
			default: break;
		}
	}
}

extern uint16_t g_ADC_Buffer[2000];
extern uint16_t g_adc_process[1050];

//void fuelSignalProcess();

//extern ChannelInfoDef g_channels[2];

static void sigal_thread_entry(void* parameter)
{
    volatile int i;
	adc_signal_init();
	uint16_t *pcnt, *pSrc,*pDst,cnt;
	
	while(1)
	{
		rt_mb_recv(&signal_mb, (rt_uint32_t*)&pcnt, RT_WAITING_FOREVER);
        cnt = *pcnt;
		if(cnt % 2 == 0)
		{
//			pSrc = g_ADC_Buffer + 950; pDst = g_adc_process;
//			memcpy(pDst,pSrc,1050);
            pSrc = g_ADC_Buffer + 1000; pDst = g_adc_process;
			memcpy(pDst,pSrc,1000*2);
		}
		else
		{
//			pSrc = g_ADC_Buffer + 1950; pDst = g_adc_process;
//			memcpy(pDst,pSrc,50);
//			pSrc = g_ADC_Buffer; pDst = g_adc_process + 50;
//			memcpy(pDst,pSrc,1000);
            pSrc = g_ADC_Buffer; pDst = g_adc_process;
			memcpy(pDst,pSrc,1000*2);
		}
		global.signal_analysis.buf_in = g_adc_process;
		global.signal_analysis.len = 1000;
		global.signal_analysis.buf_index = cnt;
		global.signal_analysis.stat = 0;
		
        get_signals(&global.signal_analysis, &global.signal_cond[global.chSlct] ,&global.signals[global.chSlct]);
        if(cnt == 20)
		{
			global.signals[global.chSlct].numbers = global.signals[global.chSlct].index;
			global.signals[global.chSlct].index = 0;
			if(global.signals[global.chSlct].numbers  > 0 && global.chSlct == 0)
			{
				global.ch1_status = 1;
			}
			else
			{
				global.ch1_status = 0;
			}
			if(global.signals[global.chSlct].numbers  > 0 && global.chSlct == 1)
			{
				global.ch2_status = 1;
			}
			else
			{
				global.ch2_status = 0;
			}
		}
        //fuelSignalProcess();
    }
}

int signal_init(void)
{
	rt_err_t result;
	static char mb_pool[20];
	
	rt_mb_init(&signal_mb,"sig_mb",&mb_pool[0],sizeof(mb_pool)/4,RT_IPC_FLAG_FIFO);
		
	adc_signal_init();    
    result = rt_thread_init(&signal_thread,"signal",
        sigal_thread_entry, 
        RT_NULL, 
		&signal_stack[0], sizeof(signal_stack), 8,100);
        
    if (result == RT_EOK)
        rt_thread_startup(&signal_thread);
	
	return 0;


}
