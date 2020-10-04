#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/wait.h>     // Required for the wait queues

#define  DEVICE_NAME "wtq_static"           // The device will appear at /dev/hello_char using this value
#define  CLASS_NAME  "wtq_static_class"

MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux Static waitqueue driver");
MODULE_VERSION("1.0");

DECLARE_WAIT_QUEUE_HEAD(static_wtq);    // Static method to create workqueue


static int    majorNumber;       
static struct class*  mod_static_wtqClass  = NULL;
static struct device* mod_static_wtqDevice = NULL;


static struct task_struct *wtq_ktrd_ts;   // kthread task_struct structure
static char ktstr[10];

uint32_t read_calls = 0, write_calls = 0;
uint8_t wtq_flag, ret, err;
char timeoutstr[5] = "0";
 
//------------------------------------------------------------------------------------------------

// The prototype functions for the character driver 
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
 

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .write = dev_write,
   .release = dev_release,
};

//------------------------------------------------------------------------------------------------

// kthread function
static int kthread_func(void *arg)
{
    while(1){
        printk(KERN_INFO "mod_static_wtq: Waiting For Events.\n");

        wait_event_interruptible(static_wtq, wtq_flag != 0 );
        
        if(wtq_flag == 1){
            printk(KERN_INFO "mod_static_wtq: Event Came From Read Function call : %d\n", ++read_calls);
        }
        if(wtq_flag == 2){
            printk(KERN_INFO "mod_static_wtq: Event Came From Write Function call for %s sec timeout : %d\n", timeoutstr, ++write_calls);
            wait_event_interruptible_timeout(static_wtq, 1==0, (int)simple_strtol(timeoutstr, NULL, 10));
            printk(KERN_INFO "mod_static_wtq: Timeout event completed.\n");
        }
        if(wtq_flag == -1){
            printk(KERN_INFO "mod_static_wtq: Event Came From Exit Function\n");
            return 0;
        }
        wtq_flag = 0;
    }
    do_exit(0);

    return 0;
}

//------------------------------------------------------------------------------------------------

static int dev_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "mod_static_wtq: Device has been opened\n");

    return 0;
}


static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "mod_static_wtq: Device successfully closed\n");

    return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_static_wtq: read call\n");

    wtq_flag = 1;
    wake_up_interruptible(&static_wtq);

    return 0;
}
 

static ssize_t dev_write(struct file *filep, const char *buff, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_static_wtq: write call\n");

    wtq_flag = 2;
    memset(timeoutstr , 0 , sizeof(timeoutstr));
    if(copy_from_user(timeoutstr, buff, len)){
		return -EFAULT;
	}

    wake_up_interruptible(&static_wtq);
    
    return len;
}

//------------------------------------------------------------------------------------------------

static int __init mod_static_wtq_init(void){
    printk(KERN_INFO "mod_static_wtq: Initializing the mod_static_wtq LKM\n");
    
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if(majorNumber<0){
        printk(KERN_ALERT "mod_static_wtq failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "mod_static_wtq: registered successfully with major number %d\n", majorNumber);

    // Register the device class
    mod_static_wtqClass = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(mod_static_wtqClass)){                
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "mod_static_wtq: Failed to register device class\n");
        return PTR_ERR(mod_static_wtqClass);          
    }
    printk(KERN_INFO "mod_static_wtq: device class registered successfully\n");

    // Register the device driver
    mod_static_wtqDevice = device_create(mod_static_wtqClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if(IS_ERR(mod_static_wtqDevice)){               
        class_destroy(mod_static_wtqClass);    
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "mod_static_wtq: Failed to create the device\n");
        return PTR_ERR(mod_static_wtqDevice);
    }

    // Initialize wait queue
    init_waitqueue_head(&static_wtq);

    // kthread spawning
    printk(KERN_INFO "mod_static_wtq: Starting kthread\n");
    snprintf(ktstr, 10, "Static-Waitqueue-Thread");

    wtq_ktrd_ts = kthread_run(kthread_func, NULL, ktstr);
    if(IS_ERR(wtq_ktrd_ts)){
        printk(KERN_INFO "mod_static_wtq: ERROR: Cannot create thread\n");
        err = PTR_ERR(wtq_ktrd_ts);
        wtq_ktrd_ts = NULL;
        return err;
    }


    printk(KERN_INFO "mod_static_wtq: LKM created successfully.\n");

	return 0;
}


static void __exit mod_static_wtq_exit(void){
    printk(KERN_INFO "mod_static_wtq: Stopping Static-Waitqueue-Thread Thread\n");

    wtq_flag = -1;
    wake_up_interruptible(&static_wtq);

    printk(KERN_INFO "mod_static_wtq: Static-Waitqueue-Thread stopped successfully.\n");


    device_destroy(mod_static_wtqClass, MKDEV(majorNumber, 0));
    class_unregister(mod_static_wtqClass);                          
    class_destroy(mod_static_wtqClass);                            
    unregister_chrdev(majorNumber, DEVICE_NAME);             

    printk(KERN_INFO "mod_static_wtq: LKM removed successfully.\n");
}

//------------------------------------------------------------------------------------------------

module_init(mod_static_wtq_init);
module_exit(mod_static_wtq_exit);