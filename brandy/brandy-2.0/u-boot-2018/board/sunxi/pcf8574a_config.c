#include <common.h>
#include <fdt_support.h>
#include <i2c.h>
#include <sunxi_i2c.h>
#include <pcf8574a_config.h>

/*
PCF8574A_PORT_BASE represents pcf8574a base p_handler,it can be a random num(eg:100),but must
can be differentiated from the result of sunxi_gpio_request()(which is a memory address)
*/
#define PCF8574A_PORT_BASE   100
#define PCF8574A_PORT_NUMS   8

#define   FDT_PATH_PCF8574A                   "twi3/pcf8574a"
#define   PORT_PCF8574A                        16  //refer to dt-bindings/gpio/gpio.h PZ

static uint32_t pcf8574a_addr;
static unsigned char gpio_out = 0x00;

static int pcf857x_set(unsigned offset, int value)
{
	unsigned	bit = 1 << offset;
	int		status;
	unsigned int cur_bus_num;

	cur_bus_num = i2c_get_bus_num();
	i2c_set_bus_num(SUNXI_VIR_I2C3);

	if (value)
		gpio_out |= bit;
	else
		gpio_out &= ~bit;
	
	status = i2c_write(pcf8574a_addr, 0x00, 1, &gpio_out, 1);  //reg addr always be 0x00
	if(status){
		printk("i2c write pcf8574a failed\n");
	}

	i2c_set_bus_num(cur_bus_num);

	return status;
}

static int pcf857x_get(unsigned offset)
{
	uint8_t value;
	unsigned int cur_bus_num;

	cur_bus_num = i2c_get_bus_num();
	i2c_set_bus_num(SUNXI_VIR_I2C3);

	if(i2c_read(pcf8574a_addr, 0x00, 1, &value, 1))
	{
		i2c_set_bus_num(cur_bus_num);
		printk("i2c read pcf8574a failed\n");
		return -1;
	}

	i2c_set_bus_num(cur_bus_num);
	
	return (value < 0) ? value : !!(value & (1 << offset));
}

int pcf8574a_gpio_set_value(u32 p_handler, u32 value_to_gpio)
{
	return pcf857x_set((p_handler-PCF8574A_PORT_BASE), value_to_gpio);
}

int pcf8574a_gpio_get_value(u32 p_handler)
{
	return pcf857x_get(p_handler-PCF8574A_PORT_BASE);
}

/***************************************************************************************/
bool is_pcf8674a_port(int port)
{
	return port > PORT_PCF8574A;
}

bool is_pcf8674a_handler(int p_handler)
{
	return (p_handler>=PCF8574A_PORT_BASE) && (p_handler<PCF8574A_PORT_BASE+PCF8574A_PORT_NUMS);
}

int pcf8574a_gpio_request(user_gpio_set_t *gpio_list)
{
	return PCF8574A_PORT_BASE + gpio_list->port_num;
}

int pcf8574a_init(void)
{
	int node;
	int ret = -1;
	int i;
	
	node = fdt_path_offset(working_fdt,FDT_PATH_PCF8574A);
	if (node < 0) {
		printk("fdt_path_offset %s fail\n", FDT_PATH_PCF8574A);
		return ret;
	}

	ret = fdt_getprop_u32(working_fdt, node, "reg", &pcf8574a_addr);
	if (ret < 0)
		printk("fdt_getprop_u32 %s.%s fail\n", FDT_PATH_PCF8574A, "reg");

	for(i=0;i<8;i++){
		pcf857x_set(i, 0);
	}

	return ret;
}



















