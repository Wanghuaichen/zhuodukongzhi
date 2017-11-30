
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
	//unprotected
	uint8_t language;
    
    uint16_t measure_freq;
    float container_d;
	float sensor_freq;
	
	signal_cond_t signal_cond[2];
	uint8_t gain_1;
	uint8_t gain_2;
    /* comm */
    uint8_t comm_format;
    uint8_t comm_baud;
    uint8_t comm_databit;
    uint8_t comm_check;
    uint8_t comm_device_id; 
    
}E2P_TypeDef;


/* EEPROM */
#define E2P_OFFSET(A)               ((uint16_t)offsetof(E2P_TypeDef,A))

#define E2P_EEPROM                  E2P_OFFSET(eeprom)
#define EE_UNPROTECTED()               E2P_OFFSET(language)


#ifdef __cplusplus
}
#endif

#endif
