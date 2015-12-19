#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <asm/io.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/ioport.h>
#define DEV_NAME "backlight"
#define GPG_CONF 0x56000060
#define GPG_DATA 0x56000064
    
void __iomem *bl_conf = NULL;
void __iomem *bl_dat = NULL;

void setup_backlight_io()
{
    int tmp;

    bl_conf = ioremap(GPG_CONF, 12);
    //bl_dat = ioremap(GPG_DATA, 4);
    
    tmp = ioread32(bl_conf+0x0);
    tmp |= (0x1 << 8);
    iowrite32(tmp, bl_conf+0x0);
}

void set_backlight(unsigned int value)
{
    int tmp;
    tmp = ioread32(bl_conf + 0x4);
    if(value)
        tmp |= (1 << 4);
    else
        tmp &= ~(1 << 4);

    printk("v=%d, %x\n", value, tmp);
    iowrite32(tmp, bl_conf+0x4);

}

static ssize_t backlight_write(struct file *file, const char *buffer, size_t cnt, loff_t *pos)
{
    unsigned char val;
    int ret;
#if 1
    if (cnt == 0) {
        return cnt;
    }
    
    ret = copy_from_user(&val, buffer, sizeof val) ? -EFAULT : 0;
    if (ret) {
        return ret;
    }

    val &= 0x01;
    set_backlight(val);
    return cnt;

#else
    if(cnt <= 0)
        return cnt;

    ret = copy_from_user(&val, buffer, sizeof(val));
    printk("value = %d, %d\n", val, ret);

    val &= 0x1;
    set_backlight(val);
    return ret;
#endif
}


struct file_operations backlight_ops = {
    .owner = THIS_MODULE,
    .write = &backlight_write,
};

struct miscdevice backlight_device = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEV_NAME,
    .fops = &backlight_ops,
};

static void __exit backlight_exit(void)
{
    iounmap(bl_conf);
    iounmap(bl_dat);
    misc_deregister(&backlight_device);
}

static int __init backlight_init(void)
{
    int ret;
    ret = misc_register(&backlight_device);
    if(ret){
        goto ERR;
    }

    setup_backlight_io();
    set_backlight(1);
    return 0;
ERR:
    printk("register backlight failed\n");
    misc_deregister(&backlight_device);
    return ret;
}

module_init(backlight_init);
module_exit(backlight_exit);
MODULE_AUTHOR("KevinChen");
MODULE_LICENSE("GPL");
