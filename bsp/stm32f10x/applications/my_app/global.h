
#ifndef __GLOBAL_H
#define __GLOBAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <rtthread.h>
#include "app_config.h"

#define SIZEOF(s,m)             ( sizeof((s*)0 -> m) )
#define countof(a)              ( sizeof(a) / sizeof(*(a)) )

typedef float float32_t;
typedef double float64_t;

enum ChipType
{
    CHIP_ST = 0,
    CHIP_GD,
};

enum DATA_TYPE
{
    INT_8_T = 0,	// 8-bit interger
    INT_16_T,	// 16-bit interger
    INT_32_T,	// 32-bit interger
    INT_64_T,
    FLOAT_32_T,
    FLOAT_64_T
};

typedef struct zhudu
{
    
    // user
    
    float turbidimeter_range_1;
    float turbidimeter_range_2;
    float turbidimeter_range_3;
    float turbidimeter_range_4;
    
    int16_t canbi_set;
    int16_t bright_range_set;
    int16_t ma_cali;
    
	//0x100 start
	int16_t canbi_get;
	int16_t bright_2;
	int16_t bright_3;
	int16_t bright_range;
	int16_t temp;
	
	float turbidimeter;
	float turbidimeter_range;
	float absorbance;
	
	float a1;
	float b1;
	float a2;
	float b2;
	float a3;
	float b3;
	float a1k1;
	
	float seg_range_1;
	float seg_range_2;
	int16_t range_sel;
	
	int16_t a2_zero;
	float canbi_calc_get;
	
	uint16_t ma_stat;
	uint16_t num_cali;
	
	int16_t signal_unknown;

	// 0x200 start
	float turbidimeter_cali_buf[4];
	float absorbance_cali_buf[4];
	uint16_t cali_data_write;
	uint16_t data_val_0;
	float seg_range_3;
	float seg_range_4;
	float a4;
	float b4;
	int16_t cmd_a3_full;
	int16_t cmd_a2_zero;
	
	uint16_t dev_addr;
	
}Zhuodu_t;


typedef struct
{
    uint8_t chip_type;
    uint8_t is_disp_reverse;   
    uint8_t language;
	
    // rs485
    uint8_t comm_format;
    uint8_t comm_baud;
    uint8_t comm_parity;
    uint8_t comm_device_id;
   
    Zhuodu_t zhuodu_data;
    
    
}GlobalDef;    

#define global_offset(A)        (offsetof(GlobalDef,A))
#ifdef RUNTIME_MAKE_REAL
    GlobalDef global;

	struct rt_mailbox signal_mb;
	rt_device_t mb85_bus;
#else
    extern GlobalDef global;

    extern struct rt_mailbox signal_mb;
	extern rt_device_t mb85_bus;
	
#endif

    uint8_t get_language(void);
    void global_init(void);

#ifdef __cplusplus
}
#endif
    
#endif
