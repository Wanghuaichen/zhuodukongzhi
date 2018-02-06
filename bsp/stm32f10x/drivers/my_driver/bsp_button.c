
#include <stdio.h>
#include "platform_config.h"
#include "bsp_button.h"


static BUTTON_T s_BtnUp;		/* UP键 */
static BUTTON_T s_BtnDown;		/* DOWN键 */
static BUTTON_T s_BtnOk;		/* OK键 */
static BUTTON_T s_BtnMenu;		/* MENU键 */
static BUTTON_T s_BtnMenuUp;
static BUTTON_T s_BtnMenuDown;
static BUTTON_T s_BtnMenuOk;

static KEY_FIFO_T s_Key;		/* 按键FIFO变量,结构体 */

static void  InitButtonVar(void);
static void  InitButtonHard(void);
static void  DetectButton(BUTTON_T *_pBtn);

static uint8_t statKeyDownUp(void) 		    {return UpKeyDown()? 1:0;}
static uint8_t statKeyDownDown(void) 		    {return DownKeyDown()? 1:0;}
static uint8_t statKeyDownOk(void) 		    {return OkKeyDown()? 1:0;}
static uint8_t statKeyDownMenu(void)          {return MenuKeyDown()? 1:0;}
//组合键 有4个状态
static uint8_t statKeyDownMenuUp(void) 		    
{
    if( MenuKeyDown() && UpKeyDown() )
    {
        return 1;       // all down
    }
    else if( MenuKeyDown() && (!UpKeyDown()) )
    {
        return 2;
    }
    else if( (!MenuKeyDown()) && UpKeyDown() )
    {
        return 3;
    }
    else
    {
        return 0;       // all up
    }
}

static uint8_t statKeyDownMenuDown(void)
{
    if( MenuKeyDown() && DownKeyDown() )
    {
        return 1;
    }
    else if( MenuKeyDown() && (!DownKeyDown()) )
    {
        return 2;
    }
    else if( (!MenuKeyDown()) && DownKeyDown() )
    {
        return 3;
    }
    else
    {
        return 0;
    }
}

static uint8_t statKeyDownMenuOk(void)
{
    if( MenuKeyDown() && OkKeyDown() )
    {
        return 1;
    }
    else if( MenuKeyDown() && (!OkKeyDown()) )
    {
        return 2;
    }
    else if( (!MenuKeyDown()) && OkKeyDown() )
    {
        return 3;
    }
    else
    {
        return 0;
    }
}
/*
*********************************************************************************************************
*	函 数 名:  InitButton
*	功能说明: 初始化按键
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void InitButton(void)
{
	InitButtonVar();		/* 初始化按键变量 */
	InitButtonHard();		/* 初始化按键硬件 */
}

/*
*********************************************************************************************************
*	函 数 名: PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参：_KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
void PutKey(uint8_t _KeyCode)
{
	s_Key.Buf[s_Key.Write] = _KeyCode;

	if (++s_Key.Write  >= KEY_FIFO_SIZE)
	{
		s_Key.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参：无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint8_t GetKey(void)
{
	uint8_t ret;

	if (s_Key.Read == s_Key.Write)
	{
		return KEY_NONE;
	}
	else
	{
		ret = s_Key.Buf[s_Key.Read];

		if (++s_Key.Read >= KEY_FIFO_SIZE)
		{
			s_Key.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: InitButtonHard
*	功能说明: 初始化按键硬件
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitButtonHard(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( UPKEY_GPIO_CLK | DOWNKEY_GPIO_CLK | \
        OKKEY_GPIO_CLK | \
        MENUKEY_GPIO_CLK,\
        ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = UPKEY_PIN ;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;    
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(UPKEY_GPIO_PORT, &GPIO_InitStructure);
	
    GPIO_InitStructure.GPIO_Pin = DOWNKEY_PIN ;
    GPIO_Init(DOWNKEY_GPIO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = OKKEY_PIN ;
    GPIO_Init(OKKEY_GPIO_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = MENUKEY_PIN ;
    GPIO_Init(MENUKEY_GPIO_PORT, &GPIO_InitStructure);
}
	
/*
*********************************************************************************************************
*	函 数 名: InitButtonVar
*	功能说明: 初始化按键变量
*	形    参：strName : 例程名称字符串
*			  strDate : 例程发布日期
*	返 回 值: 无
*********************************************************************************************************
*/
static void InitButtonVar(void)
{
	/* 对按键FIFO读写指针清零 */
	s_Key.Read = 0;
	s_Key.Write = 0;
	
	/* 初始化UP按键变量，支持按下、连发（周期10ms） */
	s_BtnUp.IsKeyDownFunc = statKeyDownUp;            /* 判断按键按下的函数 */
	s_BtnUp.FilterTime = BUTTON_FILTER_TIME;        /* 按键滤波时间 */
	s_BtnUp.LongTime = BUTTON_LONG_TIME;                           /* 长按时间 */
	s_BtnUp.Count = s_BtnUp.FilterTime / 2;         /* 计数器设置为滤波时间的一半 */
	s_BtnUp.State = 0;								/* 按键缺省状态，0为未按下 */
	s_BtnUp.KeyCodeDown = KEY_DOWN_UP;              /* 按键按下的键值代码 */
	s_BtnUp.KeyCodeUp = KEY_UP_UP;                          /* 按键弹起的键值代码，0表示不检测 */
	s_BtnUp.KeyCodeLong = 0;                        /* 按键被持续按下的键值代码，0表示不检测 */
 		

	/* 初始化DOWN按键变量，支持按下、连发（周期10ms） */
	s_BtnDown.IsKeyDownFunc = statKeyDownDown;        
	s_BtnDown.FilterTime = BUTTON_FILTER_TIME;      
	s_BtnDown.LongTime = BUTTON_LONG_TIME;
	s_BtnDown.Count = s_BtnDown.FilterTime / 2;
	s_BtnDown.State = 0;
	s_BtnDown.KeyCodeDown = KEY_DOWN_DOWN;
	s_BtnDown.KeyCodeUp = KEY_UP_DOWN; 
	s_BtnDown.KeyCodeLong = 0;
    
	/* 初始化Ok按键变量，支持按下 */
	s_BtnOk.IsKeyDownFunc = statKeyDownOk;
	s_BtnOk.FilterTime = BUTTON_FILTER_TIME;
	s_BtnOk.LongTime = 2*BUTTON_LONG_TIME; 
	s_BtnOk.Count = s_BtnOk.FilterTime / 2;
	s_BtnOk.State = 0;
	s_BtnOk.KeyCodeDown = KEY_DOWN_OK;
	s_BtnOk.KeyCodeUp = KEY_UP_OK;
	s_BtnOk.KeyCodeLong = 0;
	
	// 初始化Menu按键变量，支持按下
	s_BtnMenu.IsKeyDownFunc = statKeyDownMenu; 
	s_BtnMenu.FilterTime = BUTTON_FILTER_TIME; 
	s_BtnMenu.LongTime = BUTTON_LONG_TIME;
	s_BtnMenu.Count = s_BtnMenu.FilterTime / 2;
	s_BtnMenu.State = 0;
	s_BtnMenu.KeyCodeDown = KEY_DOWN_MENU;          
	s_BtnMenu.KeyCodeUp = KEY_UP_MENU;                        
	s_BtnMenu.KeyCodeLong = 0; 
                      
    
    s_BtnMenuUp.IsKeyDownFunc = statKeyDownMenuUp;       
	s_BtnMenuUp.FilterTime = BUTTON_FILTER_TIME;     
	s_BtnMenuUp.LongTime = 0;                         
	s_BtnMenuUp.Count = s_BtnMenu.FilterTime / 2;    
	s_BtnMenuUp.State = 0;                            
	s_BtnMenuUp.KeyCodeDown = 0;         
	s_BtnMenuUp.KeyCodeUp = 0;                  
	s_BtnMenuUp.KeyCodeLong = 0;                     
	s_BtnMenuUp.keyCodeCombPrime1_2 = KEY_COMB_MENU_PRIM_UP;                       
	s_BtnMenuUp.keyCodeCombPrime2_1 = KEY_COMB_UP_PRIM_MENU;                    
    
    s_BtnMenuDown.IsKeyDownFunc = statKeyDownMenuDown;       
	s_BtnMenuDown.FilterTime = BUTTON_FILTER_TIME;     
	s_BtnMenuDown.LongTime = 0;                         
	s_BtnMenuDown.Count = s_BtnMenu.FilterTime / 2;    
	s_BtnMenuDown.State = 0;                            
	s_BtnMenuDown.KeyCodeDown = 0;         
	s_BtnMenuDown.KeyCodeUp = 0;                  
	s_BtnMenuDown.KeyCodeLong = 0;                     
	s_BtnMenuDown.keyCodeCombPrime1_2 = KEY_COMB_MENU_PRIM_DOWN;                       
	s_BtnMenuDown.keyCodeCombPrime2_1 = KEY_COMB_DOWN_PRIM_MENU;  
    
    s_BtnMenuOk.IsKeyDownFunc = statKeyDownMenuOk;       
	s_BtnMenuOk.FilterTime = BUTTON_FILTER_TIME;     
	s_BtnMenuOk.LongTime = 0;                         
	s_BtnMenuOk.Count = s_BtnMenuOk.FilterTime / 2;    
	s_BtnMenuOk.State = 0;                            
	s_BtnMenuOk.KeyCodeDown = 0;         
	s_BtnMenuOk.KeyCodeUp = 0;                  
	s_BtnMenuOk.KeyCodeLong = 0;                     
	s_BtnMenuOk.keyCodeCombPrime1_2 = KEY_DOWN_MENU_AND_OK;                       
	s_BtnMenuOk.keyCodeCombPrime2_1 = 0; 
}

/*
*********************************************************************************************************
*	函 数 名: DetectButton
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参：按键结构变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void DetectButton(BUTTON_T *_pBtn)
{
    uint8_t stat = _pBtn->IsKeyDownFunc();
    
    switch(stat)
    {
        case 0:
            if(_pBtn->State != 0)
            {                
                if(_pBtn->Count > _pBtn->FilterTime)
                {
                    _pBtn->Count = _pBtn->FilterTime;
                }
                else if(_pBtn->Count != 0)
                {
                    _pBtn->Count--;
                }
                else
                {
                    _pBtn->LongCount = 0;
                    _pBtn->State = 0;
                    if (_pBtn->KeyCodeUp > 0)
                    {
                        PutKey(_pBtn->KeyCodeUp);
                    }
                }
            }            
            break;
        case 1:
            if(_pBtn->State != 1)
            {
                if (_pBtn->Count < _pBtn->FilterTime)
                {
                    _pBtn->Count = _pBtn->FilterTime;
                }
                else if(_pBtn->Count < 2 * _pBtn->FilterTime)
                {
                    _pBtn->Count++;
                }
                else
                {                    
                    if(_pBtn->State == 0)
                    {
                        if(_pBtn->KeyCodeDown > 0)
                        {
                            PutKey(_pBtn->KeyCodeDown);
                        }                        
                    }
                    else if(_pBtn->State == 2)
                    {
                        if(_pBtn->keyCodeCombPrime1_2 > 0)
                        {
                            PutKey(_pBtn->keyCodeCombPrime1_2);
                        }
                    }
                    else if(_pBtn->State == 3)
                    {
                        if(_pBtn->keyCodeCombPrime2_1 > 0)
                        {
                            PutKey(_pBtn->keyCodeCombPrime2_1);
                        }
                    }
                    _pBtn->State = 1;
                }                    
            }
            else
            {
                if (_pBtn->LongTime > 0)
                {
                    if (_pBtn->LongCount < _pBtn->LongTime)
                    {
                        if (++_pBtn->LongCount == _pBtn->LongTime)
                        {
                            PutKey(_pBtn->KeyCodeLong);						
                        }
                    }                    
                }
            }
            break;
        case 2:
            if(_pBtn->State == 0)
            {            
                 if (_pBtn->Count < _pBtn->FilterTime)
                {
                    _pBtn->Count = _pBtn->FilterTime;
                }
                else if(_pBtn->Count < 2 * _pBtn->FilterTime)
                {
                    _pBtn->Count++;
                }
                else
                {
                    _pBtn->State = 2;
                }
            }
            else if(_pBtn->State == 1)
            {
                if(_pBtn->Count > _pBtn->FilterTime)
                {
                    _pBtn->Count = _pBtn->FilterTime;
                }
                else if(_pBtn->Count != 0)
                {
                    _pBtn->Count--;
                }
                else
                {
                    _pBtn->State = 2;
                }
            }
            break;
        case 3:
            if(_pBtn->State == 0)
            {            
                 if (_pBtn->Count < _pBtn->FilterTime)
                {
                    _pBtn->Count = _pBtn->FilterTime;
                }
                else if(_pBtn->Count < 2 * _pBtn->FilterTime)
                {
                    _pBtn->Count++;
                }
                else
                {
                    _pBtn->State = 3;
                }
            }
            else if(_pBtn->State == 1)
            {
                if(_pBtn->Count > _pBtn->FilterTime)
                {
                    _pBtn->Count = _pBtn->FilterTime;
                }
                else if(_pBtn->Count != 0)
                {
                    _pBtn->Count--;
                }
                else
                {
                    _pBtn->State = 3;
                }
            }
            break;
        default:
            break;
    }
}

/*
*********************************************************************************************************
*	函 数 名: KeyPro
*	功能说明: 检测所有按键。非阻塞状态，必须被周期性的调用。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void KeyPro(void)
{
    static uint8_t flag;
    
    if(flag == 0)
    {
        flag = 1;
        InitButton();
    }
       
    DetectButton(&s_BtnUp);        
    DetectButton(&s_BtnDown);      
    DetectButton(&s_BtnOk);        
    DetectButton(&s_BtnMenu);      
    DetectButton(&s_BtnMenuUp);      
    DetectButton(&s_BtnMenuDown);
    DetectButton(&s_BtnMenuOk);
}
