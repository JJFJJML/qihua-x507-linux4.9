/*
 * @Author: EvansLiu
 * @Date: 2020-06-09 15:16:13
 * @LastEditTime: 2020-06-11 14:26:42
 * @LastEditors: Please set LastEditors
 * @Description: FM i2c drive
 * @FilePath: \XJ-T3\lichee\linux-3.10\drivers\fm\bk1088.c
 */ 
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/cdev.h>
//#include <linux/sys_config.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/string.h>
#include "bk1088.h"
#include <asm/uaccess.h>
#include <linux/sunxi-gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>

#define USE_I2C_ADAPT	0
#define BK1088_CHIP

#define SCK_GPIO GPIOG(15)
#define SDA_GPIO GPIOG(16)

#define SPK_ON      1
#define SPK_OFF     0


#if USE_I2C_ADAPT
#define FM_NAME "bk1088E"
static const unsigned short bk1088_i2c[2] = {0x5d, I2C_CLIENT_END};
#endif

static __u32 debug_mask = 0x1;
enum {
	DEBUG_INIT = 1U << 0,
};
#define dprintk(level_mask, fmt, arg...)    if(unlikely(debug_mask & level_mask)) \
        printk("***fm<->bk1088***"fmt, ## arg)
module_param_named(debug_mask, debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);

static dev_t devid;
static struct cdev *bk1088_cdev;
static struct class *bk1088_class;
static struct device *bk1088_dev;
__u8 	b_AM = 0;
__u8	g_band_index;
__u8	g_space_index;

__u8 DRV_FM_VOICE_ON = 1;
__u8 DRV_FM_VOICE_OFF = 0;
__u8	fm_volume_mute = 0;

__s32 bandSEL_bottom[]={87000,522};
__s32 spaceSEL[]={100,9};

long validResult = 0;

int spk_gpio = -1;
static bool pa_ctl_level = 0;

static bool bk1088_spk_open = 0;
extern bool is_codec_spk_open(void);


static bool is_power_open = 0;

extern void __user *compat_alloc_user_space(unsigned long len);

enum FmWriteCmd
{
	PLAY = 0,
	VALID_JUDGE = 1,
	SET_VOLUME = 2,
};

enum FmReadCmd
{
	VALID_JUDGE_RESULT = 0,
};

#if 0
/* __s32 HW_Reg_FM[]=
{
	0x9252,
	0x1080,
	0x8281, //REG2
	0x8000,
	0x60D4,//REG4 interrupt enable
	0x81DA,//REG5 
	0x0C0E,
	0x0181,
	0xAC90,
	0xF98F,
	0x404C,//10
	0x0009,
	0x0000,
	0x0000,
	0x0000,
	0x0000,//15
	0x7B11,
	0x004D,
	0x4000,
	0x4144,
	0xC29A,//20
	0x79F8,
	0x4012,
	0x0054,
	0x341C,
	0x0000,//25
	0x0000,
	0x4CA2,
	0x8E20,
	0x0200,
	0x0000,//30
	0xA8E4,
	0x3264,
}; */

#endif

__s32 HW_Reg_FM[]=
{
	0x9252,
	0x1080,
	0x8281, //REG2婵帗绋掗敃銏ゅ极閹惧瓨缍囬柛鎰靛弾閸ゅ鏌熸搴″幋闁轰焦鎹囧顒勫Χ閸℃浼撻梻浣芥硶閸犳劙寮告繝姘闁跨噦鎷�,闂備浇娉曢崰鎰板春閸涙潙绠熼悗锝庡亜椤忕ΘEEK(闂備浇娉曢崰鎾斥枔閺囩姵濯奸悷娆忓椤忓爼姊虹捄銊ユ瀾闁哄顭烽獮蹇撶暋閺夊潡鍙�),power on ,婵炶揪绲藉锟犲极閹捐妫橀柕鍫濇椤忥拷
		     //Soft mute,闂備浇娉曢崰鎰板几婵犳艾绠€瑰嫭婢樼换渚€姊虹捄銊ユ灁閻庢碍绋撻崰濠冨緞鐎ｎ亶浼撻梻浣芥硶閸犳劙寮告繝姘闁绘垼濮ら弲鎼佹煛閸屾ê鈧牜鈧艾缍婇弻銊╂偄閸涘﹦浼勯梺褰掝棑閸忔﹢寮幘璇叉闁靛牆妫楅鍓佲偓娈垮枟濡炰粙寮幘璇叉闁靛牆妫楅鍫曟⒑鐠恒劌鏋戦柡瀣煼楠炲繘鎮滈懞銉︽闂佸搫鍊堕崐鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佺懓鍤栭幏锟�
	0x8000,
	0x60D4,//REG4 interrupt enable
	0x81DA,//REG5 
	0x0C0E,
	0x0181,
	0xAC90,
	0xF98F,
	0x404C,//10
	0x0009,
	0x0000,
	0x0000,
	0x0000,
	0x0000,//15
	0x7B11,
	0x004D,
	//Reg7(bit7~13) 闂備浇娉曢崰鎰板几婵犳艾绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佸綊顥撻崗姗€寮幘璇叉闁靛牆妫楅鍫曟⒑鐠恒劌鏋戦柡瀣煼楠炲繘鎮滈懞銉︽婵°倖顨夊▍锝夊Υ鐎ｎ喖绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘愁潐椤偓婵せ鍋撻柡浣规崌瀵剟濡堕崱妤婁紦 0x10
	//000 0000 闂備浇娉曢崰鎰板几婵犳艾绠€瑰嫭澹嗗В鍫ユ⒑鐠恒劌鏋戦柡瀣煼楠炲繘鎮滈懞銉︽闂佸搫鍊堕崐鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佸綊顥撻崗姗€寮幘缁樷挀閻犲搫鎼花濠氭煙妞嬪骸鍘撮柡浣规崌瀵剟濡堕崱妤婁紦闂備浇娉曢崰鎰板几婵犳艾绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佸綊顥撻崗姗€寮幘璇叉闁靛牆妫楅锟�
	//111 1111 闂備浇娉曢崰鎰板几婵犳艾绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佸綊顥撻崗姗€寮幘璇叉闁靛牆妫楅鍫曟偡娴ｅ憡鍤囬柡浣规崌瀵剟濡堕崱妤婁紦闂備浇娉曢崰鎰板几婵犳艾绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佸綊顥撻崗姗€寮幘璇叉闁靛牆妫楅鍫曟⒑鐠恒劌娅愰柟鍑ゆ嫹
	0x4000,
	0x4144,
	0xC29A,//20
	0x79F8,
	0x4012,
	0x0054,
	0x341C,
	0x0000,//25
	0x0000,
	0x4CA2,
	0x8E20,
	0x0200,
	0x0000,//30
	0xA8E4,
	0x3264,
};

static struct regulator *bk_io_power;
const char *bk_io_power_name = NULL;

static struct regulator *bk_vcc_power;
const char *bk_vcc_power_name = NULL;

static int bk_power_io_on_off(bool on_off){
    int ret = 0;

    if (bk_io_power_name) {
		bk_io_power = regulator_get(NULL, bk_io_power_name);
		if (!IS_ERR(bk_io_power)) {
			if (on_off) {
					ret = regulator_enable(bk_io_power);
					if (ret < 0) {
						printk("regulator bk_io_power enable failed\n");
						regulator_put(bk_io_power);
						return ret;
					}

					ret = regulator_get_voltage(bk_io_power);
					if (ret < 0) {
						printk("regulator bk_io_power get voltage failed\n");
						regulator_put(bk_io_power);
						return ret;
					}
					printk("check bk_io_power voltage: %d\n", ret);
			} 
			else {   
					ret = regulator_disable(bk_io_power);
					if (ret < 0) {
						printk("regulator bk_io_power disable failed\n");
						regulator_put(bk_io_power);
						return ret;
					}
			}
			regulator_put(bk_io_power);
		}
        else{
            printk("bk_io_power error\n");
            return -1;
        }
	}
    else{
        printk("%s:no bk_io_power find!\n", __func__);
        return -1;
    }

    return 0;
}

static int bk_power_vcc_on_off(bool on_off){
    int ret = 0;

    if (bk_vcc_power_name) {
		bk_vcc_power = regulator_get(NULL, bk_vcc_power_name);
		if (!IS_ERR(bk_vcc_power)) {
			if (on_off) {
				if(!is_power_open){
					ret = regulator_enable(bk_vcc_power);
					if (ret < 0) {
						printk("regulator bk_vcc_power enable failed\n");
						regulator_put(bk_vcc_power);
						return ret;
					}

					is_power_open = 1;

					ret = regulator_get_voltage(bk_vcc_power);
					if (ret < 0) {
						printk("regulator bk_vcc_power get voltage failed\n");
						regulator_put(bk_vcc_power);
						return ret;
					}
					printk("check bk_vcc_power voltage: %d\n", ret);
				}
			} else {

				if(is_power_open){	
					ret = regulator_disable(bk_vcc_power);
					if (ret < 0) {
						printk("regulator bk_vcc_power disable failed\n");
						regulator_put(bk_vcc_power);
						return ret;
					}
					is_power_open = 0;
				}

			}
			regulator_put(bk_vcc_power);
		}
        else{
            printk("bk_vcc_power error\n");
            return -1;
        }
	}
    else{
        printk("%s:no bk_vcc_power find!\n", __func__);
        return -1;
    }

    return 0;
}

int bk_spk_on_off(bool on_off){
	if(spk_gpio < 0){
		printk("bk1088 spk gpio invalid\n");
		return spk_gpio;
	}
	
	if(on_off == SPK_ON){
		printk("open speaker\n");
		bk1088_spk_open = 1;
		gpio_set_value(spk_gpio, pa_ctl_level);
	}
	else{
		printk("close speaker\n");
		bk1088_spk_open = 0;
		if(!is_codec_spk_open()){
			gpio_set_value(spk_gpio, !pa_ctl_level);
		}
	}

	return 0;
}

bool is_bk1088_spk_open(void){
	return bk1088_spk_open;
}
EXPORT_SYMBOL(is_bk1088_spk_open);

void gpio_init(struct device_node *np){
	int ret;
	unsigned int temp_val;
	
	ret = gpio_request(SCK_GPIO,"sck_gpio");
	if (ret < 0) {
		printk("Failed to request GPIO for SCK\n");
	}
	ret = gpio_request(SDA_GPIO,"sda_gpio");
	if (ret < 0) {
		printk("Failed to request GPIO for SDA\n");
	}

	spk_gpio = of_get_named_gpio(np, "spk_gpio", 0);
	if (spk_gpio < 0) {
		printk("bk1088 get spk_gpio failed!\n");
	}

	ret = of_property_read_u32(np, "pa_ctl_level", &temp_val);
	if (ret < 0) {
		printk("bk1088 spk ctr_level missing or invalid. using default value\n");
	} else {
		pa_ctl_level = temp_val;
	}
}

void bk_power_init(struct device_node *np){
	if(np)
	{
		if (of_property_read_string(np, "bk_power_io", &bk_io_power_name) || of_property_read_string(np, "bk_power_vcc", &bk_vcc_power_name)) {
			printk("bk1088 miss power.\n");
	        bk_io_power_name = NULL;
			bk_vcc_power_name = NULL;
		}else{
			bk_power_io_on_off(1);
			bk_power_vcc_on_off(1);
		}
	}
}

void gpio_deinit(void){
	gpio_free(SCK_GPIO);
	gpio_free(SDA_GPIO);
}

void bk_clk_low(void)
{
	gpio_set_value(SCK_GPIO,0);	
}
void bk_clk_high(void)
{
	gpio_set_value(SCK_GPIO,1);	
}
void bk_sda_low(void)
{
	gpio_set_value(SDA_GPIO,0);	
}
void bk_sda_high(void)
{
	gpio_set_value(SDA_GPIO,1);	
}
void i2c_init(void)
{
	gpio_direction_output(SCK_GPIO,0);
	gpio_direction_output(SDA_GPIO,0);
	bk_sda_high();
	bk_clk_high();
}
void i2c_start(void)
{
	i2c_init();
	udelay(1);
	bk_sda_low();
	udelay(1);
	bk_clk_low();
	udelay(1);
	bk_sda_high();
}

void i2c_stop(void)
{
	gpio_direction_output(SDA_GPIO,0);
	bk_sda_low();
	udelay(1);
	bk_clk_high();
	udelay(1);
	bk_sda_high();
	udelay(1);
}

void i2c_ack(void)
{
	gpio_direction_output(SDA_GPIO,0);
	bk_clk_low();
	bk_sda_low();
	udelay(1);

	bk_clk_high();
	udelay(1);
	bk_clk_low();
	gpio_direction_input(SDA_GPIO);
}

void i2c_nack(void)
{
	gpio_direction_output(SDA_GPIO,0);
	bk_clk_low();
	bk_sda_high();
	udelay(1);

	bk_clk_high();
	udelay(1);
	bk_clk_low();
}

__u8 bk_i2c_receive_ack(void)
{
	__u8 ackflag;
	
	gpio_direction_input(SDA_GPIO);
	udelay(1);
	bk_clk_high();
	udelay(1);	
	ackflag = (__u8 *)gpio_get_value(SDA_GPIO);
	bk_clk_low();
	udelay(1);
	return ackflag;
}
void bk_i2c_sendbyte(__u8 sendData)
{
	__u8 i;
	gpio_direction_output(SDA_GPIO,0);
	udelay(1);
	for(i = 0;i<8;i++)
	{
		if(sendData & 0x80)
		{
			bk_sda_high();
		}else
		{
			bk_sda_low();
			/* code */
		}
		udelay(1);
		bk_clk_high();
		udelay(1);
		sendData <<= 1;
		bk_clk_low();
		if(i == 7)
			gpio_direction_input(SDA_GPIO);
	}
}
__u8 bk_i2c_readbyte(void)
{
	__u8 i;
	__u8 readData=0;
	gpio_direction_input(SDA_GPIO);
	udelay(1);
	for(i = 0;i < 8;i++)
	{
		bk_clk_high();
		readData <<=1;
		udelay(1);
		if(gpio_get_value(SDA_GPIO))
			readData |= 0x01;
		bk_clk_low();
		udelay(1);
	}
	return readData;

}

void bk_i2c_read(__u8 reg,__u8 *pBuf,__u8 len)
{
	__u8 i;
	__u8 addr;
	addr = 0x80;
	i2c_start();

	bk_i2c_sendbyte(addr);
	bk_i2c_receive_ack();

	reg=reg<<1;
    reg|=0x01;
	bk_i2c_sendbyte(reg);
	bk_i2c_receive_ack();

	for(i = 0;i < len;i++)
	{
		pBuf[i] = bk_i2c_readbyte();
		i2c_ack();
	}
	pBuf[i] = bk_i2c_readbyte();
	i2c_nack();
	i2c_stop();
}

void bk_i2c_write(__u8 reg,__u8 *pBuf,__u8 len)
{
	__u8 i;
	__u8 addr;
	addr = 0x80;
	i2c_start();

	bk_i2c_sendbyte(addr);
	bk_i2c_receive_ack();
	reg=reg<<1;
	bk_i2c_sendbyte(reg);
	bk_i2c_receive_ack();	
	for(i = 0;i < len;i++)
	{
		bk_i2c_sendbyte(pBuf[i]);
		bk_i2c_receive_ack();
	}
	i2c_stop();
}
__s32 set_reg3(__u8 reg3_value)
{
	__u8 TmpData8[4];
	__u8 reg3[2];
	
	bk_i2c_read(3,TmpData8,4);
	//printk("read TmpData8 = 0x%02x%02x,0x%02x%02x\n",TmpData8[0],TmpData8[1]);
	if(reg3_value)
	{
		TmpData8[3] |= 0x80;
		//printk("reg3_value = 1,TmpData8 = 0x%02x%02x\n",TmpData8[0]);
	}
	else 
	{	
		TmpData8[3] &= 0x7f;
		//printk("reg3_value = 0, TmpData8 = 0x%02x%02x,0x%02x%02x\n",TmpData8[0],TmpData8[1]);
	}
	//printk("write TmpData8 = 0x%02x%02x,0x%02x%02x\n",TmpData8[0],TmpData8[1]);
	bk_i2c_write(3,TmpData8,4);
	
	bk_i2c_read(3,reg3,2);
	printk(" reg3 = 0x%02x%02x\n",reg3[0],reg3[1]);
}

void MuteEnable(__u8 b_enable)
{
	__u8 tmpBuff[2];

	bk_i2c_read(0x02,tmpBuff,2);

#ifdef BK1088_CHIP
	if(b_enable==1)//mute
		tmpBuff[0]|=0x60;
	else//mute enable
		tmpBuff[0]&=0x9f;
#else
	if(b_enable==1)//mute
		tmpBuff[0]|=0x40;
	else//mute enable
		tmpBuff[0]&=0xbf;
#endif

	bk_i2c_write(0x02,tmpBuff,2);
}

__s32 BK1080_FreqToChan(__s32 frequency)
{
	__s32 bottomOfBand;
	__u8 channelSpacing;
	__s32 channel;
	
	bottomOfBand=bandSEL_bottom[g_band_index];

	channelSpacing=spaceSEL[g_space_index];
	printk("bottomOfBand = %d,channelSpacing = %d\n",bottomOfBand,channelSpacing);
	channel = (frequency - bottomOfBand) / channelSpacing;
	//eDbug("channel: %d\n", channel);
	printk("channel: %d\n",channel);
	return (channel);	
}

__s32 BK1088_ChanToFreq(__s32 channel)
{
	__s32 bottomOfBand;
	__u8 channelSpacing;
	__s32 frequency;

	bottomOfBand=bandSEL_bottom[g_band_index];

	channelSpacing=spaceSEL[g_space_index];
	frequency = (channel * channelSpacing) + bottomOfBand;
	printk("frequency: %d\n",frequency);
	return (frequency);
}

/**************************************************/ 
void  BK1080_SetFreq(__s32 curFreq)
{  
	__s32 curChan;
	__u8 TmpData8[4];

	bk_i2c_read(2,TmpData8,4); //read reg2,with 4 bytes	
	curChan=BK1080_FreqToChan(curFreq); /*stop seek and tune*/
	//eDbug("curChan = %d\n",curChan);

	TmpData8[0]&=0xfe; 
	TmpData8[2] =0x0;//reset tune
	TmpData8[3]=curChan&0xff;
	bk_i2c_write(2,TmpData8,4); //write reg2,with 4 bytes


	TmpData8[2]=(curChan>>8)&0xff;
	TmpData8[2] |= 0x80;//start tune
	TmpData8[3]=curChan&0xff;
	printk("%02x\n",TmpData8[3]);
	bk_i2c_write(2,TmpData8,4); //write reg2,with 4 bytes
	
	/* __u8 reg11[2];
	bk_i2c_read(11,reg11,2);
	printk("reg 11 :0x%02x%02x\n",reg11[0],reg11[1]);
	printk("BK1080_SetFreq end\n"); */
}

void setVolume(__u8 volume)
{
	__u8 tmpBuff[2];
	{
		bk_i2c_read(0x05,tmpBuff,2);
#ifdef BK1088_CHIP
		tmpBuff[1]=tmpBuff[1]&0xe0;
		tmpBuff[1]|=(volume&0x1f);
#else
		tmpBuff[1]=tmpBuff[1]&0xf0;
		tmpBuff[1]|=(volume&0x0f);
#endif
		bk_i2c_write(0x05,tmpBuff,2);
	}
}

#if 0
__u8 GetCurVolume()
{
	__u8 tmpBuff[2];
	bk_i2c_read(0x05,tmpBuff,2);
#ifdef BK1088_CHIP
	return (__u8)(tmpBuff[1]&0x1f);
#else
	return (__u8)(tmpBuff[1]&0x0f);
#endif
}

#endif

/**
 * @description:  start seek or tune,setting reg:2 
				SKMODE:1  闂備線娼荤拹鐔煎礉鐎ｎ剛绠旈柛灞剧⊕缂嶅洭鏌涢敂璇插箺婵炲懏娲熼弻鐔衡偓娑欘焽婢ф娊鏌嶈閸撴瑩骞婂鍥╃當闁告侗鍠楃紞鍥煕閿旇骞楁繛鍛礋閺屾稓鈧綆鍋嗛惌瀣亜閹烘挾娲存鐐寸墪楗即宕ㄩ鐐版喚
				SEEKUP:1  闂備礁鎲＄喊宥夊垂瀹曞洨绠旈柛宀€鍋涚粻瑙勭節婵犲倹鍣归柛瀣舵嫹
				SEEK: 1   濠电偠鎻紞鈧柛搴☆煼瀹曟顫滈埀顒勭嵁閺嵮€鍫柛娑卞幖娴滃爼姊洪崨濠冣拻闁哥姵娲熷畷妤呮晸閿燂拷
				SKAFCRL:1 闂備線娼荤拹鐔煎礉瀹ュ绠栫€规洖娲﹂～鏇㈡煏韫囥儳鎮肩紒鐘崇墱缁辨帞鈧綆浜堕崕搴ㄦ煠閸偄鐏撮柡浣哥Ф娴狅妇绮电€ｎ偅鎮欓梻浣告惈椤戝棛绮氱紞鐑燫L闂備礁鎼€氱兘宕归崼鏇熷仧妞ゆ洍鍋撻柟顖氬暣瀹曠喖顢欓悙顑芥灁闂備焦瀵х粙鎴︽儗娓氣偓椤㈡艾顫㈠⿻顣噉nel闂備礁鎼€氱兘宕归崡鐐嶆盯濡舵径濠冪€梺鍓插亖閸ㄦ椽鎮鹃敓锟�
 * @param {NULL} 
 * @return: null
 */
void start_seek_tune(void)
{
	__u8 REG2[2] = {0x87,0x01};
	__u8 REG3[2] = {0x00,0x00};
	__u8 REG5[2] = {0x3d,0xdf}; // 0011 1101 1101 1111 SEEKTH[15:9] = 30
	__u8 REG6[2] = {0x01,0xef}; // 0000 0001 1110 1111闂備胶鍋ㄩ崕濠氬箯閿燂拷	SKSNR[10:4] =  30
	__u8 REG10[2] = {0x00,0x4c};
	bk_i2c_write(2,REG2,2);
	bk_i2c_write(3,REG3,2);
	bk_i2c_write(5,REG5,2);
	bk_i2c_write(6,REG6,2);
	bk_i2c_write(10,REG10,2);
	
	bk_i2c_read(2,REG2,2);
	printk("fix reg 2: 0x%02x%02x \n",REG2[0],REG2[1]);
	bk_i2c_read(3,REG3,2);
	printk("fix reg 3: 0x%02x%02x \n",REG3[0],REG3[1]);
	bk_i2c_read(5,REG5,2);
	printk("fix reg 5: 0x%02x%02x \n",REG5[0],REG5[1]);
	bk_i2c_read(6,REG6,2);
	printk("fix reg 6: 0x%02x%02x \n",REG6[0],REG6[1]);
	bk_i2c_read(10,REG10,2);
	printk("fix reg 10: 0x%02x%02x \n",REG10[0],REG10[1]);
	
}
void stop_seek_tune(void)
{
	__u8 REG2[2] = {0};
	
	bk_i2c_read(2,REG2,2);
	printk("reg 2: 0x%02x%02x \n",REG2[0],REG2[1]);
	//write reg 2:0x8381
	REG2[0] = 0x82;
	REG2[1] = 0x81;
	bk_i2c_write(2,REG2,2);
	bk_i2c_read(2,REG2,2);
	printk("stop reg 2: 0x%02x%02x \n",REG2[0],REG2[1]);

}

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void get_seek_freq(void)
{
	__u8 REG10[2] = {0};
	__u8 REG11[2] = {0};
	start_seek_tune();
	while(1)
	{
		bk_i2c_read(10,REG10,2);
		printk("reg 10:0x%02x%02x \n RDSR: %d\nSTC: %d\nSF/BL: %d\n",REG10[0],REG10[1],(REG10[0] & 0x80)>>7,(REG10[0] & 0x40)>>6,(REG10[0] & 0x20)>>5);
		bk_i2c_read(11,REG11,2);
		printk("reg 11:0x%02x%02x \n",REG11[0],REG11[1]);
		mdelay(500);
		if((REG10[0] & 0x40)>>6 == 1)
		{
			printk("SEEK END:\n");
		}
		if((REG10[0] & 0x20)>>5 == 1)
		{
			break;
		}
		
	}
	printk("seek end!\n");	
}


void chip_bk1088_init(__u8 band_am_initial)
{
	__u8 index;
	__u8 bk1088_writeData[66];

	mdelay(200);
	printk("bk1088: band_am_initial is %d\n",band_am_initial);
	for(index = 0; index < 33; index++)
	{
		if(band_am_initial == 0)
		{
			bk1088_writeData[index*2] = (HW_Reg_FM[index] >> 8)&0xff;
			bk1088_writeData[index*2+1] = (HW_Reg_FM[index])&0xff;
		}
	}
	printk("bk1088 chip init\n");
	bk_i2c_write(0,&(bk1088_writeData[0]),66);//start from reg2,total 60 byte
	mdelay(250);//delay 250ms
	bk1088_writeData[3*2+1]&=0x7f;
	bk_i2c_write(3,&(bk1088_writeData[3*2]),2);
	bk1088_writeData[3*2+1]|=0x80;
	bk_i2c_write(3,&(bk1088_writeData[3*2]),2);  //toggle reg25 bit7 1 to 0 then 1.
	mdelay(60);
}
__s32 fm_mute(__s32 voice_onoff)
{
	if(voice_onoff == DRV_FM_VOICE_OFF)
	{
		fm_volume_mute = 1;
		MuteEnable(1);
	}
	else if(voice_onoff == DRV_FM_VOICE_ON)
	{
		fm_volume_mute = 0;
		MuteEnable(0);
	}
	return 1;
}
__u32 fm_init(__u8 band)
{
	printk("bk1088: fm_init band is %d\n",band);
	if(band == 0)
	{
		g_band_index = 0;
		g_space_index = 0;
		b_AM = 0;
	}
	else
	{
		g_band_index =1;
		g_space_index = 1;
		b_AM = 1;
	}
	chip_bk1088_init(band);
	setVolume(0x0c);

	//MuteEnable(1);  //VOICE_OFF
	fm_mute(DRV_FM_VOICE_OFF);

	return 1;
}
__s32  fm_exit(void)
{
	__u8 TmpData8[10];
	TmpData8[0] &= 0x02; 
	TmpData8[1] |=0xa0;
	bk_i2c_write(2,TmpData8,2);
	return 1;
}

__s32 fm_play(__s32 freq)
{
	printk("fm_play freq = %d\n",freq);

	bk_power_io_on_off(1);
	bk_power_vcc_on_off(1);
	mdelay(50);
	fm_init(0);
	bk_spk_on_off(SPK_ON);

	if(fm_volume_mute)
	{
		fm_mute(DRV_FM_VOICE_ON);
	}
	BK1080_SetFreq(freq);
	return 1;
}

/* __s32 g_last_freq_deviation_value;
__s32 g_param_RSSI_FM = 10;
__s32 g_param_SNR_FM = 2;
__s32 g_param_impuse_counter_FM = 10;
__s32 g_param_RSSI_AM = 10;
__s32 g_param_SNR_AM = 10;
__s32 g_param_FREQ_THREASHOLD = 10; */

__u8 g_param_FREQ_THREASHOLD = 75;
__u8 g_param_LAST_FREQ_THREASHOLD = 50;
__u8 g_param_RSSI_FM = 39;  
__u8 g_param_SNR_FM = 4;  
__u8 g_param_RSSI_AM = 35;
__u8 g_param_SNR_AM = 6;
__u8 g_param_impuse_counter_FM = 14;
__u8 g_param_impuse_counter_AM;
__s32 g_last_freq_deviation_value;
__u8 BK1080_ValidStop(__s32 freq)
{
	static __s32 last_tuned_freq=0;
	__s32 cur_freq_deviation;
	__u8 TmpData8[10];
	__u8 impuse_counter;
	__u8 search_ok=1;
	//__s32 tmpint;
	//__u8 TmpData2[2];
	__s32 retry_times=500;

	fm_mute(1);

	//闂備浇娉曢崰鎰板几婵犳艾绠柣銈庡灱濞差枠eg3[15]闂備浇娉曢崰鎰板几婵犳艾绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘炬嫹0闂備浇娉曢崰鎰板几婵犳艾绠柣鎴ｅГ閺呮悂鏌￠崒妯衡偓鏍偓姘秺閺屻劑鎮㈤崨濠勪紕闂佺懓鍤栭幏锟�1
	//........................
	set_reg3(0);
	mdelay(10);
	set_reg3(1);
	//.............. !!! ..............
	
	BK1080_SetFreq(freq);
	mdelay(50);

	if(96000 == freq || 102000 == freq)
	{
		printk("96000 return FALSE----\n");
		printk("102000 return FALSE----\n");
        search_ok=0;//false
		return 0;			
	}
	
	do
	{
		bk_i2c_read(10,TmpData8,2);//start from reg7,with 8by
		//printk("doing the while\n");
		if(TmpData8[0]&0x40)	//STC 
		{
			/* __u8 reg11[2];
			bk_i2c_read(11,reg11,2);
			printk("reg 11 :0x%02x%02x\n",reg11[0],reg11[1]); */
			break;
		}
		//Delay1us(1);
		mdelay(1);

        if((retry_times--)==0)
		{
			printk("---Err,  STC Timeout ---\n");
            search_ok=0;//false
			return 0;		
		}
	}while(1);
	
	//new added 2009-05-30
	bk_i2c_read(7,TmpData8,10);//start from reg7,with 10bytes

#ifdef BK1088_CHIP
	cur_freq_deviation=TmpData8[4];
	cur_freq_deviation<<=8;
	cur_freq_deviation|=TmpData8[5];
	cur_freq_deviation=cur_freq_deviation>>7;
#else
	cur_freq_deviation=TmpData8[0];
	cur_freq_deviation<<=8;
	cur_freq_deviation|=TmpData8[1];
	cur_freq_deviation=cur_freq_deviation>>4;
#endif
	//eDbug("b_AM = %d\n",b_AM);
	
	do
	{
		if(TmpData8[6]&0x10)    //check AFCRL bit12
		{
			//eDbug("######Errot AFC########\n");
			//search_ok=0;//false
			//break;
		}

#ifndef BK1088_CHIP
		if(TmpData8[7]<g_param_RSSI_FM) //RSSI<10
		{
			search_ok=0;//false
			break;
		}

		if( (TmpData8[1]&0xf) <g_param_SNR_FM) //SNR<4
		{
			search_ok=0;//false
			break;
		}
		impuse_counter=(TmpData8[8]>>2)&0xf;//REG11<13:10>
		if(impuse_counter>g_param_impuse_counter_FM)
		{
			search_ok=0;//false
			break;
		}

#else
		if(b_AM==0)//FM
		{
			printk("RSSI = %d\n",TmpData8[7]&0x7f);
			printk("SNR = %d\n",TmpData8[5]&0x7f);
			printk("impuse_counter = %d\n",TmpData8[6]&0xf);
			if((TmpData8[7]&0x7f)<g_param_RSSI_FM) //RSSI<10
			{
				printk("Error RSSI = %d\n",TmpData8[7]&0x7f);
				search_ok=0;//false
				break;
			}

			if( (TmpData8[5]&0x7f) <g_param_SNR_FM) //SNR<2
			{
				printk("Error SNR = %d\n",TmpData8[5]&0x7f);
				search_ok=0;//false
				break;
			}

			impuse_counter=(TmpData8[6])&0xf;//REG10<11:8>
			
			if(impuse_counter>g_param_impuse_counter_FM)
			{
				printk("Error impuse_counter = %d\n",impuse_counter);
				search_ok=0;//false
				break;
			}
			//break;

		}
		else//AM
		{
			printk("RSSI = %d\n",TmpData8[7]&0x7f);
			printk("SNR = %d\n",TmpData8[5]&0x7f);
			if((TmpData8[7]&0x7f)<g_param_RSSI_AM) //RSSI<10
			{
				printk("Error RSSI = %d\n",TmpData8[7]&0x7f);
				search_ok=0;//false
				break;
			}

			if( (TmpData8[5]&0x7f) <g_param_SNR_AM) //SNR<0x20
			{
				printk("Error SNR = %d\n",TmpData8[5]&0x7f);
				search_ok=0;//false
				break;
			}
			break;//OK
		}

#endif
		//add frequency devation check

		if(b_AM==0)//FM
		{
			printk("cur_freq_deviation = %d\n",cur_freq_deviation);
			if( (cur_freq_deviation>=g_param_FREQ_THREASHOLD)&&(cur_freq_deviation<=(MAX_FREQ_DEVIATION-g_param_FREQ_THREASHOLD)))
			{
				printk("Error cur_freq_deviation = %d\n",cur_freq_deviation);
				last_tuned_freq=freq;//save last tuned freqency
				g_last_freq_deviation_value=cur_freq_deviation;
				search_ok=0;//false
				break;		
			}
		}
	}while(0);//new added
	last_tuned_freq=freq;//save last tuned freqency
	g_last_freq_deviation_value=cur_freq_deviation;
	fm_mute(0);
	return search_ok; //OK
}

__s32  fm_auto_search(__s32 freq, __u32 search_dir)
{
	printk("auto freq = %d\n",freq);
	/* int max_freq = 43570;
	int min_freq = 35098; */
	while(1)
	{
		if(b_AM)
		{
			/* if(freq > rda5820_info_AM.max_freq)
			{
				return EPDK_FAIL;
			} */
		}
		else 
		{
			/* if(freq > max_freq || freq < min_freq)
			{
				return -1;
			} */
		}
		
		
		
		fm_mute(DRV_FM_VOICE_OFF);
		if(BK1080_ValidStop(freq))
		{
			printk(" -- freq Valid -- \n");
			fm_mute(DRV_FM_VOICE_ON);
			return freq;
		}
		else
		{
			printk(" --Err, freq Invalid -- \n");
			return freq|0xff000000;
		}
		printk("line:%d,freq: %d\n",__LINE__,freq);
	}
}

int bk1088_open(struct inode *inode, struct file *file){	
	dprintk(DEBUG_INIT,"%s\n",__func__);
	return 0;
}

int bk1088_release(struct inode *inode, struct file *file){
	dprintk(DEBUG_INIT,"%s\n",__func__);

	//MuteEnable(1);  //VOICE_OFF
	fm_mute(DRV_FM_VOICE_OFF);
	bk_spk_on_off(SPK_OFF);
	bk_power_vcc_on_off(0);

	return 0;
} 

ssize_t bk1088_read(struct file *file, char __user *buf, size_t count, loff_t *ppos){
	//copy_to_user(buf,,count);
	//int cmd = buf[0];
	char toUserBuf[1];
	switch(VALID_JUDGE_RESULT)
	{
		case VALID_JUDGE_RESULT:
			toUserBuf[0] = validResult;
			printk("toUserBuf[0] = %d\n",toUserBuf[0]);
			copy_to_user(buf,toUserBuf,1);
		break;
	default:
        break;
	}
	dprintk(DEBUG_INIT,"%s\n",__func__);
	return 0;
}

ssize_t bk1088_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
	//int recieveBuf[3];
	//copy_from_user(recieveBuf,buf,count);
	//printk("recieveBuf[0] = %d,recieveBuf[1] = %d,recieveBuf[2] = %d\n",recieveBuf[0],recieveBuf[1],recieveBuf[2]);
	int cmd = buf[0];
	__s32 freq = buf[1];
	
	printk("buf[0] = %d,buf[1] = %d,buf[2] = %d",buf[0],buf[1],buf[2]);
	switch(cmd)
	{
		case PLAY:
			fm_play(freq);
		break;
		case VALID_JUDGE:
			validResult = BK1080_ValidStop(freq);
			printk("validResult = %d\n",validResult);
		break;
	default:
        break;
	}
	printk("write freq = %d\n",freq);
	dprintk(DEBUG_INIT,"%s\n",__func__);
	return 0;
}

long bk1088_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	//printk("arg[0] = %d,arg[1] = %d,arg[2] = %d\n",arg[0],arg[1],arg[2]);
	unsigned long karg[4];
	unsigned long ubuffer[4] = {0, 0, 0, 0};
	int i;
	
	printk("enter bk1088 ioctl\n");
	if (copy_from_user((void*)karg,(void __user*)arg,4*sizeof(unsigned long))) {
		printk("copy_from_user fail\n");
		return -EFAULT;
	}
	
	ubuffer[0] = *(unsigned long*)karg;
	ubuffer[1] = (*(unsigned long*)(karg+1));
	ubuffer[2] = (*(unsigned long*)(karg+2));
	ubuffer[3] = (*(unsigned long*)(karg+3));
	
	for(i = 0; i<4; i++)
		printk("ubuffer[%d] = %ld,cmd = %d\n",i,ubuffer[i],cmd);
	
	__s32 freq = ubuffer[0];
	__u8 volume = (__u8)ubuffer[0];
	
	switch(cmd)
	{
		case PLAY:
			printk("play freq = %d\n",freq);
			fm_play(freq);
		break;
		case VALID_JUDGE:
			printk("stop freq = %d\n",freq);

			bk_power_io_on_off(1);
			bk_power_vcc_on_off(1);
			//mdelay(50);
			fm_init(0);
			bk_spk_on_off(SPK_ON);

			validResult = BK1080_ValidStop(freq);
			printk("validResult = %d\n",validResult);
			//validResult = 152;
			return validResult;
		break;
		case SET_VOLUME:
			if(volume > 31){
				volume = 31;
			}
			printk("set volume:%ld\n", volume);
			setVolume(volume);
		break;
	default:
        break;
	}
	
	dprintk(DEBUG_INIT,"%s\n",__func__);
	return 100;
}

long bk1088_compact_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	compat_uptr_t karg[4];
	unsigned long __user *ubuffer;

	if (copy_from_user((void *)karg, (void __user *)arg, 4 * sizeof(compat_uptr_t))) {
		printk("copy_from_user fail\n");
		return -EFAULT;
	}

	ubuffer = compat_alloc_user_space(4 * sizeof(unsigned long));
	if (!access_ok(VERIFY_WRITE, ubuffer, 4 * sizeof(unsigned long)))
		return -EFAULT;

	if (put_user(karg[0], &ubuffer[0]) ||
	    put_user(karg[1], &ubuffer[1]) ||
	    put_user(karg[2], &ubuffer[2]) || 
		put_user(karg[3], &ubuffer[3])) {
		printk("put_user fail\n");
		return -EFAULT;
	}

	return bk1088_ioctl(file, cmd, (unsigned long)ubuffer);
}

static const struct file_operations bk1088_fops = {
	.owner    = THIS_MODULE,
	.open     = bk1088_open,
	.release  = bk1088_release,
	.write    = bk1088_write,
	.read     = bk1088_read,
	.unlocked_ioctl = bk1088_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = bk1088_compact_ioctl,
#endif
};
#if USE_I2C_ADAPT
static s32 bk1088_i2c_transfer(struct i2c_client *client, u8 *buf, s32 len)
{
    struct i2c_msg msg;
    s32 ret = -1;
    s32 retries = 0;

    msg.flags = !I2C_M_RD;
    msg.addr  = client->addr;
    msg.len   = len;
    msg.buf   = buf;

    while(retries < 5)
    {
        ret = i2c_transfer(client->adapter, &msg, 1);
        if (ret == 1)break;
        retries++;
    }
    if((retries >= 5)) {
    }
    return ret;
}

static s32 bk1088_i2c_read(u8 reg){
	u8 buf[3];
	buf[0] = BK1088_I2C_ADDR_R;
	buf[1] = reg;
	bk1088_i2c_transfer(bk1088_i2c_client, buf, 3);
}

static s32 bk1088_i2c_write(){
}

static int bk1088_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id){
	return 0;
}

static int bk1088_i2c_remove(struct i2c_client *client){
	return 0;
}

static const struct i2c_device_id bk1088_i2c_id[] = {
        { FM_NAME, 0 },
		{ }
};

static int bk1088_detect(struct i2c_client *client, struct i2c_board_info *info)
{
	struct i2c_adapter *adapter = client->adapter;
    int  ret = -1;
	dprintk(DEBUG_INIT,"the adapter number is %d\n", adapter->nr);

    if (!i2c_check_functionality(adapter, I2C_FUNC_SMBUS_BYTE_DATA)) {
		dprintk(DEBUG_INIT,"======return=====\n");
		return -ENODEV;
    }

    if(twi_id == adapter->nr) {
		strlcpy(info->type, FM_NAME, I2C_NAME_SIZE);
		dprintk(DEBUG_INIT,"======detect ok !=====\n");
	} else {
	        return -ENODEV;
	}
}

static struct i2c_driver bk1088_i2c_driver = {
	.class          = I2C_CLASS_HWMON,
    .probe      = bk1088_i2c_probe,
    .remove     = bk1088_i2c_remove,
    .id_table   = bk1088_i2c_id,
    .driver = {
    	.name   = FM_NAME,
        .owner    = THIS_MODULE,
    },
    .address_list	= bk1088_i2c,
	.detect 	= bk1088_detect,
};
#endif

/*  __u32 Set_default_sksnr(void)
{
	__u8 REG6[2];
	__u32 SKSNR;
	bk_i2c_read(6,REG6,2);
	SKSNR = REG6[0]&0x7f;
	printk("SKSNR = %d\n",SKSNR);
}

 __u32 Set_default_sksnr(void)
{
	__u8 REG6[2];
	__u32 SKSNR;
	bk_i2c_read(6,REG6,2);
	SKSNR = REG6[0]&0x7f;
	printk("SKSNR = %d\n",SKSNR);
}

 __u32 Get_default_sksnr(void)
{
	__u8 REG6[2];
	__u32 SKSNR;
	bk_i2c_read(6,REG6,2);
	SKSNR = REG6[0]&0x7f;
	printk("SKSNR = %d\n",SKSNR);
}

 __u32 Get_default_rssi(void)
{
	__u8 REG5[2];
	__u32 RSSI;
	bk_i2c_read(5,REG5,2);
	RSSI = REG5[0]&0xfe;
	printk("SEEKTH = %d\n",RSSI);
	return RSSI;
} */

__s32 set_reg2(__u8 reg2_value)
{
	__u8 TmpData8[2];
	
	bk_i2c_read(2,TmpData8,2);
	//printk("read TmpData8 = 0x%02x%02x,0x%02x%02x\n",TmpData8[0],TmpData8[1]);
	if(reg2_value)
	{
		TmpData8[0] |= 0x03;
		//printk("reg3_value = 1,TmpData8 = 0x%02x%02x\n",TmpData8[0]);
	}
	else 
	{	
		TmpData8[0] &= 0xf7;
		//printk("reg3_value = 0, TmpData8 = 0x%02x%02x,0x%02x%02x\n",TmpData8[0],TmpData8[1]);
	}
	//printk("write TmpData8 = 0x%02x%02x,0x%02x%02x\n",TmpData8[0],TmpData8[1]);
	bk_i2c_write(2,TmpData8,2);
	
	__u8 reg2[2];
	bk_i2c_read(2,reg2,2);
	printk(" reg2 = 0x%02x%02x\n",reg2[0],reg2[1]);
}

static int fm_bk1088_init(void){
	dprintk(DEBUG_INIT,"%s\n",__func__);
	int err;
	int i,j;
	__s32 m;
	u8 deviceID[2] = {0x37,0xDA};	
	u8 testbuff[66] = {0};
	u16 channel[1]={0};
	struct device_node *np = NULL;

	alloc_chrdev_region(&devid, 0, 1, "bk1088");
	bk1088_cdev = cdev_alloc();
	cdev_init(bk1088_cdev, &bk1088_fops);
	bk1088_cdev->owner = THIS_MODULE;
	err = cdev_add(bk1088_cdev, devid, 1);
	if (err) {
		printk("cdev_add fail\n");
		return -1;
	}

	bk1088_class = class_create(THIS_MODULE, "bk1088");
	if (IS_ERR(bk1088_class))	{
		printk("class_create fail\n");
		return -1;
	}

	bk1088_dev = device_create(bk1088_class, NULL, devid, NULL, "bk1088");
#if USE_I2C_ADAPT	
	ret = i2c_add_driver(&bk1088_i2c_driver);
#else

	np = of_find_node_by_path("/soc@03000000/fm_bk1088");
	if(!np){
		printk("find bk1088 node fail\n");
	}

	gpio_init(np);
	bk_power_init(np);
#endif
	fm_init(0);	

	for(i = 0;i < 32;i++)
	{
		bk_i2c_read(i,testbuff,2);
		for(j=0;j<2;j++)
		{
			printk("%d, %02x\n",i,testbuff[j]);
		}
	}

	bk_power_vcc_on_off(0);

	/* i=0;
	int ret;
	__s32 baseband = 87100;
	__s32 space = 100;
	__s32 curFreq = baseband;
	while(1)
	{
		printk("enter while\n");
		if(curFreq > 108000)
		{
			printk("curFreq is > 108000,so restart ");
			curFreq = baseband;
			i = 0;
		}
		printk("ready to enter auto search\n");
		ret = fm_auto_search(curFreq,NULL);
		if(ret == -1)
		{
			printk("ret = -1,curFreq = %d\n",ret);
			break;
		}	
		printk("curFreq = %d\n",curFreq);
		i++;
		curFreq = baseband + i*space;
		mdelay(5000);
		curFreq = 90500;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 91800;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 93900;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 95800;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 97200;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 100500;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 101200;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 102500;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 103000;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 104200;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 105000;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
		curFreq = 107100;
		fm_auto_search(curFreq,NULL);
		mdelay(5000);
	} */
	
	//fm_auto_search(104300,NULL);
	//fm_auto_search(97100,NULL);
	/* set_reg2(1);
	while(1){
		__u8 reg11[2];
		bk_i2c_read(11,reg11,2);
		printk(" reg11 = 0x%02x%02x\n",reg11[0],reg11[1]);
	} */
	
	//get_seek_freq();
	//fm_play(90500);
	/*  printk("reg 3:0x%02x%02x\n",testbuff[0],testbuff[1]);
	 bk_i2c_read(11,testbuff,2);
	 channel[0] = ((testbuff[0] << 8) | testbuff[1]); //get reg:11 [14-0] data
	 printk("channel is :0x%02x\n",channel[0] & 0x7FFF);
	 BK1088_ChanToFreq(testbuff[1]);
	 printk("reg 11:0x%02x%02x\n",testbuff[0],testbuff[1]);
	 get_seek_freq(); */

	

	return 0;
}

static void fm_bk1088_exit(void){
	dprintk(DEBUG_INIT,"%s\n",__func__);
#if USE_I2C_ADAPT
	i2c_del_driver(&bk1088_i2c_driver);
#else
	gpio_deinit();
#endif
	device_destroy(bk1088_class,devid);
	class_destroy(bk1088_class);
	cdev_del(bk1088_cdev);
}




module_init(fm_bk1088_init);
module_exit(fm_bk1088_exit);
MODULE_AUTHOR("LYF");
MODULE_DESCRIPTION("fm driver");
MODULE_LICENSE("GPL");
