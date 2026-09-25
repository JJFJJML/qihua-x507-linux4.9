/*
 * A V4L2 driver for nvp6158 cameras.
 *
 * Copyright (c) 2017 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 * Authors: Michael Chen <michaelchen@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/videodev2.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mediabus.h>
#include <media/v4l2-subdev.h>

#include "camera.h"
#include "sensor_helper.h"
//#include "nvp6158_drv.h"
//#include "common.h"

MODULE_AUTHOR("cl");
MODULE_DESCRIPTION("A low-level driver for bt1120 sensors");
MODULE_LICENSE("GPL");

#define MCLK (27 * 1000 * 1000)
#define CLK_POL V4L2_MBUS_PCLK_SAMPLE_FALLING
#define DOUBLE_CLK_POL (V4L2_MBUS_PCLK_SAMPLE_FALLING | V4L2_MBUS_PCLK_SAMPLE_RISING)
#define V4L2_IDENT_SENSOR 0x00a0

/*
 * Our nominal (default) frame rate.
 */
#define SENSOR_FRAME_RATE 30

#define I2C_ADDR 0x88
#define SENSOR_NAME "tp9930_ncsi"

static struct regval_list sensor_regs[] = {

};

typedef struct{
	addr_type nRegAddr;
	/**< Register Address */
	data_type nRegValue;
	/**< data */
	unsigned int nDelay;
	/**< Delay to be applied, after the register is programmed */
} BspUtils_tp9930I2cParams;


static BspUtils_tp9930I2cParams TP9930_720P25_DataSet[] = {
  //video setting
	{0x40, 0x04, 0x00},
	{0x02, 0x4e, 0x00},
	{0x05, 0x00, 0x00},
	{0x06, 0x32, 0x00},
	{0x07, 0xc0, 0x00},
	{0x08, 0x00, 0x00},
	{0x09, 0x24, 0x00},
	{0x0a, 0x48, 0x00},
	{0x0b, 0xc0, 0x00},
	{0x0c, 0x13, 0x00},
	{0x0d, 0x71, 0x00},
	{0x0e, 0x00, 0x00},
	{0x0f, 0x00, 0x00},
	{0x10, 0x00, 0x00},
	{0x11, 0x40, 0x00},
	{0x12, 0x40, 0x00},
	{0x13, 0x00, 0x00},
	{0x14, 0x00, 0x00},
	{0x15, 0x13, 0x00},
	{0x16, 0x16, 0x00},
	{0x17, 0x00, 0x00},
	{0x18, 0x19, 0x00},
	{0x19, 0xd0, 0x00},
	{0x1a, 0x25, 0x00},
	{0x1b, 0x00, 0x00},
	{0x1c, 0x07, 0x00},
	{0x1d, 0xbc, 0x00},
	{0x1e, 0x60, 0x00},
	{0x1f, 0x06, 0x00},
	{0x20, 0x40, 0x00},
	{0x21, 0x46, 0x00},
	{0x22, 0x36, 0x00},
	{0x23, 0x3c, 0x00},
	{0x24, 0x04, 0x00},
	{0x25, 0xfe, 0x00},
	{0x26, 0x01, 0x00},
	{0x27, 0x2d, 0x00},
	{0x28, 0x00, 0x00},
	{0x29, 0x48, 0x00},
	{0x2a, 0x30, 0x00},
	{0x2b, 0x60, 0x00},
	{0x2c, 0x3a, 0x00},
	{0x2d, 0x5a, 0x00},
	{0x2e, 0x40, 0x00},
	{0x2f, 0x06, 0x00},
	{0x30, 0x9e, 0x00},
	{0x31, 0x20, 0x00},
	{0x32, 0x01, 0x00},
	{0x33, 0x90, 0x00},
	{0x35, 0x25, 0x00},
	{0x36, 0xca, 0x00},
	{0x37, 0x00, 0x00},
	{0x38, 0x00, 0x00},
	{0x39, 0x18, 0x00},
	{0x3a, 0x32, 0x00},
	{0x3b, 0x26, 0x00},
	{0x3c, 0x00, 0x00},
	{0x3d, 0x60, 0x00},
	{0x3e, 0x00, 0x00},
	{0x3f, 0x00, 0x00},
	//channel ID
	{0x40, 0x00, 0x00},
	{0x34, 0x10, 0x00},
	{0x40, 0x01, 0x00},
	{0x34, 0x11, 0x00},
	{0x40, 0x02, 0x00},
	{0x34, 0x12, 0x00},
	{0x40, 0x03, 0x00},
	{0x34, 0x13, 0x00},
	//output format
	{0x4f, 0x03, 0x00},
	{0x50, 0xA3, 0x00},
	{0x52, 0xE7, 0x00},		//f5
	{0xf1, 0x04, 0x00},
	{0xf2, 0x77, 0x00},
	{0xf3, 0x77, 0x00},
	{0xf4, 0x00, 0x00},
	{0xf5, 0x0f, 0x00},
	{0xf6, 0x10, 0x00},
	{0xf8, 0x54, 0x00},		//46
	{0xfa, 0x88, 0x00},
	{0xfb, 0x88, 0x00},
	//output enable
	{0x4d, 0x07, 0x00},
	{0x4e, 0x05, 0x00},
};


static int sensor_s_sw_stby(struct v4l2_subdev *sd, int on_off)
{
	if (on_off)
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
	else
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
	return 0;
}

static int sensor_power(struct v4l2_subdev *sd, int on)
{
	switch (on) {
	case STBY_ON:
		sensor_dbg("CSI_SUBDEV_STBY_ON!\n");
		sensor_s_sw_stby(sd, ON);
		break;
	case STBY_OFF:
		sensor_dbg("CSI_SUBDEV_STBY_OFF!\n");
		sensor_s_sw_stby(sd, OFF);
		break;
	case PWR_ON:
		sensor_dbg("CSI_SUBDEV_PWR_ON!\n");
		
		//msleep(30000);
		//printk("sleep over!");
		
		cci_lock(sd);
		vin_gpio_set_status(sd, RESET, CSI_GPIO_HIGH);
		vin_gpio_set_status(sd, PWDN, CSI_GPIO_HIGH);
		vin_set_mclk_freq(sd, MCLK);
		vin_set_mclk(sd, ON);
		vin_set_pmu_channel(sd, CAMERAVDD, ON);
		vin_set_pmu_channel(sd, IOVDD, ON);
		vin_set_pmu_channel(sd, DVDD, ON);
		vin_set_pmu_channel(sd, AVDD, ON);
		usleep_range(100, 120);
		vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
		vin_gpio_write(sd, PWDN, CSI_GPIO_HIGH);
		usleep_range(1000, 1200);
		cci_unlock(sd);
		
		break;
	case PWR_OFF:
		sensor_dbg("CSI_SUBDEV_PWR_OFF!\n");
		cci_lock(sd);
		vin_set_mclk(sd, OFF);
		vin_gpio_set_status(sd, RESET, CSI_GPIO_HIGH);
		vin_set_pmu_channel(sd, CAMERAVDD, OFF);
		vin_set_pmu_channel(sd, IOVDD, OFF);
		vin_set_pmu_channel(sd, DVDD, OFF);
		vin_set_pmu_channel(sd, AVDD, OFF);
		vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
		vin_gpio_set_status(sd, RESET, CSI_GPIO_LOW);
		usleep_range(1000, 1200);
		cci_unlock(sd);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int sensor_reset(struct v4l2_subdev *sd, u32 val)
{
	vin_gpio_write(sd, RESET, CSI_GPIO_LOW);
	usleep_range(5000, 6000);
	vin_gpio_write(sd, RESET, CSI_GPIO_HIGH);
	usleep_range(5000, 6000);
	return 0;
}

static int sensor_detect(struct v4l2_subdev *sd)
{
	data_type rdval;
	
	sensor_read(sd, 0xfe, &rdval);
	printk("sensor id = 0x%x\n", rdval);
	sensor_read(sd, 0xff, &rdval);
	printk("sensor id = 0x%x\n", rdval);

	return 0;
}

static int sensor_init(struct v4l2_subdev *sd, u32 val)
{
	int ret;
	struct sensor_info *info = to_state(sd);

	/*Make sure it is a target sensor */
	ret = sensor_detect(sd);
	if (ret) {
		sensor_err("chip found is not an target chip.\n");
		return ret;
	}

	info->focus_status = 0;
	info->low_speed = 0;
	info->width = 1280; /* VGA_WIDTH; */
	info->height = 720; /* VGA_HEIGHT; */
	info->hflip = 0;
	info->vflip = 0;
	info->gain = 0;

	info->tpf.numerator = 1;
	info->tpf.denominator = 25; /* 30fps */

	info->preview_first_flag = 1;
	return 0;
}

static long sensor_ioctl(struct v4l2_subdev *sd, unsigned int cmd, void *arg)
{
	int ret = 0;
	struct sensor_info *info = to_state(sd);

	switch (cmd) {
	case GET_CURRENT_WIN_CFG:
		if (info->current_wins != NULL) {
			memcpy(arg, info->current_wins,
			       sizeof(struct sensor_win_size));
			ret = 0;
		} else {
			sensor_err("empty wins!\n");
			ret = -1;
		}
		break;
	case SET_FPS:
		break;
	case VIDIOC_VIN_SENSOR_CFG_REQ:
		sensor_cfg_req(sd, (struct sensor_config *)arg);
		break;
	default:
		return -EINVAL;
	}
	return ret;
}

/*
 * Store information about the video data format.
 */
static struct sensor_format_struct sensor_formats[] = {
	{
	.desc = "BT656 4CH",
#if 1 /*BT1120*/
	.mbus_code = MEDIA_BUS_FMT_YUYV8_1X16,
#else /*BT656*/
	.mbus_code = MEDIA_BUS_FMT_UYVY8_2X8,
#endif
	.regs = NULL,
	.regs_size = 0,
	.bpp = 2,
	},
};
#define N_FMTS ARRAY_SIZE(sensor_formats)

/*
 * Then there is the issue of window sizes.  Try to capture the info here.
 */

static struct sensor_win_size sensor_win_sizes[] = {
	{
	.width = 1280,
	.height = 720,
	.hoffset = 0,
	.voffset = 0,
	.fps_fixed = 25,
	.regs = sensor_regs,
	.regs_size = ARRAY_SIZE(sensor_regs),
	.set_size = NULL,
	},
};

#define N_WIN_SIZES (ARRAY_SIZE(sensor_win_sizes))

static int sensor_g_mbus_config(struct v4l2_subdev *sd,
				struct v4l2_mbus_config *cfg)
{
	//struct sensor_info *info = to_state(sd);

	cfg->type = V4L2_MBUS_BT656;

	cfg->flags = CLK_POL | CSI_CH_0 | CSI_CH_1 | CSI_CH_2 | CSI_CH_3;
	/* cfg->flags = CLK_POL | CSI_CH_0; */

	return 0;
}

void TP9930_PLL_Reset(struct v4l2_subdev *sd)
{
    data_type val;
	data_type tmp;

	sensor_write(sd, 0x40, 0x00);
    //output disable
    sensor_write(sd, 0x4d, 0x00);
    sensor_write(sd, 0x4e, 0x00);
    //PLL reset
    sensor_read(sd, 0x44, &val);
    sensor_write(sd, 0x44, val|0x40);
	sensor_read(sd, 0xf4, &tmp);
    sensor_write(sd, 0xf4, tmp|0x80);
    msleep(10);
    sensor_write(sd, 0x44, val);


    sensor_write(sd, 0x40, 0x04);
    sensor_write(sd, 0x3b, 0x20);
    sensor_write(sd, 0x3d, 0xe0);
    sensor_write(sd, 0x3d, 0x60);
    sensor_write(sd, 0x3b, 0x25);
    sensor_write(sd, 0x40, 0x40);
    sensor_write(sd, 0x7a, 0x20);
    sensor_write(sd, 0x3c, 0x20);
    sensor_write(sd, 0x3c, 0x00);
    sensor_write(sd, 0x7a, 0x25);
    sensor_write(sd, 0x40, 0x00);

    sensor_write(sd, 0x44, 0x17);
    sensor_write(sd, 0x43, 0x12);
    sensor_write(sd, 0x45, 0x09);
}


int tp9930_init_hardware(struct v4l2_subdev *sd)
{
	int ret = 0;
	int i;
	data_type val;
	data_type tmp;
	BspUtils_tp9930I2cParams *p = TP9930_720P25_DataSet;		
	int reglen = sizeof(TP9930_720P25_DataSet)/sizeof(TP9930_720P25_DataSet[0]);


	printk("%s + reglen = %d\n", __FUNCTION__, reglen);

	TP9930_PLL_Reset(sd);
	
	for (i = 0; i < reglen; i++) {
		ret = sensor_write(sd, p[i].nRegAddr,  p[i].nRegValue);
		if(ret < 0)
			break;
		//usleep_range(p[i].nDelay, p[i].nDelay);
		
		msleep(1);
	}

	//sensor_write(sd, 0x2a, 0x3c);
    sensor_read(sd, 0x44, &val);
	msleep(1);
    sensor_write(sd, 0x44, val|0x40);
	msleep(1);
	sensor_read(sd, 0xf4, &tmp);
	msleep(1);
    sensor_write(sd, 0xf4, tmp|0x80);
    msleep(10);
    sensor_write(sd, 0x44, val);

	return 0;
}


static int sensor_reg_init(struct sensor_info *info)
{
	struct v4l2_subdev *sd = &info->sd;
	struct sensor_format_struct *sensor_fmt = info->fmt;
	struct sensor_win_size *wsize = info->current_wins;
	
	sensor_write_array(sd, sensor_fmt->regs, sensor_fmt->regs_size);

	if (wsize->regs)
		sensor_write_array(sd, wsize->regs, wsize->regs_size);

	if (wsize->set_size)
		wsize->set_size(sd);

	info->fmt = sensor_fmt;
	info->width = wsize->width;
	info->height = wsize->height;

	tp9930_init_hardware(sd);

	return 0;
}

static int sensor_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct sensor_info *info = to_state(sd);

	sensor_print("%s on = %d, %d*%d %x\n", __func__, enable,
		     info->current_wins->width, info->current_wins->height,
		     info->fmt->mbus_code);

	if (!enable)
		return 0;
	
	return sensor_reg_init(info);
}

/* ----------------------------------------------------------------------- */

static const struct v4l2_subdev_core_ops sensor_core_ops = {
	.reset = sensor_reset,
	.init = sensor_init,
	.s_power = sensor_power,
	.ioctl = sensor_ioctl,
};

static const struct v4l2_subdev_video_ops sensor_video_ops = {
	.s_parm = sensor_s_parm,
	.g_parm = sensor_g_parm,
	.s_stream = sensor_s_stream,
	.g_mbus_config = sensor_g_mbus_config,
};

static const struct v4l2_subdev_pad_ops sensor_pad_ops = {
	.enum_mbus_code = sensor_enum_mbus_code,
	.enum_frame_size = sensor_enum_frame_size,
	.get_fmt = sensor_get_fmt,
	.set_fmt = sensor_set_fmt,
};

static const struct v4l2_subdev_ops sensor_ops = {
	.core = &sensor_core_ops,
	.video = &sensor_video_ops,
	.pad = &sensor_pad_ops,
};

/* ----------------------------------------------------------------------- */
static struct cci_driver cci_drv = {
	.name = SENSOR_NAME,
	.addr_width = CCI_BITS_8,
	.data_width = CCI_BITS_8,
};

static int sensor_probe(struct i2c_client *client,
			const struct i2c_device_id *id)
{

	struct sensor_info *info;
	printk("tp9930_ncsi probe +\n");
	info = kzalloc(sizeof(struct sensor_info), GFP_KERNEL);
	if (info == NULL)
		return -ENOMEM;
	cci_dev_probe_helper(&info->sd, client, &sensor_ops, &cci_drv);
	mutex_init(&info->lock);

	info->fmt = &sensor_formats[0];
	info->fmt_pt = &sensor_formats[0];
	info->win_pt = &sensor_win_sizes[0];
	info->fmt_num = N_FMTS;
	info->win_size_num = N_WIN_SIZES;
	info->sensor_field = V4L2_FIELD_NONE;
	printk("tp9930_ncsi probe -\n");

	return 0;
}

static int sensor_remove(struct i2c_client *client)
{
	struct v4l2_subdev *sd;

	sd = cci_dev_remove_helper(client, &cci_drv);
	kfree(to_state(sd));
	return 0;
}

static const struct i2c_device_id sensor_id[] = {
	{SENSOR_NAME, 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, sensor_id);

static struct i2c_driver sensor_driver = {
	.driver = {
		.owner = THIS_MODULE,
		.name = SENSOR_NAME,
	},
	.probe = sensor_probe,
	.remove = sensor_remove,
	.id_table = sensor_id,
};
static __init int init_sensor(void)
{
	return cci_dev_init_helper(&sensor_driver);
}

static __exit void exit_sensor(void)
{
	cci_dev_exit_helper(&sensor_driver);
}

#ifdef CONFIG_VIDEO_SUNXI_VIN_SPECIAL
subsys_initcall_sync(init_sensor);
#else
module_init(init_sensor);
#endif
module_exit(exit_sensor);































