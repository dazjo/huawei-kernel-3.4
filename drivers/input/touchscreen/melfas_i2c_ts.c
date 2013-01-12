/* drivers/input/touchscreen/melfas_i2c_ts.c
 *
 * Copyright (C) 2010 Huawei, Inc.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
 
#include <linux/input.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <mach/gpio.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <mach/mpp.h>
#include <mach/vreg.h>
#include <linux/namei.h>
#include <linux/io.h>
#include <linux/vmalloc.h>
#include <asm/uaccess.h>
#include <asm/mach-types.h>
#include <linux/earlysuspend.h>
#include <linux/hardware_self_adapt.h>
#include <asm/system.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <linux/hw_dev_dec.h>
#endif
/*add this macro for probe phase debugging*/
//#define TS_MELFAS_DEBUG
#include <linux/touch_platform_config.h>
#undef TS_MELFAS_DEBUG
#ifdef TS_MELFAS_DEBUG
#define TS_DEBUG_MELFAS(fmt, args...) printk(KERN_INFO fmt, ##args)
#else
#define TS_DEBUG_MELFAS(fmt,args...)
#endif
static int melfas_debug_mask;
module_param_named(melfas_debug, melfas_debug_mask, int, S_IRUGO | S_IWUSR | S_IWGRP);
#define MELFAS_DEBUG(fmt, args...) do { \
	if(melfas_debug_mask) { \
		printk(KERN_ERR fmt, ##args); \
		} \
} while(0)
/* add new variable */
static bool first_int_flag = true;
#define TS_X_OFFSET		1
#define TS_Y_OFFSET		TS_X_OFFSET
#define TS_SCL_GPIO		131
#define TS_SDA_GPIO		132

#define TS_READ_EVENT_PACKET_SIZE    0x0F
#define TS_READ_START_ADDR 	         0x10
#define TS_READ_VERSION_ADDR         0xF0 //Start read H/W, S/W Version
#define TS_READ_REGS_LEN             66

#define TOUCH_TYPE_NONE       0
#define TOUCH_TYPE_SCREEN     1
#define TOUCH_TYPE_KEY        2

#define PRESS_KEY				1
#define RELEASE_KEY		  0

#define MELFAS_MAX_TOUCH       2
#define I2C_RETRY_CNT			5
struct muti_touch_info
{
    int id;
	int action; 
	int fingerX;
	int fingerY;
	int width;
	int strength;
};

static struct muti_touch_info g_Mtouch_info[MELFAS_MAX_TOUCH];

static int reset_pin = 0;

#define MELFAS_I2C_NAME "melfas-ts"

/*delete some lines*/
static int lcd_x = 0;
static int lcd_y = 0;
static int lcd_all = 0;


struct melfas_ts_data {
	uint16_t addr;
	struct i2c_client *client;
	struct input_dev *input_dev;
    struct input_dev *key_input;
    struct workqueue_struct *melfas_wq;
	struct work_struct  work;
	int use_irq;
	struct hrtimer timer;	
	int (*power)(struct i2c_client* client, int on);
	struct early_suspend early_suspend;
	
    bool is_first_point;
    bool use_touch_key;
    int reported_finger_count;
    bool support_multi_touch;
    uint16_t last_x; 
	uint16_t last_y;
	uint8_t key_index_save;
	unsigned int x_max;
	unsigned int y_max;
};

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h);
static void melfas_ts_late_resume(struct early_suspend *h);
#endif
static irqreturn_t melfas_ts_irq_handler(int irq, void *dev_id);
static struct i2c_client *g_client = NULL;
#define TS_INT_GPIO		82
#define MELFAS_UPDATE_FIRMWARE_MODE_ADDR   0x7D
#define MCSDL_TRANSFER_LENGTH              128		/* Program & Read flash block size */
#define MCSDL_MAX_FILE_LENGTH              62*1024
static int melfas_ts_power(struct i2c_client *client, int on);
static ssize_t update_firmware_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf);
static ssize_t update_firmware_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);
static int mcsdl_enter_download_mode(void);
static int mcsdl_erase_flash(void);
static int mcsdl_read_flash(uint8_t *pBuffer, uint16_t nAddr_start, uint8_t cLength);
static int mcsdl_prepare_program(void);
static int mcsdl_program_flash(uint8_t *pBuffer, uint16_t nAddr_start, uint8_t cLength);
static int mcsdl_download(uint8_t *pgm_data,  uint16_t length );
static int ts_firmware_file(void);
static int i2c_update_firmware(void); 


/*chomod, modify the right*/
static struct kobj_attribute update_firmware_attribute = {
	.attr = {.name = "update_firmware", .mode = 0664},
	.show = update_firmware_show,
	.store = update_firmware_store,
};

static int mcsdl_enter_download_mode(void)
{
	int i;
	int ret = 0;
	int read_data = 0;
	uint8_t enter_code[14] = { 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 1 };

	/* CE set output and low */
	ret = gpio_tlmm_config(GPIO_CFG(reset_pin, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	ret = gpio_direction_output(reset_pin, 1);//0 -> 1

	/* config SCL/SDA output mode and set low  */
	ret = gpio_tlmm_config(GPIO_CFG(TS_SCL_GPIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	ret = gpio_tlmm_config(GPIO_CFG(TS_SDA_GPIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	ret = gpio_direction_output(TS_SCL_GPIO, 0);
	ret = gpio_direction_output(TS_SDA_GPIO, 0);

	/* INTR set output and low */
	ret = gpio_tlmm_config(GPIO_CFG(TS_INT_GPIO, 0, GPIO_CFG_OUTPUT, GPIO_CFG_NO_PULL, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	ret = gpio_direction_output(TS_INT_GPIO, 0);
	mdelay(75);

	ret = gpio_direction_output(reset_pin, 0);			/* CE set high */ // 1 -> 0
	ret = gpio_direction_output(TS_SDA_GPIO, 1);			/* SDA set high */
	mdelay(25);												/* must delay 25msec */

	/* input download mode single */
	for (i=0; i<14; i++) {
		if( enter_code[i] ){
			ret = gpio_direction_output(TS_INT_GPIO, 1);	/* INTR set high */
		} else {
			ret = gpio_direction_output(TS_INT_GPIO, 0);	/* INTR set low */
		}

		ret = gpio_direction_output(TS_SCL_GPIO, 1);		/* SCL set high */
		udelay(15);
		ret = gpio_direction_output(TS_SCL_GPIO, 0);		/* SCL set low */
		ret = gpio_direction_output(TS_INT_GPIO, 0);		/* INTR set low */
		udelay(100);
	}
	ret = gpio_direction_output(TS_SCL_GPIO, 1);			/* SCL set high */
	udelay(100);
	ret = gpio_direction_output(TS_INT_GPIO, 1);			/* INTR set high */
	mdelay(1);

	/* config I/O to i2c mode */
	ret = gpio_tlmm_config(GPIO_CFG(TS_SCL_GPIO, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);

	ret = gpio_tlmm_config(GPIO_CFG(TS_SDA_GPIO, 2, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
	
	mdelay(1);

	read_data = i2c_smbus_read_byte(g_client);
	if (read_data != 0x55) {							/* 0x55 is i2c slave ready status */
		printk("mcsdl enter download mode error\n");
		return -1;
	}

	return 0;		/* success */
}

static int mcsdl_erase_flash(void)
{
	int i;
	int ret = 0;
	int read_data = 0;
	uint8_t i2c_buffer[4] = {0x0F, 						/* isp erase timing cmd */
							0x01, 						/* isp erase timing value 0 */
							0xD4, 						/* isp erase timing value 1 */
							0xC0};						/* isp erase timing value 2 */

	/* Send Erase Setting code */
	for(i=0; i<4; i++){
		ret = i2c_smbus_write_byte(g_client, i2c_buffer[i]);
		if( 0 != ret ){
			printk("mcsdl prepare erase flash error\n");
			return -1;
		}
		udelay(15);
	}
	udelay(500);

	/* Read Result */
	read_data = i2c_smbus_read_byte(g_client);
	if (read_data != 0x8F) {							/* isp ack prepare erase done */
		printk("mcsdl erase flash error0\n");
		return -1;
	}
	mdelay(1);

	/*Send Erase code */
	ret = i2c_smbus_write_byte(g_client, 0x02);
	if( 0 != ret ){
		printk("mcsdl send erase code error\n");
		return -1;
	}
	mdelay(45);
	read_data = i2c_smbus_read_byte(g_client);
	if (read_data != 0x82) {
		printk("mcsdl erase flash error1\n");
		return -1;
	}
	return 0;
}


static int mcsdl_read_flash(uint8_t *pBuffer, uint16_t nAddr_start, uint8_t cLength)
{
	int i;
	int ret = 0;
	int read_data = 0;
	uint8_t   cmd[4];

	cmd[0] = 0x04;											/* isp read flash cmd */
	cmd[1] = (uint8_t)((nAddr_start >> 8) & 0xFF);
	cmd[2] = (uint8_t)((nAddr_start     ) & 0xFF);
	cmd[3] = cLength;

	/* send read command */
 	for(i=0; i<4; i++){
		ret = i2c_smbus_write_byte(g_client, cmd[i]);
		udelay(15);
		if( 0 != ret ){
			printk("mcsdl send read flash cmd error0\n");
			return -1;
		}
 	}

	/* Read 'Result of command' */
	read_data = i2c_smbus_read_byte(g_client);
	if (read_data != 0x84) {								/* 0x84 is prepare read flash OK status */
		printk("mcsdl send read flash cmd error1\n");
		return -1;
	}

	/* Read Data  [ Cmd[3] = Size ] */
	for(i=0; i<(int)cmd[3]; i++){
		udelay(100);
		read_data = i2c_smbus_read_byte(g_client);
		if ( read_data<0 ) {
			printk("mcsdl read flash error\n");
			return -1;
		}
		*pBuffer++ = read_data;
	}
	return 0;
}

static int mcsdl_prepare_program(void)
{
	int i;
	int ret = 0;
	int read_data = 0;
	uint8_t i2c_buffer[4] = {0x0F,							/* isp program flash cmd */
							0x00,							/* isp program timing value 0 */
							0x00,							/* isp program timing value 1 */
							0x78};							/* isp program timing value 2 */

	/* Write Program timing information */
	for(i=0; i<4; i++){
		ret = i2c_smbus_write_byte(g_client, i2c_buffer[i]);
		if( 0 != ret ){
			printk("mcsdl Write Program timing information error\n");
			return -1;
		}
		udelay(15);
	}
	udelay(500);

	/* Read Result */
	read_data = i2c_smbus_read_byte(g_client);
	if (read_data != 0x8F) {								/* 0x8F is prepare program flash OK status */	
		printk("mcsdl prepare program error\n");
		return -1;
	}
	ndelay(100);

	return 0;
}

static int mcsdl_program_flash(uint8_t *pBuffer, uint16_t nAddr_start, uint8_t cLength)
{
	int i;
	int ret = 0;
	int read_data = 0;
	uint8_t   cmd[4];

	cmd[0] = 0x03;											/* isp program flash cmd */
	cmd[1] = (uint8_t)((nAddr_start >> 8) & 0xFF);
	cmd[2] = (uint8_t)((nAddr_start     ) & 0xFF);
	cmd[3] = cLength;

	/* send PGM flash command */
 	for(i=0; i<4; i++){
		ret = i2c_smbus_write_byte(g_client, cmd[i]);
		udelay(15);
		if( 0 != ret ){
			printk("mcsdl send PGM flash cmd error0\n");
			return -1;
		}
 	}

	/* Check command result */
	read_data = i2c_smbus_read_byte(g_client);
	if (read_data != 0x83) {
		printk("mcsdl send PGM flash cmd error1\n");
		return -1;
	}
	udelay(150);

	/* Program Data */
	for(i=0; i<(int)cmd[3]; i+=2){
		ret = i2c_smbus_write_byte(g_client, pBuffer[i+1]);
		if( 0 != ret ){
			printk("mcsdl Program Data error0\n");
			return -1;
		}
		udelay(100);
		ret = i2c_smbus_write_byte(g_client, pBuffer[i]);
		if( 0 != ret ){
			printk("mcsdl Program Data error1\n");
			return -1;
		}
		udelay(150);
	}
	return 0;
}

static int mcsdl_download(uint8_t *pgm_data,  uint16_t length )
{
	int i;
	int ret;
	uint8_t   cLength;
	uint16_t  nStart_address=0;
	uint8_t   buffer[MCSDL_TRANSFER_LENGTH];

	/* Enter Download mode */
	ret = mcsdl_enter_download_mode();
	if (0 != ret) {
		return -1;
	} 
	printk("mcsdl enter download mode success!\n");
	mdelay(1);

	/* erase flash */
	printk("Erasing...\n");
	ret = mcsdl_erase_flash();
	if (0 != ret) {
		return -1;
	}
	printk("Erase OK!\n");
	mdelay(1);

	/* Verify erase */
	printk("Verifying erase...\n");
	ret = mcsdl_read_flash(buffer, 0x00, 16);		/* Must be '0xFF' after erase */
	if (0 != ret) {
		return -1;
	}
	for(i=0; i<16; i++){
		if( buffer[i] != 0xFF ){
			printk("Verify flash error\n");
			return -1;
		}
	}
	mdelay(1);

	/* Prepare for Program flash */
	printk("Preparing Program...\n");
	ret = mcsdl_prepare_program();
	if (0 != ret) {
		return -1;
	}
	mdelay(1);

	/* Program flash */
	printk("Programing flash...");
	nStart_address = 0;
	cLength  = MCSDL_TRANSFER_LENGTH;
	for( nStart_address = 0; nStart_address < length; nStart_address+=cLength ){
		printk("#");
		if( ( length - nStart_address ) < MCSDL_TRANSFER_LENGTH ){
			cLength  = (uint8_t)(length - nStart_address);
			cLength += (cLength%2);									/* For odd length */
		}
		ret = mcsdl_program_flash(&pgm_data[nStart_address], nStart_address, cLength);
		if (0 != ret) {
			printk("\nProgram flash failed.\n");
			return -1;
		}
		ndelay(500);
	}

	/* Verify flash */
	printk("\nVerifying flash...");
	nStart_address = 0;
	cLength  = MCSDL_TRANSFER_LENGTH;
	for( nStart_address = 0; nStart_address < length; nStart_address+=cLength ){
		printk("#");
		if( ( length - nStart_address ) < MCSDL_TRANSFER_LENGTH ){
			cLength  = (uint8_t)(length - nStart_address);
			cLength += (cLength%2);									/* For odd length */
		}
		ret = mcsdl_read_flash(buffer, nStart_address, cLength);

		if (0 != ret) {
			printk("\nVerify flash read failed.\n");
			return -1;
		}
		for(i=0; i<(int)cLength; i++){
			if( buffer[i] != pgm_data[nStart_address+i] ){
				printk("\nVerify flash compare failed.\n");
				return -1;
			}
		}
		ndelay(500);
	}
	printk("\n");

	/* Reset command */
	mdelay(1);
	ret = i2c_smbus_write_byte(g_client, 0x07);						/* 0x07 is reset cmd */
	if( 0 != ret ){
		printk("reset error\n");
		return -1;
	}
	mdelay(180);

	return 0;
}

static int ts_firmware_file(void)
{
	int ret;
	struct kobject *kobject_ts;
	kobject_ts = kobject_create_and_add("touch_screen", NULL);
	if (!kobject_ts) {
		printk("create kobjetct error!\n");
		return -1;
	}
	ret = sysfs_create_file(kobject_ts, &update_firmware_attribute.attr);
	if (ret) {
		kobject_put(kobject_ts);
		printk("create file error\n");
		return -1;
	}
	return 0;	
}

/*
 * The "update_firmware" file where a static variable is read from and written to.
 */
static ssize_t update_firmware_show(struct kobject *kobj, struct kobj_attribute *attr,char *buf)
{
	return 1;
}

static ssize_t 
update_firmware_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	//int i;
	int ret = -1;
	int ret1;
	struct melfas_ts_data *ts = i2c_get_clientdata(g_client);

	printk("Update_firmware_store.\n");
//#ifndef CONFIG_MELFAS_RESTORE_FIRMWARE
//	if ((buf[0] == '2')&&(buf[1] == '\0')) {

		/* driver detect its device  */
/*		for (i = 0; i < 3; i++) {
			ret = i2c_smbus_read_byte_data(g_client, 0x00);
			if (ret >= 0) {
				goto firmware_find_device;
			}
		}
		printk("Dont find melfas_ts device\n");
		return -1;

firmware_find_device: */
//		ret = i2c_smbus_read_byte_data(g_client, 0x21);			/* read firmware version */
//		printk("%s: reg21 = 0x%x\n", __FUNCTION__, ret);
//#else
	if (buf[0] == '2') {
//#endif
		disable_irq(g_client->irq);
		free_irq(g_client->irq, ts);
		g_client->addr = MELFAS_UPDATE_FIRMWARE_MODE_ADDR;

		/*update firmware*/
		ret = i2c_update_firmware();

		gpio_request(g_client->irq, g_client->name);
		gpio_direction_input(g_client->irq);


		ret1 = request_irq(g_client->irq, melfas_ts_irq_handler,
				IRQF_TRIGGER_FALLING, g_client->name, ts);
		if (0 != ret1)
		{
			printk("request irq failed!\n");
		}
		mdelay(10);
		g_client->addr = 0x23;									/* touchscreen i2c addr */
		enable_irq(g_client->irq);

		if( 0 != ret ){
			printk("Update firmware failed!\n\n");
		} else {
			printk("update firmware success!\n\n");
			mdelay(200);										/* for "printk" */
#ifdef CONFIG_MELFAS_RESTORE_FIRMWARE
			mdelay(8000);
#else
			arm_pm_restart(0,NULL);
#endif
		}
	}

	return ret;
 }

static int i2c_update_firmware(void)
{
	char *buf;
	struct file	*filp;
    struct inode *inode = NULL;
	mm_segment_t oldfs;
    uint16_t	length;
	int ret = 0;           //"/data/melfas_ts_update_firmware.bin";
	const char filename[]="/sdcard/update/melfas.bin";

	/* open file */
    oldfs = get_fs();
    set_fs(KERNEL_DS);
    filp = filp_open(filename, O_RDONLY, S_IRUSR);
    if (IS_ERR(filp)) {
        printk("%s: file %s filp_open error\n", __FUNCTION__,filename);
        set_fs(oldfs);
        return -1;
    }

    if (!filp->f_op) {
        printk("%s: File Operation Method Error\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
        return -1;
    }

    inode = filp->f_path.dentry->d_inode;
    if (!inode) {
        printk("%s: Get inode from filp failed\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
        return -1;
    }
    printk("%s file offset opsition: %u\n", __FUNCTION__, (unsigned)filp->f_pos);

    /* file's size */
    length = i_size_read(inode->i_mapping->host);
   	printk("%s: length=%d\n", __FUNCTION__, length);
	if (!( length > 0 && length < MCSDL_MAX_FILE_LENGTH )){
		printk("file size error\n");
		filp_close(filp, NULL);
        set_fs(oldfs);
		return -1;
	}

	/* allocation buff size */
	buf = vmalloc((length+(length%2)));		/* buf size if even */
	if (!buf) {
		printk("alloctation memory failed\n");
		filp_close(filp, NULL);
        set_fs(oldfs);
		return -1;
	}
	if ( length%2 == 1 ) {
		buf[length] = 0xFF;						  		/* Fill Empty space */
	}

    /* read data */
    if (filp->f_op->read(filp, buf, length, &filp->f_pos) != length) {
        printk("%s: file read error\n", __FUNCTION__);
        filp_close(filp, NULL);
        set_fs(oldfs);
		vfree(buf);
        return -1;
    }

#ifndef CONFIG_MELFAS_RESTORE_FIRMWARE
	/* disable other I2C device */
	/*if (&TS_updateFW_gs_data->timer != NULL) {
		if (TS_updateFW_gs_data->use_irq) {
			disable_irq(TS_updateFW_gs_data->client->irq);
		} else {
			hrtimer_cancel(&TS_updateFW_gs_data->timer);
		}
		cancel_work_sync(&TS_updateFW_gs_data->work);
		mutex_lock(&TS_updateFW_gs_data->mlock);
		i2c_smbus_write_byte_data(TS_updateFW_gs_data->client, 0x20, 0);
		mutex_unlock(&TS_updateFW_gs_data->mlock);
		printk("hrtimer_cancel_GS\n");
	}
	if (&TS_updateFW_aps_data->timer != NULL) {
		hrtimer_cancel(&TS_updateFW_aps_data->timer);
		cancel_work_sync(&TS_updateFW_aps_data->work);
		if (TS_updateFW_aps_wq) {
			printk("destroy_aps_wq\n");
			destroy_workqueue(TS_updateFW_aps_wq);
		}
		mutex_lock(&TS_updateFW_aps_data->mlock);
		i2c_smbus_write_byte_data(TS_updateFW_aps_data->client, 0, 0);
		mutex_unlock(&TS_updateFW_aps_data->mlock);
		printk("hrtimer_cancel_APS\n");
	}
	mdelay(1000);*/
#endif

	ret = mcsdl_download(buf, length+(length%2));

 	filp_close(filp, NULL);
    set_fs(oldfs);
	vfree(buf);
	printk("%s: free file buffer\n", __FUNCTION__);
	return ret;
}

static struct i2c_msg query_i2c_msg_name[2];
static uint8_t query_name[5];
static uint8_t vision_reg = 0xF1;
/* same as in proc_misc.c */
static int proc_calc_metrics(char *page, char **start, off_t off, int count, int *eof, int len)
{
	if (len <= off + count)
		*eof = 1;
	*start = page + off;
	len -= off;
	if (len > count)
		len = count;
	if (len < 0)
		len = 0;
	return len;
}
/*get some touchscreen info from tp chip by I2C*/
static int tp_read_input_name(void)
{
	int ret;
	
	query_i2c_msg_name[0].addr = g_client->addr;
	query_i2c_msg_name[0].flags = 0;
	query_i2c_msg_name[0].buf = &vision_reg;
	query_i2c_msg_name[0].len = 1;

	query_i2c_msg_name[1].addr = g_client->addr;
	query_i2c_msg_name[1].flags = I2C_M_RD;
	query_i2c_msg_name[1].buf = query_name;
	query_i2c_msg_name[1].len = sizeof(query_name);

	ret = i2c_transfer(g_client->adapter, query_i2c_msg_name, 2);
	if (ret < 0) 
	{
		printk(KERN_ERR "%s: i2c_transfer failed\n", __func__);
	}
	
	return ret;

}
static int tp_read_proc(
	char *page, char **start, off_t off, int count, int *eof, void *data)
{
	int len;
	len = snprintf(page, PAGE_SIZE, "TP_TYPE:"
			"%s\n"
			"Manufacturer ID:%x\n"
			"Product Properties:%x\n"
			"Customer Family:%x\n"
			"Firmware Revision:%x\n",
			"melfas",query_name[0], query_name[3], query_name[4], query_name[2]);

	return proc_calc_metrics(page, start, off, count, eof, len);
    
}
/*report 0 to generate a touch-up event*/
static void clear_pressed_point_status(struct melfas_ts_data *ts)
{
    int i =0;
    if (ts->support_multi_touch)
	{
		for (i = 0; i < MELFAS_MAX_TOUCH; i++)
		{
            input_report_abs(ts->input_dev, ABS_MT_POSITION_X,  0);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_Y,  0);
            input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0);
            input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0);
			input_mt_sync(ts->input_dev);				
        }		
	}
    else
	{
		input_report_abs(ts->input_dev, ABS_X, 0);
		input_report_abs(ts->input_dev, ABS_Y, 0);
		input_report_abs(ts->input_dev, ABS_PRESSURE, 0);
		input_report_key(ts->input_dev, BTN_TOUCH, 0);
		input_sync(ts->input_dev);
	}
    input_sync(ts->input_dev);
    memset(g_Mtouch_info, 0, sizeof(g_Mtouch_info));
}
/* add function to get module ID */
#define OFILM_MODULE 0X00
#define MUTTO_MODULE 0X01
#define TRULY_MODULE 0X02
#define BYD_MODULE   0X03
#define TPK_MODULE   0X04
#define CMI_MODULE   0X05
#define EELY_MODULE  0X06

static char touch_info[50] = {0};
char * get_melfas_touch_info(void)
{
	char * module_name = NULL;

	if (g_client == NULL)
		return NULL;

	switch(query_name[4])
	{
		case OFILM_MODULE:
			module_name = "OFILM";
			break;
		case MUTTO_MODULE:
			module_name = "MUTTO";
			break;
		case TRULY_MODULE:
			module_name = "TRULY";
			break;
		case BYD_MODULE:
			module_name = "BYD";
			break;
		case TPK_MODULE:
			module_name = "TPK";
			break;
		case CMI_MODULE:
			module_name = "CMI";
			break;
		case EELY_MODULE:
			module_name = "EELY";
			break;
		default:
			break;
	}
	
	sprintf(touch_info,"melfas-%s.%d",module_name,query_name[2]);

	return touch_info;
}
static void melfas_ts_work_func(struct work_struct *work)
{
	struct melfas_ts_data *ts = container_of(work, struct melfas_ts_data, work);
	int ret = 0, i;
	uint8_t buf[TS_READ_REGS_LEN];
	uint8_t read_num = 0;
	uint8_t touchAction = 0, touchType = 0, fingerID = 0;
	int k = 0;
	u8 finger_pressed_count = 0;

	TS_DEBUG_MELFAS(KERN_ERR "melfas_ts_work_func\n");
	if (ts == NULL)
		MELFAS_DEBUG(KERN_ERR "melfas_ts_work_func : TS NULL\n"); 

	/*   Simple send transaction:  
	 * S Addr Wr [A]  Data [A] Data [A] ... [A] Data [A] P   
	 * Simple recv transaction:    	
	 * S Addr Rd [A]  [Data] A [Data] A ... A [Data] NA P    
	 */    
    for( i=0; i<I2C_RETRY_CNT; i++ )
    {
        buf[0] = TS_READ_EVENT_PACKET_SIZE;
        ret = i2c_master_send(ts->client, buf, 1);

        if (ret >= 0)
        {
            ret = i2c_master_recv(ts->client, buf, 1);

			MELFAS_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_recv [%d]:%d\n", ret,buf[0]);

            if (ret == 1)
            {
                read_num = buf[0];
                if(read_num > TS_READ_REGS_LEN)
                    read_num = TS_READ_REGS_LEN;
                break; // i2c success
            }
        }
    }
    if(ret < 0 ) // ret < 0
    {
		MELFAS_DEBUG(KERN_ERR "melfas_ts_work_func: i2c failed\n");     
		enable_irq(ts->client->irq);
		return ; 
    }

    for( i=0; i<I2C_RETRY_CNT; i++ )
    {
		buf[0] = TS_READ_START_ADDR;
		ret = i2c_master_send(ts->client, buf, 1);
		MELFAS_DEBUG(KERN_ERR "melfas_ts_work_func : i2c_master_send [%d]\n", ret);

		if (ret >= 0)
		{ 	
			ret = i2c_master_recv(ts->client, buf, read_num); 
            if (ret == read_num)
			{
				break;
			}
        }
	}
		
    if(ret < 0) 
    {
        printk(KERN_ERR "melfas_ts_work_func: i2c failed\n");
		if (ts->use_irq)
		{
        	enable_irq(ts->client->irq);
		}
        return ;
    }
    else // MIP (Melfas Interface Protocol)
    {

        /*printf debug info*/
        for(k=0; k < read_num; k++)
        {
            MELFAS_DEBUG("%s:register[0x%x]= 0x%x \n",__FUNCTION__, TS_READ_START_ADDR+k, buf[k]);
        }
				 
		for(i = 0; i < read_num; i = i + 6)  
		{            
			touchAction = ((buf[i] & 0x80) == 0x80);
			touchType = (buf[i] & 0x60)>>5; 
			fingerID = (buf[i] & 0x0F);
			
			MELFAS_DEBUG("%s:touchAction:0x%x,touchType:0x%x,fingerID:0x%x \n",__FUNCTION__,touchAction,touchType,fingerID);

			if(touchType == TOUCH_TYPE_NONE)
			{
			} 
			else if(touchType == TOUCH_TYPE_SCREEN)
			{ 
                g_Mtouch_info[fingerID-1].id = fingerID;
				g_Mtouch_info[fingerID-1].action = touchAction;
				g_Mtouch_info[fingerID-1].fingerX = (buf[i + 1] & 0x0F) << 8 | buf[i + 2];
				g_Mtouch_info[fingerID-1].fingerY = (buf[i + 1] & 0xF0) << 4 | buf[i + 3]; 
				g_Mtouch_info[fingerID-1].width = buf[i + 4];   
				g_Mtouch_info[fingerID-1].strength = buf[i + 5];  
								

				MELFAS_DEBUG(KERN_ERR "melfas_ts_work_func: Touch ID: %d, x: %d, y: %d, z: %d w: %d\n", 
						i, g_Mtouch_info[fingerID-1].fingerX, g_Mtouch_info[fingerID-1].fingerY, g_Mtouch_info[fingerID-1].strength, g_Mtouch_info[fingerID-1].width);          
			}
		}   
	} 
	if (ts->support_multi_touch)
	{
		for (i = 0; i < fingerID; i++)
		{
            input_report_abs(ts->input_dev, ABS_MT_POSITION_X,  g_Mtouch_info[i].fingerX);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_Y,  g_Mtouch_info[i].fingerY);
            input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, g_Mtouch_info[i].strength);
            //input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, g_Mtouch_info[i].width);
			input_report_abs(ts->input_dev, ABS_MT_PRESSURE, g_Mtouch_info[i].strength);
			input_mt_sync(ts->input_dev);	

			if (g_Mtouch_info[i].strength)
				finger_pressed_count++;
        }
		input_report_key(ts->input_dev, BTN_TOUCH, finger_pressed_count);
	}
	else
	{
		input_report_abs(ts->input_dev, ABS_X, g_Mtouch_info[0].fingerX);
		input_report_abs(ts->input_dev, ABS_Y, g_Mtouch_info[0].fingerY);	
		/* reports pressure value for inputdevice*/
		input_report_abs(ts->input_dev, ABS_PRESSURE, g_Mtouch_info[0].strength);
		/*when report width , framework don't identify the point*/
		//input_report_abs(ts->input_dev, ABS_TOOL_WIDTH, g_Mtouch_info[0].width/10);
		input_report_key(ts->input_dev, BTN_TOUCH, g_Mtouch_info[0].action);
		input_sync(ts->input_dev);
	}

    input_sync(ts->input_dev);
		
	if (ts->use_irq)
	{
	    enable_irq(ts->client->irq);
	    MELFAS_DEBUG("melfas_ts_work_func,enable_irq\n");
	}
}

static enum hrtimer_restart melfas_ts_timer_func(struct hrtimer *timer)
{
	struct melfas_ts_data *ts = container_of(timer, struct melfas_ts_data, timer);
	MELFAS_DEBUG("melfas_ts_timer_func\n");
	queue_work(ts->melfas_wq, &ts->work);
	hrtimer_start(&ts->timer, ktime_set(0, 12500000), HRTIMER_MODE_REL);
	return HRTIMER_NORESTART;
}

static irqreturn_t melfas_ts_irq_handler(int irq, void *dev_id)
{
	struct melfas_ts_data *ts = dev_id;
	if (first_int_flag)
	{
		first_int_flag = false;
		return IRQ_HANDLED;
	}
	disable_irq_nosync(ts->client->irq);
 	MELFAS_DEBUG("melfas_ts_irq_handler: disable irq\n");
	queue_work(ts->melfas_wq, &ts->work);
	return IRQ_HANDLED;
}

static int melfas_ts_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct melfas_ts_data *ts;
	struct proc_dir_entry *d_entry;
	int ret = 0;
	int i;
	struct touch_hw_platform_data *touch_pdata = NULL;
	struct tp_resolution_conversion tp_type_self_check;
	
	TS_DEBUG_MELFAS(" In melfas_ts_probe: \n");

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) 
	{
		pr_err(KERN_ERR "melfas_ts_probe: need I2C_FUNC_I2C\n");
		ret = -ENODEV;
		goto err_check_functionality_failed;
	}

	touch_pdata = client->dev.platform_data;
	if(NULL == touch_pdata)
	{
		printk("the touch_pdata is NULL please check the init code !\n");
		goto err_check_functionality_failed;
	}

    /* get reset pin */
	/* change function name */
	if(touch_pdata->get_touch_reset_gpio)
	{
		reset_pin = touch_pdata->get_touch_reset_gpio();
	}
	
	if(touch_pdata->read_touch_probe_flag)
	{
		ret = touch_pdata->read_touch_probe_flag();
	}
	if(ret)
	{
		printk(KERN_ERR "%s: the touch driver has detected! \n", __func__);
		return -ENODEV;
	}
	else
	{
		printk(KERN_ERR "%s: it's the first touch driver! \n", __func__);
	}

	/* power on touchscreen */   
	/*make power on*/
    if(touch_pdata->touch_power)
    {
        ret = touch_pdata->touch_power(1);
    }
    if(ret)
    {
        printk(KERN_ERR "%s: power on failed \n", __func__);
        goto err_check_functionality_failed;
    }
	/* change function name */
	if(touch_pdata->get_touch_resolution)
    {
        ret = touch_pdata->get_touch_resolution(&tp_type_self_check);
        if(ret < 0)
        {
            printk(KERN_ERR "%s: reset failed \n", __func__);
            goto err_power_failed;
        }
        else
        {
            lcd_x = tp_type_self_check.lcd_x;
            lcd_y = tp_type_self_check.lcd_y;
            lcd_all = tp_type_self_check.lcd_all;
        }
    }
	/*delete some lines*/


	melfas_ts_power(client,1);
	msleep(200);  /* wait for device reset; */
	
	for(i = 0; i < 3; i++) 
	{		
		ret = i2c_smbus_read_byte_data(client, 0x00);
		TS_DEBUG_MELFAS("id:%d\n",ret);
		if (ret < 0)
			continue;
		else
			break;
	}
	if (i == 3) 
	{	
		pr_err("%s:check %d times, but dont find melfas_ts device\n",__FUNCTION__, i);
		melfas_ts_power(client,0);

		goto err_find_touchpanel_failed;
	}

	g_client = client;
	for (i = 0 ; i < 3; i++) 
	{
		ret= ts_firmware_file();   
		if (!ret)
			break;
	}

	/*create entry in proc dir for touchscreen info */
	ret = tp_read_input_name();
	if(!ret)
	{
		printk("the tp input name is query error!\n ");
	}
	
	d_entry = create_proc_entry("tp_hw_type", S_IRUGO | S_IWUSR | S_IWGRP, NULL);
	if (d_entry) 
	{
		d_entry->read_proc = tp_read_proc;
		d_entry->data = NULL;
	}
	
	ts = kzalloc(sizeof(*ts), GFP_KERNEL);
	if (ts == NULL) 
	{
		ret = -ENOMEM;
		goto err_alloc_data_failed;
	}

	ts->client = client;
	i2c_set_clientdata(client, ts);

	ts->melfas_wq = create_singlethread_workqueue("melfas_wq");
	if (!ts->melfas_wq) 
	{
		pr_err("%s:create melfas_wq error\n",__FUNCTION__);
		ret = -ENOMEM;
		goto err_destroy_wq;
	}
	INIT_WORK(&ts->work, melfas_ts_work_func);
		
    ts->is_first_point = true;
    ts->support_multi_touch = client->flags;
	/*delete some lines*/

	ts->input_dev = input_allocate_device();
	if (ts->input_dev == NULL) {
		ret = -ENOMEM;
		pr_err("melfas_ts_probe: Failed to allocate touch input device\n");
		goto err_input_dev_alloc_failed;
	}
	ts->input_dev->name = "melfas-touchscreen";
	ts->input_dev->phys = client->name;
	set_bit(EV_SYN, ts->input_dev->evbit);
	set_bit(EV_KEY, ts->input_dev->evbit);
	set_bit(BTN_TOUCH, ts->input_dev->keybit);
	set_bit(EV_ABS, ts->input_dev->evbit);
	set_bit(ABS_X, ts->input_dev->absbit);
	set_bit(ABS_Y, ts->input_dev->absbit);
	set_bit(KEY_NUMLOCK, ts->input_dev->keybit);


	/*add INPUT_PROP_DIRECT property*/
	set_bit(INPUT_PROP_DIRECT,ts->input_dev->propbit);

    if(ts->support_multi_touch)
    {
    	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X, 0, lcd_x, 0, 0);
    	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y, 0, lcd_y, 0, 0);
    	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);
    	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 15, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_MT_PRESSURE, 0,	255, 0, 0);
    }
	else
	{
		input_set_abs_params(ts->input_dev, ABS_X, 0, lcd_x, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_Y, 0, lcd_y, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_PRESSURE, 0, 255, 0, 0);
		input_set_abs_params(ts->input_dev, ABS_TOOL_WIDTH, 0, 255, 0, 0);
	}
	
	ret = input_register_device(ts->input_dev);
	if (ret) 
	{
		pr_err("melfas_ts_probe: Unable to register %s input device\n", ts->input_dev->name);
		goto err_input_register_device_failed;
	}
	else 
	{
		TS_DEBUG_MELFAS("melfas input device registered\n");
	}
  
	/*delete some lines*/
	/*set tht bit means detected the tocuh yet in the next time we don't probe it*/
    if(touch_pdata->set_touch_probe_flag)
    {
        touch_pdata->set_touch_probe_flag(1);
    }
	
	if (client->irq) 
    {
		/* change function name */
		if(touch_pdata->set_touch_interrupt_gpio)
		{
			ret = touch_pdata->set_touch_interrupt_gpio();		
		}
		if (ret < 0) 
		{
	   	 	pr_err("%s: gpio_tlmm_config(%#x)=%d\n", __func__, client->irq, ret);
			ret = -EIO;
			goto err_key_input_register_device_failed; 
		}

		if (request_irq(client->irq, melfas_ts_irq_handler,
				IRQF_TRIGGER_FALLING, client->name, ts) >= 0) 
		{
			TS_DEBUG_MELFAS("Received IRQ!\n");
			ts->use_irq = 1;
			if (irq_set_irq_wake(client->irq, 1) < 0)
            {
            	printk(KERN_ERR "failed to set IRQ wake\n");
            }         
		} 
        else 
        {
			TS_DEBUG_MELFAS("Failed to request IRQ!\n");
		}
	}

	if (!ts->use_irq) 
	{
		hrtimer_init(&ts->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
		ts->timer.function = melfas_ts_timer_func;
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	}

#ifdef CONFIG_HAS_EARLYSUSPEND
	ts->early_suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	ts->early_suspend.suspend = melfas_ts_early_suspend;
	ts->early_suspend.resume = melfas_ts_late_resume;
	register_early_suspend(&ts->early_suspend);
#endif

	/*move before*/
	printk(KERN_INFO "melfas_ts_probe: Start touchscreen %s in %s mode\n", ts->input_dev->name, ts->use_irq ? "interrupt" : "polling");
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
	/* detect current device successful, set the flag as present */
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
#endif

	return 0;

err_key_input_register_device_failed:
    if(ts->use_touch_key)
    {
	    input_free_device(ts->key_input);
	}
/*delete one line*/

err_input_register_device_failed:
	input_free_device(ts->input_dev);
//err_pdt_read_failed:
err_input_dev_alloc_failed:
err_destroy_wq:
	destroy_workqueue(ts->melfas_wq);
	kfree(ts);
err_alloc_data_failed:
err_find_touchpanel_failed:
err_power_failed:
    if(touch_pdata->touch_power)
	{
		ret = touch_pdata->touch_power(0); 
	}
	printk(KERN_ERR "the power is off: vreg_s3 = %d \n ", ret);   
err_check_functionality_failed:
	return ret;
}

static int melfas_ts_power(struct i2c_client *client, int on)
{
    int ret = 0;
    TS_DEBUG_MELFAS("melfas_ts_power on = %d\n", on);

    ret = gpio_tlmm_config(GPIO_CFG(reset_pin, 0, GPIO_CFG_OUTPUT, GPIO_CFG_PULL_UP, GPIO_CFG_2MA), GPIO_CFG_ENABLE);
    if(ret < 0)
    {
        pr_err("%s: fail to config reset_pin(#%d)\n", __func__,reset_pin);
    }

	/*the CE PIN is reverse*/
    if (on) 
    {			  
    	ret = gpio_direction_output(reset_pin, 0);
    	if (ret) 
    	{
    		pr_err("%s: Failed to configure power on = (%d)\n",__func__, ret);
    	} 
    }
    else 
    {
    	ret = gpio_direction_output(reset_pin, 1);
    	if (ret) 
    	{
    		pr_err("%s: Failed to configure power off = (%d)\n",__func__, ret);
    	}       	
    }	

	return ret;	
}

static int melfas_ts_remove(struct i2c_client *client)
{
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	unregister_early_suspend(&ts->early_suspend);
	if (ts->use_irq)
		free_irq(client->irq, ts);
	else
		hrtimer_cancel(&ts->timer);
	input_unregister_device(ts->input_dev);
	kfree(ts);
	return 0;
}

static int melfas_ts_suspend(struct i2c_client *client, pm_message_t mesg)
{
	int ret;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	MELFAS_DEBUG("In melfas_ts_suspend\n");
	
	if (ts->use_irq)
		disable_irq(client->irq);
	else
		hrtimer_cancel(&ts->timer);
	ret = cancel_work_sync(&ts->work);
	if (ret && ts->use_irq) /* if work was pending disable-count is now 2 */
	{
		enable_irq(client->irq);
	}
	first_int_flag = true;
	ret = melfas_ts_power(client,0);
	if (ret < 0)
	{
		pr_err("melfas_ts_suspend: power off failed\n");
	}
	
	return 0;
}

static int melfas_ts_resume(struct i2c_client *client)
{
	int ret,i;
	struct melfas_ts_data *ts = i2c_get_clientdata(client);
	MELFAS_DEBUG("In melfas_ts_resume\n");

    /*clear the unreleased touch status to solve the TP disfunction*/
    for (i = 0; i < MELFAS_MAX_TOUCH; i++)
    {
        if (0 != g_Mtouch_info[i].strength)
        {
            MELFAS_DEBUG("clear the pressed point status, fingerID is %d\n",i);
            clear_pressed_point_status(ts);
            break;
        }
    }

	ret = melfas_ts_power(client,1);	
	if (ret < 0) 
	{
	    pr_err("melfas_ts_resume: power on failed\n");			
	}

	msleep(200);  /* wait for device reset; */
	
	if (ts->use_irq) 
	{
		enable_irq(client->irq);
	}

	else
		hrtimer_start(&ts->timer, ktime_set(1, 0), HRTIMER_MODE_REL);
	return 0;
}

#ifdef CONFIG_HAS_EARLYSUSPEND
static void melfas_ts_early_suspend(struct early_suspend *h)
{
	struct melfas_ts_data *ts;

	MELFAS_DEBUG("melfas_ts_early_suspend\n");
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	melfas_ts_suspend(ts->client, PMSG_SUSPEND);  
}

static void melfas_ts_late_resume(struct early_suspend *h)
{
	struct melfas_ts_data *ts;

	MELFAS_DEBUG("melfas_ts_late_resume\n");
	ts = container_of(h, struct melfas_ts_data, early_suspend);
	melfas_ts_resume(ts->client);	
}
#endif

static const struct i2c_device_id melfas_ts_id[] = {
	{ MELFAS_I2C_NAME, 0 },
	{ }
};

static struct i2c_driver melfas_ts_driver = {
	.probe		= melfas_ts_probe,
	.remove		= melfas_ts_remove,
#ifndef CONFIG_HAS_EARLYSUSPEND
	.suspend	= melfas_ts_suspend,
	.resume		= melfas_ts_resume,
#endif
	.id_table	= melfas_ts_id,
	.driver = {
		.name	= MELFAS_I2C_NAME,
	},
};

static int __devinit melfas_ts_init(void)
{
	TS_DEBUG_MELFAS(KERN_ERR "melfas_ts_init\n ");
	return i2c_add_driver(&melfas_ts_driver);
}

static void __exit melfas_ts_exit(void)
{
	i2c_del_driver(&melfas_ts_driver);
}

module_init(melfas_ts_init);
module_exit(melfas_ts_exit);

MODULE_DESCRIPTION("Melfas Touchscreen Driver");
MODULE_LICENSE("GPL");
