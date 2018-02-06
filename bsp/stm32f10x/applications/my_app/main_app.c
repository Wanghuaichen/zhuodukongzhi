
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
#include "mb.h"
#include "mb_m.h"


ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t main_stack[ 2048 ];
static struct rt_thread main_thread;

int rt_hw_fm24cl16_init(const char *fm_device_name, const char *i2c_bus);

static uint8_t s_buf[128];
static void main_thread_entry(void* parameter)
{
    rt_uint32_t i = 0;
    uint16_t ee_init;
    
	global.is_disp_reverse = 0;     // 1
    
	rt_hw_lcd_init();

	rt_hw_fm24cl16_init("mb85","i2c1");
	mb85_bus = rt_device_find("mb85");
    rt_device_open(mb85_bus, RT_DEVICE_OFLAG_RDWR | RT_DEVICE_FLAG_STREAM );
    
    global_init();
    
	menu_init();
 
    while (1)
    {   
        menu_main();
        rt_thread_delay( RT_TICK_PER_SECOND / 100 ); 
    }
}

static void modbus_app_thread_entry(void* parameter)
{    
    rt_thread_delay( 100);
    static uint16_t usModbusUserData[2] = {0x12,0x23};
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    
    while (1)
    {
#if 1        
        eMBMasterReqReadHoldingRegister(1,0x100,37,RT_WAITING_FOREVER);
        rt_thread_delay( RT_TICK_PER_SECOND / 2 );
        
        eMBMasterReqReadHoldingRegister(1,0x200,29,RT_WAITING_FOREVER);
        rt_thread_delay( RT_TICK_PER_SECOND / 2);
#endif  
        usModbusUserData[1] += 1;
        //eMBMasterReqWriteMultipleHoldingRegister(1,0x100,2,usModbusUserData,RT_WAITING_FOREVER); 
		//eMBMasterReqReadHoldingRegister(1,3,8,RT_WAITING_FOREVER);
//        float swap_f32(float dat);
//        data.dat = swap_f32(1.00f);
//        eMBMasterReqWriteMultipleHoldingRegister(1,0x100+23,2,data.buf,RT_WAITING_FOREVER);
        
        rt_thread_delay( RT_TICK_PER_SECOND);
    }
}

static void modbus_s_thread_entry(void* parameter)
{
    uint32_t baud;
    
    rt_thread_delay(RT_TICK_PER_SECOND / 100);
    switch(global.comm_baud)
    {
        case 0: baud = 1200; break;
        case 1: baud = 2400; break;
        case 2: baud = 4800; break;
        case 3: baud = 9600; break;
        case 4: baud = 19200; break;
        case 5: baud = 38400; break;
    }   
    eMBInit(global.comm_format, global.comm_device_id,/* 3 */ 2, baud,  global.comm_parity);
	eMBEnable();
    while(1)
    {
        eMBPoll();
    }
}

static void modbus_m_thread_entry(void* parameter)
{
    uint32_t baud;
    
    rt_thread_delay(RT_TICK_PER_SECOND / 100);
    
    eMBMasterInit(MB_RTU,/* 2 */ 3, 9600,  MB_PAR_NONE);           // 2
	eMBMasterEnable();
    while(1)
    {
        eMBMasterPoll();
    }
}

static void lcd_thread_entry(void* parameter)
{
    rt_thread_delay(RT_TICK_PER_SECOND / 100);
    
    while(1)
    {
        lcd_buf_flush();
        rt_thread_delay(RT_TICK_PER_SECOND / 20);
    }
}

void key_thread_entry(void* parameter)
{
    while(1)
    {
        KeyPro();
        rt_thread_delay(RT_TICK_PER_SECOND / 100);
    }
}

int rt_main_app_init(void)
{
	rt_err_t result;
    rt_thread_t key_thread,modbus_s_thread,modbus_m_thread,modbus_app_thread,lcd_thread;
    
    /* init lcd thread */
    result = rt_thread_init(&main_thread,
                            "main",
                            main_thread_entry,
                            RT_NULL,
                            (rt_uint8_t*)&main_stack[0],
                            sizeof(main_stack),
                            10,
                            10);
    
    if (result == RT_EOK)
    {
        rt_thread_startup(&main_thread);
    }
    
    modbus_s_thread = rt_thread_create("modbus-s",
                            modbus_s_thread_entry,
                            RT_NULL,
                            1024,
                            12,
                            1);
                            
    if (modbus_s_thread != RT_NULL)
    {
        //rt_thread_startup(modbus_s_thread);
    }
    
    modbus_m_thread = rt_thread_create("modbus-m",
                            modbus_m_thread_entry,
                            RT_NULL,
                            1024,
                            13,
                            1);
                            
    if (modbus_m_thread != RT_NULL)
    {
        rt_thread_startup(modbus_m_thread);
    }
    
    modbus_app_thread = rt_thread_create("modbus-a",
                            modbus_app_thread_entry,
                            RT_NULL,
                            1024,
                            10,
                            5);                        
                            
    if (modbus_app_thread != RT_NULL)
    {
        rt_thread_startup(modbus_app_thread);
    }
    
    lcd_thread = rt_thread_create("lcd",
                            lcd_thread_entry,
                            RT_NULL,
                            512,
                            5,
                            2);
                            
    if (lcd_thread != RT_NULL)
    {
        rt_thread_startup(lcd_thread);
    }
    
    key_thread = rt_thread_create("key",
                            key_thread_entry,
                            RT_NULL,
                            512,
                            8,
                            1);
                            
    if (key_thread != RT_NULL)
    {
        rt_thread_startup(key_thread);
    }
    return 0;
}

