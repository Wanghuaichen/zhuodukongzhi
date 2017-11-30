/*
***********************************************************
文件名	：		bsp_i2c_gpio.h
版本	：
描述	：
作者	：
日期	：
***********************************************************
*/


#ifndef __BSP_I2C_GPIO_H
#define __BSP_I2C_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>

#define I2C_WR	0		/* 写控制bit */
#define I2C_RD	1		/* 读控制bit */

void i2c_Start(void);
void i2c_Stop(void);
void i2c_SendByte(uint8_t _ucByte);
uint8_t i2c_ReadByte(void);
uint8_t i2c_WaitAck(void);
void i2c_Ack(void);
void i2c_NAck(void);
uint8_t i2c_CheckDevice(uint8_t _Address);

#ifdef __cplusplus
}
#endif

#endif
