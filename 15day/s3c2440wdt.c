#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <asm/io.h>
#include <linux/clk.h>
#include <linux/ioport.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/interrupt.h>

#define WDT_NAME "s3c2440_wdt"

#define S3C2440_PA_WDT (0x53000000)
#define S3C2440_SZ_WDT SZ_16
#define IRQ_S3C2440WDT 83

#define S3C2440_WTCON (0x0)
#define S3C2440_WTDAT (0x4)
#define S3C2440_WTCNT (0x8)

void __iomem *wdt_base;
struct resource *wdt_mem = NULL;
struct resource *wdt_irq = NULL;
struct clk *wdt_clk = NULL;

static int tmr_margin = 15;
static unsigned int wdt_count;

static DEFINE_SPINLOCK(wdt_spinlock);

struct file_operations wdt_fops = {
    .owner = THIS_MODULE,
};

struct miscdevice wdt_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = WDT_NAME,
    .fops = &wdt_fops,
};

int s3c2440wdt_set_heartbeat(unsigned int timeout)
{
    unsigned int freq = clk_get_rate(wdt_clk);
    unsigned int counter;
    unsigned int divisor = 1;
    unsigned long wtcon;

    if(timeout < 1)
        return -EINVAL;

    printk("%s: timeout=%d, freq=%d\n", __func__, timeout, freq);
    freq /= 128;
    counter = timeout * freq;

    printk("%s: count=%d, timeout=%d, freq=%d\n", __func__, counter, timeout, freq);

    if (counter >= 0x10000) {
        for (divisor = 1; divisor <= 0x100; divisor++) {
            if ((counter / divisor) < 0x10000)
                break;
        }
        
        if ((counter / divisor) >= 0x10000) {
            printk("timeout %d too big\n", timeout);
            return -EINVAL;
        }
    }

    printk("%s: timeout=%d, divisor=%d, count=%d (%08x)\n", __func__, timeout, divisor, counter, counter/divisor);


    tmr_margin = timeout;
    counter /= divisor;
    wdt_count = counter;

    wtcon = readl(wdt_base + S3C2440_WTCON);
    wtcon &= 0xff;
    wtcon |= ((divisor -1 ) << 8);

    writel(counter, wdt_base + S3C2440_WTCNT);
    writel(wtcon, wdt_base + S3C2440_WTCON);

    return 0;
}

void s3c2440wdt_keepalive(void)
{
    spin_lock(&wdt_spinlock);
    writel(wdt_count, wdt_base + S3C2440_WTCNT);
    spin_unlock(&wdt_spinlock);
}

void s3c2440wdt_stop(void)
{
    unsigned long wdt_con;
    spin_lock(&wdt_spinlock);
    wdt_con = readl(wdt_base + S3C2440_WTCON);
    wdt_con &= ~((0x1) | (0x1 << 5) | (0x1 << 2));
    writel(wdt_con, wdt_base + S3C2440_WTCON);
    spin_unlock(&wdt_spinlock);
}

void s3c2440wdt_start(void)
{
    unsigned long wdt_con;

    s3c2440wdt_stop();
    
    spin_lock(&wdt_spinlock);

    wdt_con = readl(wdt_base + S3C2440_WTCON);
    wdt_con |= (0x1 << 5) | (0x3 << 3);

    //wdt_con &= ~(0x1 << 2);
    wdt_con |= (0x1 << 0);
    wdt_con |= (0x1 << 2);

    writel(wdt_con, wdt_base + S3C2440_WTCON);
    writel(wdt_count, wdt_base + S3C2440_WTCNT);
    writel(wdt_count, wdt_base + S3C2440_WTDAT);

    spin_unlock(&wdt_spinlock);


}

static irqreturn_t s3c2440wdt_irq(int irqno, void *param)
{
    printk("watchdog timer expired (irq)");
    s3c2440wdt_keepalive();
    return IRQ_HANDLED;
}

static int driver_probe(struct platform_device *pdev)
{
    struct device *dev = NULL;
    struct resource *res = NULL;
    int size, ret;

    printk(KERN_INFO "platform match ok, probe = %p\n", pdev);

    dev = &pdev->dev;

    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if(res == NULL){
        printk(KERN_ERR "no memory resource specified\n");
        return -ENOENT;
    }
    printk(KERN_INFO "pa = 0x%x, 0x%x\n", res->start, res->end);

    size = (res->end - res->start) + 1;
    wdt_mem = request_mem_region(res->start, size, pdev->name);
    if(wdt_mem == NULL){
        printk(KERN_INFO "failed to get memory region\n");
        ret = -ENOENT;
        goto ERR1;
    }

    wdt_base = ioremap(res->start, size);
    if(wdt_base == NULL){
        printk(KERN_ERR "ioremap failed\n");
    }

    wdt_irq = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if(wdt_irq == NULL){
        printk(KERN_ERR "no irq resource specified\n");
    }

    ret = request_irq(wdt_irq->start, s3c2440wdt_irq, 0, pdev->name, pdev);
    if(ret != 0){
        printk(KERN_ERR "install irq failed, %d, %d\n", wdt_irq->start, ret);
    }

    wdt_clk = clk_get(&pdev->dev, "watchdog");

    clk_enable(wdt_clk);

    misc_register(&wdt_misc);
    
    //s3c2440wdt_set_heartbeat(15); 

    //s3c2440wdt_start();

    return 0;

ERR1:
    return ret;
}

static int driver_remove(struct platform_device *pdev)
{
    s3c2440wdt_stop();
    release_resource(wdt_mem);

    clk_disable(wdt_clk);
    clk_put(wdt_clk);

    iounmap(wdt_base);

    misc_deregister(&wdt_misc);

    printk(KERN_INFO "platform driver removed\n");
    return 0;
}

void s3c2440wdt_release(struct device *dev)
{
    printk("the s3c2440wdt device release\n");
}

static struct resource s3c2440wdt_resource[] = {
    [0] = {
        .start = S3C2440_PA_WDT,
        .end = S3C2440_PA_WDT + S3C2440_SZ_WDT - 1,
        .flags = IORESOURCE_MEM,
    },
    [1] = {
        .start = IRQ_S3C2440WDT,
        .end = IRQ_S3C2440WDT,
        .flags = IORESOURCE_IRQ,
    }
};

struct platform_device s3c2440wdt_device = {
    .id = -1,
    .name = "s3c2440wdt",
    .num_resources = ARRAY_SIZE(s3c2440wdt_resource),
    .resource = s3c2440wdt_resource,
    .dev.release = s3c2440wdt_release,
};

struct platform_driver s3c2440wdt_driver = {
    .probe = driver_probe,
    .remove = driver_remove,
    .driver = {
        .name = "s3c2410-wdt",
        .owner = THIS_MODULE,
    },
};

static int __init s3c2440wdt_init(void)
{
    
    //platform_device_register(&s3c2440wdt_device);
    platform_driver_register(&s3c2440wdt_driver);
    return 0;
}

static void __exit s3c2440wdt_exit(void)
{
    //platform_device_unregister(&s3c2440wdt_device);
    platform_driver_unregister(&s3c2440wdt_driver);
}

module_init(s3c2440wdt_init);
module_exit(s3c2440wdt_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Blackrose");
