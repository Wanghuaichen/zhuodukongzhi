#include "spi_soft.h"

static uint8_t init_flag;

static void mcp41xxx_init(void)
{
   SPI_1_Init(0);
   SPI_2_Init(0);
}

void mcp41xxx_write(uint8_t dev,uint8_t cmd,uint8_t dat)
{
    if(init_flag == 0)
    {
        init_flag = 1;
        mcp41xxx_init();
    }
	if(dev == 1)
	{
		SPI_1_CS_0();
		SPI_1_SendByte(cmd);
		SPI_1_SendByte(dat);
		SPI_1_CS_1();
	}
	else
	{
		SPI_2_CS_0();
		SPI_2_SendByte(cmd);
		SPI_2_SendByte(dat);
		SPI_2_CS_1();
	}
}

void mcp41xxx_set_pot(uint8_t dev,uint8_t pot_index,uint8_t dat)
{
    if(init_flag == 0)
    {
        init_flag = 1;
        mcp41xxx_init();
    }
    
	if(dev == 1)
	{
		SPI_1_CS_0();
		if(pot_index == 0)
			SPI_1_SendByte(0x11);
		else
			SPI_1_SendByte(0x12);
		SPI_1_SendByte(dat);
		SPI_1_CS_1();
	}
	else
	{
		SPI_2_CS_0();
		if(pot_index == 0)
			SPI_2_SendByte(0x11);
		else
			SPI_2_SendByte(0x12);
		SPI_2_SendByte(dat);
		SPI_2_CS_1();
	}
}
