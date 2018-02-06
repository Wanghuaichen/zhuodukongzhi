
#include <rtthread.h>
#include <rtdevice.h>
#include "lcd12864.h"
#include "bsp_button.h"
#include "global.h"
#include "menu_frame.h"
#include "menu_proc.h"
#include "conf_store.h"
#include "12864_ui.h"
//#include "bsp_24xx02.h"

/*
a(0) b(0.0) d(0.1) c(0.2) e(0.0.0) f(0.0.1) g(0.1.0) h(0.2.0) i(0.2.1)
			a
		  / |  \
		b	d	c
	  / |	|   |  \
	e	f	g	h	i
*/

typedef enum SET_STAT 
{
    SET_OK = 0,
    SET_ING,
    SET_SAVE,
    SET_MAX_MIN_ERROR,
    SET_EXIT,
}SET_STAT;
typedef struct SetIntFloat SetIntFloatDef;

struct SetIntFloat
{
    uint8_t dataType;
    uint8_t width;
    double setData;
    double min;
    double max;
    double *pRetVal;
};

static void get_utf8_len(char *pStr,uint16_t *pTotalLen,uint16_t *pAsciiLen);

#define SWAP16(A)   (uint16_t)((uint16_t)(A) << 8) | ((uint16_t)(A) >> 8)
#define SWAP32(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                                                 (((uint32_t)(A) & 0x00ff0000) >> 8) | \
                                                 (((uint32_t)(A) & 0x0000ff00) << 8) | \
                                                 (((uint32_t)(A) & 0x000000ff) << 24))

float swap_f32(float dat)
{
    uint32_t dat_32 = *(uint32_t*)&dat;
    dat_32 = SWAP32(dat_32);
    return *(float*) &dat_32;
}

void disp_main_screen(uint8_t x,uint8_t y,char *p,uint8_t width,uint8_t reverse_index);

/* user function def */
void menu_main_disp(uint8_t msg);

void menu_rs485_protocal(uint8_t msg);
void menu_rs485_baud(uint8_t msg);
void menu_rs485_databit(uint8_t msg);
void menu_rs485_check(uint8_t msg);
void menu_rs485_id(uint8_t msg);
void menu_slave_rs485_id(uint8_t msg);

void menu_range_sel(uint8_t msg);
void menu_range_1(uint8_t msg);
void menu_range_2(uint8_t msg);
void menu_range_3(uint8_t msg);
void menu_range_4(uint8_t msg);

void menu_linear_cali_point(uint8_t msg);
void menu_linear_cali(uint8_t msg);
void menu_linear_coeff_setup(uint8_t msg);
void menu_4_20_cali(uint8_t msg);    
    
void menu_reset_factory(uint8_t msg);


const static MenuTreeDef s_Menu[MENU_MAX] =
{
	/* note:the class and index must be continous */
// a(0) b(0.0) d(0.1) c(0.2) e(0.0.0) f(0.0.1) g(0.1.0) h(0.2.0) i(0.2.1)
	{"0",{"root","root"}, menu_main_disp},
	
	{"0.0",{"Range Setup" , "量程设置" },menu_default},
    {"0.1",{"Linear Cali Setup" , "线性校准" },menu_default},
    {"0.2",{"Linear Coeff Setup" , "线性系数设置" },menu_linear_coeff_setup},
    {"0.3",{"4-20 mA Setup" , "4-20mA校准" },menu_4_20_cali},
    {"0.4",{"485 Setup" , "485设置" },menu_default},
    
    {"0.0.0",{"Range Setup" , "量程选择" },menu_range_sel},
    {"0.0.1",{"Range Setup" , "量程-1设置" },menu_range_1},
    {"0.0.2",{"Range Setup" , "量程-2设置" },menu_range_2},
    {"0.0.3",{"Range Setup" , "量程-3设置" },menu_range_3},
    {"0.0.4",{"Range Setup" , "量程-4设置" },menu_range_4},
    
    {"0.1.0",{" " , "校准点数设置" },menu_linear_cali_point},
    {"0.1.1",{" " , "浊度校准" },menu_linear_cali},
    
    {"0.4.0",{"Modbus Addr" , "本机地址设置" },menu_rs485_id},
    {"0.4.1",{"Modbus baud" , "波特率设置" },menu_rs485_baud},
    {"0.4.2",{"Modbus baud" , "数据位" },menu_rs485_databit},
    {"0.4.3",{"Modbus baud" , "校验位" },menu_rs485_check},    
    {"0.4.4",{"Slave Addr" , "浊度地址设置" },menu_slave_rs485_id},
    
};

void menu_init(void);
SET_STAT set_select(uint8_t msg,void *pArray,uint8_t sizeEle,uint8_t sizeArray,uint16_t currIndex,uint8_t *pRetVal);
SET_STAT set_int_float_min_max(uint8_t msg,SetIntFloatDef setIntFloat);

void disp_select(char *p,uint8_t reverse);
void disp_int_float(char *p,uint8_t reverse);

/* display ram def */
static char s_DispBuf[ROW][25];
static uint8_t s_cur;;
static char s_set_buf[25];
static SetIntFloatDef setIntFloat;

static uint8_t s_tmp_buf[ROW][25];

char* s_blank = "                ";

void menu_init(void)
{
    global.language = 1;
	menu_var_init((pMenuTreeDef)s_Menu);
}

/*
***********************************************
*/
static void screen_ch1_set(uint8_t* pscreen_index,uint8_t msg);

static void screen_ch123_display(uint8_t* pscreen_index,uint8_t msg);
         
static void screen_zhuodu_display(uint8_t* pscreen_index,uint8_t msg);
        
void menu_main_disp(uint8_t msg)
{
    static uint8_t screen_index = 1;
    
    switch(screen_index)
    {
        case 0:
            screen_ch1_set(&screen_index,msg);
            break;
        case 1:
            screen_ch123_display(&screen_index,msg);
            break;
        case 2:
            screen_zhuodu_display(&screen_index,msg);
            break;        
        default: break;
    }
}

SET_STAT set_mainscreen_val(uint8_t msg,SetIntFloatDef setIntFloat);
static void screen_ch1_set(uint8_t* pscreen_index,uint8_t msg)
{
    /* stat : 0 - screen select 1 - value select 2 - value modifing 3 - save or not & return to 0 */
    static uint8_t stat = 0,curr_select ,cur,timer,is_float = 0,set_stat = SET_OK;
    SetIntFloatDef SetVal;
    double dat;
    const char* yesno = {"<-是        否->"};
    
    switch(stat)
    {
        case 0:
            switch(msg)
            {
            case KEY_DOWN_MENU:
                    if(*pscreen_index > 0)
                        (*pscreen_index)--;
                    break;
            case KEY_DOWN_OK:
                    if(*pscreen_index < 2)
                        (*pscreen_index)++;
                    break;
            case KEY_DOWN_DOWN:
                stat = 1;
                if(curr_select < 3)
                    curr_select++;
                else if(curr_select == 3)
                    curr_select = 0;
                break;
            case KEY_DOWN_UP:
                stat = 1;
                if(curr_select > 0)
                    curr_select--;
                else if(curr_select == 0)
                    curr_select = 3;
                break;
            break;
            case KEY_DOWN_MENU_AND_OK:            
                menu_default(NULL);            
            default:break;
            }
            break;
        case 1:
            switch(msg)
            {
            case KEY_DOWN_MENU:
                    stat = 0;
                    curr_select = 0;
                    break;
            case KEY_DOWN_OK:
                    stat = 2;
                    break;
            case KEY_DOWN_DOWN:
                if(curr_select < 3)
                    curr_select++;
                else if(curr_select == 3)
                    curr_select = 0;
                if(curr_select == 2)
                    is_float = 1;
                else
                    is_float = 0;
                break;
            case KEY_DOWN_UP:
                if(curr_select > 0)
                    curr_select--;
                else if(curr_select == 0)
                    curr_select = 3;
                if(curr_select == 2)
                    is_float = 1;
                else
                    is_float = 0;
                break;
            break;
            case KEY_DOWN_MENU_AND_OK:            
                menu_default(NULL);            
            default:break;
            }
            break;
        case 2:
            switch(curr_select)
            {
                case 0:
                    SetVal.dataType = INT_16_T;
                    SetVal.width = 5;
                    SetVal.max = 32000;
                    SetVal.min = 0;
                    SetVal.setData = global.zhuodu_data.canbi_set;
                    SetVal.pRetVal = &dat;
                
                    set_stat = set_mainscreen_val(msg,SetVal);
                    if(set_stat == SET_ING)
                    {
                        break;
                    }
                    else if(set_stat == SET_MAX_MIN_ERROR)
                    {
                        stat = 1;
                        break;
                    }
                    else if(set_stat == SET_OK)
                    {
                        global.zhuodu_data.canbi_set = dat;
                        stat = 1;
                    }                        
                    break;
                case 1:
                    
                    break;
                case 2:
                    break;                            
                default:
                    break;
            }
            
            break;
        case 3:
            switch(msg)
            {
                case KEY_DOWN_MENU:
                    //TODO save modified value & return
                    stat = 1;
                    cur = 0;
                    break;
                case KEY_DOWN_OK:
                    //DO NOT save modified value & return
                    stat = 1;
                    cur = 0;
                    break;
                case KEY_DOWN_MENU_AND_OK:
                    //DO NOT save modified value & return
                    stat = 0;
                    curr_select = 0;
                    cur = 0;
                    menu_default(NULL);
                    break;
                default:
                    break;
            }
            break;
        default: break;
    }
    // display
    switch(stat)
    {
        case 0:
            snprintf(&s_tmp_buf[0][0],25,"CH1目标 %5d",global.zhuodu_data.canbi_set);
            LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
            snprintf(&s_tmp_buf[1][0],25,"波动范围 %5d",global.zhuodu_data.bright_range);
            LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
            snprintf(&s_tmp_buf[2][0],25,"参比系数 %4.2f",global.zhuodu_data.a1k1);
            LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
            snprintf(&s_tmp_buf[3][0],25,"参比计算 %5f",global.zhuodu_data.canbi_calc_get);
            LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
            break;
        case 1:
            snprintf(&s_tmp_buf[0][0],25,"CH1目标 %5d",global.zhuodu_data.canbi_set);
            snprintf(&s_tmp_buf[1][0],25,"波动范围 %5d",global.zhuodu_data.bright_range);
            snprintf(&s_tmp_buf[2][0],25,"参比系数 %4.2f",global.zhuodu_data.a1k1);
            snprintf(&s_tmp_buf[3][0],25,"参比计算 %5f",global.zhuodu_data.canbi_calc_get);
            switch(curr_select)
            {
                case 0:
                    LCD_DisplayString(0,0,&s_tmp_buf[0][0],REVERSE);
                    LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
                    LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
                    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
                    break;
                case 1:
                    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
                    LCD_DisplayString(0,16,&s_tmp_buf[1][0],REVERSE);
                    LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
                    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
                    break;
                case 2:
                    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
                    LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
                    LCD_DisplayString(0,32,&s_tmp_buf[2][0],REVERSE);
                    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
                    break;
                case 3:
                    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
                    LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
                    LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
                    LCD_DisplayString(0,48,&s_tmp_buf[3][0],REVERSE);
                    break;
                default:
                    break;
            }
            break;
        case 2:
//            snprintf(&s_tmp_buf[0][0],25,"CH1目标 %5d",global.zhuodu_data.canbi_set);
//            snprintf(&s_tmp_buf[1][0],25,"波动范围 %5d",global.zhuodu_data.bright_range);
//            snprintf(&s_tmp_buf[2][0],25,"参比系数 %4.2f",global.zhuodu_data.a1k1);
//            snprintf(&s_tmp_buf[3][0],25,"参比计算 %5f",global.zhuodu_data.canbi_calc_get);
            switch(curr_select)
            {
                case 0:
                    switch(set_stat)
                    {
                        case SET_ING:
                            disp_main_screen(56,0,s_set_buf,5,s_cur);
                            LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
                            LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
                            LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
                            break;
                        case SET_SAVE:
                            disp_main_screen(56,0,s_set_buf,5,s_cur);
                            LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
                            LCD_DisplayString(0,32,(char*)yesno,NOT_REVERSE);
                            LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
                            break;
                        case SET_OK:
                            LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
                            break;
                        case SET_EXIT:
                            LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
                            break;
                        default:
                            break;
                    }
                    break;
                case 1:
                    break;
                case 2:
                    break;
                case 3:
                    break;
                default:
                    break;
            }
            break;
    }
    
}

static void screen_ch123_display(uint8_t* pscreen_index,uint8_t msg)
{
    switch(msg)
    {
        case KEY_DOWN_MENU:
            if((*pscreen_index) > 0)
                (*pscreen_index)--;
        break;
        case KEY_DOWN_OK:
            if((*pscreen_index) < 2)
                (*pscreen_index)++;
        break;
        case KEY_DOWN_DOWN:            
            break;
        case KEY_DOWN_UP:
            break;
        case KEY_DOWN_MENU_AND_OK:            
            menu_default(NULL);
            break;
        default:
            break;
    }
    
    snprintf(&s_tmp_buf[0][0],25,"  CH1参比%5d%s",global.zhuodu_data.canbi_get,s_blank);
    snprintf(&s_tmp_buf[1][0],25,"  CH2直射%5d%s",global.zhuodu_data.bright_2,s_blank);
    snprintf(&s_tmp_buf[2][0],25,"  CH3折射%5d%s",global.zhuodu_data.bright_3,s_blank);
    snprintf(&s_tmp_buf[3][0],25,"    温度 %4.1f%s",global.zhuodu_data.temp/10.0f,s_blank);
    
    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(0,16,&s_tmp_buf[1][0],NOT_REVERSE);
    LCD_DisplayString(0,32,&s_tmp_buf[2][0],NOT_REVERSE);
    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
}
         
static void screen_zhuodu_display(uint8_t* pscreen_index,uint8_t msg)
{
    switch(msg)
    {
        case KEY_DOWN_MENU:
        if(*pscreen_index > 0)
            (*pscreen_index)--;
        break;
        case KEY_DOWN_OK:
        if(*pscreen_index < 2)
            (*pscreen_index)++;
        break;
        case KEY_DOWN_DOWN:            
        break;
        case KEY_DOWN_UP:
        break;
        case KEY_DOWN_MENU_AND_OK:            
            menu_default(NULL);
            break;
        default:
            break;
    }
    snprintf(&s_tmp_buf[0][0],25,"   吸光度%5.3f%s",global.zhuodu_data.absorbance,s_blank);
    snprintf(&s_tmp_buf[1][0],25,"     浊度%5.3f%s",global.zhuodu_data.turbidimeter,s_blank);
    
    LCD_DisplayString(0,0,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,16,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(0,32,&s_tmp_buf[1][0],NOT_REVERSE);
    LCD_DisplayString(0,48,s_blank,NOT_REVERSE);
}

void menu_range_sel(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* const titleLang[] = { "          " , "量程选择        "};
    const char* const commFormatStr[][LANG_MAX]=
    {
        {"1","1"},
        {"2","2"},
        {"3","3"},
        {"4","4"},
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    
    setStat = set_select( msg, commFormatStr,sizeof(commFormatStr[0]),4, global.zhuodu_data.range_sel, &retVal);
   
    if( setStat == SET_OK  )
    {
        eMBMasterReqWriteHoldingRegister(1,0x100+29,retVal,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.range_sel = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_range_1(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { " " , "量程-1设置      "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.zhuodu_data.seg_range_1;
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);        
        eMBMasterReqWriteMultipleHoldingRegister(1,0x100+25 ,2,data.buf,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.seg_range_1 = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
void menu_range_2(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { " " , "量程-2设置      "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.zhuodu_data.seg_range_2;
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);
        eMBMasterReqWriteMultipleHoldingRegister(1,0x100+27 ,2,data.buf,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.seg_range_2 = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
void menu_range_3(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { " " , "量程-3设置      "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.zhuodu_data.seg_range_3;
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);
        eMBMasterReqWriteMultipleHoldingRegister(1,530 ,2,data.buf,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.seg_range_3 = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
void menu_range_4(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { " " , "量程-4设置      "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.zhuodu_data.seg_range_4;
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);
        eMBMasterReqWriteMultipleHoldingRegister(1,532 ,2,data.buf,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.seg_range_4 = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
void menu_linear_cali_point(uint8_t msg)
{
    uint16_t buf[5];
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* const titleLang[] = { "          " , "校准点设置        "};
    const char* const commFormatStr[][LANG_MAX]=
    {
        {"2","2"},
        {"3","3"},
        {"4","4"},
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    
    if(global.zhuodu_data.num_cali < 2)
        global.zhuodu_data.num_cali = 2;
    
    setStat = set_select( msg, commFormatStr,sizeof(commFormatStr[0]),3, global.zhuodu_data.num_cali-2, &retVal);
   
    if( setStat == SET_OK  )
    {
        eMBMasterReqWriteHoldingRegister(1,291,retVal+2,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.range_sel = retVal+2;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_linear_cali(uint8_t msg)
{
    
}
void menu_linear_coeff_setup(uint8_t msg)
{
}
void menu_4_20_cali(uint8_t msg)
{
}

SET_STAT set_mainscreen_val(uint8_t msg,SetIntFloatDef setIntFloat)
{
	uint8_t lang = get_language();
    static uint8_t state,flag1st,flagDotDisp;	/* in exit proc,it should be reset */
	uint8_t ascii;
    uint8_t data_type = setIntFloat.dataType;
	double retVal;
    int32_t ee_val_32;
    const char* yesno[] = {"<-Yes       No->","<-是        否->","<-Yes       No->"};
    
	if(flag1st == 0)
	{
		flag1st = 1;
        state = 0;
        s_cur = 0;
        ee_val_32 = setIntFloat.setData;
		switch(data_type)
		{   
		case INT_8_T:  
            snprintf(s_set_buf,4,"%3d",ee_val_32); flagDotDisp = 0; 
        break;
		case INT_16_T: 
            snprintf(s_set_buf,6,"%5d",ee_val_32); flagDotDisp = 0;
        break;
		case FLOAT_32_T: snprintf(s_set_buf,17,"%05.3f",setIntFloat.setData);flagDotDisp = 1; 
            break;
		default: return SET_EXIT;
		}       
	}
    //disp_int_float(s_set_buf,1);    
    
	switch(msg)
	{
	case KEY_DOWN_UP:
		if(state != 0)
			return SET_SAVE;
		else
		{
			ascii = ++s_set_buf[s_cur];
			if(s_cur == 0)
			{
                switch(ascii)
				{
				case '9' + 1:
                    if(flagDotDisp) 
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '-';
					return SET_ING;
				case '.' + 1: s_set_buf[s_cur] = '-'; return SET_ING;
				case '-' + 1: s_set_buf[s_cur] = '0'; return SET_ING;
				default: return SET_ING;
				}
			}
			else
			{
                switch(ascii)
				{
				case '9' + 1:
                    if(flagDotDisp)
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '0';
					return SET_ING;
				case '.' + 1: s_set_buf[s_cur] = '0'; return SET_ING;
				default: return SET_ING;
				}
			}
		}
	case KEY_DOWN_DOWN:
		if(state != 0)
            return SET_SAVE;
		else
		{
			ascii = --s_set_buf[s_cur];
			if(s_cur == 0)
			{
                switch(ascii)
				{
				case '0' - 1: s_set_buf[s_cur] = '-'; return SET_ING;
				case '-' - 1:
                    if(flagDotDisp)
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '9';
					return SET_ING;
				case '.' - 1: s_set_buf[s_cur] = '9'; return SET_ING;
				default: return SET_ING;
				}
			}
			else
			{
                switch(ascii)
				{
				case '0' - 1:
                    if(flagDotDisp)
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '9';
                    return SET_ING;
				case '.' - 1: s_set_buf[s_cur] = '9'; return SET_ING;
				default: return SET_ING;
				}
			}
		}
	case KEY_DOWN_MENU:
        if(state == 0)
        {
            state = 1; return SET_SAVE;
        }
        if(state == 1)
        { 
            flag1st = 0;
            switch(data_type)
			{
			case INT_8_T: sscanf(s_set_buf,"%3lf",&retVal); break;
			case INT_16_T: sscanf(s_set_buf,"%5lf",&retVal); break;
            case FLOAT_32_T: sscanf(s_set_buf,"%5lf",&retVal); break;	
			default: sscanf(s_set_buf,"%5lf",&retVal); break;
			}
            if(retVal > setIntFloat.max || retVal < setIntFloat.min)
            {
                flag1st = 0; return SET_MAX_MIN_ERROR;
            }
            else
            {
                *(setIntFloat.pRetVal) = retVal;
                return SET_OK;
            }
        }
	case KEY_DOWN_OK:
		if(state == 0)
		{			
            switch(data_type)
			{
			case INT_8_T: if(++s_cur == 3) s_cur = 0; return SET_ING;
			case INT_16_T: if(++s_cur == 5) s_cur = 0; return SET_ING;
            case FLOAT_32_T: if(++s_cur == 5) s_cur = 0; return SET_ING;
            default: return SET_ING;
            }
		}
		else
		{
            flag1st = 0;
            return SET_EXIT;
		}
	default: return SET_ING;
	}
}
/*
***********************************************
*/
void menu_slave_rs485_id(uint8_t msg)
{
}

void menu_rs485_protocal(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* const titleLang[] = { "Protocal        " , "通讯格式        "};
    const char* const commFormatStr[][LANG_MAX]=
    {
        {"Modbus-RTU","Modbus-RTU"},
        {"Modbus-ASCII","Modbus-ASCII"},
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    
    setStat = set_select( msg, commFormatStr,sizeof(commFormatStr[0]),2, global.comm_format, &retVal);
   
    if( setStat == SET_OK  )
    {
        global.comm_format = retVal;
        //ee_WriteBytes((uint8_t*)&retVal, E2P_OFFSET(comm_format), 1);
        rt_device_write(mb85_bus, E2P_OFFSET(comm_format), (uint8_t*)&retVal, 1);
        
        uint32_t baud;
        switch(global.comm_baud)
        {
            case 0: baud = 1200; break;
            case 1: baud = 2400; break;
            case 2: baud = 4800; break;
            case 3: baud = 9600; break;
            case 4: baud = 19200; break;
            case 5: baud = 38400; break;
        }        
        eMBInit(global.comm_format, global.comm_device_id, 3, baud,  global.comm_parity);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
    
void menu_rs485_baud(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* const titleLang[] = { "Baudrate        " , "波特率          "};
    const char* const commBaudStr[][LANG_MAX]=
    {
        {"1200","1200"},
        {"2400","2400"},
        {"4800","4800"},
        {"9600","9600"},
        {"19200","19200"},
        {"38400","38400"},
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_select( msg, commBaudStr,sizeof(commBaudStr[0]),countof(commBaudStr), global.comm_baud, &retVal);
    if( setStat == SET_OK  )
    {
        global.comm_baud = retVal;
        //ee_WriteBytes((uint8_t*)&retVal, E2P_OFFSET(comm_baud), 1);
        rt_device_write(mb85_bus, E2P_OFFSET(comm_baud), (uint8_t*)&retVal, 1);
        uint32_t baud;
        switch(global.comm_baud)
        {
            case 0: baud = 1200; break;
            case 1: baud = 2400; break;
            case 2: baud = 4800; break;
            case 3: baud = 9600; break;
            case 4: baud = 19200; break;
            case 5: baud = 38400; break;
        }
        eMBInit(global.comm_format, global.comm_device_id, 3, baud,  global.comm_parity);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_rs485_databit(uint8_t msg)
{
    uint8_t lang = get_language();
    uint8_t index;
    const char* const titleLang[] = { "Data Bit        " , "数据位            "};
    
    //ee_ReadBytes((uint8_t*)&index, E2P_OFFSET(comm_format), 1);
    //rt_device_read(mb85_bus, E2P_OFFSET(comm_format), (uint8_t*)&index, 1);
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    LCD_DisplayString(0,16,"                ",NOT_REVERSE);
    LCD_DisplayString(0,32,"                ",NOT_REVERSE);
    //if(index == 0)
    {
        LCD_DisplayString(0,48,"               8",NOT_REVERSE);
    }
    //else if(index == 1)
    {
        //LCD_DisplayString(0,48,"               7",NOT_REVERSE);
    }
    if(msg == KEY_DOWN_MENU)
        leaf_exit(NULL);
}

void menu_rs485_check(uint8_t msg)
{
    uint8_t lang = get_language();
    static uint8_t flag1st;
    SET_STAT setStat = 0;
    uint8_t currIndex = 0,retVal;
    const char* const titleLang[] = { "Check Mode      " , "校验方式            "};
    const char* const commCheckStr[][LANG_MAX]=
    {
        {"None","无校验"},
        {"Odd","奇校验"},
        {"Even","偶校验"},
    };
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    
    setStat = set_select( msg, commCheckStr,sizeof(commCheckStr[0]),2, currIndex, &retVal);
        
    if( setStat == SET_OK  )
    {
        global.comm_parity = retVal;
        //ee_WriteBytes((uint8_t*)&retVal, E2P_OFFSET(comm_parity), 1);
        rt_device_write(mb85_bus, E2P_OFFSET(comm_parity), (uint8_t*)&retVal, 1);
        uint32_t baud;
        switch(global.comm_baud)
        {
            case 0: baud = 1200; break;
            case 1: baud = 2400; break;
            case 2: baud = 4800; break;
            case 3: baud = 9600; break;
            case 4: baud = 19200; break;
            case 5: baud = 38400; break;
        }
        eMBInit(global.comm_format, global.comm_device_id, 3, baud,  global.comm_parity);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_rs485_id(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { "Device ID       " , "设备地址        "};
          
    setIntFloat.dataType = INT_8_T;
    setIntFloat.setData = global.comm_device_id;
    setIntFloat.max = 247.0f;
    setIntFloat.min = 1.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.comm_device_id = retVal;
        //ee_WriteBytes((uint8_t*)&retVal, E2P_OFFSET(comm_device_id), 1);
        rt_device_write(mb85_bus, E2P_OFFSET(comm_device_id), (uint8_t*)&retVal, 1);
        uint32_t baud;
        switch(global.comm_baud)
        {
            case 0: baud = 1200; break;
            case 1: baud = 2400; break;
            case 2: baud = 4800; break;
            case 3: baud = 9600; break;
            case 4: baud = 19200; break;
            case 5: baud = 38400; break;
        }
        eMBInit(global.comm_format, global.comm_device_id, 3, baud,  global.comm_parity);
        
        leaf_exit(NULL);
    }
    else if(setStat == SET_MAX_MIN_ERROR)
    {
        
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}


/*
***********************************************
*/
void menu_reset_factory(uint8_t msg)
{
    uint8_t lang = get_language();
    static uint8_t flag;
    const char* const titleLang[] = { "Reset Factory   ","恢复出厂设置    ","Reset Factory   "};
    const char* pStr[][3] = {{"No","否","No"},{"Yes","是","Yes"}};

    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    LCD_DisplayString(0,16,"                ",NOT_REVERSE);
    LCD_DisplayString(0,32,"                ",NOT_REVERSE);
    disp_select( (char*)pStr[flag][lang],REVERSE);

    switch(msg)
    {
        case KEY_DOWN_UP: if(flag == 0) flag = 1; else flag = 0; break;
        case KEY_DOWN_DOWN: if(flag == 0) flag = 1; else flag = 0; break;
        case KEY_DOWN_MENU: leaf_exit(NULL); flag = 0; break;
        case KEY_DOWN_OK: 
            if(flag == 1) 
            {restore_factory_settings(); NVIC_SystemReset(); break;}
        default: break;
    }
}

/*
***********************************************
*/
SET_STAT set_select(uint8_t msg,void *pArray,uint8_t sizeEle,uint8_t sizeArray,uint16_t currIndex,uint8_t *pRetVal)
{
	uint8_t lang = get_language();
    uint16_t totalLen,asciiLen,hzLen;
	static uint8_t ee_val,flag,flag1st;	/* in exit proc,it should be reset */
	char **ppStr;
    const char* yesno[] = {"<-Yes       No->","<-是        否->","<-Yes       No->"};
	//printf("ee_val:%d flag:%d --\r",ee_val,flag);
	if(flag1st == 0)
	{
        flag1st = 1;
        flag = 0;
        ee_val = currIndex;
		ppStr = (char**)((uint32_t)pArray + ee_val*sizeEle);
        
        get_utf8_len(ppStr[lang], &totalLen, &asciiLen);
        hzLen = (totalLen - asciiLen ) / 3;
        snprintf(&s_DispBuf[1][0],25,"%-.*s%s",16 - ((hzLen<<1) + asciiLen),s_blank,ppStr[lang]);
        
		LCD_DisplayString(0,16,&s_DispBuf[1][0],NOT_REVERSE);
	}
	ppStr = (char**)((uint32_t)pArray + ee_val*sizeEle);
    get_utf8_len(ppStr[lang], &totalLen, &asciiLen);
    hzLen = (totalLen - asciiLen ) / 3;
    snprintf(s_set_buf,25,"%-s",ppStr[lang]);
    
	//disp_select(s_set_buf,flag);
    disp_select(s_set_buf,1);
    
    if(flag == 0)
    {
        //strncpy(&s_DispBuf[2][0],"                ",17);
    	LCD_DisplayString(0,16*2,"                ",NOT_REVERSE);
    }
    else
    {
        LCD_DisplayString(0,16*2,(char*)yesno[lang],NOT_REVERSE);
    }
	switch(msg)
	{
	case KEY_DOWN_UP:
		if(flag == 1)
		{
			return SET_ING;
		}
		if(ee_val == 0)
		{
			ee_val = sizeArray - 1;	return SET_ING;
		}
		else
		{
			ee_val--; return SET_ING;
		}
	case KEY_DOWN_DOWN:
		if(flag == 1)
		{
			return SET_ING;
		}
		if(ee_val == (sizeArray - 1) )
		{
			ee_val = 0; return SET_ING;
		}
		else if(ee_val < sizeArray - 1)
		{
			ee_val++; return SET_ING;
		}
	case KEY_DOWN_MENU:
		if(flag == 0)
        {
            flag = 1;
            return SET_ING;
        }
        if(flag == 1)
        {            
            flag1st = 0;
            *pRetVal = ee_val;
            return SET_OK;
        }
	case KEY_DOWN_OK:
		if(flag == 0)
        {
			if(ee_val == (sizeArray - 1) )
			{
				ee_val = 0; return SET_ING;
			}
			else if(ee_val < sizeArray - 1)
			{
				ee_val++; return SET_ING;
			}
        }
        flag1st = 0;
		return SET_EXIT;
	default: return SET_ING ;
	}
}

SET_STAT set_int_float_min_max(uint8_t msg,SetIntFloatDef setIntFloat)
{
	uint8_t lang = get_language();
    static uint8_t state,flag1st,flagDotDisp;	/* in exit proc,it should be reset */
	uint8_t ascii;
    uint8_t data_type = setIntFloat.dataType;
	double retVal;
    int32_t ee_val_32;
    const char* yesno[] = {"<-Yes       No->","<-是        否->","<-Yes       No->"};
    
	if(flag1st == 0)
	{
		flag1st = 1;
        state = 0;
        s_cur = 0;
        ee_val_32 = setIntFloat.setData;
		switch(data_type)
		{   
		case INT_8_T:  
            snprintf(s_set_buf,4,"%03d",ee_val_32); flagDotDisp = 0;
            snprintf(&s_DispBuf[1][0],17,"%16d",ee_val_32); break;
		case INT_16_T: 
            snprintf(s_set_buf,6,"%05d",ee_val_32); flagDotDisp = 0; 
            snprintf(&s_DispBuf[1][0],17,"%16d",ee_val_32); break;
		case INT_32_T: snprintf(s_set_buf,7,"%06ld",ee_val_32);  flagDotDisp = 0;
            snprintf(&s_DispBuf[1][0],17,"%16ld",ee_val_32); break;
        case FLOAT_32_T: snprintf(s_set_buf,17,"%08.1f",setIntFloat.setData);flagDotDisp = 1; 
            snprintf(&s_DispBuf[1][0],17,"%16.1f",ee_val_32); break;
		default: return SET_EXIT;
		}       
	}
    disp_int_float(s_set_buf,1);    
    if(state == 0)
    {
        strncpy(&s_DispBuf[2][0],"                ",17); 
    }
    if(state == 1)
    {
    	//strncpy(&s_DispBuf[2][0],yesno[lang],17);
        strcpy(&s_DispBuf[2][0],yesno[lang]);
    }
    LCD_DisplayString(0,16*1,&s_DispBuf[1][0],NOT_REVERSE);
    LCD_DisplayString(0,16*2,&s_DispBuf[2][0],NOT_REVERSE);
    
	switch(msg)
	{
	case KEY_DOWN_UP:
		if(state != 0)
			return SET_ING;
		else
		{
			ascii = ++s_set_buf[s_cur];
			if(s_cur == 0)
			{
                switch(ascii)
				{
				case '9' + 1:
                    if(flagDotDisp) 
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '-';
					return SET_ING;
				case '.' + 1: s_set_buf[s_cur] = '-'; return SET_ING;
				case '-' + 1: s_set_buf[s_cur] = '0'; return SET_ING;
				default: return SET_ING;
				}
			}
			else
			{
                switch(ascii)
				{
				case '9' + 1:
                    if(flagDotDisp)
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '0';
					return SET_ING;
				case '.' + 1: s_set_buf[s_cur] = '0'; return SET_ING;
				default: return SET_ING;
				}
			}
		}
	case KEY_DOWN_DOWN:
		if(state != 0)
            return SET_ING;
		else
		{
			ascii = --s_set_buf[s_cur];
			if(s_cur == 0)
			{
                switch(ascii)
				{
				case '0' - 1: s_set_buf[s_cur] = '-'; return SET_ING;
				case '-' - 1:
                    if(flagDotDisp)
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '9';
					return SET_ING;
				case '.' - 1: s_set_buf[s_cur] = '9'; return SET_ING;
				default: return SET_ING;
				}
			}
			else
			{
                switch(ascii)
				{
				case '0' - 1:
                    if(flagDotDisp)
                        s_set_buf[s_cur] = '.';
                    else
                        s_set_buf[s_cur] = '9';
                    return SET_ING;
				case '.' - 1: s_set_buf[s_cur] = '9'; return SET_ING;
				default: return SET_ING;
				}
			}
		}
	case KEY_DOWN_MENU:
        if(state == 0)
        {
            state = 1; return SET_ING;
        }
        if(state == 1)
        { 
            flag1st = 0;
            switch(data_type)
			{
			case INT_8_T: sscanf(s_set_buf,"%3lf",&retVal); break;
			case INT_16_T: sscanf(s_set_buf,"%5lf",&retVal); break;
			case INT_32_T: sscanf(s_set_buf,"%6lf",&retVal); break;	
            case FLOAT_32_T: sscanf(s_set_buf,"%8lf",&retVal); break;	
			default: sscanf(s_set_buf,"%10lf",&retVal); break;
			}
            if(retVal > setIntFloat.max || retVal < setIntFloat.min)
            {
                flag1st = 0; return SET_MAX_MIN_ERROR;
            }
            else
            {
                *(setIntFloat.pRetVal) = retVal;
                return SET_OK;
            }
        }
	case KEY_DOWN_OK:
		if(state == 0)
		{			
            switch(data_type)
			{
			case INT_8_T: if(++s_cur == 3) s_cur = 0; return SET_ING;
			case INT_16_T: if(++s_cur == 5) s_cur = 0; return SET_ING;
			case INT_32_T: if(++s_cur == 6) s_cur = 0; return SET_ING;
            case FLOAT_32_T: if(++s_cur == 10) s_cur = 0; return SET_ING;
            default: return SET_ING;
            }
		}
		else
		{
            flag1st = 0;
            return SET_EXIT;
		}
	default: return SET_ING;
	}
}

void disp_select(char *p,uint8_t reverse)
{
    char buf[25];
    int len;
    uint16_t totalLen,asciiLen,hzLen;
    
    get_utf8_len(p, &totalLen, &asciiLen);
    hzLen = (totalLen - asciiLen ) / 3;
    len = ((hzLen<<1)+asciiLen);
    
    if(reverse != REVERSE)
    {
        snprintf(buf,25,"%-.*s%s",16 - len,s_blank,p);
        LCD_DisplayString(0,48,buf,NOT_REVERSE);        
    }
    else
    {
        snprintf(buf,17,"%-.*s",16 - len,s_blank);    
        LCD_DisplayString(0,48,buf,NOT_REVERSE);
        snprintf(buf,25," %-s",p);    
        LCD_DisplayString((16-len - 1)*8,48,buf,REVERSE);        
    }
}

void disp_int_float(char *p,uint8_t reverse)
{
    char buf[17];
    int len = strlen(p);
    
    if(reverse != REVERSE)
    {
        snprintf(&s_DispBuf[ROW - 1][0],17,"%16s",p);    
        LCD_DisplayString(0,48,&s_DispBuf[ROW - 1][0],NOT_REVERSE);
    }
    else
    {
        snprintf(buf,17,"%.*s%.*s",16 - len,s_blank,s_cur,p);    
        LCD_DisplayString(0,48,buf,NOT_REVERSE);
        buf[0] = p[s_cur];buf[1] = 0;
        LCD_DisplayString( (16-len+s_cur)*8,48,buf,REVERSE);
        snprintf(buf,17,"%s",&p[s_cur + 1]);    
        LCD_DisplayString((16-len+1+s_cur)*8,48,buf,NOT_REVERSE);        
    }
}

void disp_main_screen(uint8_t x,uint8_t y,char *p,uint8_t width,uint8_t reverse_index)
{
    char buf[17];
    int len = strlen(p);
    
    snprintf(buf,width+1,"%.*s",reverse_index,p);    
    LCD_DisplayString(x,y,buf,NOT_REVERSE);
    buf[0] = p[reverse_index];buf[1] = 0;
    LCD_DisplayString( (x+reverse_index)*8,y,buf,REVERSE);
    snprintf(buf,width+1,"%s",&p[reverse_index + 1]);    
    LCD_DisplayString((x+1+reverse_index)*8,y,buf,NOT_REVERSE);  
}

/*
***********************************************
*/

static void get_utf8_len(char *pStr,uint16_t *pTotalLen,uint16_t *pAsciiLen)
{
    int i,j = 0,len;
    *pTotalLen = len = strlen(pStr);
    for(i = 0; i < len; i++)
    {
        if(pStr[i] < 0x80)
            j++;
    }
    *pAsciiLen = j;
}
