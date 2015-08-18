#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

static int hello_buffer = 0x5;

dev_t devt;
struct cdev char_device;
struct class *dev_class = NULL;
struct device *dev_device = NULL;

ssize_t hello_read(struct file* fp, char *buf, size_t len, loff_t *offset)
{
    if(copy_to_user(buf, &hello_buffer, sizeof(hello_buffer)) != 0){
        printk(KERN_ALERT "copy data to app failed\n");
        return -1;
    }
    
    return sizeof(hello_buffer);
}

ssize_t hello_write(struct file* fp, const char __user *buf, size_t len, loff_t *offset)
{
    if(copy_from_user(&hello_buffer, buf, len)){
        printk(KERN_ALERT "copy data from app failed\n");

        return -2;
    }
    
    return sizeof(hello_buffer);
}

struct file_operations hello_ops = {
    .owner = THIS_MODULE,
    .read = hello_read,
    .write = hello_write,
};

static int __init hello_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&devt, 0, 1, "hello-driver");
    if(ret < 0){
        goto ERR1;
    }else
        printk(KERN_ALERT "alloc device number success\n");
#if 1
    cdev_init(&char_device, &hello_ops); 

    ret = cdev_add(&char_device, devt, 1);
    if(ret){
        goto ERR2;
    }else
        printk(KERN_ALERT "add char device to kernel\n");

    dev_class = class_create(THIS_MODULE, "hello_driver");
    if(IS_ERR(dev_class)){
        printk(KERN_ALERT "class create failed\n");
        goto ERR3;
    }

    dev_device = device_create(dev_class, NULL, devt, NULL, "hello_dev_node");
    if(dev_device == NULL){
        printk(KERN_ALERT "device create failed\n");
        goto ERR4;
    }
#endif 
    printk(KERN_ALERT "hello-driver init success\n");

    return 0;

ERR4:
    class_destroy(dev_class);
ERR3:
    cdev_del(&char_device);
ERR2:
    unregister_chrdev_region(0, 1);
ERR1:
    return -1;
}

static void __exit hello_exit(void)
{
#if 1
    device_destroy(dev_class, devt);
    class_destroy(dev_class);
    cdev_del(&char_device);
#endif
    unregister_chrdev_region(devt, 1); 
    printk(KERN_ALERT "hello-driver removed, Goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
