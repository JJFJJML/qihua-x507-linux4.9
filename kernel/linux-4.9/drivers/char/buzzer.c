#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/sunxi-gpio.h>
#include <linux/miscdevice.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/fs.h>
#include <linux/pwm.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/delay.h>

//#include <asm/system.h>

static u32 debug_mask = 0x0;
enum {
	DEBUG_INIT = 1U << 0,
	DEBUG_DBG = 1U << 1,
	DEBUG_ERROR = 1U << 2,
};
#define dprintk(level_mask, fmt, arg...)	if(unlikely(debug_mask & level_mask)) \
        printk("[buzzer] : "fmt, ## arg)
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);


struct buzzer_data_t{
	int buzzer_pwm_ch;
	int buzzer_pwm_freq;
	struct pwm_device *buzzer_pwm_dev;
};

struct buzzer_data_t buzzer_data;

/* static int buzzer_open(struct inode *inode, struct file *file)
{
	printk("%s\n", __func__);

	pwm_enable(buzzer_data.buzzer_pwm_dev);
    return 0;
}

static int buzzer_close(struct inode *inode, struct file *file)
{
	printk("%s\n", __func__);

	pwm_disable(buzzer_data.buzzer_pwm_dev);
    return 0;
} */

ssize_t buzzer_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{	
	static char data[128];
	
    dprintk(DEBUG_INIT,"%s\n",__func__);
//	dprintk(DEBUG_DBG,"count：%d\n",count);
	
/* 	if(count == 2)
    { */
		if(copy_from_user(data, (void __user *)buf, count)){
			dprintk(DEBUG_ERROR,"copy err\n");
			return 0;
		}
		dprintk(DEBUG_INIT,"copy ok\n");
		dprintk(DEBUG_DBG,"data：%s\n",data);
		
		if(data[0] == '1' )
		{
			dprintk(DEBUG_INIT,"buzzer_open\n");
			pwm_enable(buzzer_data.buzzer_pwm_dev);
		}
		else if(data[0] == '0' )
		{
			dprintk(DEBUG_INIT,"buzzer_close\n");
			pwm_disable(buzzer_data.buzzer_pwm_dev);
		}	
		else {
			dprintk(DEBUG_ERROR,"input command err\n");
			return -1;
		}
/* 	} else {
			dprintk(DEBUG_ERROR,"input command err\n");
			return -1;
		} */
	
    return count;

}


static struct file_operations buzzer_fops = {
    .owner 		= THIS_MODULE,
	.write 		= buzzer_write,
};

static struct miscdevice buzzer_dev_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "buzzer",
    .fops = &buzzer_fops,  
};

static int buzzer_probe(struct platform_device *pdev)
{
	int ret;
	int pwm_period_ns;
	struct device_node *np;
	
	printk("%s\n", __func__);
	np = pdev->dev.of_node;

	ret = of_property_read_u32(np, "buzzer_pwm_ch", &buzzer_data.buzzer_pwm_ch);
	if(ret)
		dprintk(DEBUG_ERROR,"get buzzer_pwm_ch is fail\n");
	
	ret = of_property_read_u32(np, "buzzer_pwm_freq", &buzzer_data.buzzer_pwm_freq);
	if(ret)
		dprintk(DEBUG_ERROR,"get buzzer_pwm_freq is fail\n");
   
	pwm_period_ns = (int)1000000/(buzzer_data.buzzer_pwm_freq/1000);
	
	buzzer_data.buzzer_pwm_dev = pwm_request(buzzer_data.buzzer_pwm_ch, "buzzer_pwm");
   	if(IS_ERR(buzzer_data.buzzer_pwm_dev))
	{
		dprintk(DEBUG_ERROR,"unable to request pwm\n");
	}
	
//	printk("pwm_period_ns: %d\n", pwm_period_ns);
	
	pwm_set_polarity(buzzer_data.buzzer_pwm_dev,PWM_POLARITY_NORMAL);
	pwm_config(buzzer_data.buzzer_pwm_dev,pwm_period_ns/2, pwm_period_ns);
	
//	pwm_enable(buzzer_data.buzzer_pwm_dev);
	
    misc_register(&buzzer_dev_misc);
	
	return 0;
}


static int buzzer_remove(struct platform_device *pdev)
{
	pwm_free(buzzer_data.buzzer_pwm_dev);
	
    misc_deregister(&buzzer_dev_misc);
    
    return 0; 
}

static const struct of_device_id platform_gpio_match[] = {
	{ .compatible = "worldchip,buzzer-pwm", },
	{},
};

MODULE_DEVICE_TABLE(of, platform_gpio_match);

static struct platform_driver buzzer_driver = {
	.probe		= buzzer_probe,
	.remove		= buzzer_remove,
	.driver		= {
		.name	= "buzzer",
		.owner	= THIS_MODULE,
		.of_match_table = platform_gpio_match,
	},
};

module_platform_driver(buzzer_driver);

MODULE_AUTHOR("yhd");
MODULE_DESCRIPTION("buzzer-pwm");
MODULE_LICENSE("GPL");