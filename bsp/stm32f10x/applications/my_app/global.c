
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
    uint8_t tmp_u8;
    int i;
    uint16_t eeprom;
    
    rt_device_read(mb85_bus, E2P_OFFSET(eeprom), (uint8_t*)&eeprom, 2);
	
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
   
}
