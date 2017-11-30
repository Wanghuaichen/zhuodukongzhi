
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "global.h"
#include "platform_config.h"
#include "lcd12864.h"
#include "bsp_button.h"
#include "menu_frame.h"
#include "menu_proc.h"
#include "signal_hard.h"


ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t main_stack[ 2048 ];
static struct rt_thread main_thread;

int rt_hw_fm24cl64_init(const char *fm_device_name, const char *i2c_bus);

static void main_thread_entry(void* parameter)
{
    rt_uint32_t i = 0;
    uint16_t ee_init;
    
	global.is_disp_reverse = 1;
    rt_kprintf("\r\n lcd init ******************************\r\n");
    
	rt_hw_lcd_init();

#if 0	
	rt_hw_fm24cl64_init("mb85","i2c1");
	mb85_bus = rt_device_find("mb85");
    rt_device_open(mb85_bus, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM );
#endif   
    global_init();
    
    rt_kprintf("lcd welcome \r\n");
    LCD_DisplayString(0,0,"                ",NOT_REVERSE);
    LCD_DisplayString(0,48,"                ",NOT_REVERSE);
    LCD_DisplayString(16,16," 欢 迎 使 用     ",NOT_REVERSE);
    rt_thread_delay(RT_TICK_PER_SECOND / 2);
    LCD_DisplayString(16,32," 液 位 开 关    ",NOT_REVERSE);
    rt_thread_delay(RT_TICK_PER_SECOND / 2);
	
    rt_kprintf("menu init\r\n");
	menu_init();
 
    while (1)
    {   
        menu_main();
        rt_thread_delay( RT_TICK_PER_SECOND / 100 ); 

    }
}

static rt_uint8_t pulse_stack[ 1024 ];
static struct rt_thread pulse_thread;

static void _udelay(rt_uint32_t us)
{
    rt_uint32_t delta;
    
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
    
    delta = SysTick->VAL;
    
    while (delta - SysTick->VAL< us);
}

static void pulse_thread_entry(void* parameter)
{
    rt_uint32_t i;
    
    rt_thread_delay( 100);
    rt_kprintf("pwm init\r\n");
    pulse_send_init(global.sensor_freq,42);    
	
    while (1)
    {
        rt_kprintf("pwm send\r\n");
		pulse_send();
		rt_thread_delay( RT_TICK_PER_SECOND / 20);
		pulse_release();
		
		rt_thread_delay( RT_TICK_PER_SECOND );
#if 0		
		//switch_ch1();
        //rt_thread_delay( 5);
		for(i = 0; i < 5; i++)
        {
            pulse_send();		
            rt_thread_delay( RT_TICK_PER_SECOND / 10);
        }
        
        //TODO: select ch2
		//switch_ch2();
		//rt_thread_delay( 5);
        for(i = 0; i < 5; i++)
        {
            pulse_send();
            rt_thread_delay( RT_TICK_PER_SECOND / 10);
        }
#endif		
    }
}

static struct rt_timer timer1;
static void timeout1(void* parameter);

int rt_main_app_init(void)
{
	rt_err_t result;
    
    /* init lcd thread */
    result = rt_thread_init(&main_thread,
                            "main",
                            main_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&main_stack[0],
                            sizeof(main_stack),
                            10,
                            100);
    
    if (result == RT_EOK)
    {
        rt_kprintf("start main\r\n");
        rt_thread_startup(&main_thread);
    }
    else
    {
        rt_kprintf(" main err:%d\r\n",result);
    }

    result = rt_thread_init(&pulse_thread,
                            "pwm",
                            pulse_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&pulse_stack[0],
                            sizeof(pulse_stack),
                            5,
                            100);
                            
    if (result == RT_EOK)
    {
        rt_thread_startup(&pulse_thread);
    }
    
    rt_timer_init(&timer1, "timer1",  
                    timeout1, 
                    RT_NULL, 
                    1,
                    RT_TIMER_FLAG_PERIODIC); 

    rt_timer_start(&timer1);
    
    int signal_init(void);
    signal_init();
    
    return 0;
}

void timeout1(void* parameter)
{
    KeyPro();
}

