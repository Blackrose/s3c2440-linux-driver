#include <linux/module.h>
#include <linux/init.h>


void fun(void)
{
    printk(KERN_ALERT "I am called by someone\n");
}

EXPORT_SYMBOL(fun);

static int __init fun_init(void)
{
    printk(KERN_ALERT "fun driver loaded\n");
}

static void __exit fun_exit(void)
{

    printk(KERN_ALERT "fun driver unloaded\n");
}

module_init(fun_init);
module_exit(fun_exit);
MODULE_LICENSE("GPL");
