#ifndef __TEMP_PRESS_HW_H
#define __TEMP_PRESS_HW_H

#include <stdint.h>
#include <stdbool.h>

bool get_temp_press_adc(uint16_t *ptemp,uint16_t *ppress);
void adc_temp_press_init(void);

#endif
