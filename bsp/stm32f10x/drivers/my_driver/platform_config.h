
#ifndef __PLATFORM_CONFIG_H
#define __PLATFORM_CONFIG_H

#ifdef __cplusplus
extern "C" {
#endif
    
// Includes ----------------------------------------------

#include "stm32f10x.h"

/** button */
#define UPKEY_GPIO_CLK              RCC_APB2Periph_GPIOB
#define UPKEY_GPIO_PORT             GPIOB
#define UPKEY_PIN                   GPIO_Pin_6
#define DOWNKEY_GPIO_CLK            RCC_APB2Periph_GPIOB
#define DOWNKEY_GPIO_PORT           GPIOB
#define DOWNKEY_PIN                 GPIO_Pin_5
#define MENUKEY_GPIO_CLK            RCC_APB2Periph_GPIOB
#define MENUKEY_GPIO_PORT           GPIOB
#define MENUKEY_PIN                 GPIO_Pin_7
#define OKKEY_GPIO_CLK              RCC_APB2Periph_GPIOB
#define OKKEY_GPIO_PORT             GPIOB
#define OKKEY_PIN                   GPIO_Pin_4


#define UpKeyDown()                 (GPIO_ReadInputDataBit(UPKEY_GPIO_PORT, UPKEY_PIN) == Bit_RESET)
#define DownKeyDown()               (GPIO_ReadInputDataBit(DOWNKEY_GPIO_PORT, DOWNKEY_PIN) == Bit_RESET)
#define MenuKeyDown()               (GPIO_ReadInputDataBit(MENUKEY_GPIO_PORT, MENUKEY_PIN) == Bit_RESET)
#define OkKeyDown()                 (GPIO_ReadInputDataBit(OKKEY_GPIO_PORT, OKKEY_PIN) == Bit_RESET)
    
/** I2C */

#define I2C_SCL_PORT	            GPIOA
#define I2C_SDA_PORT	            GPIOC
#define I2C_GPIO_CLK 	            ( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC)
#define I2C_SCL_PIN		            GPIO_Pin_8			
#define I2C_SDA_PIN		            GPIO_Pin_9			

#define I2C_SCL_1()  I2C_SCL_PORT->BSRR = I2C_SCL_PIN				/* SCL = 1 */
#define I2C_SCL_0()  I2C_SCL_PORT->BRR = I2C_SCL_PIN				/* SCL = 0 */

#define I2C_SDA_1()  I2C_SDA_PORT->BSRR = I2C_SDA_PIN				/* SDA = 1 */
#define I2C_SDA_0()  I2C_SDA_PORT->BRR = I2C_SDA_PIN				/* SDA = 0 */

#define I2C_SCL_READ()  ((I2C_SCL_PORT->IDR & I2C_SCL_PIN) != 0)	/* read SCL state */
#define I2C_SDA_READ()  ((I2C_SDA_PORT->IDR & I2C_SDA_PIN) != 0)	/* read SDA state */	

#ifdef __cplusplus
}
#endif

#endif

