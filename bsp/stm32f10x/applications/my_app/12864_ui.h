#ifndef __12864_UI_H
#define __12864_UI_H

#include <stdint.h>

typedef enum BOX_STAT
{
	UNFOCUSED,
	FOCUSED,
	MODIFING,
	SAVING,
}BOX_STAT;

typedef enum 
{
	INTEGER,
	FLOAT,
}UI_DATA_TYPE;

typedef struct
{
	uint8_t x;
	uint8_t y;
	uint8_t is_focused:1;
    uint8_t is_show:1;
	uint8_t label_stat:6;
	uint8_t set_index;	
}Window_t;

typedef struct
{
	uint8_t x;
	uint8_t y;
	uint8_t is_focused:1;
    uint8_t is_readonly:1;
    uint8_t is_show:1;
	uint8_t box_stat:5;
	uint8_t box_width;
	uint8_t data_width;
	uint8_t set_index;
	uint8_t data_type;	
	uint32_t set_integer;
	float set_float;
	float min;
	float max;
	uint8_t set_buf[17];
	
}NumberBox_t;

BOX_STAT set_number_box(uint8_t msg,NumberBox_t* pBox);

typedef struct
{
	uint8_t x;
	uint8_t y;
	uint8_t is_focused:1;
    uint8_t is_show:1;
	uint8_t label_stat:6;
	uint8_t width;
	uint8_t label_buf[25];	
}Label_t;

void label_show(Label_t *label);

#endif
