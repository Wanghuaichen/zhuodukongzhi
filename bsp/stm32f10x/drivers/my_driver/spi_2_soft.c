
/*
*****************************************************
Mode 0: CPOL:0  CPHA:0
Mode 1: CPOL:0  CPHA:1
Mode 2: CPOL:1  CPHA:0
Mode 3: CPOL:1  CPHA:1
*****************************************************
*/
#include "platform_config.h"
#include <stdint.h>
#include "stm32f10x.h"

#define SPI_GPIO_PORT     GPIOC
#define SPI_CS_PIN        GPIO_Pin_6
#define SPI_CLK_PIN        GPIO_Pin_7
#define SPI_MOSI_PIN        GPIO_Pin_8
#define SPI_MISO_PIN        GPIO_Pin_9
          
#if 0	
#else	

    #define SPI_CS_1()    SPI_GPIO_PORT->BSRR = SPI_CS_PIN                  
    #define SPI_CS_0()    SPI_GPIO_PORT->BRR = SPI_CS_PIN         
    #define SPI_CLK_1()   SPI_GPIO_PORT->BSRR = SPI_CLK_PIN                 /* CLK = 1 */
	#define SPI_CLK_0()   SPI_GPIO_PORT->BRR = SPI_CLK_PIN                  /* CLK = 0 */
	#define SPI_MOSI_1()  SPI_GPIO_PORT->BSRR = SPI_MOSI_PIN				/* MOSI = 1 */
	#define SPI_MOSI_0()  SPI_GPIO_PORT->BRR = SPI_MOSI_PIN				    /* MOSI = 0 */
	
    //#define SPI_MISO_READ()  ((SPI_GPIO_PORT->IDR & SPI_MISO_PIN) != 0)	    
    #define SPI_MISO_READ()  (1)	    
#endif

static uint8_t spi_mode;
static char SPI_Mode_0 (char SPI_byte);
static char SPI_Mode_1 (char SPI_byte);
static char SPI_Mode_2 (char SPI_byte);
static char SPI_Mode_3 (char SPI_byte);


static void SPI_DeInit(void)
{
    spi_mode = 0;
	SPI_CS_1();
}

void SPI_2_Init(uint8_t mode)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC , ENABLE);	

	GPIO_InitStructure.GPIO_Pin = SPI_CS_PIN | SPI_CLK_PIN| SPI_MOSI_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_Init(SPI_GPIO_PORT, &GPIO_InitStructure);
    
    spi_mode = mode;
	switch(mode)
	{
	    case 0:SPI_CLK_0();break;
		case 1:SPI_CLK_1();break;
		case 2:SPI_CLK_0();break;
		case 3:SPI_CLK_1();break;
		default:SPI_CLK_0();break;
	}
	SPI_CS_0();
}

void SPI_2_CS_0(void)
{
    SPI_CS_0();
}
void SPI_2_CS_1(void)
{
    SPI_CS_1();
}

char SPI_2_SendByte(char ch)
{
	switch(spi_mode)
	{
	    case 0:return SPI_Mode_0(ch);
		case 1:return SPI_Mode_1(ch);
		case 2:return SPI_Mode_2(ch);
		case 3:return SPI_Mode_3(ch);
		default:return SPI_Mode_0(ch);
	}
}

static char SPI_Mode_0 (char SPI_byte)   
{   
   unsigned char SPI_count;            // counter for SPI transaction   
   
   for (SPI_count = 8; SPI_count > 0; SPI_count--) // single byte SPI loop   
   {  
      if(SPI_byte & 0x80)
	  {
	    SPI_MOSI_1();
	  }
	  else
	  {
	    SPI_MOSI_0();
	  }
      SPI_byte = SPI_byte << 1;        // shift next bit into MSB   
         
      SPI_CLK_1();                      // set CLK high   
   
      SPI_byte |= SPI_MISO_READ();                // capture current bit on MISO   
   
      SPI_CLK_0();                      // set CLK low   
   }   
   return (SPI_byte);   
   
} // END SPI_Mode_0 

static char SPI_Mode_1 (char SPI_byte)   
{   
   unsigned char SPI_count;            // counter for SPI transaction   
   
   for (SPI_count = 8; SPI_count > 0; SPI_count--) // single byte SPI loop   
   {  
      SPI_CLK_1();                      // set CLK high   
      if(SPI_byte & 0x80)
	  {
	    SPI_MOSI_1();
	  }
	  else
	  {
	    SPI_MOSI_0();
	  }
      SPI_byte = SPI_byte << 1;         // shift next bit into MSB   
         
      SPI_CLK_0();                      // set CLK low   
   
      SPI_byte |= SPI_MISO_READ();      // capture current bit on MISO   
   }   
   return (SPI_byte);   
   
} // END SPI_Mode_1 

static char SPI_Mode_2 (char SPI_byte)   
{   
   unsigned char SPI_count;            // counter for SPI transaction   
   
   for (SPI_count = 8; SPI_count > 0; SPI_count--) // single byte SPI loop   
   {  
      if(SPI_byte & 0x80)
	  {
	    SPI_MOSI_1();
	  }
	  else
	  {
	    SPI_MOSI_0();
	  }
      SPI_byte = SPI_byte << 1;         // shift next bit into MSB   
         
      SPI_CLK_0();                      // set CLK low   
   
      SPI_byte |= SPI_MISO_READ();      // capture current bit on MISO   
   
      SPI_CLK_1();                      // set CLK high   
   }   
   return (SPI_byte);   
   
} // END SPI_Mode_2 

static char SPI_Mode_3 (char SPI_byte)   
{   
   unsigned char SPI_count;               // counter for SPI transaction   
   
   for (SPI_count = 8; SPI_count > 0; SPI_count--) // single byte SPI loop   
   {  
      SPI_CLK_0();                       // set CLK low   
      if(SPI_byte & 0x80)
	  {
	    SPI_MOSI_1();
	  }
	  else
	  {
	    SPI_MOSI_0();
	  }
      SPI_byte = SPI_byte << 1;         // shift next bit into MSB   
         
      SPI_CLK_1();                      // set CLK high   
   
      SPI_byte |= SPI_MISO_READ();      // capture current bit on MISO   
   }   
   return (SPI_byte);   
   
} // END SPI_Mode_3 
