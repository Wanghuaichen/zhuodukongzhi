
#ifndef __CONF_STORE_H
#define __CONF_STORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>    
#include <math.h>
#include "global.h"
 
#define    EE_FULL_RECOVERY_ING   0x6056
#define    EE_FULL_RECOVERY_OVER  0x60A6
#define	   EE_RESET_FACTORY_ING   0x6506
#define    EE_RESET_FACTORY_OVER  0x6A06

 
typedef struct
{
    /* if all data in the eeprom is 0xff or 0x00 then it will be initialized with factory-setting */
    uint16_t eeprom;
    uint16_t ee_rw_test;
    uint8_t lcd_contrast;
	uint8_t is_disp_reverse;
    uint32_t password;
    
	//unprotected
    uint8_t unprotected;
    uint8_t language;
	/* comm */
    uint8_t comm_format;
    uint8_t comm_baud;
    uint8_t comm_databit;
    uint8_t comm_parity;
    uint8_t comm_device_id;
    
    uint8_t unit;
    int16_t temp_compensate;
    uint32_t serial_no;
    
    
}E2P_TypeDef;


/* EEPROM */
#define E2P_OFFSET(A)               ((uint16_t)offsetof(E2P_TypeDef,A))

#define E2P_EEPROM                  E2P_OFFSET(eeprom)
#define EE_UNPROTECTED()               E2P_OFFSET(unprotected)


#ifdef __cplusplus
}
#endif

#endif
