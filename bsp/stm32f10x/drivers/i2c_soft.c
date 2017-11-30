
#include "platform_config.h"
#include <rtdevice.h>

void stm32_set_sda(void *data, rt_int32_t state)
{
    if(state == 1)
        I2C_SDA_1();
    else if(state == 0)
        I2C_SDA_0();
}

void stm32_set_scl(void *data, rt_int32_t state)
{
    if(state == 1)
        I2C_SCL_1();
    else if(state == 0)
        I2C_SCL_0();
}

rt_int32_t stm32_get_sda(void *data)
{
    return (rt_int32_t)I2C_SDA_READ();
}

rt_int32_t stm32_get_scl(void *data)
{
    return (rt_int32_t)I2C_SCL_READ();
}

void stm32_udelay(rt_uint32_t us)
{
    rt_uint32_t delta;
    
    us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
    
    delta = SysTick->VAL;
    
    while (delta - SysTick->VAL< us);
}

void stm32_mdelay(rt_uint32_t ms)
{
      stm32_udelay(ms * 1000);
}

static const struct  rt_i2c_bit_ops stm32_i2c_bit_ops =
{
    (void*)0xaa,     //no use in set_sda,set_scl,get_sda,get_scl
    stm32_set_sda,
    stm32_set_scl,
    stm32_get_sda,
    stm32_get_scl,
    stm32_udelay,
    20, 
};

static void i2c_hard_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(I2C_GPIO_CLK, ENABLE);

	GPIO_InitStructure.GPIO_Pin = I2C_SCL_PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;    
	GPIO_Init(I2C_SCL_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = I2C_SDA_PIN;
    GPIO_Init(I2C_SDA_PORT, &GPIO_InitStructure);

}

int rt_hw_i2c_init(void)
{
    static struct rt_i2c_bus_device stm32_i2c;//"static" add by me. It must be add "static", or it will be hard fault
    
    i2c_hard_init();
    
    rt_memset((void *)&stm32_i2c, 0, sizeof(struct rt_i2c_bus_device));
    stm32_i2c.priv = (void *)&stm32_i2c_bit_ops;
    rt_i2c_bit_add_bus(&stm32_i2c, "i2c1");   
        
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_i2c_init);//rt_hw_i2c_init will be called in rt_components_board_init()