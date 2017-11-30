
#define RUNTIME_MAKE_REAL
#include "global.h"

#include "rtthread.h"
#include "conf_store.h"
#include "bsp_24xx02.h"

uint8_t get_language(void)
{
    return global.lang;
}

static uint8_t s_buf[128];

void global_init(void)
{
    uint8_t tmp_u8;
    uint16_t eeprom;
    
    //rt_device_read(mb85_bus, 0, (uint8_t*)s_buf, 50);
    //rt_device_read(mb85_bus, /*E2P_OFFSET(eeprom)*/ 0, (uint8_t*)&eeprom, 2);
	ee_ReadBytes((uint8_t*)&eeprom, E2P_OFFSET(eeprom), 2);
	
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
    #if 0

    rt_device_read(mb85_bus, E2P_OFFSET(lcd_contrast), (uint8_t*)&tmp_u8, 1);
    // action
    rt_device_read(mb85_bus, E2P_OFFSET(is_disp_reverse), (uint8_t*)&tmp_u8, 1);
    // action
    rt_device_read(mb85_bus, E2P_OFFSET(language), (uint8_t*)&global.lang, 1);
    
    rt_device_read(mb85_bus, E2P_OFFSET(container_d), (uint8_t*)&global.container_d, 4);
    
    rt_device_read(mb85_bus, E2P_OFFSET(measure_freq), (uint8_t*)&global.measure_freq, 4);
    
    rt_device_read(mb85_bus, E2P_OFFSET(sensor_freq), (uint8_t*)&global.sensor_freq, 4);
    
    rt_device_read(mb85_bus, E2P_OFFSET(signal_cond[0].start_mag), (uint8_t*)&global.signal_cond[0].start_mag, sizeof(signal_cond_t)*2);

    #else
    ee_ReadBytes( (uint8_t*)&tmp_u8,E2P_OFFSET(lcd_contrast), 1);
    // action
    ee_ReadBytes((uint8_t*)&tmp_u8,E2P_OFFSET(is_disp_reverse),  1);
	global.is_disp_reverse = 1;
    // action
    ee_ReadBytes((uint8_t*)&global.lang, E2P_OFFSET(language),  1);
    
    ee_ReadBytes((uint8_t*)&global.container_d, E2P_OFFSET(container_d), 4);
    
    ee_ReadBytes((uint8_t*)&global.measure_freq, E2P_OFFSET(measure_freq),  4);
    
    ee_ReadBytes((uint8_t*)&global.sensor_freq, E2P_OFFSET(sensor_freq),  4);
	
	ee_ReadBytes((uint8_t*)&global.gain_1, E2P_OFFSET(gain_1),  1);
	
	ee_ReadBytes((uint8_t*)&global.gain_2, E2P_OFFSET(gain_2),  1);
    
    ee_ReadBytes((uint8_t*)&global.signal_cond[0].start_mag, E2P_OFFSET(signal_cond[0].start_mag),  sizeof(signal_cond_t)*2);
    
    #endif
    
}
