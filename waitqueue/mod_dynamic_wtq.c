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

#define  DEVICE_NAME "wtq_dynamic"           // The device will appear at /dev/hello_char using this value
#define  CLASS_NAME  "wtq_dynamic_class"

MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux Dynamic waitqueue driver");
MODULE_VERSION("1.0");

wait_queue_head_t dynamic_wtq;    // Dynamic method to create workqueue


static int    majorNumber;       
static struct class*  mod_dynamic_wtqClass  = NULL;
static struct device* mod_dynamic_wtqDevice = NULL;


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
        printk(KERN_INFO "mod_dynamic_wtq: Waiting For Events.\n");

        wait_event_interruptible(dynamic_wtq, wtq_flag);     // waiting for events condition based on wtq_flag
        
        if(wtq_flag == 1){
            printk(KERN_INFO "mod_dynamic_wtq: Event Came From Read Function call : %d\n", ++read_calls);
        }
        if(wtq_flag == 2){
            printk(KERN_INFO "mod_dynamic_wtq: Event Came From Write Function call for %ld sec timeout : %d\n", 
                                                    simple_strtol(timeoutstr, NULL, 10), 
                                                    ++write_calls);     
                                                    
            wait_event_interruptible_timeout(dynamic_wtq, 0, HZ*simple_strtol(timeoutstr, NULL, 10)); // waiting for events condition based on timeout value set in char device
            printk(KERN_INFO "mod_dynamic_wtq: Timeout event %d completed.\n", write_calls);
        }
        if(wtq_flag == -1){
            printk(KERN_INFO "mod_dynamic_wtq: Event Came From Exit Function\n");
            return 0;
        }
        wtq_flag = 0;
    }
    do_exit(0);

    return 0;
}

//------------------------------------------------------------------------------------------------

static int dev_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "mod_dynamic_wtq: Device has been opened\n");

    return 0;
}


static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "mod_dynamic_wtq: Device successfully closed\n");

    return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_dynamic_wtq: read call\n");

    wtq_flag = 1;
    wake_up_interruptible(&dynamic_wtq);

    return 0;
}
 

static ssize_t dev_write(struct file *filep, const char *buff, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_dynamic_wtq: write call\n");

    wtq_flag = 2;
    memset(timeoutstr , 0 , sizeof(timeoutstr));
    if(copy_from_user(timeoutstr, buff, len)){
		return -EFAULT;
	}

    wake_up_interruptible(&dynamic_wtq);
    
    return len;
}

//------------------------------------------------------------------------------------------------

static int __init mod_dynamic_wtq_init(void){
    printk(KERN_INFO "mod_dynamic_wtq: Initializing the mod_dynamic_wtq LKM\n");
    
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if(majorNumber<0){
        printk(KERN_ALERT "mod_dynamic_wtq failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "mod_dynamic_wtq: registered successfully with major number %d\n", majorNumber);

    // Register the device class
    mod_dynamic_wtqClass = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(mod_dynamic_wtqClass)){                
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "mod_dynamic_wtq: Failed to register device class\n");
        return PTR_ERR(mod_dynamic_wtqClass);          
    }
    printk(KERN_INFO "mod_dynamic_wtq: device class registered successfully\n");

    // Register the device driver
    mod_dynamic_wtqDevice = device_create(mod_dynamic_wtqClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if(IS_ERR(mod_dynamic_wtqDevice)){               
        class_destroy(mod_dynamic_wtqClass);    
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "mod_dynamic_wtq: Failed to create the device\n");
        return PTR_ERR(mod_dynamic_wtqDevice);
    }

    // Initialize wait queue
    init_waitqueue_head(&dynamic_wtq);

    // kthread spawning
    printk(KERN_INFO "mod_dynamic_wtq: Starting kthread\n");
    snprintf(ktstr, 10, "Dynamic-Waitqueue-Thread");

    wtq_ktrd_ts = kthread_run(kthread_func, NULL, ktstr);
    if(IS_ERR(wtq_ktrd_ts)){
        printk(KERN_INFO "mod_dynamic_wtq: ERROR: Cannot create thread\n");
        err = PTR_ERR(wtq_ktrd_ts);
        wtq_ktrd_ts = NULL;
        return err;
    }


    printk(KERN_INFO "mod_dynamic_wtq: LKM created successfully.\n");

	return 0;
}


static void __exit mod_dynamic_wtq_exit(void){
    printk(KERN_INFO "mod_dynamic_wtq: Stopping Dynamic-Waitqueue-Thread Thread\n");

    wtq_flag = -1;
    wake_up_interruptible(&dynamic_wtq);

    printk(KERN_INFO "mod_dynamic_wtq: Dynamic-Waitqueue-Thread stopped successfully.\n");


    device_destroy(mod_dynamic_wtqClass, MKDEV(majorNumber, 0));
    class_unregister(mod_dynamic_wtqClass);                          
    class_destroy(mod_dynamic_wtqClass);                            
    unregister_chrdev(majorNumber, DEVICE_NAME);             

    printk(KERN_INFO "mod_dynamic_wtq: LKM removed successfully.\n");
}

//------------------------------------------------------------------------------------------------

module_init(mod_dynamic_wtq_init);
module_exit(mod_dynamic_wtq_exit);