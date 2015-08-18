#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/uaccess.h>

#define GPG_CON 0x56000060
#define GPG_DAT 0x56000064

void* con_va;
void* dat_va;

dev_t button_devt;
struct cdev button_cdev = 
{
    .owner = THIS_MODULE,
};
struct class *button_class = NULL;
struct device *button_device = NULL;

ssize_t button_read(struct file *filp, char *buf, size_t len, loff_t *offset)
{
    unsigned long tmp;

    tmp = ioread32(dat_va);
    tmp = tmp & (1 << 3);
    printk(KERN_ALERT "%x\n", tmp);
    if(copy_to_user(buf, &tmp, sizeof(tmp))){
        printk(KERN_ALERT "copyt ot user error\n");
        return -1;
    }

    return sizeof(tmp);
}

struct file_operations button_fops =
{
    .owner = THIS_MODULE,
    .read = button_read,
};

int __init button_init(void)
{
    int ret;
    unsigned long tmp;

    ret = alloc_chrdev_region(&button_devt, 0, 1, "buttons");
    if(ret < 0){
        printk(KERN_ALERT "alloc device number error\n");
        goto ERR1;
    }

    cdev_init(&button_cdev, &button_fops);

    ret = cdev_add(&button_cdev, button_devt, 1);
    if(ret < 0){
        printk(KERN_ALERT "cdev add error\n");
        goto ERR2;
    }

    button_class = class_create(THIS_MODULE, "buttons");
    if(!button_class){
        printk(KERN_ALERT "class create error\n");
        goto ERR3;
    }

    button_device = device_create(button_class, NULL, button_devt, NULL, "buttons");
    if(!button_device){
        printk(KERN_ALERT "device create error\n");
        goto ERR4;
    }

    con_va = ioremap(GPG_CON, 4);
    dat_va = ioremap(GPG_DAT, 4);
    
    tmp = ioread32(con_va);
    tmp |= (0 << 0) | (0 << 7) | (0 << 11) | (0 << 13) | (0 << 15) | (0 << 23);
    iowrite32(tmp, con_va);

    printk(KERN_ALERT "buttons driver init successful\n");
    return 0;

ERR4:
    class_destroy(button_class);
ERR3:
    cdev_del(&button_cdev);
ERR2:
    unregister_chrdev_region(button_devt, 1);
ERR1:
    return -1;
}

void __exit button_exit(void)
{
    iounmap(con_va);
    iounmap(dat_va);

    device_destroy(button_class, button_devt);
    class_destroy(button_class);
    cdev_del(&button_cdev);
    unregister_chrdev_region(button_devt, 1);
    printk(KERN_ALERT "buttons driver removed successful\n");
}

module_init(button_init);
module_exit(button_exit);

MODULE_AUTHOR("Blackrose");
MODULE_LICENSE("GPL");
