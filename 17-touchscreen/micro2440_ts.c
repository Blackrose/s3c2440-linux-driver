#include <linux/init.h>
#include <linux/module.h>

static int __init ts_init(void)
{

}

static void __exit ts_exit(void)
{

}

module_init(ts_init);
module_exit(ts_init);
MODULE_AUTHOR("KevinChen");
MODULE_LICENSE("GPL");
