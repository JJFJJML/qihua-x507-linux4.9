#ifndef __PCF8574A_H
#define __PCF8574A_H


#include <sys_config.h>


extern int pcf8574a_init(void);
extern int pcf8574a_gpio_set_value(u32 p_handler, u32 value_to_gpio);
extern int pcf8574a_gpio_get_value(u32 p_handler);

extern int pcf8574a_gpio_request(user_gpio_set_t *gpio_list);
extern bool is_pcf8674a_port(int port);
extern bool is_pcf8674a_handler(int p_handler);











#endif






























