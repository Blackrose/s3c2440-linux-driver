#include <linux/init.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>

#define GPB_CON 0x56000010
#define GPB_DAT 0x56000014
#define GPIO_OUTPUT 0x1

#define LED_PA_START GPB_CON
#define LED_SZ 4

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
        .start = LED_PA_START,
        .end = LED_PA_START + LED_SZ - 1,
        .flags = IORESOURCE_MEM,
        .name = "led_res",
    },
};

void led_release(struct device *dev)
{
    printk(KERN_ALERT "Device is released\n");
}

struct platform_device led_dev = {
    .name = "micro2440-led",
    .id = -1,
    .num_resources = ARRAY_SIZE(led_res),
    .resource = led_res,
    .dev = {
       .release = led_release,
    }
};

static int __init leds_init(void)
{
    platform_device_register(&led_dev);
    
    return 0;
}

static void __exit leds_exit(void)
{
   
    platform_device_unregister(&led_dev);
    
    printk(KERN_ALERT "leds has been exited\n");
}

module_init(leds_init);
module_exit(leds_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
