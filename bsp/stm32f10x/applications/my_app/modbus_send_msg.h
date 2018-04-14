#ifndef __MODBUS_SEND_MSG_H
#define __MODBUS_SEND_MSG_H


typedef struct modbus_msg
{
	uint8_t retry_times;
	uint8_t dev_addr;
	uint8_t cmd;
	uint16_t data_addr;
	uint8_t data_type;
	union dat
	{
		float dat_f32;
		uint16_t dat_u16;
	}dat;
}modbus_msg_t;


#define SWAP16(A)   (uint16_t)((uint16_t)(A) << 8) | ((uint16_t)(A) >> 8)
#define SWAP32(A)  ((((uint32_t)(A) & 0xff000000) >> 24) | \
                                                 (((uint32_t)(A) & 0x00ff0000) >> 8) | \
                                                 (((uint32_t)(A) & 0x0000ff00) << 8) | \
                                                 (((uint32_t)(A) & 0x000000ff) << 24))

#define SWAP_32(A)  ((((uint32_t)(A) & 0xffff0000) >> 16) | \
                                                 (((uint32_t)(A) & 0x0000ffff) << 16))

static inline float swap_f32(float dat)
{
    uint32_t dat_32 = *(uint32_t*)&dat;
    dat_32 = SWAP_32(dat_32);
    return *(float*) &dat_32;
}

#endif
