
#ifndef __MENU_FRAME_H
#define __MENU_FRAME_H
#ifdef __cplusplus
extern "C" {
#endif
    
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "bsp_button.h"
#include "lcd12864.h"    
    
#ifndef NULL
#define NULL 0
#endif
    
#define LANG_MAX		2	
#define STAGE_MAX 		5
#define INDEX_MAX		30
#define MENU_MAX 		70
#define LINE_MAX		4

#define ROW 	LINE_MAX
#define COL		25
        
typedef enum
{
    STATUS_NODE,
    STATUS_LEAF,
    /* alarm or other exception */
    STATUS_EXCEPTION,

}MenuStatusDef;


typedef struct MenuTree MenuTreeDef,*pMenuTreeDef;
struct MenuTree
{
	const char *pIndex;
	const char *pTitle[LANG_MAX];
	void (*pFunc)(uint8_t msg);
};

void jump2func( void(*pFunc)(uint8_t msg) );
void jump2submenu(char* pIndex);
void menu_main(void);
void menu_var_init(pMenuTreeDef pMenu);
void menu_default(uint8_t msg);
void leaf_exit( void(*pFunc)(void) );

#ifdef __cplusplus
}
#endif

#endif
