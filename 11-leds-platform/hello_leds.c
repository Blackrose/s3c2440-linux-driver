#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

#define GPB_CON 0x56000010
#define GPIO_OUTPUT 0x1
#define GPB_DATA 0x4

#define LED_SZ 0x8

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

struct resource led_res[] = 
{
    [0] = {
        .start = GPB_CON,
        .end = GPB_CON + LED_SZ - 1,
        .flags = IORESOURCE_MEM,
        .name = "led_res",
    },
};

void led_release(struct device *dev)
{
    printk(KERN_ALERT "Device is released\n");
}

struct platform_device dev = {
    .name = "micro2440-led",
    .id = -1,
    .num_resources = ARRAY_SIZE(led_res),
    .resource = led_res,
    .dev = {
       .release = led_release,
    }
};

ssize_t leds_write(struct file *filp, const char *buf, size_t len, loff_t *offset)
{
    int tmp = 0;
    if(len > sizeof(leds_buf))
        return -1;
    if(copy_from_user(&leds_buf, buf, len)){
        printk(KERN_ALERT "write error\n");
        return -2;
    }

    if(leds_buf > 0){
        // turn on the leds
        tmp = ioread32(config_base + GPB_DATA);
        tmp &= ~((1 << 5) | (1 << 6) | (1 << 7) | (1 << 8));
        iowrite32(tmp, config_base + GPB_DATA);

    }else{
        // turn off the leds
        
        tmp = ioread32(config_base + GPB_DATA);
        tmp |= (0x1 << 5) | (0x1 << 6) | (0x1 << 7) | (0x1 << 8);
        iowrite32(tmp, config_base + GPB_DATA);

    }

    return len;
}

struct file_operations leds_fops = 
{
    .owner = THIS_MODULE,
    .write = leds_write,
};


int led_probe(struct platform_device *pdev)
{
    int ret;
    unsigned long tmp;
    struct resource *res;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(!res)
        return -EBUSY;


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

    config_base = ioremap(res->start, res->end - res->start + 1);
    if(config_base == NULL){
        printk(KERN_ALERT "map io error\n");
        goto ERR4;
    }


    tmp = ioread32(config_base);
    tmp |= (GPIO_OUTPUT << 10) | (GPIO_OUTPUT << 12) | (GPIO_OUTPUT << 14) | (GPIO_OUTPUT << 16);
    iowrite32(tmp, config_base);

    printk(KERN_ALERT "leds init success\n");
    return 0;

ERR4:
    release_resource(led_res);
ERR3:
    cdev_del(&leds_cdev);
ERR2:
    unregister_chrdev_region(devt, 1);
ERR1:
    return -1;
}

int led_remove(struct platform_device *pdev)
{
    iounmap(config_base);
    
    device_destroy(leds_class, devt);
    class_destroy(leds_class);
    
    cdev_del(&leds_cdev);
    unregister_chrdev_region(devt, 1);

    return 0;
}

struct platform_driver drv = {
    .probe = led_probe,
    .remove = led_remove,
    .driver = {
        .name = "micro2440-led"
    }
};

static int __init leds_init(void)
{
    platform_device_register(&dev);
    platform_driver_register(&drv);
    
    return 0;
}

static void __exit leds_exit(void)
{
   
    platform_driver_unregister(&drv);
    platform_device_unregister(&dev);
    
    printk(KERN_ALERT "leds has been exited\n");
}

module_init(leds_init);
module_exit(leds_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
