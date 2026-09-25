/*
 * driver/gpio/tda7729/tda7729.c
 *
 * audio dsp tda7729 sample code version 1.0
 *
 *  Create Date : 2019/11/26
 *
 * Create by   : wxh
 *
 *
 */
#include "tda7729.h"

#define DEVICE_NAME "tda7729"


static struct tda7729_config {
	u32 tda7729_used;
	u32 twi_id;
	u32 address;
	u32 power_gpio;
} tda7729_config_info;

static struct i2c_client *this_client;
int major;

static struct class *tda7729_class;
static dev_t devid;
static struct cdev *my_cdev;

static const struct i2c_device_id tda7729_id[] = {
	{DEVICE_NAME, 0},
	{}
};

/* #ifdef __DEBUG_ON
#define printk(fmt, arg...) \
	printk(KERR"%s-<%d>: " fmt, __func__, __LINE__, ##arg)
#else
#define printk(...)
#endif */

static int tda7729_i2c_writebyte(u8 addr, u8 para)
{
	int ret;
	u8 buf[3] = {0};
	printk("TDA7729 : reg 0x%x[0x%x]\n",addr, para);
	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr, .flags = 0, .len = 2, .buf = buf,
		},
	};

	buf[0] = addr;
	buf[1] = para;
	ret = i2c_transfer(this_client->adapter, msg, 1);
	return ret;
}

static int aw_i2c_readbyte(u8 addr)
{
	int ret;
	u8 buf[2] = {0};
	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr, .flags = 0, .len = 1, .buf = &buf[0],
		},
		{
			.addr = this_client->addr, .flags = I2C_M_RD, .len = 1, .buf = &buf[1],
		},
	};

	buf[0] = addr;
	ret = i2c_transfer(this_client->adapter, msg, 2);
	return buf[1];
}

static int aw_i2c_readOnebyte(u8 addr)
{
	int ret;
	u8 buf[2] = {0};
	struct i2c_msg msg[] = {
		{
			.addr = this_client->addr, .flags = I2C_M_RD, .len = 2, .buf = buf,
		},
	};

	buf[0] = addr;
	ret = i2c_transfer(this_client->adapter, msg, 1);
	return buf[1];
}

static void tda7729_start(void)
{
	printk("TDA7729 : %s %d\n",__func__, sizeof(tda7729_init_tab));
	int i = 0;
	for(i = 0; i < INIT_TAB_SIZE; i++){
		//printk("TDA7729 : reg 0x%x -- 0x%x\n",tda7729_init_tab[i].reg, tda7729_init_tab[i].data);
		tda7729_i2c_writebyte(tda7729_init_tab[i].reg, tda7729_init_tab[i].data);
	}
/* 	tda7729_i2c_writebyte(0xA0, 0x42);
	tda7729_i2c_writebyte(0xA1, 0x02);
	tda7729_i2c_writebyte(0xA2, 0x52);
	tda7729_i2c_writebyte(0xA3, 0x80);
	tda7729_i2c_writebyte(0xA4, 0x00);
	tda7729_i2c_writebyte(0xA5, 0xFF);
	tda7729_i2c_writebyte(0xA6, 0x00);
	tda7729_i2c_writebyte(0xA7, 0x04);
	tda7729_i2c_writebyte(0xA8, 0x11);
	tda7729_i2c_writebyte(0xA9, 0x1F);
	tda7729_i2c_writebyte(0xAA, 0x1F);
	tda7729_i2c_writebyte(0xAB, 0x1F);
	tda7729_i2c_writebyte(0xAC, 0x80);
	tda7729_i2c_writebyte(0xAD, 0x10);
	tda7729_i2c_writebyte(0xAE, 0x10);
	tda7729_i2c_writebyte(0xAF, 0x10);
	tda7729_i2c_writebyte(0xb0, 0x10);
	tda7729_i2c_writebyte(0xb1, 0x10);
	tda7729_i2c_writebyte(0xb2, 0x10); */
}

static char tda7729_get_init_value(char reg){
	int i = 0;
	for(i = 0; i < INIT_TAB_SIZE; i++){
		if(reg == tda7729_init_tab[i].reg)
			return tda7729_init_tab[i].data;
	}
	return -1;
}

static void tda7729_set_init_value(char reg, char data){
	int i = 0;
	for(i = 0; i < INIT_TAB_SIZE; i++){
		if(reg == tda7729_init_tab[i].reg){
			tda7729_init_tab[i].data = data;
			return 0;
		}
	}
	return -1;
}

static void tda7729_input_config(Input_Config_t cfg){
	char data = tda7729_get_init_value(MAIN_SELECTOR_CONFIG);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",MAIN_SELECTOR_CONFIG);
	}
	char value = (cfg << 5) | (data & 0x1f);
	tda7729_set_init_value(MAIN_SELECTOR_CONFIG, value);
	tda7729_i2c_writebyte(MAIN_SELECTOR_CONFIG, value);
}

static void tda7729_main_selector_source(Selector_Config_t cfg){
	char data = tda7729_get_init_value(MAIN_SELECTOR_CONFIG);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",MAIN_SELECTOR_CONFIG);
	}
	char value = cfg | (data & 0xf8);
	tda7729_set_init_value(MAIN_SELECTOR_CONFIG, value);
	tda7729_i2c_writebyte(MAIN_SELECTOR_CONFIG, value);
}

static void tda7729_main_selector_mute(Mute_t cfg){
	char data = tda7729_get_init_value(SOFT_MUTE);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",SOFT_MUTE);
	}
	char value = cfg | (data & 0xfe);
	tda7729_set_init_value(SOFT_MUTE, value);
	tda7729_i2c_writebyte(SOFT_MUTE, value);
}

static void tda7729_snd_selector_source(Selector_Config_t cfg){
	char data = tda7729_get_init_value(SND_SELECTOR_CONFIG);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",SND_SELECTOR_CONFIG);
	}
	char value = cfg | (data & 0xf8);
	tda7729_set_init_value(SND_SELECTOR_CONFIG, value);
	tda7729_i2c_writebyte(SND_SELECTOR_CONFIG, value);
}

static void tda7729_mix_selector_source(Selector_Config_t cfg){
	char data = tda7729_get_init_value(MIX_SELECTOR_CONFIG);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",MIX_SELECTOR_CONFIG);
	}
	char value = cfg | (data & 0xf8);
	tda7729_set_init_value(MIX_SELECTOR_CONFIG, value);
	tda7729_i2c_writebyte(MIX_SELECTOR_CONFIG, value);
}

static void tda7729_mix_gain(Mix_Gian_t M_Gian){
	char data = tda7729_get_init_value(MIX_SELECTOR_CONFIG);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",MIX_SELECTOR_CONFIG);
	}
	data = (data & 0x7) | (M_Gian << 3);
	tda7729_set_init_value(MIX_SELECTOR_CONFIG, data);
	tda7729_i2c_writebyte(MIX_SELECTOR_CONFIG, data);
}

static void tda7729_mix_to_speak_path(Mix_Path_t M_Path, int onoff){
	char data = tda7729_get_init_value(MIX_CONTROL);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",MIX_CONTROL);
	}
	switch(M_Path){
		case PATH_MIX_FL:
			if(onoff)
				data = data & 0xfe;
			else
				data = data | 0x1;
			break;
		case PATH_MIX_FR:
			if(onoff)
				data = data & 0xfd;
			else
				data = data | 0x2;
			break;
		case PATH_MIX_RL:
			if(onoff)
				data = data & 0xfb;
			else
				data = data | 0x4;
			break;
		case PATH_MIX_RR:
			if(onoff)
				data = data & 0xf7;
			else
				data = data | 0x8;
			break;
		default:
			printk("TDA7729 : %s unknown mix path[%d] to speak\n",M_Path, __func__);
	}
	tda7729_set_init_value(MIX_CONTROL, data);
	tda7729_i2c_writebyte(MIX_CONTROL, data);
}

static void tda7729_set_direct_path(Direct_Path_t D_Path, int onoff){
	char data = tda7729_get_init_value(SND_SELECTOR_CONFIG);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",SND_SELECTOR_CONFIG);
	}
	switch(D_Path){
		case PATH_BYPASS_QD2_FRL:
			if(onoff)
				data = data & 0xdf;
			else
				data = data | 0x20;
			break;
		case PATH_BYPASS_QD3_RLR:
			if(onoff)
				data = data & 0xbf;
			else
				data = data | 0x40;
			break;
		case PATH_BYPASS_QD4_SW:
			if(onoff)
				data = data & 0x7f;
			else
				data = data | 0x80;
			break;
		default:
			printk("TDA7729 : %s unknown direct path[%d]\n",D_Path, __func__);
	}
	tda7729_set_init_value(SND_SELECTOR_CONFIG, data);
	tda7729_i2c_writebyte(SND_SELECTOR_CONFIG, data);
}

static void tda7729_set_speak_source(Speaker_Path_t S_Path){
	char data = tda7729_get_init_value(MIX_CONTROL);
	if(data < 0){
		printk("TDA7729 : unknown init data for reg [0x%x]\n",MIX_CONTROL);
	}
	if(S_Path == PATH_DIRECT_SND_SELECTOR_RLR)
		data = data & 0xef;
	if(S_Path == PATH_MAIN_SELECTOR_RLR)
		data = data | 0x10;
	tda7729_set_init_value(MIX_CONTROL, data);
	tda7729_i2c_writebyte(MIX_CONTROL, data);
}

static void tda7729_selector_init(Input_Config_t I_cfg, Selector_Config_t Main_cfg, Selector_Config_t Mix_cfg, Selector_Config_t Snd_cfg){
	printk("TDA7729 : %s\n",__func__);
	tda7729_input_config(I_cfg);
	tda7729_main_selector_source(Main_cfg);
	tda7729_snd_selector_source(Snd_cfg);
	tda7729_mix_selector_source(Mix_cfg);
}

static int tda7729_probe(struct i2c_client *client,
		const struct i2c_device_id *id)
{
	int err,status,i;
	printk("TDA7729 : %s\n",__func__);
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		err = -ENODEV;
		return err;
	}
	
	this_client = client;
	tda7729_start();
	//配置selector音源
	tda7729_selector_init(INPUT_CFG2, Selector_In1, Selector_In3, Selector_In0);
	//禁止直通模式
	tda7729_set_direct_path(PATH_BYPASS_QD2_FRL, OFF);
	tda7729_set_direct_path(PATH_BYPASS_QD3_RLR, OFF);
	tda7729_set_direct_path(PATH_BYPASS_QD4_SW, OFF);
	//禁止混音模式
	tda7729_mix_to_speak_path(PATH_MIX_FL, OFF);
	tda7729_mix_to_speak_path(PATH_MIX_FR, OFF);
	tda7729_mix_to_speak_path(PATH_MIX_RL, OFF);
	tda7729_mix_to_speak_path(PATH_MIX_RR, OFF);
	//设置r speak音源
	tda7729_set_speak_source(PATH_MAIN_SELECTOR_RLR);
	//tda7729_mix_gain(VOLUME_0DB);
	//tda7729_set_direct_path(PATH_BYPASS_QD2_FRL, ON);
	return 0;
}

static int tda7729_remove(struct i2c_client *client)
{
	i2c_set_clientdata(client, NULL);
	return 0;
}

static const unsigned short normal_i2c[] = {0x88 >> 1, I2C_CLIENT_END};

static int tda7729_detect(struct i2c_client *client,
		struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;

	if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA))
		return -ENODEV;

	if ((tda7729_config_info.twi_id == adapter->nr) &&
			(client->addr == tda7729_config_info.address)) {
		int ret = -1;
		char rdbuf[8];
		struct i2c_msg msgs[] = {
			{
				.addr = client->addr,
				.flags = 0,
				.len = 1,
				.buf = rdbuf,
			},
			{
				.addr = client->addr,
				.flags = I2C_M_RD,
				.len = 1,
				.buf = rdbuf,
			},
		};
		memset(rdbuf, 0, sizeof(rdbuf));
		ret = i2c_transfer(client->adapter, msgs, 2);
		if (ret < 0) {
			pr_err("i2c_transfer error\n");
			return -ENODEV;
		}

		strlcpy(info->type, DEVICE_NAME, I2C_NAME_SIZE);
		printk("detect tda7729 chip\n");

		return 0;
	} else {
		return -ENODEV;
	}
}

#if defined(CONFIG_PM)
static int tda7729_suspend(struct device *dev)
{

	return 0;
}
static int tda7729_resume(struct device *dev)
{
	if (gpio_direction_output(tda7729_config_info.power_gpio, 1) != 0) {
		pr_err("tda7729_resume power_gpio set err!");
		return -1;
	}
	tda7729_start();

	return 0;
}
#endif

static struct dev_pm_ops tda7729_pm_ops = {
	.suspend = tda7729_suspend,
	.resume  = tda7729_resume,
};
MODULE_DEVICE_TABLE(i2c, tda7729_id);

static struct i2c_driver tda7729_driver = {
	.class = I2C_CLASS_HWMON,
	.probe = tda7729_probe,
	.remove = tda7729_remove,

	.id_table = tda7729_id,
	.detect = tda7729_detect,
	.driver = {
		.name = DEVICE_NAME, .owner = THIS_MODULE,
#if defined(CONFIG_PM)
		.pm = &tda7729_pm_ops,
#endif
	},
	.address_list = normal_i2c,
};

static int script_data_init(void)
{
	struct device_node *np = NULL;
	struct gpio_config config;
	int ret;
	printk("TDA7729 : %s\n",__func__);
	np = of_find_node_by_name(NULL, "tda7729");

	if (!np)
		pr_err("ERROR! get TDA7729 failed\n");

	ret = of_property_read_u32(np, "tda7729_twi_id",
			&tda7729_config_info.twi_id);
	if (ret) {
		pr_err("get tda7729_twi_id is fail, %d\n", ret);
		return -1;
	}

	ret = of_property_read_u32(np, "twi_addr",
			&tda7729_config_info.address);
	if (ret) {
		pr_err("get twi_address is fail, %d\n", ret);
		return -1;
	}

	tda7729_config_info.power_gpio =
		of_get_named_gpio_flags(np, "tda7729_power", 0,
				(enum of_gpio_flags *)&config);

	if (!gpio_is_valid(tda7729_config_info.power_gpio)) {
		pr_err(" power_gpio is invalid\n");
		return -1;
	}
	if (gpio_request(tda7729_config_info.power_gpio, NULL) != 0) {
		pr_err("power_gpio_request is failed\n");
		return -1;
	}
	if (gpio_direction_output(tda7729_config_info.power_gpio, 1)
			!= 0) {
		pr_err("power_gpio set err!");
		return -1;
	}

	return 1;
}

static int dsp_ioread_open(struct inode *inode, struct file *filp)
{
	printk("debug: dsp_ioread_open\n");
	return 0;
}
static int dsp_io_release(struct inode *inode, struct file *filp)
{
	printk("my_io_release!\n");
	return 0;
}
static long tda7729_ioctl(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	//unsigned long karg[4];
	unsigned long ubuffer[4] = { 0 };
	printk("%d\n",sizeof(unsigned long));
	struct tda7729_init_data w_data;
	if(copy_from_user((void *)ubuffer, (void __user *)arg, 4 * 4/*sizeof(unsigned long)*/)){
		printk("copy_from_user fail\n");
		return -EFAULT;
	}
/* 	ubuffer[0] = *(unsigned long *)karg; //for main selector 
	ubuffer[1] = (*(unsigned long *)(karg + 1)); // for volume gain
	ubuffer[2] = (*(unsigned long *)(karg + 2)); 
	ubuffer[3] = (*(unsigned long *)(karg + 3)); */
	printk("TDA7729 : cmd : %d, arg[0] = %ld, arg[1] = %ld, arg[2] = %ld, arg[3] = %ld\n",cmd, ubuffer[0],ubuffer[1],ubuffer[2],ubuffer[3]);
	switch (cmd) {
	case CMD_POWER_OFF:
		gpio_direction_output(tda7729_config_info.power_gpio, 0);
		break;
	case CMD_POWER_ON:
		gpio_direction_output(tda7729_config_info.power_gpio, 1);
		break;
	case CMD_WRITE_REG:
		tda7729_i2c_writebyte((char)ubuffer[0], (char)ubuffer[1]);
		break;
	case CMD_READ_REG:
		break;
	case CMD_SELECT_FM:
		tda7729_main_selector_source(FM_SELECTOR_SOURCE);
		break;
	case CMD_SELECT_SPEAK:
		tda7729_main_selector_source(SPEAKER_SELECTOR_SOURCE);
		break;
	case CMD_SELECT_AUX:
		tda7729_main_selector_source(AUX_SELECTOR_SOURCE);
		break;
	case CMD_MAIN_SELECTOR_CONFIG:
		tda7729_main_selector_source((Selector_Config_t)ubuffer[0]);
		break;
	case CMD_BASS_FILTER:
		break;
	case CMD_MIDDLE_FILTER:                                                                                         break;
	case CMD_TREBLE_FILTER:
		break;
	case CMD_VOLUME_GAIN:
		break;
	default:
		break;
	}
	return 0;
}

static long tda7729_ioctrl_compat(struct file *filp,
		unsigned int cmd, unsigned long arg)
{
	unsigned long karg[4];
	unsigned long ubuffer[4] = { 0 };
	
	struct tda7729_init_data w_data;
	if(copy_from_user((void *)karg, (void __user *)arg, 4 * sizeof(unsigned long))){
		printk("copy_from_user fail\n");
		return -EFAULT;
	}
	ubuffer[0] = *(unsigned long *)karg; //for main selector 
	ubuffer[1] = (*(unsigned long *)(karg + 1)); // for volume gain
	ubuffer[2] = (*(unsigned long *)(karg + 2)); // 
	ubuffer[3] = (*(unsigned long *)(karg + 3));
	printk("TDA7729 : cmd : %d, arg[0] = %ld, arg[1] = %ld, arg[2] = %ld, arg[3] = %ld\n",cmd, ubuffer[0],ubuffer[1],ubuffer[2],ubuffer[3]);
	switch (cmd) {
	case CMD_POWER_OFF:
		gpio_direction_output(tda7729_config_info.power_gpio, 0);
		break;
	case CMD_POWER_ON:
		gpio_direction_output(tda7729_config_info.power_gpio, 1);
		break;
	case CMD_WRITE_REG:
		tda7729_i2c_writebyte(w_data.reg, w_data.data);
		break;
	case CMD_READ_REG:
		break;
	case CMD_SELECT_FM:
		tda7729_main_selector_source(FM_SELECTOR_SOURCE);
		break;
	case CMD_SELECT_SPEAK:
		tda7729_main_selector_source(SPEAKER_SELECTOR_SOURCE);
		break;
	case CMD_SELECT_AUX:
		tda7729_main_selector_source(AUX_SELECTOR_SOURCE);
		break;
	case CMD_MAIN_SELECTOR_CONFIG:
		tda7729_main_selector_source((Selector_Config_t)ubuffer[0]);
		break;
	case CMD_BASS_FILTER:
		break;
	case CMD_MIDDLE_FILTER:
		break;
	case CMD_TREBLE_FILTER:
		break;
	case CMD_VOLUME_GAIN:
		break;
	default:
		break;
	}
	return 0;
}

static const struct file_operations dsp_io_fops = {
	.owner  = THIS_MODULE,
	.open  = dsp_ioread_open,
	.unlocked_ioctl = tda7729_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = tda7729_ioctrl_compat,
#endif
	.release = dsp_io_release,
};

static int __init tda7729_init(void)
{
	int ret = -1,err = -1;
	printk("TDA7729 : %s\n",__func__);
	alloc_chrdev_region(&devid, 0, 1, "disp");
	my_cdev = cdev_alloc();
	cdev_init(my_cdev, &dsp_io_fops);
	my_cdev->owner = THIS_MODULE;
	err = cdev_add(my_cdev, devid, 1);
	if (err) {
		printk("TDA7729 : cdev_add fail\n");
		return -1;
	}
	
	tda7729_class = class_create(THIS_MODULE, "tda7729");
	device_create(tda7729_class, NULL, devid, NULL, "tda7729");

	if (script_data_init() > 0)
		ret = i2c_add_driver(&tda7729_driver);

	return ret;
}

static void __exit tda7729_exit(void)
{
	printk("tda7729_exit\n");
	if (tda7729_config_info.power_gpio > 0)
	gpio_free(tda7729_config_info.power_gpio);
	unregister_chrdev(major, "tda7729");
	device_destroy(tda7729_class, MKDEV(major, 0));
	class_destroy(tda7729_class);
	i2c_del_driver(&tda7729_driver);

	return;
}

module_init(tda7729_init);
module_exit(tda7729_exit);

MODULE_AUTHOR("wxh");
MODULE_DESCRIPTION("external asp driver");
MODULE_LICENSE("GPL");
