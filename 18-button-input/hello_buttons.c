#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/uaccess.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/input.h>
#include <linux/delay.h>
#include <linux/sysctl.h>
#include <linux/timer.h>

#define GPG_CON 0x56000060
#define GPG_DAT 0x56000064

// (27 + 9) + 16 = 52
#define IRQNO_EINT8 52
#define IRQNO_EINT11 55
#define IRQNO_EINT13 57
#define IRQNO_EINT14 58
#define IRQNO_EINT15 59
#define IRQNO_EINT19 63

#define EINT_MODE 0x2

struct btn_resource{
    int irq_no;
    char *name;
    unsigned int pin;
    unsigned int key_val;
};

struct btn_resource button_res[] =
{
    {IRQNO_EINT8, "K1", 0, KEY_L},
    {IRQNO_EINT11, "K2", 3, KEY_S},
    {IRQNO_EINT13, "K3", 5, KEY_ENTER},
    {IRQNO_EINT14, "K4", 6, KEY_LEFTSHIFT},
    {IRQNO_EINT15, "K5", 7, KEY_A},
    {IRQNO_EINT19, "K6", 11, KEY_B},
};


void* con_va;
void* dat_va;

static struct input_dev *buttons_dev;
static struct timer_list buttons_timer;
struct btn_resource *btn_res;

unsigned int get_pin(unsigned int pin)
{

    unsigned long tmp;

    tmp = ioread16(dat_va);

    //printk(KERN_ALERT "%x\n", tmp);
#if 0
    button_value[0] = !((tmp >> 0) & 0x1);
    button_value[1] = !((tmp >> 3) & 0x1);
    button_value[2] = !((tmp >> 5) & 0x1);
    button_value[3] = !((tmp >> 6) & 0x1);
    button_value[4] = !((tmp >> 7) & 0x1);
    button_value[5] = !((tmp >> 11) & 0x1);
#endif
    return ((tmp >> pin) & 0x1);

}


irqreturn_t button_irq(int irq, void *dev_id)
{
    
    //tasklet_schedule(&my_tasklet);
    btn_res = (struct btn_resource*)dev_id;
    mod_timer(&buttons_timer, jiffies+HZ/100);

    return IRQ_RETVAL(IRQ_HANDLED);
}

static void buttons_timer_func(unsigned long data)
{
    struct btn_resource *res = btn_res;
    unsigned int pinval;

    if(!btn_res)
        return;
    pinval = get_pin(res->pin);

    if(pinval){
        input_event(buttons_dev, EV_KEY, res->key_val, 0);
        input_sync(buttons_dev);
    }else{
        input_event(buttons_dev, EV_KEY, res->key_val, 1);
        input_sync(buttons_dev);
    }
}
#if 0
int button_open(struct inode *inode, struct file *filp)
{
    unsigned long tmp;
    int ret, i = 0;
#if 1   
    tmp = ioread32(con_va);
    tmp |= (EINT_MODE << 0) | (EINT_MODE << 6) | (EINT_MODE << 10) | (EINT_MODE << 12)
        | (EINT_MODE << 14) | (EINT_MODE << 22);
    iowrite32(tmp, con_va);
#endif
    tasklet_init(&my_tasklet, tasklet_routine, 0);
#if 0
    ret = request_irq(IRQNO_EINT8, button_irq, IRQ_TYPE_EDGE_BOTH, "K1", NULL);
        if(ret < 0){
            printk(KERN_ALERT "request irq error, %d", ret);
            return -1;
        }
#endif 
#if 1
    for(i = 0; i < 6; i++){
        ret = request_irq(button_res[i].irq_no, button_irq, IRQ_TYPE_EDGE_BOTH, button_res[i].button_name, NULL);
        if(ret < 0){
            printk(KERN_ALERT "request irq error, %d, index%d\n", ret, i);
            break;
        }

    }
#endif
    return 0;
}
ssize_t button_read(struct file *filp, char *buf, size_t len, loff_t *offset)
{
    if(copy_to_user(buf, (const void*)button_value, sizeof(button_value))){
        printk(KERN_ALERT "copyt ot user error\n");
        return -1;
    }

    return sizeof(button_value);
}

int button_release(struct inode *inode, struct file *filp)
{
    int i;
        
#if 0 
    free_irq(IRQNO_EINT8, NULL);
#else
    for(i = 0; i < 6;i++){
        free_irq(button_res[i].irq_no, NULL);
    }
#endif
    return 0;
}

struct file_operations button_fops =
{
    .owner = THIS_MODULE,
    .read = button_read,
    .open = button_open,
    .release = button_release,
};

#endif
int __init button_init(void)
{
    int ret;
    int i, tmp;
    buttons_dev = input_allocate_device();

    set_bit(EV_KEY, buttons_dev->evbit);
    set_bit(EV_REP, buttons_dev->keybit);

    set_bit(KEY_L, buttons_dev->keybit);
    set_bit(KEY_S, buttons_dev->keybit);
    set_bit(KEY_ENTER, buttons_dev->keybit);
    set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);

    input_register_device(buttons_dev);

    init_timer(&buttons_timer);
    buttons_timer.function = buttons_timer_func;
    add_timer(&buttons_timer);
 
    for(i = 0; i < 6; i++){
        ret = request_irq(button_res[i].irq_no, button_irq, IRQ_TYPE_EDGE_BOTH, button_res[i].name, &button_res[i]);
        if(ret < 0){
            printk(KERN_ALERT "request irq error, %d, index%d\n", ret, i);
            break;
        }

    }
#if 0 
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
#endif
    con_va = ioremap(GPG_CON, 4);
    dat_va = ioremap(GPG_DAT, 4);
 
    tmp = ioread32(con_va);
    tmp |= (EINT_MODE << 0) | (EINT_MODE << 6) | (EINT_MODE << 10) | (EINT_MODE << 12)
        | (EINT_MODE << 14) | (EINT_MODE << 22);
    iowrite32(tmp, con_va);
   

    printk(KERN_ALERT "buttons driver init successful\n");
    return 0;
#if 0
ERR4:
    class_destroy(button_class);
ERR3:
    cdev_del(&button_cdev);
ERR2:
    unregister_chrdev_region(button_devt, 1);
ERR1:
    return -1;
#endif
}

void __exit button_exit(void)
{
    iounmap(con_va);
    iounmap(dat_va);
#if 0
    device_destroy(button_class, button_devt);
    class_destroy(button_class);
    cdev_del(&button_cdev);
    unregister_chrdev_region(button_devt, 1);
    3
#endif
    printk(KERN_ALERT "buttons driver removed successful\n");
}

module_init(button_init);
module_exit(button_exit);

MODULE_AUTHOR("Blackrose");
MODULE_LICENSE("GPL");
