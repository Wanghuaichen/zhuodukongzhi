
#ifndef __LCD12864_H
#define __LCD12864_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "stm32f10x.h"

typedef enum
{    
    NOT_REVERSE,
    REVERSE,
}LCD_REVERSE;


void rt_hw_lcd_init(void);
void ClearLCD(void);
void LCD_PortConfig(void);
    
//x: 0 - 7
//y: 0 -127

void transfer_cmd(uint8_t cmd);
void contrast_control_up(void);
void contrast_control_down(void);
void contrast_init(void);

void LCD_DisplayString(uint16_t Xpos, uint16_t Ypos, char *ptr,uint8_t reverse);
void LCD_DisplayFlowNum( char *ptr,uint8_t maxLen);
void LCD_DisplayFlowUnit(char *c);
void LCD_DisplayChar(uint8_t Xpos,uint8_t Ypos,char* addr);



#ifdef __cplusplus
}
#endif

#endif

