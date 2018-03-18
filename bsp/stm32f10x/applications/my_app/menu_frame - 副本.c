
#include "menu_frame.h"

#define ENTER 	0
#define BACK 	1

#define __INLINE         __inline
#define __STATIC_INLINE  static __inline

typedef struct MenuFuncReload MenuFuncReloadDef,pMenuFuncReloadDef;
struct MenuFuncReload
{
	bool reload;	
	void(*virtualFunc)(uint8_t msg);
};
static MenuFuncReloadDef s_FuncReload;

/* variable declare */
static uint8_t s_MenuStatus = STATUS_LEAF;
static uint8_t s_CurrIndex,s_CurrLine,s_nSubNode,s_IndexStackPtr,s_LinePtr;
static pMenuTreeDef s_IndexPtrStack[STAGE_MAX];				// empty-ascend
static uint8_t s_LineStack[STAGE_MAX];   			// index in every stage should display in line s_Index_line[x]
static pMenuTreeDef s_CurrNodeList[INDEX_MAX];
static pMenuTreeDef psMenu,psMenuCurr;				// current pointer of menu
/* display ram */
static char s_disp_ram[ROW][25];
/* function declare */
static void push_index(pMenuTreeDef index);
static pMenuTreeDef pop_index(void);
static void push_line(uint8_t line);
static uint8_t pop_line(void);

void menu_back(void);
static uint8_t get_curr_index(void);
static uint8_t update_node_list(char *str);
static void node_proc(uint8_t msg);
static void disp_node(void);

/* function realization */

void dummy(uint8_t msg)
{
    return;
}

static uint8_t deepth_indexstk(void)
{
    return s_IndexStackPtr;
}

__STATIC_INLINE bool is_indexstk_empty(void)
{
    if(s_IndexStackPtr == 0)
    {
        return true;
    }
    else
    {    
        return false;
    }
}

__STATIC_INLINE bool is_indexstk_full(void)
{
    if(s_IndexStackPtr == STAGE_MAX - 1)
    {
        return true;
    }
    else
    {    
        return false;
    }
}

__STATIC_INLINE void push_index(pMenuTreeDef index)
{
	if(s_IndexStackPtr < STAGE_MAX)
	{
		s_IndexPtrStack[s_IndexStackPtr++] = index;
	}
}

__STATIC_INLINE pMenuTreeDef pop_index(void)
{
	if(s_IndexStackPtr > 0)
	{
		return s_IndexPtrStack[--s_IndexStackPtr];
	}
	return NULL;
}

__STATIC_INLINE void push_line(uint8_t line)
{
	if(s_LinePtr < STAGE_MAX)
	{
		s_LineStack[s_LinePtr++] = line;
	}
}

__STATIC_INLINE uint8_t pop_line(void)
{
	if(s_LinePtr > 0)
	{
		return s_LineStack[--s_LinePtr];
	}
	return 0;
}

void jump2func( void(*pFunc)(uint8_t msg) )
{
	if(pFunc != NULL)
	{
		s_FuncReload.virtualFunc = pFunc;
		s_FuncReload.reload = true;
	}
	else
	{
		s_FuncReload.reload = false;
	}
}
void menu_init(void);
void menu_main(void)
{
	uint8_t key;
    static uint16_t tim;
        
    key = GetKey();
	if(s_MenuStatus == STATUS_LEAF)
	{
		if(s_FuncReload.reload == false)
		{
			psMenuCurr->pFunc(key);
		}
		else
		{
			s_FuncReload.virtualFunc(key);
		}
		return;
	}
	else
	{
//        if(key == KEY_NONE)
//        {
//            tim++;
//            if(tim == 600)
//            {
//                tim = 0;
//                menu_init();
//            }
//        }
//        else
//        {
//            tim = 0;
//        }
		node_proc(key);
        disp_node();
	}
}

static void node_proc(uint8_t msg)
{
	switch(msg)
	{
	case KEY_DOWN_UP:
		if(s_CurrIndex == 0 && s_CurrLine == 0)
		{
			s_CurrIndex = s_nSubNode - 1;
			if(s_nSubNode < 4)
				s_CurrLine = s_nSubNode - 1;
			else
				s_CurrLine = LINE_MAX - 1;
			psMenuCurr = s_CurrNodeList[s_CurrIndex];
			break;
		}
		else
		{
			s_CurrIndex--;
		}
		if(s_CurrLine > 0)
		{
			s_CurrLine--;
		}
		psMenuCurr = s_CurrNodeList[s_CurrIndex];
		//printf("%s,max:%d,index:%d,line:%d\n",psMenuCurr->pTitle[lang],s_nSubNode,s_CurrIndex,s_CurrLine);
		break;
	case KEY_DOWN_DOWN:
		if(s_CurrIndex == (s_nSubNode - 1) )
		{
			s_CurrIndex = 0;
			s_CurrLine = 0;
			psMenuCurr = s_CurrNodeList[s_CurrIndex];
			break;
		}
		if(s_CurrIndex < s_nSubNode - 1)
		{
			s_CurrIndex++;
		}
		if( (s_CurrLine < LINE_MAX - 1) && (s_CurrLine < s_nSubNode - 1) )
		{
			s_CurrLine++;
		}
		psMenuCurr = s_CurrNodeList[s_CurrIndex];
		//printf("%s,max:%d,index:%d,line:%d\n",psMenuCurr->pTitle[lang],s_nSubNode,s_CurrIndex,s_CurrLine);
		break;
	case KEY_DOWN_OK:	
		if( (psMenuCurr->pFunc != NULL) && (is_indexstk_empty() == false) )
		{
			s_MenuStatus = STATUS_LEAF;
		}
        else
        {
            menu_default();
        }
		break;
	case KEY_DOWN_MENU:
		menu_back();
		break;
	default:
		break;
	}
}

void menu_var_init(pMenuTreeDef pMenu)
{
	uint8_t i;
    
	s_FuncReload.reload = false;
    s_FuncReload.virtualFunc = dummy;
	psMenu = pMenu;						// init psMenu(the first addr of the menu-buffer)
	for( i = 0; i < MENU_MAX; i++)
	{
		/* pIndex = "0" */
        if( strcmp( (pMenu + i)->pIndex,"0") == 0 )
		{
			psMenuCurr = pMenu + i;		// init psMenuCurr
			break;
		}
	}
	s_MenuStatus = STATUS_LEAF;
	s_CurrIndex = 0;
	s_CurrLine = 0;
	s_nSubNode = 0;
	s_IndexStackPtr = 0;
	s_LinePtr = 0;
	for(i = 0; i < STAGE_MAX; i++)
	{
		s_IndexPtrStack[i] = 0;
		s_LineStack[i] = 0;
	}
	for(i = 0; i < INDEX_MAX; i++)
	{
		s_CurrNodeList[i] = 0;
	}
}

void menu_default(void)
{
	jump2menu(NULL);
}

void jump2menu(char *pIndex)
{
	char tmpOut[20];
	
    s_MenuStatus = STATUS_NODE;
	if(pIndex == NULL)
	{
		strncpy(tmpOut,psMenuCurr->pIndex,18 - 1);
		strncat(tmpOut,".",2);
	}
	else
	{
		strncpy(tmpOut,pIndex,18 - 1);
		strncat(tmpOut,".",2);
	}
	
	if(update_node_list(tmpOut))
	{
		push_index(psMenuCurr);
		s_CurrIndex = 0;
		push_line(s_CurrLine);
		s_CurrLine = 0;
		psMenuCurr = s_CurrNodeList[s_CurrIndex];
	}
}

void menu_back(void)
{	
	char *p,*ptmp;
	char tmpOut[20];
	if(is_indexstk_empty() == false)
	{
		psMenuCurr = pop_index();
		s_CurrLine = pop_line();
		s_CurrIndex = get_curr_index();
		if( strcmp(psMenuCurr->pIndex,"0") == 0 )
		{
			s_MenuStatus = STATUS_LEAF;
			s_CurrNodeList[0] = psMenuCurr;
			s_nSubNode = 1;
			return ;
		}
		strncpy(tmpOut,psMenuCurr->pIndex,18 - 1);
		ptmp = p = tmpOut;
		while( (p = strchr(p,'.')) )
		{
			ptmp = p;
			p = p + 1;
		}
		*(ptmp + 1) = 0;			
		 update_node_list(tmpOut);
	}
}

static uint8_t get_curr_index(void)
{
	uint8_t index;
	int n;
	char *p,*ptmp;
	ptmp = p = (char*)psMenuCurr->pIndex;
    
	while( (p = strchr(p,'.')) )
	{
		ptmp = p;
		p = p + 1;
	}
	sscanf( ptmp + 1,"%3d",&n);
	//n = atoi(ptmp + 1);
	index = n;
	return index;
}

static uint8_t update_node_list(char *str)
{	
	uint8_t i,num1,updateFlag = 0,subnodeTmp,lenStr;
	char *p;
	
	lenStr = strlen(str);
    subnodeTmp = s_nSubNode;
	s_nSubNode = 0;
	for(i = 0; i < MENU_MAX; i++)
	{
		if( (psMenu + i)->pIndex == NULL)
		{
			continue;
		}
		if(0 == strncmp( (psMenu + i)->pIndex,str,lenStr ) )
		{
			p = (char*)(psMenu + i)->pIndex;
			if( !strchr(p + lenStr,'.') )
			{
				sscanf( p + lenStr,"%3d",&num1);
				//num1 = atoi(p + lenStr);
				s_CurrNodeList[num1] = psMenu + i;
				s_nSubNode++;
			}
		}
	}
    if(s_nSubNode == 0)
    {
        s_nSubNode = subnodeTmp;
    }
    else
    {
        updateFlag = 1;
    }	
    return updateFlag;
}


void leaf_exit( void(*exit_callback)(void) )
{
	s_MenuStatus = STATUS_NODE;
	s_FuncReload.reload = false;
    if(exit_callback != NULL)
    {
        exit_callback();
    }
}

void disp_node(void)
{
	uint8_t lang = get_language();
	int8_t i,line = s_CurrLine;
	int j = 0;

	if( (s_MenuStatus == STATUS_LEAF) )
    {
        return;
    }
	for(i = -s_CurrLine; i < LINE_MAX - s_CurrLine; i++)
	{
		if(++j < s_nSubNode + 1)
		{
			//becareful overflow
            if(lang == 1)
            {
                snprintf(&s_disp_ram[line + i][1],24,"%-24s   ",(psMenuCurr + i)->pTitle[lang]);
                //strncpy(&s_disp_ram[line + i][1],(psMenuCurr + i)->pTitle[lang],15);
                s_disp_ram[line + i][24] = '\0';
            }
            else
            {
                snprintf(&s_disp_ram[line + i][0],17,"%-16s   ",(psMenuCurr + i)->pTitle[lang]);
                //strncpy(&s_disp_ram[line + i][1],(psMenuCurr + i)->pTitle[lang],15);
                s_disp_ram[line + i][16] = '\0';
            }
		}
		else
		{
            if(lang == 1)
            {
                snprintf(&s_disp_ram[line + i][1],24,"%-24s   "," ");
                //strncpy(&s_disp_ram[line + i][1] , "               ",15);
                s_disp_ram[line + i][24] = '\0';
            }
            else
            {
                snprintf(&s_disp_ram[line + i][0],17,"%-16s   "," ");
                //strncpy(&s_disp_ram[line + i][1] , "               ",15);
                s_disp_ram[line + i][16] = '\0';
            }
		}
	}
	// display
	//system("cls");
	for(i = 0; i < LINE_MAX; i++)
	{
		if(i == s_CurrLine)
		{
			//printf("%c%.*s---\n",16,COL - 1,&s_disp_ram[i][0]);
            if(lang == 1)
                s_disp_ram[i][0] = ' ';//16;
            LCD_DisplayString(0,i*16,(uint8_t*)&s_disp_ram[i][0],REVERSE);
		}
		else
		{
			//printf("%c%.*s---\n",' ',COL - 1,&s_disp_ram[i][0]);
            if(lang == 1)
                s_disp_ram[i][0] = ' ';
            LCD_DisplayString(0,i*16,(uint8_t*)&s_disp_ram[i][0],NOT_REVERSE);
		}
	}
}

