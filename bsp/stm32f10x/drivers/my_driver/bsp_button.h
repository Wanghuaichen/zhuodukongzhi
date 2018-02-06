
#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

#ifdef __cplusplus
extern "C" {
#endif
	
#include <stdint.h>
#include <rtthread.h>


#define BUTTON_FILTER_TIME 	(2)
#define BUTTON_LONG_TIME 	(100)	    /* 持续1秒，认为长按事件 */

typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t Count;			/* 滤波器计数器 */
	uint8_t FilterTime;		/* 滤波时间(最大255,表示2550ms) */
	uint16_t LongCount;		/* 长按计数器 */
	uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
	uint8_t  State;			/* 按键当前状态（按下还是弹起） */
	uint8_t KeyCodeUp;		/* 按键弹起的键值代码, 0表示不检测按键弹起 */
	uint8_t KeyCodeDown;	/* 按键按下的键值代码, 0表示不检测按键按下 */
	uint8_t KeyCodeLong;	/* 按键长按的键值代码, 0表示不检测长按 */
    uint8_t keyCodeCombPrime1_2;
    uint8_t keyCodeCombPrime2_1;
}BUTTON_T;

typedef enum
{
	KEY_NONE = 0,			/* 0 表示按键事件 */

	KEY_DOWN_UP,		/* UP键按下 */
	KEY_UP_UP,
    KEY_LONG_UP,
    
    KEY_DOWN_DOWN,		/* DOWN键按下 */
	KEY_UP_DOWN,
    KEY_LONG_DOWN,
    
    KEY_DOWN_MENU,		/* MENU键按下 */
	KEY_UP_MENU,
    KEY_LONG_MENU,
    
    KEY_DOWN_OK,		/* OK键按下 */
	KEY_UP_OK,
    KEY_LONG_OK,
    
    KEY_DOWN_MENU_AND_OK,
    KEY_UP_MENU_AND_OK,
    KEY_LONG_MENU_AND_OK,
    
    /*
    KEY_DOWN_MENU_UP,
    KEY_UP_MENU_UP,
    KEY_LONG_MENU_UP,
    
    KEY_DOWN_MENU_DOWN,
    KEY_UP_MENU_DOWN,
    KEY_LONG_MENU_DOWN,
    */
    KEY_COMB_MENU_PRIM_UP,
    KEY_COMB_UP_PRIM_MENU,
    
    KEY_COMB_MENU_PRIM_DOWN,
    KEY_COMB_DOWN_PRIM_MENU,
}KEY_ENUM;

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	20
typedef struct
{
	uint8_t Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t Read;					/* 缓冲区读指针 */
	uint8_t Write;					/* 缓冲区写指针 */
}KEY_FIFO_T;

/* 供外部调用的函数声明 */
void InitButton(void);
void PutKey(uint8_t _KeyCode);
uint8_t GetKey(void);
void KeyPro(void);

#ifdef __cplusplus
}
#endif

#endif


