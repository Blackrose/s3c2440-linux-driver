#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

static ssize_t hello_read(struct file *filep, char* buf, size_t len, loff_t *off);

dev_t chr_dev;
struct class *cdev_class;
struct cdev char_device;
struct file_operations cdev_fops = {
    .owner = THIS_MODULE,
    .read = hello_read,
};

char buffer = 0x1;

static ssize_t hello_read(struct file *filep, char* buf, size_t len, loff_t *off)
{
    if(copy_to_user(buf, &buffer, sizeof(buffer)))
        return -1;

    return sizeof(buffer);
}

static int __init hello_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&chr_dev, 0, 1, "hello_driver");

    if(ret < 0){
        printk(KERN_ALERT "alloc chrdev error\n");
        goto ERR1;
    }else{
        printk(KERN_ALERT "alloc chrdev success\n");
    }

    cdev_init(&char_device, &cdev_fops);

    ret = cdev_add(&char_device, chr_dev, 1);
    if(ret < 0)
        goto ERR2;
    else
        printk(KERN_ALERT "char device add success\n");

    return 0;

ERR2:
    cdev_del(&char_device);
    unregister_chrdev_region(chr_dev, 1);

ERR1:
    return ret;
}

static void __exit hello_exit(void)
{
    cdev_del(&char_device);
    unregister_chrdev_region(chr_dev, 1);
    printk("hello-driver has been removed, Goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
