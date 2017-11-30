#ifndef __MCP41XXX_H
#define __MCP41XXX_H

#include <stdint.h>

void mcp41xxx_write(uint8_t dev,uint8_t cmd,uint8_t dat);
void mcp41xxx_set_pot(uint8_t dev,uint8_t pot_index,uint8_t dat);

#endif
