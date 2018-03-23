
#define RUNTIME_MAKE_REAL
#include "global.h"

#include "rtthread.h"
#include "conf_store.h"
//#include "bsp_24xx02.h"

uint8_t get_language(void)
{
    return global.language;
}

static uint8_t s_buf[128];

void global_init(void)
{
    int i;
    uint8_t tmp_u8,len;
    rt_err_t err_no;
    uint16_t eeprom;
    uint32_t tmp_u32;
    
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
    // action
    rt_device_read(mb85_bus, E2P_OFFSET(is_disp_reverse), (uint8_t*)&tmp_u8, 1);
    // action
    
    rt_device_read(mb85_bus, E2P_OFFSET(password), (uint8_t*)&tmp_u32, 4);
    
    if(tmp_u32 > 999999)
    {
        tmp_u32 = 100000;
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
}
