/*
 * stk8baxx.c - Linux driver for sensortek stk8baxx accelerometer
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mutex.h>
//#include <linux/earlysuspend.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/miscdevice.h>

#include "../init-input.h"
//#include <mach/hardware.h>
#include <linux/fs.h>


/*****************************************************************************
 * Config table
 *****************************************************************************/

/*****************************************************************************
 * stk8baxx register, start
 *****************************************************************************/
#define STK_REG_CHIPID          0x00
#define STK_REG_XOUT1           0x02
#define STK_REG_XOUT2           0x03
#define STK_REG_YOUT1           0x04
#define STK_REG_YOUT2           0x05
#define STK_REG_ZOUT1           0x06
#define STK_REG_ZOUT2           0x07
#define STK_REG_INTSTS1         0x09
#define STK_REG_RANGESEL        0x0F
#define STK_REG_BWSEL           0x10
#define STK_REG_POWMODE         0x11
#define STK_REG_SWRST           0x14
#define STK_REG_INTEN1          0x16
#define STK_REG_INTEN2          0x17
#define STK_REG_INTMAP1         0x19
#define STK_REG_INTMAP2         0x1A
#define STK_REG_INTCFG1         0x20
#define STK_REG_INTCFG2         0x21
#define STK_REG_SLOPEDLY        0x27
#define STK_REG_SLOPETHD        0x28
#define STK_REG_SIGMOT1         0x29
#define STK_REG_SIGMOT2         0x2A
#define STK_REG_SIGMOT3         0x2B
#define STK_REG_INTFCFG         0x34
#define STK_REG_OFSTCOMP1       0x36
#define STK_REG_OFSTX           0x38
#define STK_REG_OFSTY           0x39
#define STK_REG_OFSTZ           0x3A

/* STK_REG_CHIPID */
#define STK8BA50_R_ID                   0x86
#define STK8BA53_ID                     0x87

/* STK_REG_INTSTS1 */
#define STK_INTSTS1_SIG_MOT_STS         0x1
#define STK_INTSTS1_ANY_MOT_STS         0x4

/* STK_REG_RANGESEL */
#define STK_RANGESEL_2G                 0x3
#define STK_RANGESEL_4G                 0x5
#define STK_RANGESEL_8G                 0x8
#define STK_RANGESEL_BW_MASK            0xF
#define STK_RANGESEL_DEF                STK_RANGESEL_2G
typedef enum
{
    STK_2G = STK_RANGESEL_2G,
    STK_4G = STK_RANGESEL_4G,
    STK_8G = STK_RANGESEL_8G
} stk_rangesel;

/* STK_REG_BWSEL */
#define STK_BWSEL_INIT_ODR              0x0A    /* ODR = BW x 2 = 62.5Hz */
/* ODR: 31.25, 62.5, 125 */
const static int STK_SAMPLE_TIME[] = {32000, 16000, 8000}; /* usec */
#define STK_SPTIME_BASE                 0x9     /* for 32000, ODR:31.25 */
#define STK_SPTIME_BOUND                0xB     /* for 8000, ODR:125 */

/* STK_REG_POWMODE */
#define STK_PWMD_SUSPEND                0x80
#define STK_PWMD_LOWPOWER               0x40
#define STK_PWMD_NORMAL                 0x00
#define STK_PWMD_SLP_MASK               0x3E

/* STK_REG_SWRST */
#define STK_SWRST_VAL                   0xB6

/* STK_REG_INTEN1 */
#define STK_INTEN1_SLP_EN_XYZ           0x07

/* STK_REG_INTEN2 */
#define STK_INTEN2_DATA_EN              0x10

/* STK_REG_INTMAP1 */
#define STK_INTMAP1_SIGMOT2INT1         0x01
#define STK_INTMAP1_ANYMOT2INT1         0x04

/* STK_REG_INTMAP2 */
#define STK_INTMAP2_DATA2INT1           0x01

/* STK_REG_INTCFG1 */
#define STK_INTCFG1_INT1_ACTIVE_H       0x01
#define STK_INTCFG1_INT1_OD_PUSHPULL    0x00


/* STK_REG_INTCFG2 */
#define STK_INTCFG2_NOLATCHED           0x00
#define STK_INTCFG2_LATCHED             0x0F
#define STK_INTCFG2_INT_RST             0x80

/* STK_REG_SLOPETHD */
#define STK_SLOPETHD_DEF                0x14

/* STK_REG_SIGMOT1 */
#define STK_SIGMOT1_SKIP_TIME_3SEC      0x96    /* default value */

/* STK_REG_SIGMOT2 */
#define STK_SIGMOT2_SIG_MOT_EN          0x02
#define STK_SIGMOT2_ANY_MOT_EN          0x04

/* STK_REG_SIGMOT3 */
#define STK_SIGMOT3_PROOF_TIME_1SEC     0x32    /* default value */

/* STK_REG_INTFCFG */
#define STK_INTFCFG_I2C_WDT_EN          0x04

/* STK_REG_OFSTCOMP1 */
#define STK_OFSTCOMP1_OFST_RST          0x80

/* STK_REG_OFSTx */
#define STK_OFST_LSB                    128     /* 8 bits for +-1G */
/*****************************************************************************
 * stk8baxx register, end
 *****************************************************************************/
/*****************************************************************************
 * Variables, Macro
 *****************************************************************************/
#define STK_ACC_DRIVER_VERSION  "0.0.1"
#define STK_ACC_TAG             "[stk_accel]"
#define STK_NAME                "stk8baxx"
#define STK_SLAVE_ADDRESS       0x18

#define STK_PRINT_FUN()             \
    printk(KERN_INFO STK_ACC_TAG" %s\n", __func__)
#define STK_PRINT_ERR(fmt, ...) \
    printk(KERN_ERR STK_ACC_TAG" %s %d: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)
#define STK_PRINT_LOG(fmt, ...) \
    printk(KERN_INFO STK_ACC_TAG" %s %d: "fmt"\n", __func__, __LINE__, ## __VA_ARGS__)

#define INPUT_FUZZ  0
#define INPUT_FLAT  0

// if use i2c detect, set 1
#define USE_DETECT   0
static int gUseDetect;
static int startup(void);

static struct sensor_config_info gsensor_info =  {
    .input_type = GSENSOR_TYPE,
};

static __u32 twi_id = 0;
static const unsigned short normal_i2c[2] = {STK_SLAVE_ADDRESS, I2C_CLIENT_END};

struct stk_data {
    /* platform related */
    struct i2c_client *client;
    /* system operation */
    struct mutex reg_lock;          /* mutex lock for register R/W */
    int pid;                        /* PID */
    struct workqueue_struct *wq;    /* workqueue */
    struct work_struct accel_work;
    struct hrtimer accel_hrtimer;
    struct input_dev *input_dev;    /* Input device */
    atomic_t enabled;               /* chip is enabled or not */
    u8 power_mode;
    int sensitivity;                /* sensitivity, bit number per G */
    s16 xyz[3];                     /* The latest data of xyz */
    int sampleRateUs;               /* Sample rate in us */
};
/*****************************************************************************
 * Driver
 *****************************************************************************/
/**
 * @brief: Register writing via I2C
 *
 * @param[in] stk: struct stk_data *
 * @param[in] reg: Register address
 * @param[in] val: Value, what you want to write.
 *
 * @return: Success or fail
 *      0: Success
 *      others: Fail
 */
static int stk_reg_write(struct stk_data *stk, u8 reg, u8 val) {
    int ret = 0;

    mutex_lock(&stk->reg_lock);
    ret = i2c_smbus_write_byte_data(stk->client, reg, val);
    mutex_unlock(&stk->reg_lock);

    if (ret)
    {
        STK_PRINT_ERR("transfer failed to write reg:0x%X with val:0x%X, error=%d",
                reg, val, ret);
    }

    return ret;
}

/**
 * @brief: Register reading via I2C.
 *
 * @param[in] stk: struct stk_data *
 * @param[in] reg: Register address
 * @param[in] len: 0/1, for normal usage.
 * @param[in] val: Value, the point address what you want to read.
 */
static int stk_reg_read(struct stk_data *stk, u8 reg, int len, u8 *val) {
    int ret = 0;
    struct i2c_msg msgs[2] = {
        {
            .addr = stk->client->addr,
            .flags = 0,
            .len = 1,
            .buf = &reg
        },
        {
            .addr = stk->client->addr,
            .flags = I2C_M_RD,
            .len = (0 >= len) ? 1 : len,
            .buf = val
        }
    };
    mutex_lock(&stk->reg_lock);
    ret = i2c_transfer(stk->client->adapter, msgs, 2);
    mutex_unlock(&stk->reg_lock);

    if (2 == ret) {
        return 0;
    } else if (0 > ret) {
        STK_PRINT_ERR("transfer failed to read reg:0x%X with len:%d, error=%d",
                reg, len, ret);
        return ret;
    } else {
        STK_PRINT_ERR("size error in reading reg:0x%X with len:%d, error=%d",
                reg, len, ret);
        return -1;
    }
}

/**
 * @brief: Read PID and write to stk_data.pid
 *
 * @param[in/out] stk: struct stk_data *
 *
 * @return: Success or fail
 *      0: Success
 *      others: Fail
 */
static int stk_get_pid(struct stk_data *stk)
{
    int ret = 0;
    u8 val = 0;

    ret = stk_reg_read(stk, STK_REG_CHIPID, 0, &val);

    if (ret) {
        STK_PRINT_ERR("falied to read PID");
        stk->pid = 0;
        return ret;
    }

    stk->pid = (int)val;
    STK_PRINT_LOG("PID 0x%X", stk->pid);
    return 0;
}

/*
 * @brief: SW reset for stk accel
 *
 * @param[in/out] stk: struct stk_data *
 *
 * @return: Success or fail.
 *          0: Success
 *          others: Fail
 */
static int stk_sw_reset(struct stk_data *stk)
{
    int ret = 0;
    ret = stk_reg_write(stk, STK_REG_SWRST, STK_SWRST_VAL);

    if (ret)
        return ret;

    usleep_range(1000, 2000);
    stk->power_mode = STK_PWMD_NORMAL;
    atomic_set(&stk->enabled, 1);
    return 0;
}

/*
 * @brief: Turn ON/OFF the power state of IC.
 *
 * @param[in/out] stk: struct stk_data *
 * @param[in] en: turn ON/OFF
 *              0 for suspend mode;
 *              1 for normal mode.
 */
static void stk_set_enable(struct stk_data *stk, char en)
{
    if (en == atomic_read(&stk->enabled))
        return;

    if (en)
    {
        if (stk_reg_write(stk, STK_REG_POWMODE, STK_PWMD_NORMAL))
            return;

        hrtimer_start(&stk->accel_hrtimer, ktime_set(0, stk->sampleRateUs*1000), HRTIMER_MODE_REL);
    }
    else
    {
        if (stk_reg_write(stk, STK_REG_POWMODE, STK_PWMD_SUSPEND))
            return;

        hrtimer_cancel(&stk->accel_hrtimer);
    }
    atomic_set(&stk->enabled, en);
}

/*
 * @brief: Get sensitivity. Set result to stk_data.sensitivity.
 *          sensitivity = number bit per G (LSB/g)
 *          Example: RANGESEL=8g, 12 bits for STK8BAXX full resolution
 *          Ans: number bit per G = 2^12 / (8x2) = 256 (LSB/g)
 *
 * @param[in/out] stk: struct stk_data *
 */
static void stk_get_sensitivity(struct stk_data *stk)
{
    u8 val = 0;
    stk->sensitivity = 0;

    if ( 0 == stk_reg_read(stk, STK_REG_RANGESEL, 0, &val))
    {
        val &= STK_RANGESEL_BW_MASK;

        switch (val)
        {
            case STK_RANGESEL_2G:
                stk->sensitivity = 1024;
                break;

            case STK_RANGESEL_4G:
                stk->sensitivity = 512;
                break;

            case STK_RANGESEL_8G:
                stk->sensitivity = 256;
                break;

            default:
                break;
        }
    }

    if (STK8BA50_R_ID == stk->pid) {
        stk->sensitivity /= 4;
    }
}

/*
 * @brief: Set range
 *          1. Setting STK_REG_RANGESEL
 *          2. Calculate sensitivity and store to stk_data.sensitivity
 *
 * @param[in/out] stk: struct stk_data *
 * @param[in] range: range for STK_REG_RANGESEL
 *              STK_2G
 *              STK_4G
 *              STK_8G
 *
 * @return: Success or fail
 *          0: Success
 *          others: Fail
 */
static int stk_range_selection(struct stk_data *stk, stk_rangesel range)
{
    int ret = 0;
    ret = stk_reg_write(stk, STK_REG_RANGESEL, range);

    if (ret)
        return ret;

    stk_get_sensitivity(stk);
    return 0;
}

/**
 * @brief: Set BW
 *
 * @param[in/out] stk: struct stk_data *
 * @param[in] sampleRateUs: Sample rate in usec
 *
 * @return: Success or fail
 *      0: Success
 *      others: Fail
 */
static int stk_odr_set(struct stk_data *stk, int sampleRateUs)
{
    int ret = 0;
    unsigned char sr_no;

    for (sr_no = 0; sr_no <= STK_SPTIME_BOUND - STK_SPTIME_BASE; sr_no++) {
        if (sampleRateUs >= STK_SAMPLE_TIME[sr_no])
            break;
    }

    if (STK_SPTIME_BOUND - STK_SPTIME_BASE + 1 <= sr_no) {
        sr_no = STK_SPTIME_BOUND - STK_SPTIME_BASE;
    }

    ret = stk_reg_write(stk, STK_REG_BWSEL, sr_no + STK_SPTIME_BASE);

    if (ret) {
        STK_PRINT_ERR("Failed to change ODR");
        return ret;
    }

    stk->sampleRateUs = STK_SAMPLE_TIME[sr_no];

    STK_PRINT_LOG("ODR change success, sampleRateUs=%d, sr_no=%d",
            stk->sampleRateUs, sr_no);
    return 0;
}

/**
 * @brief: read accel raw data from register.
 *
 * @param[in/out] stk: struct stk_data *
 */
static void stk_read_accel_rawdata(struct stk_data *stk)
{
    u8 dataL = 0;
    u8 dataH = 0;

    if (stk_reg_read(stk, STK_REG_XOUT1, 0, &dataL))
        return;

    if (stk_reg_read(stk, STK_REG_XOUT2, 0, &dataH))
        return;

    stk->xyz[0] = dataH << 8 | dataL;
    if (STK8BA53_ID == stk->pid) {
        stk->xyz[0] >>= 4;
    } else {
        stk->xyz[0] >>= 6;
    }

    if (stk_reg_read(stk, STK_REG_YOUT1, 0, &dataL))
        return;

    if (stk_reg_read(stk, STK_REG_YOUT2, 0, &dataH))
        return;

    stk->xyz[1] = dataH << 8 | dataL;
    if (STK8BA53_ID == stk->pid) {
        stk->xyz[1] >>= 4;
    } else {
        stk->xyz[1] >>= 6;
    }

    if (stk_reg_read(stk, STK_REG_ZOUT1, 0, &dataL))
        return;

    if (stk_reg_read(stk, STK_REG_ZOUT2, 0, &dataH))
        return;

    stk->xyz[2] = dataH << 8 | dataL;
    if (STK8BA53_ID == stk->pid) {
        stk->xyz[2] >>= 4;
    } else {
        stk->xyz[2] >>= 6;
    }
}

/**
 * @brief: Input device setup for accel
 *
 * @param[in] stk: struct stk_data *
 *
 * @return: Success or fail
 *      0: Success
 *      others: Fail
 */
static int stk_input_setup(struct stk_data *stk)
{
    int ret = 0;

    stk->input_dev = input_allocate_device();

    if (!stk->input_dev) {
        STK_PRINT_ERR("input_allocate_device failed");
        return -ENOMEM;
    }

    STK_PRINT_LOG(" 111"); // vivien add for debug
    stk->input_dev->name = STK_NAME;
    stk->input_dev->id.bustype = BUS_I2C;
    input_set_capability(stk->input_dev, EV_ABS, ABS_MISC);
    input_set_abs_params(stk->input_dev, ABS_X, -32*8, 32*8, INPUT_FUZZ, INPUT_FLAT);
    input_set_abs_params(stk->input_dev, ABS_Y, -32*8, 32*8, INPUT_FUZZ, INPUT_FLAT);
    input_set_abs_params(stk->input_dev, ABS_Z, -32*8, 32*8, INPUT_FUZZ, INPUT_FLAT);
    STK_PRINT_LOG(" 222"); // vivien add for debug
    input_set_drvdata(stk->input_dev, stk);
    STK_PRINT_LOG(" 333"); // vivien add for debug

    ret = input_register_device(stk->input_dev);
    if (ret) {
        STK_PRINT_ERR("Unable to register input device: %s",
                stk->input_dev->name);
        input_free_device(stk->input_dev);
        return ret;
    }

    return 0;
}

#ifndef CONFIG_HAS_EARLYSUSPEND
/**
 */
static int stk_suspend(struct device *dev, pm_message_t mesg) {
    struct i2c_client *client = to_i2c_client(dev);
    struct stk_data *stk = i2c_get_clientdata(client);

    hrtimer_cancel(&stk->accel_hrtimer);

    if (atomic_read(&stk->enabled)) {
        stk_reg_write(stk, STK_REG_POWMODE, STK_PWMD_SUSPEND);
    }

    STK_PRINT_FUN();
    return 0;
}

/**
 */
static int stk_resume(struct device *dev) {
    struct i2c_client *client = to_i2c_client(dev);
    struct stk_data *stk = i2c_get_clientdata(client);

    if (atomic_read(&stk->enabled)) {
        stk_reg_write(stk, STK_REG_POWMODE, STK_PWMD_NORMAL);
    }

    hrtimer_start(&stk->accel_hrtimer, ktime_set(1, 0), HRTIMER_MODE_REL);

    STK_PRINT_FUN();
    return 0;
}
#endif /* CONFIG_HAS_EARLYSUSPEND */

/*
 * @brief: Initialize some data in stk_data.
 *
 * @param[in/out] stk: struct stk_data *
 */
static void stk_data_initialize(struct stk_data *stk)
{
    atomic_set(&stk->enabled, 0);
    stk->power_mode = STK_PWMD_NORMAL;
    STK_PRINT_LOG("stk_data_initialize done.");
}

/*
 * @brief: stk accel register initialize
 *
 * @param[in/out] stk: struct stk_data *
 *
 * @return: Success or fail.
 *          0: Success
 *          others: Fail
 */
static int stk_reg_init(struct stk_data *stk)
{
    int ret = 0;

    /* SW reset */
    ret = stk_sw_reset(stk);

    if (ret) {
        return ret;
    }

    /* SIGMOT2 */
    ret = stk_reg_write(stk, STK_REG_SIGMOT2, 0);

    if (ret) {
        return ret;
    }

    ret = stk_range_selection(stk, STK_2G);

    if (ret) {
        return ret;
    }

    ret = stk_odr_set(stk, STK_SAMPLE_TIME[STK_BWSEL_INIT_ODR - STK_SPTIME_BASE]);

    if (ret) {
        return ret;
    }

    /* i2c watchdog enable */
    ret = stk_reg_write(stk, STK_REG_INTFCFG, STK_INTFCFG_I2C_WDT_EN);

    if (ret) {
        return ret;
    }

    return 0;
}

/*
 * @brief: Report accel data to /sys/class/input/inputX/capabilities/rel
 *
 * @param[in/out] stk: struct stk_data *
 */
static void stk_report_accel_data(struct stk_data *stk)
{
    if (!stk->input_dev)
    {
        STK_PRINT_ERR("No input device for accel data");
        return;
    }
	 
   // input_report_rel(stk->input_dev, ABS_X, (int)stk->xyz[0]);
  //  input_report_rel(stk->input_dev, ABS_Y, (int)stk->xyz[1]);
   // input_report_rel(stk->input_dev, ABS_Z, (int)stk->xyz[2]);
   
   input_report_abs(stk->input_dev, ABS_X, (int)stk->xyz[0]);
   input_report_abs(stk->input_dev, ABS_Y, (int)stk->xyz[1]);
   input_report_abs(stk->input_dev, ABS_Z, (int)stk->xyz[2]);
    input_sync(stk->input_dev);
	
}

/*
 * @brief: This function will send stk->accel_work(stk_accel_wq) to queue.
 *
 * @param[in] timer: struct hrtimer *
 *
 * @return: HRTIMER_RESTART.
 */
static enum hrtimer_restart stk_timer_func(struct hrtimer *timer)
{
    struct stk_data *stk = container_of(timer, struct stk_data, accel_hrtimer);

    queue_work(stk->wq, &stk->accel_work);
    hrtimer_start(&stk->accel_hrtimer, ktime_set(0, stk->sampleRateUs*1000), HRTIMER_MODE_REL);
    return HRTIMER_RESTART;
}

/*
 * @brief: Queue work list.
 *          1. Read accel data, then report to userspace.
 *
 * @param[in] work: struct work_struct *
 */
static void stk_accel_wq(struct work_struct *work)
{
    struct stk_data *stk = container_of(work, struct stk_data, accel_work);

    stk_read_accel_rawdata(stk);
    stk_report_accel_data(stk);
}

/**
 * @brief: Get power status
 *          Send 0 or 1 to userspace.
 *
 * @param[in] dev: struct device *
 * @param[in] attr: struct device_attribute *
 * @param[in/out] buf: char *
 *
 * @return: ssize_t
 */
static ssize_t stk_enable_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct stk_data *stk = dev_get_drvdata(dev);
    char en = (char)atomic_read(&stk->enabled);

    return sprintf(buf, "%d\n", en);
}

/**
 * @brief: Set power status
 *          Get 0 or 1 from userspace, then set stk power status.
 *
 * @param[in] dev: struct device *
 * @param[in] attr: struct device_attribute *
 * @param[in/out] buf: char *
 * @param[in] count: size_t
 *
 * @return: ssize_t
 */
static ssize_t stk_enable_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    struct stk_data *stk = dev_get_drvdata(dev);
    unsigned long data;

    if (NULL == buf) {
        STK_PRINT_ERR("NULL == buf");
        return -1;
    }

    data = simple_strtoul(buf, NULL, 10);
    if (0 < data) {
        stk_set_enable(stk, 1);
    } else {
        stk_set_enable(stk, 0);
    }

    return count;
}

/**
 * @brief: Get delay value in usec
 *          Send delay in usec to userspce.
 *
 * @param[in] dev: struct device *
 * @param[in] attr: struct device_attribute *
 * @param[in/out] buf: char *
 *
 * @return: ssize_t
 */
static ssize_t stk_delay_show(struct device *dev,
        struct device_attribute *attr, char *buf)
{
    struct stk_data *stk = dev_get_drvdata(dev);
    return sprintf(buf, "%dus\n", stk->sampleRateUs);
}

/**
 * @brief: Set delay value in usec
 *          Get delay value in usec from userspace, then write to register.
 *
 * @param[in] dev: struct device *
 * @param[in] attr: struct device_attribute *
 * @param[in/out] buf: char *
 * @param[in] count: size_t
 *
 * @return: ssize_t
 */
static ssize_t stk_delay_store(struct device *dev,
        struct device_attribute *attr, const char *buf, size_t count)
{
    struct stk_data *stk = dev_get_drvdata(dev);
    unsigned long data;

    if (NULL == buf) {
        STK_PRINT_ERR("NULL == buf");
        return -1;
    }

    data = simple_strtoul(buf, NULL, 10);
    stk_odr_set(stk, (int)data);
    return count;
}

static DEVICE_ATTR(enable, 0660, stk_enable_show, stk_enable_store);
static DEVICE_ATTR(delay, 0660, stk_delay_show, stk_delay_store);

static struct attribute *stk_attributes[] = 
{
    &dev_attr_enable.attr,
    &dev_attr_delay.attr,
    NULL
};

static struct attribute_group stk_attr_group =  {
    .attrs = stk_attributes,
};

/**
 * @brief: Proble function for i2c_driver
 *
 * @param[in] client: struct i2c_client *
 * @param[in] id: struct i2c_device_id *
 *
 * @return: Success or fail
 *      0: Success
 *      other: Fail
 */
static int stk_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    int ret = 0;
    struct stk_data *stk = NULL;

	if (gsensor_info.dev == NULL)
		gsensor_info.dev = &client->dev;
#if !USE_DETECT
	if (!gUseDetect && startup() != 0)
		return 0;
#endif

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        STK_PRINT_ERR("i2c_check_functionality failed");
        return -ENODEV;
    }

    stk = kzalloc(sizeof(struct stk_data), GFP_KERNEL);
    if (NULL == stk)
    {
        STK_PRINT_ERR("memory allocation error");
        return -ENOMEM;
    }

    stk->client = client;
    i2c_set_clientdata(client, stk);
    mutex_init(&stk->reg_lock);

    /* Check PID */
    if (stk_get_pid(stk)) {
        goto err_free_mem;
    }
    if (STK8BA50_R_ID != stk->pid
            && STK8BA53_ID != stk->pid) {
        ret = -EINVAL;
        STK_PRINT_ERR("chip id not match");
        goto err_free_mem;
    }

    stk_data_initialize(stk);

    /* Register initialize */
    if (stk_reg_init(stk))
    {
        STK_PRINT_ERR("stk_reg_init failed");
        goto err_free_mem;
    }

    if (stk_input_setup(stk)) {
        STK_PRINT_ERR("stk_input_setup failed");
        goto err_free_mem;
    }

    /* sysfs: creat file system */
    ret = sysfs_create_group(&stk->input_dev->dev.kobj, &stk_attr_group);
    if (ret) {
        STK_PRINT_ERR("sysfs_create_group failed");
        goto err_sysfs;
    }

    /* polling data usage */
    stk->wq = create_singlethread_workqueue("stk_wq");
    if (!stk->wq) {
        STK_PRINT_ERR("create_singlethread_workqueue failed");
        ret = -ENOMEM;
        goto err_create_thread;
    }
    INIT_WORK(&stk->accel_work, stk_accel_wq);
    hrtimer_init(&stk->accel_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
    stk->accel_hrtimer.function = stk_timer_func;
    hrtimer_start(&stk->accel_hrtimer, ktime_set(1, 0), HRTIMER_MODE_REL);

    /* Success */
    return 0;

    /* Error occur */
    cancel_work_sync(&stk->accel_work);
    destroy_workqueue(stk->wq);
err_create_thread:
    sysfs_remove_group(&stk->input_dev->dev.kobj,
                       &stk_attr_group);
err_sysfs:
    input_free_device(stk->input_dev);
    input_unregister_device(stk->input_dev);
err_free_mem:
    mutex_destroy(&stk->reg_lock);
    kfree(stk);
    return ret;
}

/**
 */
static int stk_remove(struct i2c_client *client) {
    struct stk_data *stk = i2c_get_clientdata(client);

    cancel_work_sync(&stk->accel_work);
    destroy_workqueue(stk->wq);
    sysfs_remove_group(&stk->input_dev->dev.kobj,
                       &stk_attr_group);
    input_free_device(stk->input_dev);
    input_unregister_device(stk->input_dev);
    mutex_destroy(&stk->reg_lock);
    kfree(stk);
    return 0;
}

/**
 * @brief: Device detection callback for automatic device creation
 *
 * @param[in] client: struct i2c_client *
 * @param[in] info: struct i2c_board_info *
 *
 * @return: Success or error
 *      0: SUCCESS
 *      <0: error
 */
static int stk_detect(struct i2c_client *client, struct i2c_board_info *info) {
	struct i2c_adapter *adapter = client->adapter;

    STK_PRINT_FUN();

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        return -ENODEV;
    }

	if(twi_id == adapter->nr) {
        STK_PRINT_LOG("addr=0x%X", client->addr);
        strlcpy(info->type, STK_NAME, I2C_NAME_SIZE);
        return 0;
    }

    return -ENODEV;
}

static const struct i2c_device_id stk_id[] = {
    {STK_NAME, 0},
    {}
};

MODULE_DEVICE_TABLE(i2c, stk_id);

#if !USE_DETECT
static const struct of_device_id stk_of_match[] = {
    {.compatible = "allwinner,stkbaxx"},
    {},
};
#endif

static struct i2c_driver stk_i2c_driver = {
    .class = I2C_CLASS_HWMON,
    .driver = {
        .name   = STK_NAME,
        .owner  = THIS_MODULE,
#if !USE_DETECT
        .of_match_table = stk_of_match,
#endif
#ifndef CONFIG_HAS_EARLYSUSPEND
        .suspend    = stk_suspend,
        .resume     = stk_resume,
#endif /* CONFIG_HAS_EARLYSUSPEND */
    },
    .id_table       = stk_id,
    .probe          = stk_probe,
    .remove         = stk_remove,
#if USE_DETECT
    .detect         = stk_detect,
#endif
    .address_list   = normal_i2c,
};

/**
 */
 
static int startup(void){
	if (input_sensor_startup(&(gsensor_info.input_type))) {
        STK_PRINT_ERR("input_sensor_startup failed");
        return -1;
    }

    if (input_sensor_init(&(gsensor_info.input_type))) {
        STK_PRINT_ERR("ctp_ops.init_platform_resource error");
    }

    twi_id = gsensor_info.twi_id;
    STK_PRINT_LOG("after fetch_sysconfig_para: normal_i2c:0x%hX", normal_i2c[0]);

    input_set_power_enable(&(gsensor_info.input_type), 1);
	return 0;
}
	
static int __init stk8baxx_init(void)
{
    int ret = -1;
    STK_PRINT_FUN();

#if !USE_DETECT
	struct device_node *np = NULL;
	char *p = NULL;
#endif

#if USE_DETECT
	if (startup() != 0)
		return -1;
#else
	np = of_find_node_by_name(NULL, "gsensor");
	if (np && np->parent) {
		p = (char *) np->parent->name;
		// if not set USE_DETECT but do not config ctp in twi, also use i2c detect
		if (strncmp(p, "twi", 3) != 0) {
			gUseDetect = 1;
			if (startup() != 0)
				return -1;
			stk_i2c_driver.detect = stk_detect;
		}
	}
#endif

    ret = i2c_add_driver(&stk_i2c_driver);

    if (ret < 0)
    {
        STK_PRINT_ERR("i2c_addr_driver failed");
        return -ENODEV;
    }

    return ret;
}

/**
 */
static void __exit stk8baxx_exit(void)
{
    STK_PRINT_FUN();
    i2c_del_driver(&stk_i2c_driver);
    input_set_power_enable(&(gsensor_info.input_type), 0);
    input_sensor_free(&(gsensor_info.input_type));
}

module_init(stk8baxx_init);
module_exit(stk8baxx_exit);

MODULE_AUTHOR("Sensortek");
MODULE_DESCRIPTION("stk8baxx 3-Axis accelerometer driver");
MODULE_LICENSE("GPL");
MODULE_VERSION(STK_ACC_DRIVER_VERSION);
