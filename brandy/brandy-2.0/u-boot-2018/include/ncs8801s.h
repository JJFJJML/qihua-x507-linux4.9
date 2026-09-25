#ifndef _NCS8801S_H_
#define _NCS8801S_H_
#include <common.h>
#include <i2c.h>
#include <sunxi_i2c.h>
#include <sys_config.h>


static inline int ncs8801s_i2c_readByte(unsigned char dev_addr_temp, unsigned char addr_temp)
{
	uint8_t dev_addr = dev_addr_temp;
	uint32_t addr = addr_temp;
	uint8_t buffer = 0x00;
	i2c_read(dev_addr,addr, 1, &buffer, 1);
	return buffer;
}

static inline int ncs8801s_i2c_writeByte(unsigned char dev_addr_temp,unsigned char addr_temp, unsigned char data)
{
	uint8_t dev_addr = dev_addr_temp;
	uint32_t addr = addr_temp;
	uint8_t buffer = data;
	return i2c_write(dev_addr,addr, 1, &buffer, 1);
}

#endif
