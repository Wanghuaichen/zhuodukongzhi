
#include "app_config.h"
#include "global.h"
#include "conf_store.h"

const E2P_TypeDef c_FactorySetting = 
{
    /* if all data in the eeprom is 0xffff or 0x0000 then it will be initialized with factory-setting */
    EE_FULL_RECOVERY_OVER,
    0x55AA,
    30,
    1,
    100000,
    
    0.13,0.87,4,20,
    0.13,0.87,4,20,
    
    0,// not protected
	1,
    
    0,3,0,0,1,1,   //rs485
    0,
    0,
    586007,
    
};


void restore_factory_settings(void)
{
    uint16_t ee_init = EE_RESET_FACTORY_ING;
    uint16_t addr;
    uint8_t *p;

    rt_device_write(mb85_bus, 0, (uint8_t*)&ee_init, 2);
    
    addr = EE_UNPROTECTED();
    p = (uint8_t*)&c_FactorySetting + addr;
    rt_device_write(mb85_bus, addr, (uint8_t*)p, sizeof(c_FactorySetting) - addr);
    
    ee_init = EE_RESET_FACTORY_OVER;
    rt_device_write(mb85_bus, 0, (uint8_t*)&ee_init, 2);
    
}

void full_reset_eeprom(void)
{
    uint8_t tmp_u8;
    uint16_t ee_init = EE_FULL_RECOVERY_ING;
    uint8_t *p = (uint8_t*)&c_FactorySetting;
    
    //ee_WriteBytes((uint8_t*)&ee_init,0,2);
    rt_device_write(mb85_bus, 0, (uint8_t*)&ee_init, 2);
    
    //ee_WriteBytes(p+2,2,sizeof(c_FactorySetting) - 2);
    rt_device_write(mb85_bus, 2, (uint8_t*)(p+2), sizeof(c_FactorySetting) - 2);
    //set the init over flag
    ee_init = EE_FULL_RECOVERY_OVER;
    //ee_WriteBytes((uint8_t*)&ee_init,0,2);
    rt_device_write(mb85_bus, 0, (uint8_t*)&ee_init, 2);
}

