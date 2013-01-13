#if (defined(CONFIG_HUAWEI_KERNEL))

#include <linux/module.h>	/* kernel module definitions */
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/notifier.h>
#include <linux/proc_fs.h>
#include <linux/spinlock.h>
#include <linux/timer.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>

#include <linux/irq.h>
#include <linux/param.h>
#include <linux/bitops.h>
#include <linux/termios.h>
#include <linux/gpio.h>
#include <mach/msm_serial_hs.h>
#include <linux/mtk6252_dev.h>

#define GS_INFO(fmt, arg...) printk(KERN_INFO "GPIOSLEEP: " fmt "\n" , ## arg)
#define GS_ERR(fmt, arg...)  printk(KERN_ERR "%s: " fmt "\n" , __func__ , ## arg)
#define GS_DBG(fmt, arg...)  pr_debug("%s: " fmt "\n" , __func__ , ## arg)

#define ENABLE_IRQ_AND_TIMER 1 // for debug, disable irq and timer when undefine this macro

/*if need open log, define GPIO_SLEEP_DBG */
//#define GPIO_SLEEP_DBG  
#ifndef GPIO_SLEEP_DBG
#undef GS_DBG
#define GS_DBG(fmt, arg...)
#endif

#define PROC_DIR	"gpiosleep"

struct gpiosleep_info {
	unsigned host_wake;
	unsigned ext_wake;
	unsigned host_wake_irq;
	struct uart_port *uport;
};

/* work function */
static void gpiosleep_sleep_work(struct work_struct *work);

/* work queue */
/* Macros for handling sleep work */
static struct workqueue_struct *gpiosleep_workqueue=NULL;
static struct work_struct  sleep_work;

#define gpiosleep_rx_busy()     queue_work(gpiosleep_workqueue, &sleep_work)
#define gpiosleep_tx_busy()     queue_work(gpiosleep_workqueue, &sleep_work)
#define gpiosleep_rx_idle()     queue_work(gpiosleep_workqueue, &sleep_work)
#define gpiosleep_tx_idle()     queue_work(gpiosleep_workqueue, &sleep_work)

/* 3 seconds timeout */
#define TX_TIMER_INTERVAL	3

/* state variable names and bit positions */
#define FLAG_PROTO	0x01
#define FLAG_TXDATA	0x02
#define FLAG_ASLEEP	0x04

static struct gpiosleep_info *gsi;

/* module usage */
static atomic_t open_count = ATOMIC_INIT(1);

/** Global state flags */
static unsigned long flags=0;

/** Tasklet to respond to change in hostwake line */
static struct tasklet_struct hostwake_task;

/** Transmission timer */
static struct timer_list tx_timer;

/** Lock for state transitions */
static spinlock_t rw_lock;


struct proc_dir_entry *gpiosleep_dir;

/*
 * Local functions
 */

static void hsuart_power(int on)
{
    if(NULL == gsi->uport)
    {
        GS_ERR("gsi->uport is null");
        return;
    }
    if (on) {
		msm_hs_request_clock_on(gsi->uport);
		msm_hs_set_mctrl(gsi->uport, TIOCM_RTS);
        GS_ERR("$$hsuart_power on!");
	} else {
		msm_hs_set_mctrl(gsi->uport, 0);
		msm_hs_request_clock_off(gsi->uport);
        GS_ERR("$$hsuart_power off!");
	}
}


void gpiosleep_uart_open(struct uart_port *uport)
{
    unsigned long irq_flags;
    
	GS_DBG("gpiosleep_uart_open");
	spin_lock_irqsave(&rw_lock, irq_flags);

	if (!test_bit(FLAG_PROTO, &flags)) {
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return;
	}

	spin_unlock_irqrestore(&rw_lock, irq_flags);
    
    hsuart_power(1); // must do this, other wise, mtk_cmux set baud rate before uart power on.
 
	if(gsi->uport == NULL) {
		GS_DBG("gpiosleep_uart_open done");
		gsi->uport = uport;
	}
}


void gpiosleep_uart_close(struct uart_port *uport)
{
	GS_DBG("gpiosleep_uart_close");
	if(gsi->uport == uport) {
		GS_DBG("gpiosleep_uart_close done");		
		gsi->uport = NULL;
	}
}

/**
 * @return 1 if the Host can go to sleep, 0 otherwise.
 */
static inline int gpiosleep_can_sleep(void)
{
    if (gpio_get_value(gsi->ext_wake))
    {
        GS_DBG("ext_wake gpio value 1");
    }else{
        GS_DBG("ext_wake gpio value 0");
    }

    if (gpio_get_value(gsi->host_wake))
    {    
        GS_DBG("host_wake gpio value 1");
    }else{
        GS_DBG("host_wake gpio value 0");
    }
    
    if ((gsi->uport != NULL))
    {
        GS_DBG("uport != NULL");
    }else{
        GS_DBG("uport == NULL");
    }
	/* check if MSM_WAKE_MTK_GPIO and MTK_WAKE_MSM_GPIO are both deasserted */
    return gpio_get_value(gsi->ext_wake) &&
		gpio_get_value(gsi->host_wake) &&
		(gsi->uport != NULL);
}

void gpiosleep_sleep_wakeup(void)
{
    if (test_bit(FLAG_ASLEEP, &flags)) {
    	GS_DBG("waking up...");
    	/* Start the timer */
    	mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
    	gpio_set_value(gsi->ext_wake, 0);
        GS_DBG("ext_wake gpio set value 0");
    	clear_bit(FLAG_ASLEEP, &flags);
    	/*Activating UART */
    	hsuart_power(1);
    }
    else
    {
        /*Tx idle, Rx busy, we must also make host_wake asserted, that is low
        * 1 means mtk chip can sleep, in gpiosleep.c
        */
        /* Here we depend on the status of MSM gpio, for stability */
        if(1 == gpio_get_value(gsi->host_wake))
        {
            GS_DBG("-gpiosleep_sleep_wakeup wakeup mtk chip");
            /*0 means wakup MTK chip */
            gpio_set_value(gsi->ext_wake, 0);  
            GS_DBG("ext_wake gpio set value 0");
            mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
        } else
        {
            GS_DBG("hit here?");
        }
    }

}

/**
 * @brief@  main sleep work handling function which update the flags
 * and activate and deactivate UART ,check FIFO.
 */
static void gpiosleep_sleep_work(struct work_struct *work)
{
	if (gpiosleep_can_sleep()) {
		/* already asleep, this is an error case */
		if (test_bit(FLAG_ASLEEP, &flags)) {
			GS_DBG("already asleep");
			return;
		}

		if (msm_hs_tx_empty(gsi->uport)) {
			GS_DBG("going to sleep...");
			set_bit(FLAG_ASLEEP, &flags);
			/*Deactivating UART */
			hsuart_power(0);
		} else {
            GS_DBG("fifo has data to send, not sleep...");
		    mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL * HZ));
			return;
		}
	} else {
		gpiosleep_sleep_wakeup();
	}
}

/**
 * A tasklet function that runs in tasklet context and reads the value
 * of the HOST_WAKE GPIO pin and further defer the work.
 * @param data Not used.
 */
static void gpiosleep_hostwake_task(unsigned long data)
{
	GS_DBG("hostwake line change");

	spin_lock(&rw_lock);

	if (gpio_get_value(gsi->host_wake))
		gpiosleep_rx_busy();
	else
		gpiosleep_rx_idle();

	spin_unlock(&rw_lock);
}

/**
 * Handles proper timer action when outgoing data is delivered to the
 * uart tx buffer. Sets FLAG_TXDATA.
 */
void gpiosleep_outgoing_data(void)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

    if (!test_bit(FLAG_PROTO, &flags)) {
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return;
	}
	/* log data passing by */
	set_bit(FLAG_TXDATA, &flags);

    /* Fix the bug sleeping function called from invalid context */
	/* if the tx side is sleeping... */
	if (gpio_get_value(gsi->ext_wake)) {
		GS_DBG("tx was sleeping");
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		gpiosleep_sleep_wakeup();
	}
	else
	{
		spin_unlock_irqrestore(&rw_lock, irq_flags);
	}
}


/**
 * Handles transmission timer expiration.
 * @param data Not used.
 */
static void gpiosleep_tx_timer_expire(unsigned long data)
{
#ifdef ENABLE_IRQ_AND_TIMER
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

	GS_DBG("Tx timer expired");

	/* were we silent during the last timeout? */
	if (!test_bit(FLAG_TXDATA, &flags)) {
		GS_DBG("Tx has been idle");
		gpio_set_value(gsi->ext_wake, 1);
        GS_DBG("ext_wake gpio set value 1");
		gpiosleep_tx_idle();
	} else {
		GS_DBG("Tx data during last period");
		mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL*HZ));
	}

	/* clear the incoming data flag */
	clear_bit(FLAG_TXDATA, &flags);

	spin_unlock_irqrestore(&rw_lock, irq_flags);
#endif //#ifdef ENABLE_IRQ_AND_TIMER 
}

/**
 * Schedules a tasklet to run when receiving an interrupt on the
 * <code>HOST_WAKE</code> GPIO pin.
 * @param irq Not used.
 * @param dev_id Not used.
 */
#ifdef ENABLE_IRQ_AND_TIMER  
static irqreturn_t gpiosleep_hostwake_isr(int irq, void *dev_id)
{
	/* schedule a tasklet to handle the change in the host wake line */
	tasklet_schedule(&hostwake_task);
	return IRQ_HANDLED;
}
#endif 

/**
 * Starts the Sleep-Mode Protocol on the Host.
 * @return On success, 0. On error, -1, and <code>errno</code> is set
 * appropriately.
 */
static int gpiosleep_start(void)
{
	int retval;
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

	if (test_bit(FLAG_PROTO, &flags)) {
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return 0;
	}

	spin_unlock_irqrestore(&rw_lock, irq_flags);

	if (!atomic_dec_and_test(&open_count)) {
		atomic_inc(&open_count);
		return -EBUSY;
	}

	/* start the timer */
	mod_timer(&tx_timer, jiffies + (TX_TIMER_INTERVAL*HZ));

	/* assert MTK_WAKE sleep */
	gpio_set_value(gsi->ext_wake, 1);
    GS_DBG("ext_wake gpio set value 1");

#ifdef ENABLE_IRQ_AND_TIMER
	retval = request_irq(gsi->host_wake_irq, gpiosleep_hostwake_isr,
				IRQF_DISABLED | IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
				"gpiosleep hostwake", NULL);
#endif //#ifdef ENABLE_IRQ_AND_TIMER 
	if (retval  < 0) {
		GS_ERR("Couldn't acquire GPIOSLEEP_HOST_WAKE IRQ");
		goto fail;
	}

	retval = enable_irq_wake(gsi->host_wake_irq);
	if (retval < 0) {
		GS_ERR("Couldn't enable GPIOSLEEP_HOST_WAKE as wakeup interrupt");
		free_irq(gsi->host_wake_irq, NULL);
		goto fail;
	}

	set_bit(FLAG_PROTO, &flags);
	return 0;
fail:
	del_timer(&tx_timer);
	atomic_inc(&open_count);

	return retval;
}

/**
 * Stops the Sleep-Mode Protocol on the Host.
 */
static void gpiosleep_stop(void)
{
	unsigned long irq_flags;

	spin_lock_irqsave(&rw_lock, irq_flags);

	if (!test_bit(FLAG_PROTO, &flags)) {
		spin_unlock_irqrestore(&rw_lock, irq_flags);
		return;
	}

	/* assert MTK_WAKE */
	gpio_set_value(gsi->ext_wake, 0);
    GS_DBG("ext_wake gpio set value 0");
	del_timer(&tx_timer);
	clear_bit(FLAG_PROTO, &flags);

	if (test_bit(FLAG_ASLEEP, &flags)) {
		clear_bit(FLAG_ASLEEP, &flags);
		hsuart_power(1);
	}

	atomic_inc(&open_count);

	spin_unlock_irqrestore(&rw_lock, irq_flags);
	if (disable_irq_wake(gsi->host_wake_irq))
		GS_ERR("Couldn't disable hostwake IRQ wakeup mode\n");
	free_irq(gsi->host_wake_irq, NULL);
}
/**
 * Read the <code>MTK_WAKE</code> GPIO pin value via the proc interface.
 * When this function returns, <code>page</code> will contain a 1 if the
 * pin is high, 0 otherwise.
 * @param page Buffer for writing data.
 * @param start Not used.
 * @param offset Not used.
 * @param count Not used.
 * @param eof Whether or not there is more data to be read.
 * @param data Not used.
 * @return The number of bytes written.
 */
static int gpiosleep_read_proc_mtkwake(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	*eof = 1;
	return sprintf(page, "mtkwake:%u\n", gpio_get_value(gsi->ext_wake));
}

/**
 * Write the <code>MTK_WAKE</code> GPIO pin value via the proc interface.
 * @param file Not used.
 * @param buffer The buffer to read from.
 * @param count The number of bytes to be written.
 * @param data Not used.
 * @return On success, the number of bytes written. On error, -1, and
 * <code>errno</code> is set appropriately.
 */
static int gpiosleep_write_proc_mtkwake(struct file *file, const char *buffer,
					unsigned long count, void *data)
{
	char *buf;

	if (count < 1)
		return -EINVAL;

	buf = kmalloc(count, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	if (copy_from_user(buf, buffer, count)) {
		kfree(buf);
		return -EFAULT;
	}

	if (buf[0] == '0') {
		gpio_set_value(gsi->ext_wake, 0);
        GS_DBG("mtkwake gpio set value 0");
	} else if (buf[0] == '1') {
		gpio_set_value(gsi->ext_wake, 1);
        GS_DBG("mtkwake gpio set value 1");
	} else {
		kfree(buf);
		return -EINVAL;
	}

	kfree(buf);
	return count;
}

/**
 * Read the <code>HOST_WAKE</code> GPIO pin value via the proc interface.
 * When this function returns, <code>page</code> will contain a 1 if the pin
 * is high, 0 otherwise.
 * @param page Buffer for writing data.
 * @param start Not used.
 * @param offset Not used.
 * @param count Not used.
 * @param eof Whether or not there is more data to be read.
 * @param data Not used.
 * @return The number of bytes written.
 */
static int gpiosleep_read_proc_hostwake(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	*eof = 1;
	return sprintf(page, "hostwake: %u \n", gpio_get_value(gsi->host_wake));
}


/**
 * Read the low-power status of the Host via the proc interface.
 * When this function returns, <code>page</code> contains a 1 if the Host
 * is asleep, 0 otherwise.
 * @param page Buffer for writing data.
 * @param start Not used.
 * @param offset Not used.
 * @param count Not used.
 * @param eof Whether or not there is more data to be read.
 * @param data Not used.
 * @return The number of bytes written.
 */
static int gpiosleep_read_proc_asleep(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	unsigned int asleep;

	asleep = test_bit(FLAG_ASLEEP, &flags) ? 1 : 0;
	*eof = 1;
	return sprintf(page, "asleep: %u\n", asleep);
}

/**
 * Read the low-power protocol being used by the Host via the proc interface.
 * When this function returns, <code>page</code> will contain a 1 if the Host
 * is using the Sleep Mode Protocol, 0 otherwise.
 * @param page Buffer for writing data.
 * @param start Not used.
 * @param offset Not used.
 * @param count Not used.
 * @param eof Whether or not there is more data to be read.
 * @param data Not used.
 * @return The number of bytes written.
 */
static int gpiosleep_read_proc_proto(char *page, char **start, off_t offset,
					int count, int *eof, void *data)
{
	unsigned int proto;

	proto = test_bit(FLAG_PROTO, &flags) ? 1 : 0;
	*eof = 1;
	return sprintf(page, "proto: %u\n", proto);
}

/**
 * Modify the low-power protocol used by the Host via the proc interface.
 * @param file Not used.
 * @param buffer The buffer to read from.
 * @param count The number of bytes to be written.
 * @param data Not used.
 * @return On success, the number of bytes written. On error, -1, and
 * <code>errno</code> is set appropriately.
 */
static int gpiosleep_write_proc_proto(struct file *file, const char *buffer,
					unsigned long count, void *data)
{
	char proto;

	if (count < 1)
		return -EINVAL;

	if (copy_from_user(&proto, buffer, 1))
		return -EFAULT;

	if (proto == '0')
		gpiosleep_stop();
	else
		gpiosleep_start();

	/* claim that we wrote everything */
	return count;
}

static int gpiosleep_create_proc_entry(void)
{
    int retval;
    struct proc_dir_entry *ent;
    
    gpiosleep_dir = proc_mkdir("gpiosleep", NULL);
	if (gpiosleep_dir == NULL) {
		GS_ERR("Unable to create /proc/gpiosleep directory");
		return -ENOMEM;
	}

	/* Creating read/write "mtkwake" entry */
	ent = create_proc_entry("mtkwake", 0, gpiosleep_dir);
	if (ent == NULL) {
		GS_ERR("Unable to create /proc/%s/mtkwake entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}
	ent->read_proc = gpiosleep_read_proc_mtkwake;
	ent->write_proc = gpiosleep_write_proc_mtkwake;

	/* read only proc entries */
	if (create_proc_read_entry("hostwake", 0, gpiosleep_dir,
				gpiosleep_read_proc_hostwake, NULL) == NULL) {
		GS_ERR("Unable to create /proc/%s/hostwake entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}

	/* read/write proc entries */
	ent = create_proc_entry("proto", 0, gpiosleep_dir);
	if (ent == NULL) {
		GS_ERR("Unable to create /proc/%s/proto entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}
	ent->read_proc = gpiosleep_read_proc_proto;
	ent->write_proc = gpiosleep_write_proc_proto;

	/* read only proc entries */
	if (create_proc_read_entry("asleep", 0,
			gpiosleep_dir, gpiosleep_read_proc_asleep, NULL) == NULL) {
		GS_ERR("Unable to create /proc/%s/asleep entry", PROC_DIR);
		retval = -ENOMEM;
		goto fail;
	}

    return 0;

fail:
	remove_proc_entry("asleep", gpiosleep_dir);
	remove_proc_entry("proto", gpiosleep_dir);
	remove_proc_entry("hostwake", gpiosleep_dir);
	remove_proc_entry("mtkwake", gpiosleep_dir);
	remove_proc_entry("gpiosleep", 0);
	return retval;
    
}

static void gpiosleep_remove_proc_entry(void)
{
	remove_proc_entry("asleep", gpiosleep_dir);
	remove_proc_entry("proto", gpiosleep_dir);
	remove_proc_entry("hostwake", gpiosleep_dir);
	remove_proc_entry("mtkwake", gpiosleep_dir);
	remove_proc_entry("gpiosleep", 0);
}

static int __devinit gpiosleep_probe(struct platform_device *pdev)
{
	int ret;
    struct msm_gpiosleep_data *pdata = pdev->dev.platform_data;

    pr_debug("%s enter\n", __func__);

    ret = gpiosleep_create_proc_entry();
    if (ret)
        return ret;

    INIT_WORK(&sleep_work, gpiosleep_sleep_work);
    gpiosleep_workqueue = create_singlethread_workqueue("gpiosleep_work_queue");
    if (!gpiosleep_workqueue)
    {
        printk(KERN_ERR "%s, line %d: create_singlethread_workqueue fail!\n", __func__, __LINE__);
        return -1;
    }

    /* Initialize spinlock. */
	spin_lock_init(&rw_lock);
    
    flags = 0; /* clear all status bits */

	/* Initialize timer */
	init_timer(&tx_timer);
	tx_timer.function = gpiosleep_tx_timer_expire;
	tx_timer.data = 0;

    /* initialize host wake tasklet */
	tasklet_init(&hostwake_task, gpiosleep_hostwake_task, 0);
    
	gsi = kzalloc(sizeof(struct gpiosleep_info), GFP_KERNEL);
	if (!gsi)
		goto fail;

    if (pdata){
        gsi->host_wake = pdata->host_wake;
        ret = gpio_request(gsi->host_wake, "gpio_host_wake");
    	if (ret)
    		goto free_gsi;
    	ret = gpio_direction_input(gsi->host_wake);
    	if (ret)
    		goto free_gpio_host_wake;

        //ret = gpio_set_debounce(gsi->host_wake, 20); // set debounce time ,20ms
        //if (ret)
    	//	goto free_gpio_host_wake;
        
        gsi->ext_wake = pdata->ext_wake;
    	ret = gpio_request(gsi->ext_wake, "gpio_ext_wake");
        if (ret)
        	goto free_gpio_host_wake;
        /* assert gpio sleep */
        ret = gpio_direction_output(gsi->ext_wake, 1);
        if (ret)
        	goto free_gpio_ext_wake;
        
        gsi->host_wake_irq = gpio_to_irq(pdata->host_wake);
    }
 	
    (void)gpiosleep_start();    
        
	return 0;

free_gpio_ext_wake:
	gpio_free(gsi->ext_wake);
free_gpio_host_wake:
	gpio_free(gsi->host_wake);
free_gsi:
	kfree(gsi);
fail:
	del_timer(&tx_timer);        

	return ret;
}

static int __devexit gpiosleep_remove(struct platform_device *pdev)
{
	/* assert mtk wake */
	gpio_set_value(gsi->ext_wake, 0);
    GS_DBG("ext_wake gpio set value 0");
	if (test_bit(FLAG_PROTO, &flags)) {
		if (disable_irq_wake(gsi->host_wake_irq))
			GS_ERR("Couldn't disable hostwake IRQ wakeup mode \n");
		free_irq(gsi->host_wake_irq, NULL);
		del_timer(&tx_timer);
		if (test_bit(FLAG_ASLEEP, &flags))
			hsuart_power(1);
	}

	gpio_free(gsi->host_wake);
	gpio_free(gsi->ext_wake);
	kfree(gsi);
    destroy_workqueue(gpiosleep_workqueue);
    gpiosleep_workqueue = NULL;
    gpiosleep_remove_proc_entry();
	return 0;
}

static struct platform_driver gpiosleep_driver = {
    .probe = gpiosleep_probe, 
	.remove = __devexit_p(gpiosleep_remove),
	.driver = {
		.name = "gpiosleep",
		.owner = THIS_MODULE,
	},
};
/**
 * Initializes the module.
 * @return On success, 0. On error, -1, and <code>errno</code> is set
 * appropriately.
 */
static int __init gpiosleep_init(void)
{
      //remove rw_lock initiation
    return platform_driver_register(&gpiosleep_driver);
}

/**
 * Cleans up the module.
 */
static void __exit gpiosleep_exit(void)
{
	platform_driver_unregister(&gpiosleep_driver);
}

module_init(gpiosleep_init);
module_exit(gpiosleep_exit);

MODULE_DESCRIPTION("MTK Gpio Sleep Driver" );
#ifdef MODULE_LICENSE
MODULE_LICENSE("GPL");
#endif
#endif
