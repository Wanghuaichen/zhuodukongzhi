
#include "lcd12864.h"
#include "fonts.h"
#include "global.h"

#define LCD_GPIO_RST_PORT	GPIOC
#define LCD_GPIO_RST		GPIO_Pin_12

#define LCD_GPIO_CS_PORT	GPIOD
#define LCD_GPIO_CS			GPIO_Pin_2

#define LCD_GPIO_CD_PORT	GPIOC
#define LCD_GPIO_CD			GPIO_Pin_11

#define LCD_GPIO_SCK_PORT	GPIOC
#define LCD_GPIO_SCK		GPIO_Pin_10

#define LCD_GPIO_SDA_PORT	GPIOA
#define LCD_GPIO_SDA		GPIO_Pin_15


#define LCD_RST_1()  LCD_GPIO_RST_PORT->BSRR = LCD_GPIO_RST	
#define LCD_RST_0()  LCD_GPIO_RST_PORT->BRR = LCD_GPIO_RST

#define LCD_CS_1()  LCD_GPIO_CS_PORT->BSRR = LCD_GPIO_CS
#define LCD_CS_0()  LCD_GPIO_CS_PORT->BRR = LCD_GPIO_CS

#define LCD_CD_1()  LCD_GPIO_CD_PORT->BSRR = LCD_GPIO_CD
#define LCD_CD_0()  LCD_GPIO_CD_PORT->BRR = LCD_GPIO_CD

#define LCD_SCK_1()  LCD_GPIO_SCK_PORT->BSRR = LCD_GPIO_SCK 
#define LCD_SCK_0()  LCD_GPIO_SCK_PORT->BRR = LCD_GPIO_SCK 

#define LCD_SDA_1()  LCD_GPIO_SDA_PORT->BSRR = LCD_GPIO_SDA 
#define LCD_SDA_0()  LCD_GPIO_SDA_PORT->BRR = LCD_GPIO_SDA 

// frame buffer
static uint8_t lcd_buf[8][128];

static void delay(int i)
{
	volatile int j,k;
	for(j = i;j > i;j--)
		for(k = 110;k > 0;k--);
}

void transfer_cmd(uint8_t cmd)
{
	char i;
	LCD_CS_0();
	LCD_CD_0();
	for(i = 8; i > 0; i--)
	{
		LCD_SCK_0();
		if(cmd & 0x80)
			LCD_SDA_1();
		else
			LCD_SDA_0();
		LCD_SCK_1();
		cmd = cmd << 1;
	}
	LCD_CS_1();
}

void transfer_dat(uint8_t dat)
{
	char i;
	LCD_CS_0();
	LCD_CD_1();
	for(i = 8; i > 0; i--)
	{
		LCD_SCK_0();
		if(dat & 0x80)
			LCD_SDA_1();
		else
			LCD_SDA_0();
		LCD_SCK_1();
		dat = dat << 1;
	}
	LCD_CS_1();
}

uint8_t g_lcd_contrast = 0x28;
void contrast_control_up(void)
{
	if(g_lcd_contrast < 40)
		g_lcd_contrast++;
	transfer_cmd(0x81);		// ?????
	transfer_cmd(g_lcd_contrast);		// ??????? 0x00 ~ 0x3F
}
void contrast_control_down(void)
{
	if(g_lcd_contrast > 25)
		g_lcd_contrast--;
	transfer_cmd(0x81);		// ?????
	transfer_cmd(g_lcd_contrast);		// ??????? 0x00 ~ 0x3F
}

void contrast_init(void)
{
    if(g_lcd_contrast > 40 || g_lcd_contrast < 25)
    {
        g_lcd_contrast = 32;
    }
	transfer_cmd(0x81);
	transfer_cmd(g_lcd_contrast);
}

void lcd_init()
{
	LCD_CS_0();
	LCD_RST_0();			// low to reset
	delay(100);
	LCD_RST_1();			// reset over
	delay(20);
	transfer_cmd(0xe2);		// soft reset
	delay(5);
	transfer_cmd(0x2c);		// volatage step up step 1
	delay(5);
	transfer_cmd(0x2e);		// volatage step up step 2
	delay(5);
	transfer_cmd(0x2f);		// volatage step up step 3
	delay(5);
	transfer_cmd(0x23);		// ????? 0x20-0x27
	transfer_cmd(0x81);		// ?????
	transfer_cmd(25);		// default contrast
	transfer_cmd(0xa2);
	if(global.is_disp_reverse == 0)
    {
        transfer_cmd(0xc8);     // c8   c0
        transfer_cmd(0xa0);     // a0   a1
	}
    else
    {
        transfer_cmd(0xc0);     // c8   c0
        transfer_cmd(0xa1);     // a0   a1
    }
    transfer_cmd(0x40);
	transfer_cmd(0xaf);
	LCD_CS_1();
}

/****************************************************************************/
void LCD_PortConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);     //GPIO_Remap_SWJ_JTAGDisable,GPIO_Remap_SWJ_Disable
    
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD ,ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

/*--------------------------------------------------------------*/

//y: 0 - 7
//x: 0 -127
static void LCD_byte_pos(unsigned char y, unsigned char x)
{
	transfer_cmd(0xb0 + y);
    if(global.is_disp_reverse == 1)
        x += 4;
	transfer_cmd( ((x >> 4) & 0x0f) + 0x10);
	transfer_cmd( x & 0x0f );
}
	
/*--------------------------------------------------------------*/

void rt_hw_lcd_init(void)
{
	LCD_PortConfig();
	lcd_init();
	ClearLCD();
}

/*------------------------------------------------------------*/
void ClearLCD(void)
{    
   	LCD_DisplayString(0,0, "                ",NOT_REVERSE);
    LCD_DisplayString(0,16,"                ",NOT_REVERSE);
    LCD_DisplayString(0,32,"                ",NOT_REVERSE);
    LCD_DisplayString(0,48,"                ",NOT_REVERSE);
	lcd_buf_flush();
}

void fill_disp_buf(char* buf_up,char* buf_down,char *c,uint8_t width,uint8_t reverse)
{
	int i;
    char *cBuf_1,*cBuf_2,*cBuf_3,*cBuf_4;
	
	if(width == 8)
    {
        cBuf_1 = c;
        cBuf_2 = c + 8;
        for( i = 0;i < 8;i++)
        {
            if(reverse == REVERSE)
            {
                buf_up[i] = ~cBuf_1[i];
				buf_down[i] = ~cBuf_2[i];
            }
            else
            {
				buf_up[i] = cBuf_1[i];
				buf_down[i] = cBuf_2[i];
            }
        }
    }
    else if(width == 16)
    {
        cBuf_1 = c;
        cBuf_2 = c + 8;
        cBuf_3 = c + 16;
        cBuf_4 = c + 24;
        for( i = 0;i < 8;i++)
        {
            if(reverse == REVERSE)
            {
                buf_up[i] = ~cBuf_1[i];
                buf_up[i+8] = ~cBuf_2[i];
                buf_down[i] = ~cBuf_3[i];
                buf_down[i+8] = ~cBuf_4[i];
            }
            else
            {
                buf_up[i] = cBuf_1[i];
                buf_up[i+8] = cBuf_2[i];
                buf_down[i] = cBuf_3[i];
                buf_down[i+8] = cBuf_4[i];
            }
        }
    } 
}

/*******************************************************************************
LCD_FastDisplayString
*/
//Xpos:0-127
//Ypos:0-63
//
void LCD_DisplayString(uint16_t Xpos, uint16_t Ypos, char *ptr,uint8_t reverse)
{
	uint16_t i = 0;
	uint8_t code1,code2,code3;
	
	uint8_t len;
	char *c,*buf_up,*buf_down,*p =(char*)lcd_buf;
    
    buf_up = &p[(Ypos >> 3)*128 +Xpos];
    buf_down = &p[(Ypos >> 3)*128 +128 + Xpos];
    
	len = 0;
	while ((*ptr != 0) & (i < 16))
	{
		code1 = (uint8_t)*ptr;
		if (code1 < 0x80)	/* ascii */
		{
			c = (char*)&Ascii16[code1 * 16];
			fill_disp_buf(buf_up,buf_down,c,8, reverse);
			buf_up += 8;
			buf_down += 8;
			len += 8;
		}
		else	
		{
			code2 = *++ptr;
            code3 = *++ptr;
			if ( (code2 < 0x80) || (code3 < 0x80) )
			{
                code1 = ' ';
                c = (char*)&Ascii16[code1 * 16];
				fill_disp_buf(buf_up,buf_down,c,8, reverse);
				buf_up += 8;
				buf_down += 8;
				len += 8;
				break;
			}
			get_hz_font(code1,code2,code3,&c);
            fill_disp_buf(buf_up,buf_down,c,16, reverse);
            
			buf_up += 16;
			buf_down += 16;
			len += 16;
		}
		ptr++;			/* */
		i++;
		if(len >= 128)
		{
			break;
		}
	}
	//display
//	LCD_byte_pos(Ypos >> 3, Xpos);
//	for(i = 0; i < len; i++)
//	{
//		transfer_dat(dispBuf[0][i]);
//	}
//	LCD_byte_pos( (Ypos >> 3) + 1, Xpos);
//	for(i = 0; i < len; i++)
//	{
//		transfer_dat(dispBuf[1][i]);
//	}
}


void draw_point(uint8_t x,uint8_t y,uint8_t dat)
{
    uint8_t *p = (uint8_t*)lcd_buf;
    
    if(y > 63)
        y = 63;
    if(y < 2)
        y = 2;
    y = 63 - y;
    if(dat == 1)
    {
        p[(y >> 3)*128 +x] |=  (1 << (y%8));
    }
    else
    {
        p[(y >> 3)*128 +x] &=  ~(1 << (y%8));
    }
    LCD_byte_pos( y, x);
    transfer_dat(p[(y >> 3)*128 +x]);
}

void draw_point_in_buffer(uint8_t x,uint8_t y,uint8_t dat)
{
    uint8_t *p = (uint8_t*)lcd_buf;
    
    if(y > 63)
        y = 63;
    if(y < 2)
        y = 2;
    y = 63 - y;
    
    if(dat == 1)
    {
        p[(y >> 3)*128 +x] |=  (1 << (y%8));
    }
    else
    {
        p[(y >> 3)*128 +x] &=  ~(1 << (y%8));
    }
}

void lcd_buf_flush(void)
{
    int i,j;
    uint8_t *p = (uint8_t*)lcd_buf;
    
    for(i = 0; i < 8; i++)
    {
        LCD_byte_pos( i, 0);
        for(j = 0; j < 128; j++)
        {
            transfer_dat(p[i*128+j]);
        }
    }
}

void get_line_dat(uint8_t line,uint8_t *buf)
{
	int i;
    uint8_t *p = (uint8_t*)lcd_buf;
	
	for(i = 0; i < 128; i++)
    {
        buf[i] = p[line*128+i]; 
    }
}

void set_line_dat(uint8_t line,uint8_t *buf)
{
	int i;
    uint8_t *p = (uint8_t*)lcd_buf;
	
	for(i = 0; i < 128; i++)
    {
        p[line*128+i] = buf[i]; 
    }
}

void lcd_fill_screen(uint8_t *buf)
{
	int i;
    uint8_t *p = (uint8_t*)lcd_buf;
	
	for(i = 0; i < 128*8; i++)
    {
        p[i] = buf[i]; 
    }
}

