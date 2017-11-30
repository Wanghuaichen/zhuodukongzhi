
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
    
    // not protected
	1,              //language
    
    1000,
    30000.0f,
    166666.7f,

    {{80,20,200,37000,20,150},{80,20,200,37000,20,150}},
    1,1,
    0,3,0,0,1,   //rs485
    
};


void restore_factory_settings(void)
{
    uint16_t ee_init = EE_RESET_FACTORY_ING;
    uint16_t addr;
    uint8_t *p;

    ee_WriteBytes((uint8_t*)&ee_init,0,2);
    
    addr = EE_UNPROTECTED();
    p = (uint8_t*)&c_FactorySetting + addr;
    ee_WriteBytes(p,addr,sizeof(c_FactorySetting) - addr);
    
    ee_init = EE_RESET_FACTORY_OVER;
    ee_WriteBytes((uint8_t*)&ee_init,0,2);
    
}

void full_reset_eeprom(void)
{
    uint8_t tmp_u8;
    uint16_t ee_init = EE_FULL_RECOVERY_ING;
    uint8_t *p = (uint8_t*)&c_FactorySetting;
    
    ee_WriteBytes((uint8_t*)&ee_init,0,2);
    
    ee_WriteBytes(p+2,2,sizeof(c_FactorySetting) - 2);
    //set the init over flag
    ee_init = EE_FULL_RECOVERY_OVER;
    ee_WriteBytes((uint8_t*)&ee_init,0,2);
}


void restore_convter_settings(void)
{
    uint16_t ee_init = EE_RESET_FACTORY_ING;
    uint8_t *p = (uint8_t*)&c_FactorySetting,lcdStyle,lcdContrast;
    uint16_t addr;

    ee_WriteBytes((uint8_t*)&ee_init,0,2);
       
    ee_ReadBytes((uint8_t*)&lcdStyle,E2P_OFFSET(is_disp_reverse),1);
    ee_ReadBytes((uint8_t*)&lcdContrast,E2P_OFFSET(lcd_contrast),1);
    
    addr = EE_UNPROTECTED();
    p = (uint8_t*)&c_FactorySetting + addr;
    
    ee_WriteBytes(p,addr,sizeof(c_FactorySetting) - addr);
    
    ee_WriteBytes((uint8_t*)&lcdStyle,E2P_OFFSET(is_disp_reverse),1);
    ee_WriteBytes((uint8_t*)&lcdContrast,E2P_OFFSET(lcd_contrast),1);
    
    ee_init = EE_RESET_FACTORY_OVER;
    ee_WriteBytes((uint8_t*)&ee_init,0,2);
    
}
