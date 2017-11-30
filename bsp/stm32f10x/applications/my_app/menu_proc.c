
#include <rtthread.h>
#include <rtdevice.h>
#include "lcd12864.h"
#include "bsp_button.h"
#include "global.h"
#include "menu_frame.h"
#include "menu_proc.h"
#include "conf_store.h"
#include "bsp_24xx02.h"
#include "mcp41xxx.h"

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
    SET_MAX_MIN_ERROR,
    SET_EXIT,
}SET_STAT;
typedef struct SetIntFloat SetIntFloatDef;


char g_userPasswordBuf[7];

struct SetIntFloat
{
    uint8_t dataType;
    double setData;
    double min;
    double max;
    double *pRetVal;
};

static void get_utf8_len(char *pStr,uint16_t *pTotalLen,uint16_t *pAsciiLen);

/* user function def */
void menu_main_disp(uint8_t msg);

void menu_language(uint8_t msg);

void menu_measure_freq(uint8_t msg);
void menu_container_d(uint8_t msg);
void menu_ch0_start_mag(uint8_t msg);
void menu_ch0_end_mag(uint8_t msg);
void menu_ch0_width_min(uint8_t msg);
void menu_ch0_width_max(uint8_t msg);
void menu_ch0_start_index(uint8_t msg);
void menu_ch0_end_index(uint8_t msg);

void menu_ch1_start_mag(uint8_t msg);
void menu_ch1_end_mag(uint8_t msg);
void menu_ch1_width_min(uint8_t msg);
void menu_ch1_width_max(uint8_t msg);
void menu_ch1_start_index(uint8_t msg);
void menu_ch1_end_index(uint8_t msg);

void menu_sensor_freq(uint8_t msg);
void menu_gain_1(uint8_t msg);
void menu_gain_2(uint8_t msg);

void menu_low_curve(uint8_t msg);
void menu_high_curve(uint8_t msg);

void menu_rs485_protocal(uint8_t msg);
void menu_rs485_baud(uint8_t msg);
void menu_rs485_databit(uint8_t msg);
void menu_rs485_check(uint8_t msg);
void menu_rs485_id(uint8_t msg);

void menu_reset_factory(uint8_t msg);


uint8_t get_password(uint8_t msg,uint8_t* outBuf);
uint8_t password(uint8_t msg,uint8_t* passLevel);
void pass_proc(uint8_t msg);

const static MenuTreeDef s_Menu[MENU_MAX] =
{
	/* note:the class and index must be continous */
// a(0) b(0.0) d(0.1) c(0.2) e(0.0.0) f(0.0.1) g(0.1.0) h(0.2.0) i(0.2.1)
	{"0",{"root","root"}, menu_main_disp},
	
	{"0.0",{"Basic Setup " , "基本设置" },menu_default},
    {"0.1",{"Advanced Setup " , "高级设置"}, menu_default},
    {"0.2",{"Show Curve " , "显示曲线"}, menu_default},
    {"0.3",{"Language " , "语言设置 "}, menu_language},
    
    {"0.0.0",{"Measure Freq " , "测量频率"}, menu_measure_freq},
    //{"0.0.1",{"Container-D " , "容器直径" },menu_container_d},
	{"0.0.1",{"Low Level Setup " , "低位设置"}, menu_default},
    {"0.0.2",{"High Level Setup " , "高位设置" },menu_default},
	
    {"0.0.1.0",{"Mag Setup-1 " , "幅值设置-1"}, menu_ch0_start_mag},			// start_mag
	{"0.0.1.1",{"Mag Setup-2 " , "幅值设置-2"}, menu_ch0_end_mag},			// end_mag
	{"0.0.1.2",{"Width Setup-1 " , "宽度设置-1"}, menu_ch0_width_min},		// width_min
	{"0.0.1.3",{"Width Setup-2 " , "宽度设置-2"}, menu_ch0_width_max},		// width_max
	{"0.0.1.4",{"Time Setup -1 " , "时间设置-1"}, menu_ch0_start_index},		// start_index
	{"0.0.1.5",{"Time Setup -2 " , "时间设置-2"}, menu_ch0_end_index},		// end_index
    
	{"0.0.2.0",{"Mag Setup-1 " , "幅值设置-1"}, menu_ch1_start_mag},			// start_mag
	{"0.0.2.1",{"Mag Setup-2 " , "幅值设置-2"}, menu_ch1_end_mag},			// end_mag
	{"0.0.2.2",{"Width Setup-1 " , "宽度设置-1"}, menu_ch1_width_min},		// width_min
	{"0.0.2.3",{"Width Setup-2 " , "宽度设置-2"}, menu_ch1_width_max},		// width_max
	{"0.0.2.4",{"Time Setup -1 " , "时间设置-1"}, menu_ch1_start_index},		// start_index
	{"0.0.2.5",{"Time Setup -2 " , "时间设置-2"}, menu_ch1_end_index},		// end_index
	
    {"0.1.0",{"Sensor Freq " , "探头频率"}, menu_sensor_freq},
	{"0.1.1",{"Gain Setup -1 " , "增益设置-1"}, menu_gain_1},
	{"0.1.2",{"Gain Setup -1" , "增益设置-2"}, menu_gain_2},
	
	{"0.2.0",{"Low Level Curve " , "低位曲线"}, menu_default/*menu_low_curve*/},
	{"0.2.1",{"High Level Curve" , "高位曲线"}, menu_default/*menu_high_curve*/},
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

char* s_blank = "                ";

void menu_init(void)
{
    global.lang = 1;
	menu_var_init((pMenuTreeDef)s_Menu);
}

/*
***********************************************
*/

void menu_main_disp(uint8_t msg)
{
    static uint8_t buf[17];
    
    snprintf(buf,17,"%5d | %3d | %2d  ",global.signals[0].signal[0].index,global.signals[0].signal[0].mag,global.signals[0].signal[0].width);
    LCD_DisplayString(0,0,buf,NOT_REVERSE);
    snprintf(buf,17,"%5d | %3d | %2d  ",global.signals[1].signal[1].index,global.signals[1].signal[1].mag,global.signals[1].signal[1].width);
    LCD_DisplayString(0,48,buf,NOT_REVERSE);
    
    if(global.ch1_status == 0)
    {
		LCD_DisplayString(0,16,"   低位:  无液    ",NOT_REVERSE);
    }
    else
    {
		LCD_DisplayString(0,16,"   低位:  有液    ",NOT_REVERSE);
    }
    if(global.ch2_status == 0)
    {
		LCD_DisplayString(0,32,"   高位:  无液    ",NOT_REVERSE);
    }
    else
    {
        LCD_DisplayString(0,32,"   高位:  有液    ",NOT_REVERSE);
    }
    
    switch(msg)
    {
        case KEY_DOWN_MENU:
        break;
        case KEY_DOWN_DOWN:
        break;
        case KEY_DOWN_UP:
        break;
        case KEY_DOWN_OK:
            menu_default(NULL);
        break;
        case KEY_COMB_DOWN_PRIM_MENU:            
        break;
        case KEY_COMB_MENU_PRIM_DOWN:
        break;
        default:
            break;
    }
}

/*
***********************************************
*/

void menu_language(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT  setStat;
    uint8_t retVal;
    const char* const langSel[] = { "Selcet Language " , "语言设置        "};
    const char* const langStr[LANG_MAX][LANG_MAX]=
    {
        {"English","英语"},
        {"Chinese","简体中文"},
    };
    LCD_DisplayString(0,0,(char*)langSel[lang],NOT_REVERSE);
    setStat = set_select(msg,langStr,sizeof(langStr[0]),LANG_MAX,global.lang,&retVal);
    if(setStat == SET_OK  )
    {
        if(retVal == global.lang)
        {
            leaf_exit(NULL);
            return;
        }
        global.lang = retVal;
        ee_WriteBytes(&global.lang,E2P_OFFSET(language),1);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

/*
***********************************************
*/

void menu_measure_freq(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { "Measure Freq(ms) " , "测量频率(ms)      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.measure_freq;
    setIntFloat.max = 10000;
    setIntFloat.min = 50;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.measure_freq = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(measure_freq), (uint8_t*)&global.measure_freq, sizeof(global.measure_freq));
		ee_WriteBytes( (uint8_t*)&global.measure_freq,E2P_OFFSET(measure_freq),  sizeof(global.measure_freq));
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

/*
***********************************************
*/

void menu_container_d(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { "Container D(mm)" , "容器直径(mm)     "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.container_d;
    setIntFloat.max = 30000;
    setIntFloat.min = 100.0f;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.container_d = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(container_d), (uint8_t*)&global.container_d, sizeof(global.container_d));
        ee_WriteBytes((uint8_t*)&global.container_d,E2P_OFFSET(container_d),sizeof(global.container_d));
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}


void menu_ch0_start_mag(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Mag Setup -1 " , "幅值设置-1      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[0].start_mag;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[0].start_mag = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[0].start_mag), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes( (uint8_t*)&setData, E2P_OFFSET(signal_cond[0].start_mag), sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch0_end_mag(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Mag Setup -2 " , "幅值设置-2      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[0].end_mag;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[0].end_mag = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].end_mag), (uint8_t*)&global.signal_cond[0].end_mag, sizeof(setData));
        ee_WriteBytes((uint8_t*)&global.signal_cond[0].end_mag, E2P_OFFSET(signal_cond[0].end_mag), sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch0_width_min(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Width -1 " , "宽度设置-1      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[0].width_min;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[0].width_min = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[0].width_min), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes( (uint8_t*)&setData,E2P_OFFSET(signal_cond[0].width_min), sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch0_width_max(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Width -2     " , "宽度设置-2      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[0].width_max;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[0].width_max = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[0].width_max), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes( (uint8_t*)&setData,E2P_OFFSET(signal_cond[0].width_max),  sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch0_start_index(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Time -1     " , "时间设置-1      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[0].start_index;
    setIntFloat.max = 40000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[0].start_index = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[0].start_index), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes( (uint8_t*)&setData,E2P_OFFSET(signal_cond[0].start_index), sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch0_end_index(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Time -2     " , "时间设置-2      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[0].end_index;
    setIntFloat.max = 40000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[0].end_index = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[0].end_index), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes( (uint8_t*)&setData,E2P_OFFSET(signal_cond[0].end_index),  sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
/*
*********************************
*/
void menu_ch1_start_mag(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Mag Setup -1 " , "幅值设置-1      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[1].start_mag;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[1].start_mag = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].start_mag), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes((uint8_t*)&setData, E2P_OFFSET(signal_cond[1].start_mag),  sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch1_end_mag(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Mag Setup -2 " , "幅值设置-2      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[1].end_mag;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[1].end_mag = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].end_mag), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes(  (uint8_t*)&setData,E2P_OFFSET(signal_cond[1].end_mag), sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch1_width_min(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Width -1 " , "宽度设置-1      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[1].width_min;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[1].width_min = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].width_min), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes( (uint8_t*)&setData, E2P_OFFSET(signal_cond[1].width_min), sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch1_width_max(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Width -2     " , "宽度设置-2      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[1].width_max;
    setIntFloat.max = 20000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[1].width_max = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].width_max), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes((uint8_t*)&setData, E2P_OFFSET(signal_cond[1].width_max),  sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch1_start_index(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Time -1     " , "时间设置-1      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[1].start_index;
    setIntFloat.max = 40000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[1].start_index = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].start_index), (uint8_t*)&setData, sizeof(setData));
        ee_WriteBytes((uint8_t*)&setData, E2P_OFFSET(signal_cond[1].start_index),  sizeof(setData));
		leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_ch1_end_index(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint16_t setData;
    double retVal;
    const char* const titleLang[] = { "Time -2     " , "时间设置-2      "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.signal_cond[1].end_index;
    setIntFloat.max = 40000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.signal_cond[1].end_index = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].end_index), (uint8_t*)&setData, sizeof(setData));
		ee_WriteBytes((uint8_t*)&setData, E2P_OFFSET(signal_cond[1].end_index),  sizeof(setData));
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_sensor_freq(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { "Sensor Freq (Hz)" , "探头频率(Hz)     "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.sensor_freq;
    setIntFloat.max = 500000.0f;
    setIntFloat.min = 100000.0f;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.sensor_freq = setData;
        //rt_device_write(mb85_bus, E2P_OFFSET(sensor_freq), (uint8_t*)&global.sensor_freq, sizeof(global.sensor_freq));
		ee_WriteBytes( (uint8_t*)&global.sensor_freq, E2P_OFFSET(sensor_freq), sizeof(global.sensor_freq));
		
		void pulse_send_init(float freq,float us);
		pulse_send_init(global.sensor_freq,42 );
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_gain_1(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t setData;
    double retVal;
    const char* const titleLang[] = { "Gain Setup -1     " , "增益设置-1      "};
    
    setIntFloat.dataType = INT_8_T;
    setIntFloat.setData = global.gain_1;
    setIntFloat.max = 255;
    setIntFloat.min = 1;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.gain_1 = setData;
		mcp41xxx_set_pot(1,0,256 - global.gain_1);
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].end_index), (uint8_t*)&setData, sizeof(setData));
		ee_WriteBytes((uint8_t*)&setData, E2P_OFFSET(gain_1),  sizeof(setData));
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_gain_2(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t setData;
    double retVal;
    const char* const titleLang[] = { "Gain Setup -2     " , "增益设置-2      "};
    
    setIntFloat.dataType = INT_8_T;
    setIntFloat.setData = global.gain_2;
    setIntFloat.max = 255;
    setIntFloat.min = 1;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        setData = retVal;
        global.gain_2 = setData;
		mcp41xxx_set_pot(2,0,256 - global.gain_2);
        //rt_device_write(mb85_bus, E2P_OFFSET(signal_cond[1].end_index), (uint8_t*)&setData, sizeof(setData));
		ee_WriteBytes((uint8_t*)&setData, E2P_OFFSET(gain_2),  sizeof(setData));
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_low_curve(uint8_t msg)
{
	static uint8_t flag;
	
	if(flag == 0)
	{
		flag = 1;
		
	}
	if(msg == KEY_DOWN_MENU)
    {
		flag = 0;
        leaf_exit(NULL);
    }
}

void menu_high_curve(uint8_t msg)
{
}
	
/*
void modbus_conf(void);
void menu_rs485_protocal(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* const titleLang[] = { "Protocal        " , "仪表通讯格式        ","프로토콜        "};
    const char* const commFormatStr[][LANG_MAX]=
    {
        {"Modbus-RTU","Modbus-RTU","Modbus-RTU"},
        {"Modbus-ASCII","Modbus-ASCII","Modbus-ASCII"},
        {"Pris","Pris","Pris"},
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    if( global.commOnlyPris == 1)
    {
        LCD_DisplayString(0,16,"            Pris",NOT_REVERSE);
        LCD_DisplayString(0,32,"                ",NOT_REVERSE);
        LCD_DisplayString(0,48,"            Pris",NOT_REVERSE);
        if(msg == KEY_UP_MENU)
            leaf_exit(NULL);
        return;
    }
    setStat = set_select( msg, commFormatStr,sizeof(commFormatStr[0]),3, global.commFormat, &retVal);
   
    if( setStat == SET_OK  )
    {
        set_e2prom(&retVal,E2P_OFFSET(commFormat),INT_8_T);
        global.commFormat = retVal;
        if(global.commFormat < 2)
        {
            modbus_conf();
        }
        else
        {
            eMBClose( );
            Pris_PortInit();
        }
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
    const char* const titleLang[] = { "Baudrate        " , "仪表通讯速度          ","Baud범위        "};
    const char* const commBaudStr[][LANG_MAX]=
    {
        {"1200","1200","1200"},
        {"2400","2400","2400"},
        {"4800","4800","4800"},
        {"9600","9600","9600"},
        {"19200","19200","19200"},
        {"38400","38400","38400"},
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_select( msg, commBaudStr,sizeof(commBaudStr[0]),countof(commBaudStr), global.commBaud, &retVal);
    if( setStat == SET_OK  )
    {
        set_e2prom(&retVal,E2P_OFFSET(commBaud),INT_8_T);
        global.commBaud = retVal;
        if(global.commFormat < 2)
        {
            modbus_conf();
        }
        else
        {
            eMBClose( );
            Pris_PortInit();
        }
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
    const char* const titleLang[] = { "Data Bit        " , "通讯数据位        ","데이터비트      "};
    get_e2prom(&index,E2P_OFFSET(commFormat),INT_8_T);
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    LCD_DisplayString(0,16,"                ",NOT_REVERSE);
    LCD_DisplayString(0,32,"                ",NOT_REVERSE);
    if(index == 0 || index == 2)
    {
        LCD_DisplayString(0,48,"               8",NOT_REVERSE);
    }
    else if(index == 1)
    {
        LCD_DisplayString(0,48,"               7",NOT_REVERSE);
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
    const char* const titleLang[] = { "Check Mode      " , "通讯校验方式        ","체크모드        "};
    const char* const commCheckStr[][LANG_MAX]=
    {
        {"None","无校验","None"},
        {"Odd","奇校验","Odd"},
        {"Even","偶校验","Even"},
        
    };
    static char** ppStr;
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);

    if(global.commFormat == 2)
    {
        LCD_DisplayString(0,32,"                ",NOT_REVERSE);
        if(lang == 1)
        {
            LCD_DisplayString(0,16,"          无校验",NOT_REVERSE);
            LCD_DisplayString(0,48,"          无校验",NOT_REVERSE);
        }
        else
        {
            LCD_DisplayString(0,16,"            None",NOT_REVERSE);
            LCD_DisplayString(0,48,"            None",NOT_REVERSE);
        }        
        if(msg == KEY_DOWN_MENU)
            leaf_exit(NULL);
    }
    else
    {
        if(flag1st == 0)
        {
            flag1st = 1;
            if(global.commFormat == 0)
            {
                ppStr = (char**)commCheckStr;
                currIndex = global.commCheck ;
            }
            else if(global.commFormat == 1)
            {
                ppStr = (char**)&commCheckStr[1][0];
                if(global.commCheck == 0)
                {
                    global.commCheck = 1;
                    set_e2prom(&global.commCheck,E2P_OFFSET(commCheck),INT_8_T);
                    currIndex = 0;
                }
                else
                {
                    currIndex = global.commCheck - 1;
                }
            }
        }
        if(global.commFormat == 0)
        {
            setStat = set_select( msg, ppStr,sizeof(commCheckStr[0]),3, currIndex, &retVal);
        }
        else if(global.commFormat == 1)
        {
            setStat = set_select( msg, ppStr,sizeof(commCheckStr[0]),2, currIndex, &retVal);
        }
        if( setStat == SET_OK  )
        {
            flag1st = 0;
            if(global.commFormat == 0)
            {
                set_e2prom(&retVal,E2P_OFFSET(commCheck),INT_8_T);
                global.commCheck = retVal;
            }
            else if(global.commFormat == 1)
            {
                retVal = retVal + 1;
                global.commCheck = retVal;
                set_e2prom(&retVal,E2P_OFFSET(commCheck),INT_8_T);
            }
            modbus_conf();
            leaf_exit(NULL);
        }
        else if(setStat == SET_EXIT)
        {
            flag1st = 0;
            leaf_exit(NULL);
        }
    }
}
//void func_rs485_stopbit(uint8_t msg)

void menu_rs485_id(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t setData;
    double retVal;
    const char* const titleLang[] = { "Device ID       " , "仪表设备地址        ","장비번호        "};
          
    setIntFloat.dataType = INT_8_T;
    setIntFloat.setData = global.commDeviceID;
    setIntFloat.max = 247.0f;
    setIntFloat.min = 1.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        setData = retVal;
        set_e2prom(&setData,E2P_OFFSET(commDeviceID),INT_8_T);
        global.commDeviceID = retVal;
        if(global.commFormat < 2)
        {
            modbus_conf();
        }
        else
        {
            eMBClose( );
            Pris_PortInit();
        }
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
*/


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

/*
***********************************************
*/

uint8_t get_password(uint8_t msg,uint8_t* outBuf)
{
    uint8_t retVal,passVal;
	uint32_t i;
    
    static uint8_t flag;	/* in exit proc,it should be reset */
	uint8_t ascii;
    char buf[17];
    static uint8_t keyDown;
    static uint8_t cnt;
    uint8_t lang = get_language();
    const char* const titleLang[LANG_MAX] = {" Input Password " , "   请输入密码   "};
    const char* stars = "******";

    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    LCD_DisplayString(0,16,"                ",NOT_REVERSE);
    LCD_DisplayString(0,32,"                ",NOT_REVERSE);

	if(flag == 0)
	{
        flag = 1;
		s_cur = 0;
        //snprintf(s_set_buf,7,"000000");
		strncpy(s_set_buf,"000000",7);
	}
	//disp_pass(s_set_buf);
    
    snprintf(buf,17,"          %*s",s_cur,stars);    
    LCD_DisplayString(0,48,buf,NOT_REVERSE);
    if(keyDown == 1 && cnt < 10)
    {
        cnt++;
        buf[0] = s_set_buf[s_cur];buf[1] = 0;
    }
    else
    {
        keyDown = 0; cnt = 0;
        buf[0] = '*';buf[1] = 0;
    }
    LCD_DisplayString( (10+s_cur)*8,48,buf,REVERSE);
    snprintf(buf,17,"%s",&stars[s_cur + 1]);    
    LCD_DisplayString((11+s_cur)*8,48,buf,NOT_REVERSE);
    
	switch(msg)
	{
	case KEY_DOWN_UP:		
        keyDown = 1;
        cnt = 0;
        ascii = ++s_set_buf[s_cur];
        if(ascii == '9' + 1)
        {
            s_set_buf[s_cur] = '0';
        }
        retVal = 0;
        break;
	case KEY_DOWN_DOWN:	
        keyDown = 1;
        cnt = 0;        
        ascii = --s_set_buf[s_cur];
        if(ascii == '0' - 1)
        {
            s_set_buf[s_cur] = '9';
        }
        retVal = 0;
        break;
	case KEY_DOWN_MENU:
        flag = 0;
        keyDown = 0;
        cnt = 0;
        retVal = 1;
		strncpy(outBuf,s_set_buf,6);		
        break;
	case KEY_DOWN_OK:			
        keyDown = 1;
        cnt = 0;
        if(++s_cur == 6)
        {
            s_cur = 0;
        }
        retVal = 0;
        break;
	default:
        retVal = 0;
		break;
	}
    return retVal;
}

void pass_proc(uint8_t msg)
{
    uint8_t passWord[6];
    uint8_t pasL;
	uint32_t i,tmp_32;
    char buf[17];
	
    if(1 == get_password(msg,passWord))
    {
		if( 0 == strncmp(passWord,"880000",6) )
		{
            //NVIC_SystemReset();
		}         
		else if(0 == strncmp(passWord,"889500",6) )
		{
			//NVIC_SystemReset();
		}
        else if(0 == strncmp(passWord,"975328",6))
        {
            jump2func(NULL);
            jump2submenu("1");
        }
        else
        {
            leaf_exit(NULL);
        }
    }
}

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
