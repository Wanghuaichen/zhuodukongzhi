
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

typedef float float32_t;
typedef double float64_t;

typedef struct
{
	uint16_t index;	    // time
	uint16_t mag;
	uint16_t width;
}signal_t;

typedef struct
{
    uint16_t* buf_in;
    uint16_t len;
    uint8_t buf_index;
    uint8_t stat;
	signal_t signal;
}signal_analysis_t;


typedef struct
{
    uint8_t index;
	uint8_t numbers;
	signal_t signal[10];
}signals_t;

typedef struct
{
	uint16_t start_mag;           //          | top threshold
    uint16_t end_mag;           //          | bottom threshold
    uint16_t start_index;       //     (start)--......--(end)
    uint16_t end_index;         //
    uint16_t width_min;         //      longest width
    uint16_t width_max;
}signal_cond_t;

typedef struct
{
	uint8_t index;
	uint8_t numbers;           // 
    uint16_t wave[30];
}signal_wave_t;


typedef struct
{
    uint8_t chip_type;
    uint8_t is_disp_reverse;   
    uint8_t lang;
	
    uint8_t ch1_status;         // 0 no liquid 1: 
    uint8_t ch2_status;
    
	uint8_t chSlct;
	
	float container_d;
    
    float sensor_freq;
	uint16_t measure_freq;
    
	signals_t signals[2];
	signal_analysis_t signal_analysis;
	signal_cond_t signal_cond[2];
	signal_wave_t signal_wave[2];
	
	uint8_t gain_1;
	uint8_t gain_2;
	
    // rs485
    uint8_t comm_format;
    uint8_t comm_baud;
    uint8_t comm_check;
    uint8_t comm_device_id;
   
    float temp_k1;
    float temp_k2; 
    
    float temprature;
   
    
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
	
    uint8_t get_language(void);
    void global_init(void);
#endif


#ifdef __cplusplus
}
#endif
    
#endif
