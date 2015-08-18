#include <linux/init.h>
#include <linux/module.h>

extern void fun(void);
static int __init hello_init(void)
{
    printk(KERN_ALERT "Hello, driver\n");

    fun();
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, driver\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
