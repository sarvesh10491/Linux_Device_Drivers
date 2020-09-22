#include <linux/module.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <asm/param.h>
#include <linux/version.h>

MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux timer_api driver");  
MODULE_VERSION("1.0");


int counter;
struct timer_list dev_timer;


// Timer API callback function
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
static void timer_cb(unsigned long timerp)
{  
    timerp = (struct timer_list *)timerp;

    counter++;
    printk("timer_mod: %s(): %d sec elapsed.\n", __func__, counter);
    dev_timer.expires = jiffies + HZ;
    add_timer(timerp);

#else
static void timer_cb(struct timer_list *timerp)
{
    counter++;
    printk("timer_mod: %s(): %d sec elapsed.\n", __func__, counter);
    mod_timer(timerp, jiffies + msecs_to_jiffies(1000));
#endif

}


// Timer API LKM init function
static int __init timer_mod_init(void)
{
    counter = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,14,0))
    init_timer(&dev_timer);
    dev_timer->function = timer_cb;
    dev_timer->data = 0;
    dev_timer->expires = jiffies + HZ;
    add_timer(&dev_timer);
#else
    timer_setup(&dev_timer, timer_cb, 0);
    mod_timer(&dev_timer, jiffies + msecs_to_jiffies(1000));
#endif

    printk("timer_mod: timer_mod_init() called.\n");

    return 0;
}


// Timer API LKM exit function
static void __exit timer_mod_exit(void)
{
	del_timer(&dev_timer);
	printk("timer_mod: timer_mod_exit() called.\n");
}


module_init(timer_mod_init);
module_exit(timer_mod_exit);