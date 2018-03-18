
#include <rtthread.h>
#include <rtdevice.h>
#include "lcd12864.h"
#include "bsp_button.h"
#include "global.h"
#include "menu_frame.h"
#include "menu_proc.h"
#include "conf_store.h"
#include "12864_ui.h"
#include "mb.h"
#include "mb_m.h"
//#include "bsp_24xx02.h"

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

#define SWAP_32(A)  ((((uint32_t)(A) & 0xffff0000) >> 16) | \
                                                 (((uint32_t)(A) & 0x0000ffff) << 16))

float swap_f32(float dat)
{
    uint32_t dat_32 = *(uint32_t*)&dat;
    dat_32 = SWAP_32(dat_32);
    return *(float*) &dat_32;
}

extern const char c_formula[];

void disp_main_screen(uint8_t x,uint8_t y,char *p,uint8_t width,uint8_t reverse_index);

/* user function def */
void menu_main_disp(uint8_t msg);

void menu_rs485_protocal(uint8_t msg);
void menu_rs485_baud(uint8_t msg);
void menu_rs485_databit(uint8_t msg);
void menu_rs485_check(uint8_t msg);
void menu_rs485_id(uint8_t msg);
void menu_slave_rs485_id(uint8_t msg);

void menu_unit(uint8_t msg);

void menu_range_sel(uint8_t msg);
void menu_range_1(uint8_t msg);
void menu_range_2(uint8_t msg);
void menu_range_3(uint8_t msg);
void menu_range_4(uint8_t msg);

void menu_linear_cali_point(uint8_t msg);
void menu_linear_cali(uint8_t msg);
void menu_linear_coeff_setup(uint8_t msg);

void menu_4mA_1_cali(uint8_t msg);    
void menu_20mA_1_cali(uint8_t msg);
void menu_curr_out_1_uplimit(uint8_t msg);    
void menu_curr_out_1_downlimit(uint8_t msg);

void menu_4mA_2_cali(uint8_t msg);    
void menu_20mA_2_cali(uint8_t msg);
void menu_curr_out_2_uplimit(uint8_t msg);    
void menu_curr_out_2_downlimit(uint8_t msg);

void menu_relay_1_alarm_sel(uint8_t msg);
void menu_relay_1_alarm_value(uint8_t msg);
void menu_relay_1_alarm_delay(uint8_t msg);

void menu_relay_2_alarm_sel(uint8_t msg);
void menu_relay_2_alarm_value(uint8_t msg);
void menu_relay_2_alarm_delay(uint8_t msg);

void menu_temp_compensate(uint8_t msg);
void menu_reset_factory(uint8_t msg);
void menu_serial_no_setup(uint8_t msg);
void menu_password_setup(uint8_t msg);

void menu_disp_canbi_org_signal(uint8_t msg);
void menu_canbi_ratio_setup(uint8_t msg);
void menu_disp_canbi_calc(uint8_t msg);
void menu_disp_canbi_object(uint8_t msg);

void menu_zero_cali(uint8_t msg);
void menu_full_cali(uint8_t msg);
void menu_zero_compensate(uint8_t msg);

void menu_display_zhuodu_xiguangdu(uint8_t msg);
void menu_display_formula(uint8_t msg);
void menu_display_ch123(uint8_t msg);

void menu_4mA_zhuodu_cali(uint8_t msg);
void menu_20mA_zhuodu_cali(uint8_t msg);

const static MenuTreeDef s_Menu[MENU_MAX] =
{
	{"0",{"root","root"}, menu_main_disp},
	
    {"0.0",{"R" , "校准" },NULL},
    {"0.1",{"R" , "报警" },NULL},
    {"0.2",{"R" , "清洗" },NULL},
    {"0.3",{"R" , "电流" },NULL},
    {"0.4",{"R" , "系统" },NULL},
    {"0.5",{"R" , "通讯" },NULL},
    {"0.6",{"R" , "历史记录" },NULL},
    {"0.7",{"R" , "测试维护" },NULL},
    {"0.8",{"R" , "恢复出厂设置" },menu_reset_factory},
    
    {"0.0.0",{"R" , "单位设置" },menu_unit},
    {"0.0.1",{"R" , "量程设置" },menu_range_sel},
    {"0.0.2",{"R" , "零位校准" },menu_zero_cali},
    {"0.0.3",{"R" , "已知浓度校准" },menu_full_cali},
    {"0.0.4",{"R" , "零位补偿" },menu_zero_compensate},
    {"0.0.5",{"R" , "数据查询" },NULL},
    {"0.0.6",{"R" , "温度补偿" },menu_temp_compensate},
    
    //浊度信号 计算公式 通道1 通道2 通道3 
    {"0.0.5.0",{"R" , "浊度信号" },menu_display_zhuodu_xiguangdu},
    {"0.0.5.1",{"R" , "计算公式" },menu_display_formula},
    {"0.0.5.2",{"R" , "通道1-2-3" },menu_display_ch123},
    
    {"0.1.0",{"R" , "继电器1报警类型" },menu_relay_1_alarm_sel},
    {"0.1.1",{"R" , "继电器1报警值" },menu_relay_1_alarm_value},
    {"0.1.2",{"R" , "继电器1迟滞量" },menu_relay_1_alarm_delay},
    {"0.1.3",{"R" , "继电器2报警类型" },menu_relay_2_alarm_sel},
    {"0.1.4",{"R" , "继电器2报警值" },menu_relay_2_alarm_value},
    {"0.1.5",{"R" , "继电器2迟滞量" },menu_relay_2_alarm_delay},
    
    {"0.2.0",{"R" , "清洗时间间隔" },NULL},
    {"0.2.1",{"R" , "每次清洗时间设置" },NULL},

    {"0.3.0",{"R" , "电流-1输出上限" },menu_curr_out_1_uplimit},
    {"0.3.1",{"R" , "电流-1输出下限" },menu_curr_out_1_downlimit},
    {"0.3.2",{"R" , "电流-1-4mA校准" },menu_4mA_1_cali},
    {"0.3.3",{"R" , "电流-1-20mA校准" },menu_20mA_1_cali},
    {"0.3.4",{"R" , "电流-2输出上限" },menu_curr_out_2_uplimit},
    {"0.3.5",{"R" , "电流-2输出下限" },menu_curr_out_2_downlimit},
    {"0.3.6",{"R" , "电流-2-4mA校准" },menu_4mA_2_cali},
    {"0.3.7",{"R" , "电流-2-20mA校准" },menu_20mA_2_cali},
    
    {"0.4.0",{"R" , "采样周期" },NULL},
    {"0.4.1",{"R" , "背光选择" },NULL},
    {"0.4.2",{"R" , "密码设置" },menu_password_setup},
    {"0.4.3",{"R" , "产品序列号" },menu_serial_no_setup},
    
    
    {"0.5.0",{"Modbus Addr" , "本机地址设置" },menu_rs485_id},
    {"0.5.1",{"Modbus baud" , "波特率设置" },menu_rs485_baud},
    {"0.5.2",{"Modbus baud" , "数据位" },menu_rs485_databit},
    {"0.5.3",{"Modbus baud" , "校验位" },menu_rs485_check},    
    {"0.5.4",{"Slave Addr" , "浊度地址仪设置" },menu_slave_rs485_id},
    
    {"0.6.0",{"R" , "记录间隔" },NULL},
    {"0.6.1",{"R" , "数据查询" },NULL},
    {"0.6.2",{"R" , "上传数据" },NULL},
    
    {"0.7.0",{"R" , "电流测试" },NULL},
    {"0.7.1",{"R" , "继电器测试" },NULL},
    {"0.7.2",{"R" , "售后服务" },NULL},
    
    {"1.0",{"ss","量程选择"},menu_range_sel},
    {"1.1",{"ss","标定点数"},NULL},
    {"1.2",{"ss","电流校准"},NULL},    
    {"1.3",{"ss","信号设置"},NULL},
    {"1.4",{"ss","单位设置"},menu_unit},
    
    {"1.1.0",{" " , "校准点数设置" },menu_linear_cali_point},
    {"1.1.1",{" " , "浊度校准" },menu_linear_cali},
    
    {"1.2.0",{"R" , "浊度仪4mA校准" },menu_4mA_zhuodu_cali},
    {"1.2.1",{"R" , "浊度仪20mA校准" },menu_20mA_zhuodu_cali},
    
    {"1.3.0",{"ss","参比目标信号"},menu_disp_canbi_object},
    {"1.3.1",{"ss","参比波动范围"},NULL},
    //参比原始信号  直射信号 折射信号
    {"1.3.2",{"R" , "通道1-2-3" },menu_display_ch123},
    {"1.3.5",{"ss","参比计算信号"},menu_disp_canbi_calc},
    {"1.3.6",{"ss","参比系数"},menu_canbi_ratio_setup},
    {"1.3.7",{"ss","吸光度 & 浊度"},menu_display_zhuodu_xiguangdu},
    {"1.3.8",{"ss","2公式参比"},NULL},
    {"1.3.9",{"ss","3公式参比"},NULL},
    {"1.3.10",{"ss","计算确认"},NULL},
    
 
    
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
    
void menu_main_disp(uint8_t msg)
{
    /*
    static uint8_t screen_index = 2;
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
    */

    menu_display_zhuodu_xiguangdu(KEY_NONE);
    if(msg == KEY_DOWN_OK)
    {
        menu_default();
    }
    else if(msg == KEY_UP_MENU_AND_OK)
    {
//        GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();GetKey();
//        msg = KEY_NONE;
        jump2menu("1");
    }
}

void menu_display_zhuodu_xiguangdu (uint8_t msg)
{
    const char* const unit[] = {"NTU","mg/L"};
    
    snprintf(&s_tmp_buf[0][0],9,"%5f%s",global.zhuodu_data.turbidimeter,unit[global.unit]);
    snprintf(&s_tmp_buf[1][0],9,"%5f%s",global.zhuodu_data.absorbance,s_blank);
    
    LCD_DisplayString(0,0," 浊度 & 吸光度  ",NOT_REVERSE);
    LCD_DisplayString(0,16,"浊度  : ",NOT_REVERSE);
    LCD_DisplayString(0,32,"吸光度: ",NOT_REVERSE);
    
    LCD_DisplayString(64,16,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(64,32,&s_tmp_buf[1][0],NOT_REVERSE);
    LCD_DisplayString(0,48,s_blank,NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
}

void menu_display_formula(uint8_t msg)
{
    //draw a picture
    lcd_fill_screen((uint8_t*)c_formula);
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
    return;
}

void menu_display_ch123(uint8_t msg)
{
    snprintf(&s_tmp_buf[0][0],10,"%5f%s  ",global.zhuodu_data.canbi_calc_get,s_blank);
    snprintf(&s_tmp_buf[1][0],10,"%5d%s  ",global.zhuodu_data.bright_2,s_blank);
    snprintf(&s_tmp_buf[2][0],10,"%5d%s  ",global.zhuodu_data.bright_3,s_blank);
    
    LCD_DisplayString(0,0,"通道1: ",NOT_REVERSE);
    LCD_DisplayString(0,16,"通道2: ",NOT_REVERSE);
    LCD_DisplayString(0,32,"通道3: ",NOT_REVERSE);
    
    LCD_DisplayString(56,0,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(56,16,&s_tmp_buf[1][0],NOT_REVERSE);
    LCD_DisplayString(56,32,&s_tmp_buf[2][0],NOT_REVERSE);
    
    LCD_DisplayString(0,48,s_blank,NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
}

void menu_disp_canbi_org_signal(uint8_t msg)
{
    snprintf(&s_tmp_buf[0][0],25,"     CH1参比    ");
    snprintf(&s_tmp_buf[3][0],25,"     %5d%s  ",global.zhuodu_data.canbi_get,s_blank);
    
    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(0,16,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,32,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
}

void menu_canbi_ratio_setup(uint8_t msg)
{
    snprintf(&s_tmp_buf[0][0],25,"    参比系数    ");
    snprintf(&s_tmp_buf[3][0],25,"     %4f%s  ",global.zhuodu_data.a1k1,s_blank);
    
    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(0,16,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,32,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
}

void menu_disp_canbi_calc(uint8_t msg)
{
    snprintf(&s_tmp_buf[0][0],25,"    参比计算    ");
    snprintf(&s_tmp_buf[3][0],25,"     %4f%s  ",global.zhuodu_data.canbi_calc_get,s_blank);
    
    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(0,16,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,32,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
}

void menu_disp_canbi_object(uint8_t msg)
{   
    snprintf(&s_tmp_buf[0][0],25,"    参比目标    ");
    snprintf(&s_tmp_buf[3][0],25,"     %5d%s  ",global.zhuodu_data.canbi_set,s_blank);
    
    LCD_DisplayString(0,0,&s_tmp_buf[0][0],NOT_REVERSE);
    LCD_DisplayString(0,16,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,32,s_blank,NOT_REVERSE);
    LCD_DisplayString(0,48,&s_tmp_buf[3][0],NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
    {
        leaf_exit(NULL);
    }
}


void menu_4mA_zhuodu_cali(uint8_t msg){}
void menu_20mA_zhuodu_cali(uint8_t msg){}

void menu_unit(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* title = "单位设置        ";
    const char* const select_arr[]=
    {
        "NTU","mg/L",
    };
    LCD_DisplayString(0,0,(char*)title,NOT_REVERSE);
    
    setStat = set_select( msg,(void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.unit, &retVal);
   
    if( setStat == SET_OK  )
    {
        global.unit = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(unit), (uint8_t*)&global.unit, 1);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}


void menu_relay_1_alarm_sel(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    eMBMasterReqErrCode err_code;
    
    const char* title = "继电器-1报警类型";
    const char* const select_arr[]=
    {
        "低报警","高报警",
    };
    LCD_DisplayString(0,0,(char*)title,NOT_REVERSE);
    
    setStat = set_select( msg, (void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.relay_1_alarm_method, &retVal);
   
    if( setStat == SET_OK  )
    {
        global.relay_1_alarm_method = retVal;        
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_relay_1_alarm_value(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { " " , "继电器-1报警值"};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.relay_1_alarm_val;
    setIntFloat.max = 10000;
    setIntFloat.min = -10000;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        global.relay_1_alarm_val = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_relay_1_alarm_delay(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { " " , "继电器-1迟滞量"};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.relay_1_alarm_delay;
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        global.relay_1_alarm_delay = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}


void menu_relay_2_alarm_sel(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    eMBMasterReqErrCode err_code;
    
    const char* title = "继电器-2报警类型        ";
    const char* const select_arr[]=
    {
        "低报警","高报警",
    };
    LCD_DisplayString(0,0,(char*)title,NOT_REVERSE);
    
    setStat = set_select( msg, (void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.relay_2_alarm_method, &retVal);
   
    if( setStat == SET_OK  )
    {
        global.relay_2_alarm_method = retVal;        
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_relay_2_alarm_value(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { " " , "继电器-2报警值"};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.relay_2_alarm_val;
    setIntFloat.max = 10000;
    setIntFloat.min = -10000;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        global.relay_2_alarm_val = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_relay_2_alarm_delay(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { " " , "继电器-2迟滞量"};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.relay_2_alarm_delay;
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        global.relay_2_alarm_delay = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

    
    /*
    void menu_4mA_1_cali(uint8_t msg);    
void menu_20mA_1_cali(uint8_t msg);
void menu_curr_out_1_uplimit(uint8_t msg);    
void menu_curr_out_1_downlimit(uint8_t msg);

void menu_4mA_2_cali(uint8_t msg);    
void menu_20mA_2_cali(uint8_t msg);
void menu_curr_out_2_uplimit(uint8_t msg);    
void menu_curr_out_2_downlimit(uint8_t msg);
    */
void menu_4mA_1_cali(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    float32_t duty,duty4;
    const char* const titleLang = { "电流-1-4mA校准    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 4.0f;
    setIntFloat.max = 5.0f;
    setIntFloat.min = 3.0f;
    setIntFloat.pRetVal = &retVal;
    //PWM_Percent4_20(s_RuntimeVar.duty4);
    global.curr_out_1_stat = 1;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        setData = retVal;
        duty4 = global.curr_out_1_duty4;
        if(setData <= 5 && setData >= 3)
        {
            duty = duty4 + (4 - setData) * 0.04f;
            global.curr_out_1_duty4 = duty;
            
            rt_device_write(mb85_bus, E2P_OFFSET(curr_out_1_duty4), (uint8_t*)&global.curr_out_1_duty4, 4);
        }
        global.curr_out_1_stat = 0;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        global.curr_out_1_stat = 0;
        leaf_exit(NULL);
    }
}
void menu_20mA_1_cali(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    float32_t duty,duty20;
    const char* const titleLang = { "电流-1-20mA校准    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 20.0f;
    setIntFloat.max = 22.0f;
    setIntFloat.min = 18.0f;
    setIntFloat.pRetVal = &retVal;
    //PWM_Percent4_20(s_RuntimeVar.duty20);
    global.curr_out_1_stat = 2;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        setData = retVal;
        duty20 = global.curr_out_1_duty20;
        if(setData <= 21 && setData >= 18)
        {
            duty = duty20 + (20 - setData) * 0.04f;
            global.curr_out_1_duty20 = duty;
            
            rt_device_write(mb85_bus, E2P_OFFSET(curr_out_1_duty20), (uint8_t*)&global.curr_out_1_duty20, 4);
        }
        global.curr_out_1_stat = 0;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        global.curr_out_1_stat = 0;
        leaf_exit(NULL);
    }
}

void menu_curr_out_1_uplimit(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    const char* const titleLang = { "电流-1-输出上限    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 20.0f;
    setIntFloat.max = 22.0f;
    setIntFloat.min = 4.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.curr_out_1_uplimit = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(curr_out_1_uplimit), (uint8_t*)&global.curr_out_1_uplimit, 4);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}
    
void menu_curr_out_1_downlimit(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    const char* const titleLang = { "电流-1-输出下限    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 20.0f;
    setIntFloat.max = 22.0f;
    setIntFloat.min = 4.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.curr_out_1_downlimit = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(curr_out_1_downlimit), (uint8_t*)&global.curr_out_1_downlimit, 4);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_4mA_2_cali(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    float32_t duty,duty4;
    const char* const titleLang = { "电流-2-4mA校准    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 4.0f;
    setIntFloat.max = 5.0f;
    setIntFloat.min = 3.0f;
    setIntFloat.pRetVal = &retVal;
    //PWM_Percent4_20(s_RuntimeVar.duty4);
    global.curr_out_2_stat = 1;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        setData = retVal;
        duty4 = global.curr_out_2_duty4;
        if(setData <= 5 && setData >= 3)
        {
            duty = duty4 + (4 - setData) * 0.04f;
            global.curr_out_2_duty4 = duty;
            
            rt_device_write(mb85_bus, E2P_OFFSET(curr_out_2_duty4), (uint8_t*)&global.curr_out_2_duty4, 4);
        }
        global.curr_out_2_stat = 0;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        global.curr_out_1_stat = 0;
        leaf_exit(NULL);
    }
}
    
void menu_20mA_2_cali(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    float32_t duty,duty20;
    const char* const titleLang = { "电流-2-20mA校准    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 20.0f;
    setIntFloat.max = 22.0f;
    setIntFloat.min = 18.0f;
    setIntFloat.pRetVal = &retVal;
    //PWM_Percent4_20(s_RuntimeVar.duty20);
    global.curr_out_2_stat = 2;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        setData = retVal;
        duty20 = global.curr_out_2_duty20;
        if(setData <= 21 && setData >= 18)
        {
            duty = duty20 + (20 - setData) * 0.04f;
            global.curr_out_2_duty20 = duty;
            
            rt_device_write(mb85_bus, E2P_OFFSET(curr_out_2_duty20), (uint8_t*)&global.curr_out_2_duty20, 4);
        }
        global.curr_out_2_stat = 0;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        global.curr_out_2_stat = 0;
        leaf_exit(NULL);
    }
}

void menu_curr_out_2_uplimit(uint8_t msg)
{
    
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    const char* const titleLang = { "电流-2-输出上限    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 20.0f;
    setIntFloat.max = 22.0f;
    setIntFloat.min = 4.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.curr_out_2_uplimit = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(curr_out_2_uplimit), (uint8_t*)&global.curr_out_2_uplimit, 4);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_curr_out_2_downlimit(uint8_t msg)
{   
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    float64_t retVal;
    const char* const titleLang = { "电流-2-输出下限    "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = 20.0f;
    setIntFloat.max = 22.0f;
    setIntFloat.min = 4.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang,NOT_REVERSE);    
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.curr_out_2_downlimit = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(curr_out_2_downlimit), (uint8_t*)&global.curr_out_2_downlimit, 4);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_range_sel(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    eMBMasterReqErrCode err_code;
    
    const char* title = "量程设置        ";
    const char* const select_arr[]=
    {
        "1","2","3","4",
    };
    LCD_DisplayString(0,0,(char*)title,NOT_REVERSE);
    
    setStat = set_select( msg, (void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.zhuodu_data.range_sel, &retVal);
   
    if( setStat == SET_OK  )
    {
        eMBMasterReqWriteHoldingRegister(1,0x100+29,retVal,RT_TICK_PER_SECOND / 2); 
        if( MB_MRE_NO_ERR == err_code)
        {
            global.zhuodu_data.range_sel = retVal;
        }
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_zero_cali(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data,data2;
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    eMBMasterReqErrCode mb_err;
    
    const char* const titleLang[] = { " " , "零位校准      "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.zhuodu_data.turbidimeter_cali_buf[0];
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);
        mb_err = eMBMasterReqReadHoldingRegister(1,265 ,2,RT_TICK_PER_SECOND / 2); 
        if(mb_err == MB_MRE_NO_ERR)
        {
            data2.dat = global.zhuodu_data.absorbance;
            eMBMasterReqWriteMultipleHoldingRegister(1,512 ,2,data.buf,RT_TICK_PER_SECOND / 2);
            eMBMasterReqWriteMultipleHoldingRegister(1,520 ,2,data2.buf,RT_TICK_PER_SECOND / 2); 
        }
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_full_cali(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data,data2;
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    eMBMasterReqErrCode mb_err;
    
    const char* const titleLang[] = { " " , "已知浓度校准      "};
    
    setIntFloat.dataType = FLOAT_32_T;
    setIntFloat.setData = global.zhuodu_data.turbidimeter_cali_buf[1];
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);
        mb_err = eMBMasterReqReadHoldingRegister(1,265 ,2,RT_TICK_PER_SECOND / 2); 
        if(mb_err == MB_MRE_NO_ERR)
        {
            data2.dat = global.zhuodu_data.absorbance;
            eMBMasterReqWriteMultipleHoldingRegister(1,514 ,2,data.buf,RT_TICK_PER_SECOND / 2);
            eMBMasterReqWriteMultipleHoldingRegister(1,522 ,2,data2.buf,RT_TICK_PER_SECOND / 2);
            
            eMBMasterReqWriteHoldingRegister(1,291 ,2,RT_TICK_PER_SECOND / 2);
            eMBMasterReqWriteHoldingRegister(1,528 ,1,RT_TICK_PER_SECOND / 2);
            
        }
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_zero_compensate(uint8_t msg)
{
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    eMBMasterReqErrCode mb_err;
    
    const char* const titleLang[] = { " " , "零位补偿      "};
    setIntFloat.dataType = FLOAT_32_T;
    switch(global.zhuodu_data.range_sel)
    {
        case 0: setIntFloat.setData = global.zhuodu_data.b1; break;
        case 1: setIntFloat.setData = global.zhuodu_data.b2; break;
        case 2: setIntFloat.setData = global.zhuodu_data.b3; break;
        case 3: setIntFloat.setData = global.zhuodu_data.b4; break;
        default: setIntFloat.setData = global.zhuodu_data.b1; break;
    }
    setIntFloat.max = 10000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        data.dat = swap_f32(retVal);
        eMBMasterReqWriteMultipleHoldingRegister(1,269 ,2,data.buf,RT_TICK_PER_SECOND / 2);
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
    const char* const commFormatStr[]=
    {
        "2","3","4"
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    
    if(global.zhuodu_data.num_cali < 2)
        global.zhuodu_data.num_cali = 2;
    
    setStat = set_select( msg, (void*)commFormatStr,sizeof(commFormatStr[0]),3, global.zhuodu_data.num_cali-2, &retVal);
   
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
    /* stat : 0 - screen select 1 - value select 2 - value modifing 3 - save or not & return to 0 */
    static uint8_t stat = 0,timer,set_stat,flag,curr_select,set_flag;
    static Label_t label[8]; 
    static NumberBox_t number_box[8];
    SetIntFloatDef SetVal;
    double dat;
    const char* yesno = {"<-是        否->"};
    uint16_t i;
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    
    if(flag == 0)
    {
        flag = 1; set_flag = 0;

        ClearLCD();
		label[0].x = 0; label[0].y = 0; label[0].is_focused = 0;label[0].label_stat = 0;label[0].width = 5;label[0].is_show = 1;
		snprintf(label[0].label_buf,label[0].width,"%s","1浊");
		label[1].x = 0; label[1].y = 16; label[1].is_focused = 0;label[1].label_stat = 0;label[1].width = 5;label[1].is_show = 1;
		snprintf(label[1].label_buf,label[1].width,"%s","2浊");
		label[2].x = 0; label[2].y = 32; label[2].is_focused = 0;label[2].label_stat = 0;label[2].width = 5;label[2].is_show = 1;
		snprintf(label[2].label_buf,label[2].width,"%s","3浊");
		label[3].x = 0; label[3].y = 48; label[3].is_focused = 0;label[3].label_stat = 0;label[3].width = 5;label[3].is_show = 1;
		snprintf(label[3].label_buf,label[3].width,"%s","4浊");
		label[4].x = 64; label[4].y = 0; label[4].is_focused = 0;label[4].label_stat = 0;label[4].width = 5;label[4].is_show = 1;
		snprintf(label[4].label_buf,label[4].width,"%s","1吸");
		label[5].x = 64; label[5].y = 16; label[5].is_focused = 0;label[5].label_stat = 0;label[5].width = 5;label[5].is_show = 1;
		snprintf(label[5].label_buf,label[5].width,"%s","2吸");
		label[6].x = 64; label[6].y = 32; label[6].is_focused = 0;label[6].label_stat = 0;label[6].width = 5;label[6].is_show = 1;
		snprintf(label[6].label_buf,label[6].width,"%s","3吸");
		label[7].x = 64; label[7].y = 48; label[7].is_focused = 0;label[7].label_stat = 0;label[7].width = 5;label[7].is_show = 1;
		snprintf(label[7].label_buf,label[7].width,"%s","4吸");
		
		label_show(&label[0]); label_show(&label[1]); label_show(&label[2]); label_show(&label[3]);
		label_show(&label[4]); label_show(&label[5]); label_show(&label[6]); label_show(&label[7]);
		
        number_box[0].x = 3*8; number_box[0].y = 0 ; number_box[0].is_focused = 0; number_box[0].box_stat = 0; number_box[0].box_width = 4; number_box[0].data_width = 4; 
        number_box[0].set_index = 0; number_box[0].data_type = FLOAT; number_box[0].min = 0; number_box[0].max = 30001;number_box[0].set_float = global.zhuodu_data.turbidimeter_cali_buf[0];
        number_box[0].is_show = 1;
        
        number_box[1].x = 3*8; number_box[1].y = 16 ; number_box[1].is_focused = 0; number_box[1].box_stat = 0; number_box[1].box_width = 4; number_box[1].data_width = 4; 
        number_box[1].set_index = 0; number_box[1].data_type = FLOAT; number_box[1].min = 0; number_box[1].max = 30001;number_box[1].set_float = global.zhuodu_data.turbidimeter_cali_buf[1];
        number_box[1].is_show = 1;
		
		number_box[2].x = 3*8; number_box[2].y = 32 ; number_box[2].is_focused = 0; number_box[2].box_stat = 0; number_box[2].box_width = 4; number_box[2].data_width = 4; 
        number_box[2].set_index = 0; number_box[2].data_type = FLOAT; number_box[2].min = 0; number_box[2].max = 30001;number_box[2].set_float = global.zhuodu_data.turbidimeter_cali_buf[2];
        number_box[2].is_show = 1;
        
        number_box[3].x = 3*8; number_box[3].y = 48 ; number_box[3].is_focused = 0; number_box[3].box_stat = 0; number_box[3].box_width = 4; number_box[3].data_width = 4; 
        number_box[3].set_index = 0; number_box[3].data_type = FLOAT; number_box[3].min = 0; number_box[3].max = 30001;number_box[3].set_float = global.zhuodu_data.turbidimeter_cali_buf[3];
        number_box[3].is_show = 1;
		
		number_box[4].x = 11*8; number_box[4].y = 0 ; number_box[4].is_focused = 0; number_box[4].box_stat = 0; number_box[4].box_width = 5; number_box[4].data_width = 5; 
        number_box[4].set_index = 0; number_box[4].data_type = FLOAT; number_box[4].min = 0; number_box[4].max = 30001;number_box[4].set_float = global.zhuodu_data.absorbance_cali_buf[0];
        number_box[4].is_show = 1;
		
		number_box[5].x = 11*8; number_box[5].y = 16 ; number_box[5].is_focused = 0; number_box[5].box_stat = 0; number_box[5].box_width = 5; number_box[5].data_width = 5; 
        number_box[5].set_index = 0; number_box[5].data_type = FLOAT; number_box[5].min = 0; number_box[5].max = 30001;number_box[5].set_float = global.zhuodu_data.absorbance_cali_buf[1];
        number_box[5].is_show = 1;
		
		number_box[6].x = 11*8; number_box[6].y = 32 ; number_box[6].is_focused = 0; number_box[6].box_stat = 0; number_box[6].box_width = 5; number_box[6].data_width = 5; 
        number_box[6].set_index = 0; number_box[6].data_type = FLOAT; number_box[6].min = 0; number_box[6].max = 30001;number_box[6].set_float = global.zhuodu_data.absorbance_cali_buf[2];
        number_box[6].is_show = 1;
		
		number_box[7].x = 11*8; number_box[7].y = 48 ; number_box[7].is_focused = 0; number_box[7].box_stat = 0; number_box[7].box_width = 5; number_box[7].data_width = 5; 
        number_box[7].set_index = 0; number_box[7].data_type = FLOAT; number_box[7].min = 0; number_box[7].max = 30001;number_box[7].set_float = global.zhuodu_data.absorbance_cali_buf[3];
        number_box[7].is_show = 1;

    }
	
    switch(stat)
    {
        case 0:
            switch(msg)
            {
            case KEY_DOWN_MENU:
                    flag = 0;
                    leaf_exit(NULL);
                    break;
            case KEY_DOWN_OK:
                    break;
            case KEY_DOWN_DOWN:
                stat = 1;
                if(number_box[curr_select].box_stat == UNFOCUSED)
                {
                    number_box[curr_select].box_stat = FOCUSED;
                    msg = KEY_NONE;
                }
                break;
            case KEY_DOWN_UP:
                stat = 1;
                if(number_box[curr_select].box_stat == UNFOCUSED)
                {
                    number_box[curr_select].box_stat = FOCUSED;
                    msg = KEY_NONE;
                }                         
                break;
            break;
            default:break;
            }
            break;
        case 1:
            switch(msg)
            {
            case KEY_DOWN_MENU:
                    stat = 0;
                    break;
            case KEY_DOWN_OK:
                    stat = 2;
                    set_stat = set_number_box(msg,&number_box[i]);
                    break;
            case KEY_DOWN_DOWN:
                number_box[curr_select].box_stat = UNFOCUSED;  
                if(curr_select < 8)
                    curr_select++;
                else if(curr_select == 8)
                    curr_select = 0;
                number_box[curr_select].box_stat = FOCUSED;
                msg = KEY_NONE;
                break;
            case KEY_DOWN_UP:
                number_box[curr_select].box_stat = UNFOCUSED;
                    if(curr_select > 0)
                        curr_select--;
                    else if(curr_select == 0)
                        curr_select = 8;
                    number_box[curr_select].box_stat = FOCUSED;   
                    msg = KEY_NONE;                    
                break;
            break;
            default:break;
            }
            break;
        case 2:
            //set_stat = set_number_box(msg,&number_box[i]);
            break;
        default: break;
    }
	for(i = 0 ; i < 8; i++)
	{
        set_stat = set_number_box(msg,&number_box[i]);
        if( set_stat == SAVING)
        {
            set_flag = 1;
        }
        if(set_stat == FOCUSED && set_flag == 1)
        {
            stat = 1;
            set_flag = 0;
            
            data.dat = swap_f32(number_box[i].set_float);
            eMBMasterReqWriteMultipleHoldingRegister(1,0x200+i*2,2,data.buf,RT_TICK_PER_SECOND / 2);
        }
	}
}
void menu_linear_coeff_setup(uint8_t msg)
{
	/* stat : 0 - screen select 1 - value select 2 - value modifing 3 - save or not & return to 0 */
    static uint8_t stat = 0,timer,set_stat,flag,curr_select,set_flag;
    static Label_t label[8]; 
    static NumberBox_t number_box[8];
    SetIntFloatDef SetVal;
    double dat;
    const char* yesno = {"<-是        否->"};
    uint16_t i;
    union data_f
    {
        float dat;
        uint16_t buf[2];
    } data;
    
    if(flag == 0)
    {
        flag = 1; set_flag = 0;
        ClearLCD();
		label[0].x = 0; label[0].y = 0; label[0].is_focused = 0;label[0].label_stat = 0;label[0].width = 4;label[0].is_show = 1;
		snprintf(label[0].label_buf,label[0].width,"%s","A1 ");
		label[1].x = 0; label[1].y = 16; label[1].is_focused = 0;label[1].label_stat = 0;label[1].width = 4;label[1].is_show = 1;
		snprintf(label[1].label_buf,label[1].width,"%s","A2 ");
		label[2].x = 0; label[2].y = 32; label[2].is_focused = 0;label[2].label_stat = 0;label[2].width = 4;label[2].is_show = 1;
		snprintf(label[2].label_buf,label[2].width,"%s","A3 ");
		label[3].x = 0; label[3].y = 48; label[3].is_focused = 0;label[3].label_stat = 0;label[3].width = 4;label[3].is_show = 1;
		snprintf(label[3].label_buf,label[3].width,"%s","A4 ");
		label[4].x = 64; label[4].y = 0; label[4].is_focused = 0;label[4].label_stat = 0;label[4].width = 4;label[4].is_show = 1;
		snprintf(label[4].label_buf,label[4].width,"%s","B1 ");
		label[5].x = 64; label[5].y = 16; label[5].is_focused = 0;label[5].label_stat = 0;label[5].width = 4;label[5].is_show = 1;
		snprintf(label[5].label_buf,label[5].width,"%s","B2 ");
		label[6].x = 8*8; label[6].y = 32; label[6].is_focused = 0;label[6].label_stat = 0;label[6].width = 4;label[6].is_show = 1;
		snprintf(label[6].label_buf,label[6].width,"%s","B3 ");
		label[7].x = 8*8; label[7].y = 48; label[7].is_focused = 0;label[7].label_stat = 0;label[7].width = 4;label[7].is_show = 1;
		snprintf(label[7].label_buf,label[7].width,"%s","B4 ");
		
		label_show(&label[0]); label_show(&label[1]); label_show(&label[2]); label_show(&label[3]);
		label_show(&label[4]); label_show(&label[5]); label_show(&label[6]); label_show(&label[7]);
		
        number_box[0].x = 3*8; number_box[0].y = 0 ; number_box[0].is_focused = 0; number_box[0].box_stat = 0; number_box[0].box_width = 5; number_box[0].data_width = 5; 
        number_box[0].set_index = 0; number_box[0].data_type = FLOAT; number_box[0].min = 0; number_box[0].max = 30001;number_box[0].set_float = global.zhuodu_data.a1;
        number_box[0].is_show = 1;
        
        number_box[1].x = 3*8; number_box[1].y = 16 ; number_box[1].is_focused = 0; number_box[1].box_stat = 0; number_box[1].box_width = 5; number_box[1].data_width = 5; 
        number_box[1].set_index = 0; number_box[1].data_type = FLOAT; number_box[1].min = 0; number_box[1].max = 30001;number_box[1].set_float = global.zhuodu_data.a2;
        number_box[1].is_show = 1;
		
		number_box[2].x = 3*8; number_box[2].y = 32 ; number_box[2].is_focused = 0; number_box[2].box_stat = 0; number_box[2].box_width = 5; number_box[2].data_width = 5; 
        number_box[2].set_index = 0; number_box[2].data_type = FLOAT; number_box[2].min = 0; number_box[2].max = 30001;number_box[2].set_float = global.zhuodu_data.a3;
        number_box[2].is_show = 1;
        
        number_box[3].x = 3*8; number_box[3].y = 48 ; number_box[3].is_focused = 0; number_box[3].box_stat = 0; number_box[3].box_width = 5; number_box[3].data_width = 5; 
        number_box[3].set_index = 0; number_box[3].data_type = FLOAT; number_box[3].min = 0; number_box[3].max = 30001;number_box[3].set_float = global.zhuodu_data.a4;
        number_box[3].is_show = 1;
		
		number_box[4].x = 11*8; number_box[4].y = 0 ; number_box[4].is_focused = 0; number_box[4].box_stat = 0; number_box[4].box_width = 5; number_box[4].data_width = 5; 
        number_box[4].set_index = 0; number_box[4].data_type = FLOAT; number_box[4].min = 0; number_box[4].max = 30001;number_box[4].set_float = global.zhuodu_data.b1;
        number_box[4].is_show = 1;
		
		number_box[5].x = 11*8; number_box[5].y = 16 ; number_box[5].is_focused = 0; number_box[5].box_stat = 0; number_box[5].box_width = 5; number_box[5].data_width = 5; 
        number_box[5].set_index = 0; number_box[5].data_type = FLOAT; number_box[5].min = 0; number_box[5].max = 30001;number_box[5].set_float = global.zhuodu_data.b2;
        number_box[5].is_show = 1;
		
		number_box[6].x = 11*8; number_box[6].y = 32 ; number_box[6].is_focused = 0; number_box[6].box_stat = 0; number_box[6].box_width = 5; number_box[6].data_width = 5; 
        number_box[6].set_index = 0; number_box[6].data_type = FLOAT; number_box[6].min = 0; number_box[6].max = 30001;number_box[6].set_float = global.zhuodu_data.b3;
        number_box[6].is_show = 1;
		
		number_box[7].x = 11*8; number_box[7].y = 48 ; number_box[7].is_focused = 0; number_box[7].box_stat = 0; number_box[7].box_width = 5; number_box[7].data_width = 5; 
        number_box[7].set_index = 0; number_box[7].data_type = FLOAT; number_box[7].min = 0; number_box[7].max = 30001;number_box[7].set_float = global.zhuodu_data.b4;
        number_box[7].is_show = 1;

    }
	
    switch(stat)
    {
        case 0:
            switch(msg)
            {
            case KEY_DOWN_MENU:
                    flag = 0;
                    leaf_exit(NULL);
                    break;
            case KEY_DOWN_OK:
                    break;
            case KEY_DOWN_DOWN:
                stat = 1;
                if(number_box[curr_select].box_stat == UNFOCUSED)
                {
                    number_box[curr_select].box_stat = FOCUSED;
                    msg = KEY_NONE;
                }
                break;
            case KEY_DOWN_UP:
                stat = 1;
                if(number_box[curr_select].box_stat == UNFOCUSED)
                {
                    number_box[curr_select].box_stat = FOCUSED;
                    msg = KEY_NONE;
                }                         
                break;
            break;
            default:break;
            }
            break;
        case 1:
            switch(msg)
            {
            case KEY_DOWN_MENU:
                    stat = 0;
                    break;
            case KEY_DOWN_OK:
                    stat = 2;
                    set_stat = set_number_box(msg,&number_box[i]);
                    break;
            case KEY_DOWN_DOWN:
                number_box[curr_select].box_stat = UNFOCUSED;  
                if(curr_select < 8)
                    curr_select++;
                else if(curr_select == 8)
                    curr_select = 0;
                number_box[curr_select].box_stat = FOCUSED;
                msg = KEY_NONE;
                break;
            case KEY_DOWN_UP:
                number_box[curr_select].box_stat = UNFOCUSED;
                    if(curr_select > 0)
                        curr_select--;
                    else if(curr_select == 0)
                        curr_select = 8;
                    number_box[curr_select].box_stat = FOCUSED;   
                    msg = KEY_NONE;                    
                break;
            break;
            default:break;
            }
            break;
        case 2:
            //set_stat = set_number_box(msg,&number_box[i]);
            break;
        default: break;
    }
	for(i = 0 ; i < 8; i++)
	{
        set_stat = set_number_box(msg,&number_box[i]);
        if( set_stat == SAVING)
        {
            set_flag = 1;
        }
        if(set_stat == FOCUSED && set_flag == 1)
        {
            stat = 1;
            set_flag = 0;
            data.dat = swap_f32(number_box[i].set_float);
            
            switch(i)
            {
                case 0:
                    eMBMasterReqWriteMultipleHoldingRegister(1,0x100+11,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 1:
                    eMBMasterReqWriteMultipleHoldingRegister(1,0x100+15,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 2:
                    eMBMasterReqWriteMultipleHoldingRegister(1,0x100+19,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 3:
                    eMBMasterReqWriteMultipleHoldingRegister(1,534,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 4:
                    eMBMasterReqWriteMultipleHoldingRegister(1,0x100+13,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 5:
                    eMBMasterReqWriteMultipleHoldingRegister(1,0x100+17,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 6:
                    eMBMasterReqWriteMultipleHoldingRegister(1,0x100+21,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                case 7:
                    eMBMasterReqWriteMultipleHoldingRegister(1,536,2,data.buf,RT_TICK_PER_SECOND / 2);
                    break;
                default:
                    break;
            }
            
        }
	}
}
void menu_4mA_cali(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { " " , "4mA校准         "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.zhuodu_data.ma_cali;
    setIntFloat.max = 30000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        eMBMasterReqWriteHoldingRegister(1,290,retVal,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.ma_cali = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

void menu_20mA_cali(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float32_t setData;
    double retVal;
    const char* const titleLang[] = { " " , "20mA校准         "};
    
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.zhuodu_data.ma_cali;
    setIntFloat.max = 30000;
    setIntFloat.min = 0;
    setIntFloat.pRetVal = &retVal;
        
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK)
    {
        eMBMasterReqWriteHoldingRegister(1,290,retVal,RT_TICK_PER_SECOND / 2); 
        global.zhuodu_data.ma_cali = retVal;
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
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
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const title =  "浊度仪地址设置    ";

    setIntFloat.dataType = INT_8_T;
    setIntFloat.setData = global.zhuodu_data.zhuodu_addr;
    setIntFloat.max = 247.0f;
    setIntFloat.min = 1.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)title,NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.zhuodu_data.zhuodu_addr = retVal;
        //ee_WriteBytes((uint8_t*)&retVal, E2P_OFFSET(comm_device_id), 1);
        rt_device_write(mb85_bus, E2P_OFFSET(zhuodu_addr), (uint8_t*)&retVal, 1);
        //TODO:setup zhoudu device id
        
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

void menu_rs485_protocal(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint8_t retVal;
    const char* const titleLang[] = { "Protocal        " , "通讯格式        "};
    const char* const select_arr[]=
    {
        "Modbus-RTU","Modbus-ASCII",
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_select( msg, (void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.comm_format, &retVal);
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
    const char* const select_arr[]=
    {
        "1200","2400","4800","9600","19200","38400",
    };
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_select( msg, (void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.comm_baud, &retVal);
    
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
    LCD_DisplayString(0,48,"               8",NOT_REVERSE);
    
    if(msg == KEY_DOWN_MENU)
        leaf_exit(NULL);
}

void menu_rs485_check(uint8_t msg)
{
    uint8_t lang = get_language();
    static uint8_t flag1st;
    SET_STAT setStat = 0;
    uint8_t retVal;
    const char* const titleLang[] = { "Check Mode      " , "校验方式            "};
    const char* const select_arr[]=
    {
        "无校验","奇校验","偶校验",
    };
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    
    setStat = set_select( msg, (void*)select_arr,sizeof(select_arr[0]),countof(select_arr), global.comm_parity, &retVal);
    
    if( setStat == SET_OK  )
    {
        global.comm_parity = retVal;
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
    const char* const title =  "设备地址        ";

    setIntFloat.dataType = INT_8_T;
    setIntFloat.setData = global.comm_device_id;
    setIntFloat.max = 247.0f;
    setIntFloat.min = 1.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)title,NOT_REVERSE);
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
*****************
*/

void menu_temp_compensate(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    double retVal;
    const char* const titleLang[] = { "ssss       " , "温度补偿        "};
          
    setIntFloat.dataType = INT_16_T;
    setIntFloat.setData = global.temp_compensate;
    setIntFloat.max = 1000.0f;
    setIntFloat.min = -1000.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.temp_compensate = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(temp_compensate), (uint8_t*)&global.temp_compensate, 2);
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
    const char* const titleLang[] = { "Reset Factory   ","恢复出厂设置    "};
    const char* pStr[][3] = {{"No","否","No"},{"Yes","是"}};

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
*******************
*/
void menu_serial_no_setup(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    float64_t retVal;
    const char* const titleLang[] = { "Sensor Code    " , "传感器编码     ","Sensor Code "};
    
    setIntFloat.dataType = INT_32_T;
    setIntFloat.setData = global.serial_no;
    setIntFloat.max = 999999.0f;
    setIntFloat.min = 0.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.serial_no = retVal;
        rt_device_write(mb85_bus, E2P_OFFSET(serial_no), (uint8_t*)&global.serial_no , 4);
        leaf_exit(NULL);
    }
    else if(setStat == SET_EXIT)
    {
        leaf_exit(NULL);
    }
}

/*
*******************
*/
void menu_password_setup(uint8_t msg)
{
    uint8_t lang = get_language();
    SET_STAT setStat;
    uint32_t setData;
    float64_t retVal;
    const char* const titleLang[] = { "Password Setup  " , "密码设置        "};
    
    setIntFloat.dataType = INT_32_T;
    setIntFloat.setData = global.password;
    setIntFloat.max = 999999.0f;
    setIntFloat.min = 0.0f;
    setIntFloat.pRetVal = &retVal;
    
    LCD_DisplayString(0,0,(char*)titleLang[lang],NOT_REVERSE);
    setStat = set_int_float_min_max( msg, setIntFloat);
    if( setStat == SET_OK )
    {
        global.password  = retVal;        
        rt_device_write(mb85_bus, E2P_OFFSET(password), (uint8_t*)&global.password , 4);
        snprintf(global.password_buf,7,"%06d",global.password);
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
SET_STAT set_select(uint8_t msg,void *pArray,uint8_t sizeEle,uint8_t sizeArray,uint16_t currIndex,uint8_t *pRetVal)
{
	uint8_t lang = get_language();
    uint16_t totalLen,asciiLen,hzLen;
	static uint8_t ee_val,flag,flag1st;	/* in exit proc,it should be reset */
	char *pStr,**ppStr =(char**) pArray;
    const char* yesno[] = {"<-Yes       No->","<-是        否->"};
	//printf("ee_val:%d flag:%d --\r",ee_val,flag);
	if(flag1st == 0)
	{
        flag1st = 1;
        flag = 0;
        ee_val = currIndex;
		
        pStr = (char*) ( ppStr[ee_val]);
        
        get_utf8_len(pStr, &totalLen, &asciiLen);
        hzLen = (totalLen - asciiLen ) / 3;
        snprintf(&s_DispBuf[1][0],25,"%-.*s%s",16 - ((hzLen<<1) + asciiLen),s_blank,pStr);
        
		LCD_DisplayString(0,16,&s_DispBuf[1][0],NOT_REVERSE);
	}
	
    pStr = (char*) ( ppStr[ee_val]);
    get_utf8_len(pStr, &totalLen, &asciiLen);
    hzLen = (totalLen - asciiLen ) / 3;
    snprintf(s_set_buf,25,"%-s",pStr);
    
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
		if( 0 == strncmp(passWord,global.password_buf,6) )
		{
			
			jump2func(NULL);
            jump2menu("1");
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
