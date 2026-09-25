#ifndef __TDA7729_H_
#define __TDA7729_H_
#include <linux/i2c.h>
#include <linux/pm_runtime.h>
#ifdef CONFIG_PM
#include <linux/pm.h>
#endif
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/file.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/sunxi-gpio.h>
#include <linux/of_gpio.h>
#include <linux/pm.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define CMD_POWER_OFF				0
#define CMD_POWER_ON				1
#define CMD_WRITE_REG				5
#define CMD_READ_REG				6
#define CMD_SELECT_FM 				7
#define CMD_SELECT_SPEAK			8
#define CMD_SELECT_AUX				9
#define CMD_MAIN_SELECTOR_CONFIG 	10
#define CMD_BASS_FILTER				11
#define CMD_MIDDLE_FILTER 			12
#define CMD_TREBLE_FILTER 			13
#define CMD_VOLUME_GAIN				14

//REG
#define MAIN_SELECTOR_CONFIG 0x20
#define SND_SELECTOR_CONFIG	0x21
#define MIX_SELECTOR_CONFIG 0x22
#define MIX_CONTROL	0x23
#define SOFT_MUTE	0x24
#define SOFT_STEP_I	0x25
#define SOFT_STEP_II 0x26
#define LOUDNESS	0x27
#define VOLUME	0x28
#define TREBLE	0x29
#define MIDDLE	0x2A
#define BASS	0x2B
#define SUBWOOFER	0x2C
#define SPEAKER_LF	0x2D
#define SPEAKER_RF	0x2E
#define SPEAKER_LR	0x2F
#define SPEAKER_RR	0x30
#define SUBWOOFER_L	0x31
#define SUBWOOFER_R	0x32


#define ON 1
#define OFF 0
#define INIT_TAB_SIZE 19

#define DEFAULT_SELECTOR_SOURCE	SPEAKER_SELECTOR_SOURCE
#define FM_SELECTOR_SOURCE	Selector_In0
#define SPEAKER_SELECTOR_SOURCE	Selector_In1
#define AUX_SELECTOR_SOURCE	Selector_In5


struct tda7729_init_data {
	char reg;
	char data;
};

static struct tda7729_init_data tda7729_init_tab[] = {
	{MAIN_SELECTOR_CONFIG, 0x41},
	{SND_SELECTOR_CONFIG, 0xE8},
	{MIX_SELECTOR_CONFIG, 0x3},
	{MIX_CONTROL, 0x9f},
	{SOFT_MUTE, 0xb3},
	{SOFT_STEP_I, 0x0},
	{SOFT_STEP_II, 0x6},
	{LOUDNESS, 0x20},
	{VOLUME, 0x11},
	{TREBLE, 0x54},
	{MIDDLE, 0x4f},
	{BASS, 0x4f},
	{SUBWOOFER, 0x50},
	{SPEAKER_LF, 0x5f},
	{SPEAKER_RF, 0x5f},
	{SPEAKER_LR, 0x10},
	{SPEAKER_RR, 0x10},
	{SUBWOOFER_L, 0x10},
	{SUBWOOFER_R, 0x10}
};

typedef enum{
	INPUT_CFG0 = 0,//4*QD
	INPUT_CFG1, //2*QD+3*SE
	INPUT_CFG2, //1*QD+5*SE
	INPUT_CFG3, //1*QD+3*SE+2*MD
	INPUT_CFG4, //3*QD+1*FD
	INPUT_CFG5, //3*QD+2*SE
	INPUT_CFG6, //1*QD+2*SE+1*FD+1*MD
	INPUT_CFG7 //1*QD+3*SE+1*FD
}Input_Config_t;

typedef enum{
	MUTE_ON = 0,
	MUTE_OFF
}Mute_t;

typedef enum{
	Selector_In0 = 0,
	Selector_In1,
	Selector_In2,
	Selector_In3,
	Selector_In4,
	Selector_In5,
	Selector_In6,
	Selector_In7
}Selector_Config_t;

typedef enum{
	PATH_BYPASS_QD2_FRL = 5,
	PATH_BYPASS_QD3_RLR = 6,
	PATH_BYPASS_QD4_SW = 7,
}Direct_Path_t;

typedef enum{
	PATH_MAIN_SELECTOR_RLR = 1U << 1,
	PATH_DIRECT_SND_SELECTOR_RLR = 1U << 2,
}Speaker_Path_t;

typedef enum{
	PATH_MIX_FL = 0,
	PATH_MIX_FR,
	PATH_MIX_RL,
	PATH_MIX_RR
}Mix_Path_t;

typedef enum{
	VOLUME_0DB = 0,
	VOLUME_1DB,
	VOLUME_2DB,
	VOLUME_3DB,
	VOLUME_4DB,
	VOLUME_5DB,
	VOLUME_6DB,
	VOLUME_7DB,
	VOLUME_8DB,
	VOLUME_9DB,
	VOLUME_10DB,
	VOLUME_11DB,
	VOLUME_12DB,
	VOLUME_13DB,
	VOLUME_14DB,
	VOLUME_15DB,
	VOLUME_16DB,
	VOLUME_17DB,
	VOLUME_18DB,
	VOLUME_19DB,
	VOLUME_20DB,
	VOLUME_21DB,
	VOLUME_22DB,
	VOLUME_23DB,
	VOLUME_24DB,
	VOLUME_25DB,
	VOLUME_26DB,
	VOLUME_27DB,
	VOLUME_28DB,
	VOLUME_29DB,
	VOLUME_30DB,
	VOLUME_31DB
}Mix_Gian_t;

#endif