#include <linux/errno.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/regulator/consumer.h>

#define CHAR_DEV_NAME "gps_power_ctrl"

static dev_t devid;
static struct cdev *my_cdev;
static struct class *gps_power_class;
static struct device *device;
static struct regulator *gps_power;
const char *power_name = NULL;

static int gps_power_on_off(bool on_off){
    int ret = 0;

    if (power_name) {
		gps_power = regulator_get(NULL, power_name);
		if (!IS_ERR(gps_power)) {
			if (on_off) {
				ret = regulator_enable(gps_power);
				if (ret < 0) {
					printk("regulator gps_power enable failed\n");
					regulator_put(gps_power);
					return ret;
				}

				ret = regulator_get_voltage(gps_power);
				if (ret < 0) {
					printk("regulator gps_power get voltage failed\n");
					regulator_put(gps_power);
					return ret;
				}
				printk("check gps_power voltage: %d\n", ret);
			} else {
				ret = regulator_disable(gps_power);
				if (ret < 0) {
					printk("regulator gps_power disable failed\n");
					regulator_put(gps_power);
					return ret;
				}
			}
			regulator_put(gps_power);
		}
        else{
            printk("gps_power error\n");
            return -1;
        }
	}
    else{
        printk("%s:no gps power find!\n", __func__);
        return -1;
    }

    return 0;
}


static int gps_power_open(struct inode *inode, struct file *filp){
    printk("enter %s\n", __func__);
    return gps_power_on_off(1);
}

static int gps_power_close(struct inode *inode, struct file *filp){
    printk("enter %s\n", __func__);
    return gps_power_on_off(0);
}

static const struct file_operations gps_power_ops = {
	.owner  = THIS_MODULE,
	.open  = gps_power_open,
	.release = gps_power_close,
};

static int __init gps_power_ctrl_init(void)
{
    int err = -1;
    struct device_node *np;

	printk("gps_power_ctrl : %s\n",__func__);

    np = of_find_node_by_path("/soc@03000000/gps_power");
    if(!np){
        return -EINVAL;
    }

    if (of_property_read_string(np, "gps_power", &power_name)) {
		printk("Missing gps_power.\n");
        power_name = NULL;
	}

	alloc_chrdev_region(&devid, 0, 1, CHAR_DEV_NAME);

	my_cdev = cdev_alloc();
    my_cdev->owner = THIS_MODULE;
	cdev_init(my_cdev, &gps_power_ops);
	
	err = cdev_add(my_cdev, devid, 1);
	if (err) {
		printk("gps power ctrl : cdev_add fail\n");
		return -1;
	}
	
	gps_power_class = class_create(THIS_MODULE, CHAR_DEV_NAME);
    if (IS_ERR(gps_power_class)) {
        return PTR_ERR(gps_power_class);
    }

	device = device_create(gps_power_class, NULL, devid, NULL, CHAR_DEV_NAME);
    if(IS_ERR(device)){
        return PTR_ERR(device);
    }

    // msleep(500);
    // while(1){
    //     gps_power_on_off(1);
    //     printk("power on\n");
    //     msleep(500);
    //     gps_power_on_off(0);
    //     printk("power off\n");
    //     msleep(500);
    // }

	return 0;
}

static void __exit gps_power_ctrl_exit(void)
{
	printk("gps_power_ctrl_exit\n");

    cdev_del(my_cdev);
    unregister_chrdev_region(devid, 1);
    device_destroy(gps_power_class, devid);
	class_destroy(gps_power_class);
}

module_init(gps_power_ctrl_init);
module_exit(gps_power_ctrl_exit);

MODULE_AUTHOR("tzw");
MODULE_DESCRIPTION("gps power ctrl driver");
MODULE_LICENSE("GPL");