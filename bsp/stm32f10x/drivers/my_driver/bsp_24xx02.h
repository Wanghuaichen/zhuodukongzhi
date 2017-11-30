
#ifndef __BSP_24C02_H
#define __BSP_24C02_H

#ifdef __cplusplus
extern "C" {
#endif
    
#include <inttypes.h>
#include "bsp_i2c_gpio.h"
#include "platform_config.h"

#define EE_DEV_ADDR			0xA0		
#define EE_PAGE_SIZE		8			
#define EE_SIZE				256			

uint8_t ee_CheckOk(void);
uint8_t ee_ReadBytes(uint8_t *_pReadBuf, uint16_t _usAddress, uint16_t _usSize);
uint8_t ee_WriteBytes(uint8_t *_pWriteBuf, uint16_t _usAddress, uint16_t _usSize);

#ifdef __cplusplus
}
#endif

#endif
