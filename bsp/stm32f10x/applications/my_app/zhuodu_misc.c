#include <stdint.h>
#include "rtthread.h"
#include "global.h"
#include "rtdevice.h"

void zhuodu_alarm(void)
{
    static uint8_t flag;
    if(flag == 0)
    {
        flag = 1;
        rt_pin_mode(33, PIN_MODE_OUTPUT);
        rt_pin_mode(34, PIN_MODE_OUTPUT);
    }
	if(global.relay_1_alarm_type == 0)
    {
        if(global.zhuodu_data.turbidimeter < global.relay_1_alarm_val - global.relay_1_alarm_delay )
        {
            // exe alarm-1
            rt_pin_write(33, PIN_HIGH);
        }
        else if(global.zhuodu_data.turbidimeter > global.relay_1_alarm_val + global.relay_1_alarm_delay )
        {
            // cancel alarm-1
            rt_pin_write(33, PIN_LOW);
        }
    }
    else if(global.relay_1_alarm_type == 1)
    {
        if(global.zhuodu_data.turbidimeter > global.relay_1_alarm_val + global.relay_1_alarm_delay )
        {
            // exe alarm-1
            rt_pin_write(33, PIN_HIGH);
        }
        else if(global.zhuodu_data.turbidimeter < global.relay_1_alarm_val - global.relay_1_alarm_delay )
        {
            // cancel alarm-1
            rt_pin_write(33, PIN_LOW);
        }
    }
    // alarm-2
    if(global.relay_2_alarm_type == 0)
    {
        if(global.zhuodu_data.turbidimeter < global.relay_2_alarm_val - global.relay_2_alarm_delay )
        {
            // exe alarm-2
            rt_pin_write(34, PIN_HIGH);
        }
        else if(global.zhuodu_data.turbidimeter > global.relay_2_alarm_val + global.relay_2_alarm_delay )
        {
            // cancel alarm-2
            rt_pin_write(34, PIN_LOW);
        }
    }
    else if(global.relay_2_alarm_type == 1)
    {
        if(global.zhuodu_data.turbidimeter > global.relay_2_alarm_val + global.relay_2_alarm_delay )
        {
            // exe alarm-2
            rt_pin_write(34, PIN_HIGH);
        }
        else if(global.zhuodu_data.turbidimeter < global.relay_2_alarm_val - global.relay_2_alarm_delay )
        {
            // cancel alarm-2
            rt_pin_write(34, PIN_LOW);
        }
    }
}

