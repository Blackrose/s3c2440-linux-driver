#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEVICE_NAME "s3cadc"

#define IRQ_OFFSET (16)
#define S3C_IRQ(x) ((x) + IRQ_OFFSET)
#define IRQ_SUB(x) S3C_IRQ((x) + 54)
//#define IRQ_ADC0 IRQ_SUB(10)
#define IRQ_ADC0 (10 + 54 + 16)

typedef struct{
    wait_queue_head_t wait;
    int channel;
    int prescale;
}ADC_DEV;

DECLARE_MUTEX(ADC_LOCK);

static void __iomem *adc_base_addr;
static struct clk *adc_clock;

struct class *s3cadc_class = NULL;
struct device *s3cadc_device = NULL;

static int OwnADC = 0;
static ADC_DEV adcdev;
static int adc_data;
static volatile int ev_adc = 0;

#define ADCCON (*(volatile unsigned long *)(adc_base_addr + 0x00))
#define ADCTSC (*(volatile unsigned long *)(adc_base_addr + 0x04))
#define ADCDLY (*(volatile unsigned long *)(adc_base_addr + 0x08))
#define ADCDAT0 (*(volatile unsigned long *)(adc_base_addr + 0x0C))
#define ADCDAT1 (*(volatile unsigned long *)(adc_base_addr + 0x10))
#define ADCUPDN (*(volatile unsigned long *)(adc_base_addr + 0x14))

#define S3C2440_PA_ADC (0x58000000)

#define PRESCALE_DIS (0 << 14)
#define PRESCALE_EN (1 << 14)
#define PRSCVL(x) ((x) << 6)
#define ADC_INPUT(x) ((x) << 3)
#define ADC_START (1 << 0)
#define ADC_ENDCVT (1 << 15)

#define START_ADC_AIN(ch, prescale) \
    do{ \
        ADCCON = PRESCALE_EN | PRSCVL(prescale) | ADC_INPUT((ch)); \
        ADCCON |= ADC_START; \
    }while(0)

static irqreturn_t adcdone_int_handler(int irq, void *dev_id)
{
    if(OwnADC){
        adc_data = ADCDAT0 & 0x3ff;

        ev_adc = 1;
        wake_up_interruptible(&adcdev.wait);
    }

    return IRQ_HANDLED;
}

static ssize_t s3c2440_adc_read(struct file *filp, char *buffer, size_t count, loff_t *ppos)
{
    char str[20];
    int value;
    size_t len;
    if(down_trylock(&ADC_LOCK) == 0){
        OwnADC = 1;
        START_ADC_AIN(adcdev.channel, adcdev.prescale);
        wait_event_interruptible(adcdev.wait, ev_adc);

        ev_adc = 0;

        printk(KERN_INFO "AIN[%d] = 0x%04x, %d\n", adcdev.channel, adc_data, ADCCON & 0x80 ? 1:0);

        value = adc_data;

        OwnADC = 0;
        up(&ADC_LOCK);
    }else{
        value = -1;
    }

    len = sprintf(str, "%d\n", value);
    if(count >= len){
        int r = copy_to_user(buffer, str, len);
        return r ? r : len;
    }else{
        return -EINVAL;
    }
}

static int s3c2440_adc_open(struct inode *inode, struct file *filp)
{
    init_waitqueue_head(&(adcdev.wait));

    adcdev.channel = 0;
    adcdev.prescale = 0xff;

    printk(KERN_INFO "adc opened\n");

    return 0;
}

static int s3c2440_adc_release(struct inode *inode, struct file *filp)
{
    printk(KERN_INFO "adc closed\n");

    return 0;
}

dev_t adc_devt;
struct cdev adc_cdev = {
    .owner = THIS_MODULE,
};

struct file_operations adc_fops = {
    .owner = THIS_MODULE,
    .open = s3c2440_adc_open,
    .release = s3c2440_adc_release,
    .read = s3c2440_adc_read,
};

static int __init s3c2440adc_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&adc_devt, 0, 1, DEVICE_NAME);
    if(ret < 0){
        printk(KERN_ERR "alloc a new chracter device failed\n");
        goto ERR1;
    }

    cdev_init(&adc_cdev, &adc_fops);

    ret = cdev_add(&adc_cdev, adc_devt, 1);
    if(ret < 0){
        printk(KERN_ALERT "s3cadc cdev add error\n");
        goto ERR2;
    }
    s3cadc_class = class_create(THIS_MODULE, DEVICE_NAME);
    s3cadc_device = device_create(s3cadc_class, NULL, adc_devt, NULL, DEVICE_NAME);

    adc_base_addr = ioremap(S3C2440_PA_ADC, 0x20);
    if(adc_base_addr == NULL){
        printk(KERN_ERR "Failed to remap registers\n");
        goto ERR3;
    }

    adc_clock = clk_get(NULL, "adc");
    if(!adc_clock){
        printk(KERN_ERR "failed to get adc clock source\n");
        goto ERR3;
    }

    clk_enable(adc_clock);

    /* Normal ADC */
    ADCTSC = 0;

    ret = request_irq(IRQ_ADC0, adcdone_int_handler, IRQF_SHARED, DEVICE_NAME, &adcdev);
    if(ret){
        printk(KERN_ERR "s3cadc request irq failed\n");
        goto ERR3;
    }
    
    printk(KERN_INFO "s3cadc driver loaded successful\n");
    return 0;
ERR3:
    iounmap(adc_base_addr);
ERR2:
    unregister_chrdev_region(adc_devt, 1);
ERR1:
    return ret;
}

static void __exit s3c2440adc_exit(void)
{
    free_irq(IRQ_ADC0, &adcdev);
    iounmap(adc_base_addr);


    if(adc_clock){
        clk_disable(adc_clock);
        clk_put(adc_clock);
        adc_clock = NULL;
    }

    device_destroy(s3cadc_class, adc_devt);
    class_destroy(s3cadc_class);
    
    cdev_del(&adc_cdev);
    unregister_chrdev_region(adc_devt, 1);
    
    printk(KERN_INFO "s3cadc driver removed successful\n");

}

module_init(s3c2440adc_init);
module_exit(s3c2440adc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
