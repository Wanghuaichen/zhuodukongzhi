#ifndef  __SPI_SOFT_H
#define  __SPI_SOFT_H

#include <stdint.h>

void SPI_1_Init(uint8_t mode);
char SPI_1_SendByte(char ch);

void SPI_2_Init(uint8_t mode);
char SPI_2_SendByte(char ch);

void SPI_1_CS_0(void);
void SPI_1_CS_1(void);
void SPI_2_CS_0(void);
void SPI_2_CS_1(void);


#endif
