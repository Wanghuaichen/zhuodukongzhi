
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

void draw_point(uint8_t x,uint8_t y,uint8_t dat);
void draw_point_in_buffer(uint8_t x,uint8_t y,uint8_t dat);
void lcd_buf_flush(void);
void get_line_dat(uint8_t line,uint8_t *buf);
void set_line_dat(uint8_t line,uint8_t *buf);

#ifdef __cplusplus
}
#endif

#endif

