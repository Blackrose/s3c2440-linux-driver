#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define GPB_CON 0x56000010
#define GPB_DAT 0x56000014
#define GPIO_OUTPUT 0x1

dev_t devt;
struct cdev leds_cdev = 
{
    .owner = THIS_MODULE,
};
struct class *leds_class;
struct device *leds_device;
void *config_base;
void *data_base;

int leds_buf = 0;

ssize_t leds_write(struct file *filp, const char *buf, size_t len, loff_t *offset)
{
    unsigned long tmp;
    if(len > sizeof(leds_buf))
        return -1;
    if(copy_from_user(&leds_buf, buf, len)){
        printk(KERN_ALERT "write error\n");
        return -2;
    }

    if(leds_buf > 0){
        // turn on the leds
        
        //tmp = __raw_readl(data_base);
        tmp = ioread32(data_base);
        tmp &= ~((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
        //__raw_writel(tmp, data_base);
        iowrite32(tmp, data_base);

    }else{
        // turn off the leds
        
        //tmp = __raw_readl(data_base);
        tmp = ioread32(data_base);
        tmp |= (0x1 << 5) | (0x1 << 6) | (0x1 << 7) | (0x1 << 8);
        //__raw_writel(tmp, data_base);
        iowrite32(tmp, data_base);

    }

    return len;
}

struct file_operations leds_fops = 
{
    .owner = THIS_MODULE,
    .write = leds_write,
};


static int __init leds_init(void)
{
    int ret;
    unsigned long tmp;

    ret = alloc_chrdev_region(&devt, 0, 1, "leds");
    if(ret){
        printk(KERN_ALERT "alloc devt error\n");
        goto ERR1;
    }

    cdev_init(&leds_cdev, &leds_fops);

    ret = cdev_add(&leds_cdev, devt, 1);
    if(ret){
        printk(KERN_ALERT "add cdev error\n");
        goto ERR2;
    }

    leds_class = class_create(THIS_MODULE, "leds");

    leds_device = device_create(leds_class, NULL, devt, NULL, "leds");
    if(leds_device == NULL){
        printk(KERN_ALERT "create dev node error\n");
        goto ERR3;
    }

    config_base = ioremap(GPB_CON, 4);
    data_base = ioremap(GPB_DAT, 4);

    //tmp = __raw_readl(config_base);
    tmp = ioread32(config_base);
    tmp |= (GPIO_OUTPUT << 10) | (GPIO_OUTPUT << 12) | (GPIO_OUTPUT << 14) | (GPIO_OUTPUT << 16);
    //__raw_writel(tmp, config_base);
    iowrite32(tmp, config_base);

    
    printk(KERN_ALERT "leds init success\n");
    return 0;

ERR3:
    cdev_del(&leds_cdev);
ERR2:
    unregister_chrdev_region(devt, 1);
ERR1:
    return -1;
}


static void __exit leds_exit(void)
{
    iounmap(config_base);
    iounmap(data_base);
    
    device_destroy(leds_class, devt);
    class_destroy(leds_class);
    
    cdev_del(&leds_cdev);
    unregister_chrdev_region(devt, 1);
    
    printk(KERN_ALERT "leds has been exited\n");
}

module_init(leds_init);
module_exit(leds_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
