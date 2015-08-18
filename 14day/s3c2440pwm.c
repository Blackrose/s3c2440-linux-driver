#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>
#include <linux/clk.h>

void __iomem *pwm_base_addr;
void __iomem *gpiob_base_addr;

#define GPIOB_PYH_ADDR 0x56000010
#define PWM_PYH_ADDR 0x51000000

#define GPIOB_CON (gpiob_base_addr + 0x00)
#define GPIOB_DAT (gpiob_base_addr + 0x04)

#define PWM_TCFG0 (pwm_base_addr + 0x00)
#define PWM_TCFG1 (pwm_base_addr + 0x04)
#define PWM_TCON (pwm_base_addr + 0x08)
#define PWM_TCNTB0 (pwm_base_addr + 0x0c)
#define PWM_TCMPB0 (pwm_base_addr + 0x10)


#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP 0

#define DEVICE_NAME "s3cpwm"
dev_t s3c2440pwm_devt;
struct cdev s3c2440pwm_cdev = {
    .owner = THIS_MODULE,
};

struct class *s3c2440pwm_class = NULL;
struct device *s3c2440pwm_device = NULL;

struct semaphore lock;

void s3c2440pwm_set_freq(unsigned long freq)
{
    unsigned long gpiob_con;
    unsigned long tcon, tcnt;
    unsigned long tcfg1, tcfg0;
    struct clk *clk_p;
    unsigned long pclk;

    gpiob_con = ioread32(GPIOB_CON);
    gpiob_con |= 0x2;
    iowrite32(gpiob_con, GPIOB_CON);

    tcon = ioread32(PWM_TCON);
    tcfg1 = ioread32(PWM_TCFG1);
    tcfg0 = ioread32(PWM_TCFG0);

    tcfg0 &= ~(255 << 0);
    tcfg0 |= (50 - 1);

    tcfg1 &= ~(15 << 0);
    tcfg1 |= (3 << 0);

    iowrite32(tcfg1, PWM_TCFG1);
    iowrite32(tcfg0, PWM_TCFG0);

    clk_p = clk_get(NULL, "pclk");
    pclk = clk_get_rate(clk_p);
    tcnt = (pclk / 50 / 16) / freq;

    iowrite32(tcnt, PWM_TCNTB0);
    iowrite32(tcnt/2, PWM_TCMPB0);

    tcon &= ~0x1f;
    tcon |= 0xb;

    iowrite32(tcon, PWM_TCON);

    tcon &= ~2;
    iowrite32(tcon, PWM_TCON);
}

void s3c2440pwm_stop(void)
{
    unsigned long gpiob_con;

    gpiob_con = ioread32(GPIOB_CON);
    gpiob_con &= 0xfffffff1;
    iowrite32(gpiob_con, GPIOB_CON);
}

static int s3c2440pwm_open(struct inode *inode, struct file *filp)
{
    if(!down_trylock(&lock))
        return 0;
    else
        return -EBUSY;
}

static int s3c2440pwm_release(struct inode *inode, struct file *filp)
{
    s3c2440pwm_stop();
    up(&lock);

    return 0;
}

static int s3c2440pwm_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch(cmd){
        case PWM_IOCTL_SET_FREQ:
            if(arg == 0)
                return -EINVAL;
            s3c2440pwm_set_freq(arg);
            break;

        case PWM_IOCTL_STOP:
            s3c2440pwm_stop();
            break;
        default:
            break;
    }
    return 0;
}

struct file_operations s3c2440pwm_fops = {
    .owner = THIS_MODULE,
    .open = s3c2440pwm_open,
    .release = s3c2440pwm_release,
    .ioctl = s3c2440pwm_ioctl,
};

static int s3c2440pwm_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&s3c2440pwm_devt, 0, 1, DEVICE_NAME);
    if(ret){
        printk(KERN_ERR "alloc char device s3c2440 pwm error\n");
        goto ERR1;
    }

    cdev_init(&s3c2440pwm_cdev, &s3c2440pwm_fops);
    ret = cdev_add(&s3c2440pwm_cdev, s3c2440pwm_devt, 1);
    if(ret){
        printk(KERN_ERR "add s3c2440 pwm char device error\n");
        goto ERR2;
    }

    s3c2440pwm_class = class_create(THIS_MODULE, DEVICE_NAME);
    s3c2440pwm_device = device_create(s3c2440pwm_class, NULL, s3c2440pwm_devt, NULL, DEVICE_NAME);
    pwm_base_addr = ioremap(PWM_PYH_ADDR, 0x14);
    gpiob_base_addr = ioremap(GPIOB_PYH_ADDR, 0x8);

    init_MUTEX(&lock);
    
    printk(KERN_INFO "s3c2440 pwm loaded successfule\n");
    return 0;

ERR2:
    unregister_chrdev_region(s3c2440pwm_devt, 1);
ERR1:
    return ret;
}

static void s3c2440pwm_exit(void)
{

    iounmap(pwm_base_addr);
    iounmap(gpiob_base_addr);

    device_destroy(s3c2440pwm_class, s3c2440pwm_devt);
    class_destroy(s3c2440pwm_class);

    unregister_chrdev_region(s3c2440pwm_devt, 1); 
    printk(KERN_INFO "s3c2440 pwm removed successfule\n");
}

module_init(s3c2440pwm_init);
module_exit(s3c2440pwm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
