
#define RUNTIME_MAKE_REAL
#include "global.h"

#include "rtthread.h"
#include "conf_store.h"
//#include "bsp_24xx02.h"

uint8_t get_language(void)
{
    return global.language;
}

static void default_init(void);

void global_init(void)
{
    int i;
    uint8_t tmp_u8,len;
    rt_err_t err_no;
    uint16_t eeprom;
    uint32_t tmp_u32;
    float tmp_f32;
    
    default_init();
    
    len = rt_device_read(mb85_bus, E2P_OFFSET(eeprom), (uint8_t*)&eeprom, 2);
	
    while(len != 2)
    {
        err_no = rt_get_errno();
        rt_thread_delay(1);
        len = rt_device_read(mb85_bus, E2P_OFFSET(eeprom), (uint8_t*)&eeprom, 2);
    }
    
    switch(eeprom)
    {
        case EE_FULL_RECOVERY_ING:
            full_reset_eeprom();
            break;
        case EE_FULL_RECOVERY_OVER:
            break;
        case EE_RESET_FACTORY_ING:
            restore_factory_settings();
            break;
        case EE_RESET_FACTORY_OVER:
            break;
        default:
            full_reset_eeprom();
            break;
    }
    
    rt_device_read(mb85_bus, E2P_OFFSET(lcd_contrast), (uint8_t*)&tmp_u8, 1);
    rt_device_read(mb85_bus, E2P_OFFSET(is_disp_reverse), (uint8_t*)&tmp_u8, 1);
    rt_device_read(mb85_bus, E2P_OFFSET(password), (uint8_t*)&tmp_u32, 4);
    
    if(tmp_u32 > 999999)
    {
        tmp_u32 = 10000;
        rt_device_write(mb85_bus, E2P_OFFSET(password), (uint8_t*)&tmp_u32, 4);
    }
    global.password = tmp_u32;
	snprintf(global.password_buf,7,"%06d",global.password);

    rt_device_read(mb85_bus, E2P_OFFSET(unit), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 1)
    {
        tmp_u8 = 0;
        rt_device_write(mb85_bus, E2P_OFFSET(unit), (uint8_t*)&tmp_u8, 1);
    }
    global.unit = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(comm_format), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 1)
    {
        tmp_u8 = 0;
        rt_device_write(mb85_bus, E2P_OFFSET(comm_format), (uint8_t*)&tmp_u8, 1);
    }
    global.comm_format = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(comm_baud), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 5)
    {
        tmp_u8 = 3;
        rt_device_write(mb85_bus, E2P_OFFSET(comm_baud), (uint8_t*)&tmp_u8, 1);
    }
    global.comm_baud = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(comm_parity), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 2)
    {
        tmp_u8 = 0;
        rt_device_write(mb85_bus, E2P_OFFSET(comm_parity), (uint8_t*)&tmp_u8, 1);
    }
    global.comm_parity = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(comm_device_id), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 247)
    {
        tmp_u8 = 1;
        rt_device_write(mb85_bus, E2P_OFFSET(comm_device_id), (uint8_t*)&tmp_u8, 1);
    }
    global.comm_device_id = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(relay_1_alarm_type), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 1)
    {
        tmp_u8 = 1;
        rt_device_write(mb85_bus, E2P_OFFSET(relay_1_alarm_type), (uint8_t*)&tmp_u8, 1);
    }
    global.relay_1_alarm_type = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(relay_1_alarm_val), (uint8_t*)&tmp_f32, 4);
    global.relay_1_alarm_val = tmp_f32;
    
    rt_device_read(mb85_bus, E2P_OFFSET(relay_1_alarm_delay), (uint8_t*)&tmp_f32, 4);
    global.relay_1_alarm_delay = tmp_f32;
    
    rt_device_read(mb85_bus, E2P_OFFSET(relay_2_alarm_type), (uint8_t*)&tmp_u8, 1);
    if(tmp_u8 > 1)
    {
        tmp_u8 = 1;
        rt_device_write(mb85_bus, E2P_OFFSET(relay_2_alarm_type), (uint8_t*)&tmp_u8, 1);
    }
    global.relay_2_alarm_type = tmp_u8;
    
    rt_device_read(mb85_bus, E2P_OFFSET(relay_2_alarm_val), (uint8_t*)&tmp_f32, 4);
    global.relay_2_alarm_val = tmp_f32;
    
    rt_device_read(mb85_bus, E2P_OFFSET(relay_2_alarm_delay), (uint8_t*)&tmp_f32, 4);
    global.relay_2_alarm_delay = tmp_f32;
    
    rt_device_read(mb85_bus, E2P_OFFSET(clean_interval), (uint8_t*)&tmp_u32, 4);
    global.clean_interval = tmp_u32;
    
    rt_device_read(mb85_bus, E2P_OFFSET(clean_time), (uint8_t*)&tmp_u32, 4);
    global.clean_time = tmp_u32;
    
    rt_device_read(mb85_bus, E2P_OFFSET(backlight), (uint8_t*)&tmp_u8, 1);
    global.backlight = tmp_u8;
    
}

static void default_init(void)
{
    global.backlight = 1;
    global.clean_interval = 36000;
    global.clean_time = 1;
    global.comm_baud = 3;
    global.comm_device_id = 1;
    global.comm_format = 0;
    global.comm_parity = 0;
    global.temp_compensate = 0;
    global.temp = 250;
    global.is_disp_reverse = 0;
    global.is_relay_test = 0;
    global.unit = 0;
    global.zhuodu_data.a1 = 1;
    global.zhuodu_data.a1k1 = 1;
    global.zhuodu_data.a2 = 1;
    global.zhuodu_data.a3 = 1;
    global.zhuodu_data.a4 = 1;
    global.zhuodu_data.b1 = 0;
    global.zhuodu_data.b2 = 0;
    global.zhuodu_data.b3 = 0;
    global.zhuodu_data.b4 = 0;
    global.zhuodu_data.a2_zero = 0;
    global.zhuodu_data.bright_range = 150;
    global.zhuodu_data.canbi_set = 5000;
    global.zhuodu_data.num_cali = 2;
    global.zhuodu_data.range_sel = 0;
    global.zhuodu_data.seg_range_1 = 1000;
    global.zhuodu_data.seg_range_2 = 1000;
    global.zhuodu_data.seg_range_3 = 1000;
    global.zhuodu_data.seg_range_4 = 1000;
    global.zhuodu_data.temp = 250;
}
