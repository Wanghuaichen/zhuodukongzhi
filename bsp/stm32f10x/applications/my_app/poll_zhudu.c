#include <stdint.h>

typedef struct zhudu
{
	//0x100 start
	int16_t canbi_set;
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
	float canbi_calc_get
	
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
	int16_t a3_full;
	int16_t a2_zero;
	
	uint16_t dev_addr;	
	
}Zhuodu_t
