#include <linux/init.h>
#include <linux/module.h>


static int age = 1;
static char *name = "noname";
module_param(age, int, 0644);
module_param(name, charp, 0644);

static int __init hello_init(void)
{
    printk(KERN_ALERT "Hello, driver, %d, %s\n", age, name);

    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_ALERT "Goodbye, driver\n");     
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
