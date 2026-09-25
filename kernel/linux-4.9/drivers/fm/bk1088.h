#ifndef __BK1088__
#define __BK1088__

#define BK1088_CHIP

//Chip tag
#define CHIP_ADDR 0xc0  //0x60<<1
#define READ_OP 0x20

//BK108X register map
#define REG0_DEVICE_ID 0x00
#define REG1_CHIP_ID   0x01
#define REG2_POWER_CFG 0x02
#define REG3_CHANNEL   0x03
#define REG4_SYS_CONF1 0x04
#define REG5_SYS_CONF2 0x05
#define REG6_SYS_CONF3 0x06

#define REG7_TEST1     0x07
#define REG8_TEST2     0x08
#define REG9_BOOT_CFG  0x09
#define REGa_STATUS_RSSI  0x0a
#define RDSR_BIT 15
#define STC_BIT 14
#define SF_BL_BIT 13
#define AFCRL_BIT 12
#define REGb_READ_CHAN  0x0b
#define MAX_FREQ_DEVIATION 0x1ff

typedef struct __FM_SETTING_INFO
{
    __s32   min_freq;
    __s32   max_freq;
    __s8    area_select;
    __s8    stereo_select;
    __s8    signal_level;
    __s8    reserved;

}__fm_setting_info_t;

#endif