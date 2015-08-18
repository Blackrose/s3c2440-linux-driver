#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include "mem.h"

#define DYN

#define CMD_MAGIC 'x'
#define CMD_NR 1

#define HELLO_CMD_WEL _IO(CMD_MAGIC, 0)

int hello_open(struct inode *node, struct file *filp);
int hello_release(struct inode *node, struct file *filp);
ssize_t hello_read(struct file *filp, char __user *buf, size_t count, loff_t *offset);
ssize_t hello_write(struct file *filp, char __user *buf, size_t count, loff_t *offset);
int hello_ioctl(struct inode *node, struct file *filp, unsigned int cmd, unsigned long arg);

char kbuf[100];
int var = 1;
bool flag = false;
unsigned int major = 259;
unsigned int minor = 0;
dev_t dev;
struct page *mypage;
char* addr;

struct file_operations ops = {
    .open = hello_open,
    .release = hello_release,
    .read = hello_read,
    .write = hello_write,
    .ioctl = hello_ioctl,
};

struct cdev test_cdev={
    .owner = THIS_MODULE,
};

ssize_t hello_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    int ret;

    //memcpy(buf, "hello, app", count);

    if(copy_to_user(buf, kbuf, count)){
        ret = -EFAULT; 
    }else
        ret = count;

    return ret;

}

ssize_t hello_write(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{
    int ret;

    //memcpy(kbuf, buf, count);

    if(copy_from_user(kbuf, buf, count)){
        ret = -EFAULT;
    }else
        ret = count;

    printk("kbuf = %s\n", kbuf);

    return ret;

}

int hello_ioctl(struct inode *node, struct file *filp, unsigned int cmd, unsigned long arg)
{
    if (_IOC_TYPE(cmd) != CMD_MAGIC)
        return -EINVAL;
    if(_IOC_SIZE(cmd) > CMD_NR)
        return -EINVAL;

    switch(cmd){
        case HELLO_CMD_WEL:
            memcpy(kbuf, "the CMD is apply", 100);
            break;
        default:
            break;

    }

   return 0; 

}

int hello_open(struct inode *node, struct file *filp)
{
    printk("open file\n");

    return 0;

}

int hello_release(struct inode *node, struct file *filp)
{
    printk("close file\n");
    
    return 0;
}

static int __init hello_init(void)
{
    int ret;
    unsigned long phy, vir;

    printk(KERN_DEBUG "hello driver world\n");
    if(flag)
        printk("var = %d\n", var);
    else
        printk("you choose flag is False\n");

    mypage = alloc_pages(GFP_KERNEL, 1); 
    addr = page_address(mypage);

    phy = __pa((unsigned long)addr);
    vir = (unsigned long)__va(addr);

    printk("physical addr: %p\n", (void *)phy);
    printk("virtual addr: %p\n", (void *)vir);
    printk("alloc page at %p\n", (void *)addr);


#ifdef DYN
    ret = alloc_chrdev_region(&dev, 0, 1, "char-test");
#else
    dev = MKDEV(major, minor);
    ret = register_chrdev_region(dev, 1, "char-test");
#endif

    if (ret == 0)
        printk("Register char device success\n");
    else{
        printk("Register failed\n");
        ret = -ENODEV;
        goto error1;
    }

    cdev_init(&test_cdev, &ops);

    ret = cdev_add(&test_cdev, dev, 1);
    if (ret == 0)
        printk("add dev success\n");
    else{
        printk("add dev failed\n");
        ret = -ENODEV;
        goto error0;
    }

    memcpy(kbuf, "hello, apper", 100);

    return 0;

error1:
    unregister_chrdev_region(dev, 1);
error0:
    return ret;
}

static void __exit hello_exit(void)
{

    cdev_del(&test_cdev);
    unregister_chrdev_region(dev, 1);
    printk("good bye world\n");
}


module_init(hello_init);
module_exit(hello_exit);

module_param(var, int, 0644);
module_param(flag, bool, 0644);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Blackrose");
MODULE_VERSION("1.0");
