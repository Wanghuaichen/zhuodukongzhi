

#ifndef __ASCII16_H_
#define __ASCII16_H_

#include <stdint.h>

extern unsigned char const Ascii16[];

typedef struct 
{
    char hz[3];
    char font[32];
} T_HZ_Font;

typedef struct
{
    char ascii;
    char font[64];
} T_FLOW_Font;

uint16_t get_hz_font(uint8_t code1,uint8_t code2,uint8_t code3,char **c);
char* get_flow_font(uint8_t code);

#endif
