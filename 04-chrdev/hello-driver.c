#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/fs.h>

dev_t chr_dev;

static int __init hello_init(void)
{
    int ret;
    
    printk(KERN_ALERT "hello driver world\n");
    
    ret = alloc_chrdev_region(&chr_dev, 0, 1, "hello-driver");

    if(ret != 0){
        printk(KERN_ALERT "driver wasn't register\n");
        goto ERR1;
    }else    
        printk(KERN_ALERT "driver register success, major=%d, minor=%d\n",
                MAJOR(chr_dev), MINOR(chr_dev));

    return 0;

ERR1:
    unregister_chrdev_region(chr_dev, 1);
    return -1;
}

static void __exit hello_exit(void)
{
    unregister_chrdev_region(chr_dev, 1);

    printk(KERN_ALERT "good bye world\n");
}


module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
MODULE_VERSION("1.0");
