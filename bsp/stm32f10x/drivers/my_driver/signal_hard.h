

#ifndef __SIGNAL_HARD_H
#define __SIGNAL_HARD_H

#ifdef __cplusplus
extern "C" {
#endif
    

void adc_signal_init(void);

void pwm_init(float freq);
void pwm_start(void);
void pwm_stop(void);
    
void pulse_send_init(float freq,float us);
void pulse_release(void);
void pulse_send(void);
	
#ifdef __cplusplus
}
#endif

#endif
