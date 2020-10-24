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
#include<linux/sysfs.h> 
#include <linux/workqueue.h>     // Required for the wait queues
#include<linux/kobject.h>       // Required for struct kobject
#include <linux/interrupt.h>    // Required for irq functions
#include <asm/io.h>
#include <asm/hw_irq.h>

#define IRQ_NO 10               // Interrupt Request number

#define  DEVICE_NAME "mod_workq"
#define  SYSFS_NAME  "workq_sysfs"
#define  CLASS_NAME  "mod_workq_class"


MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux Static workqueue driver");
MODULE_VERSION("1.0");


static int    majorNumber;       
static struct class*  mod_static_workqClass  = NULL;
static struct device* mod_static_workqDevice = NULL;

struct kobject *my_kobj;
volatile int dev_value = 0;     // This attribute file be crated under our sysfs directory to which we can read/write

//------------------------------------------------------------------------------------------------

void workqueue_fn(struct work_struct *work); 
 
DECLARE_WORK(workqueue, workqueue_fn);   // Static workqueue creation method

// Workqueue function 
void workqueue_fn(struct work_struct *work){
        printk(KERN_INFO "mod_static_workq: Executing Workqueue Function\n");
}

//------------------------------------------------------------------------------------------------

// Interrupt handler for IRQ 
static irqreturn_t my_irq_handler(int irq,void *dev_id){
    printk(KERN_INFO "mod_static_workq: Shared IRQ %d: Interrupt Occurred", IRQ_NO);

    return IRQ_HANDLED;
}

//------------------------------------------------------------------------------------------------

// The prototype functions for the character driver 
static int      dev_open(struct inode *, struct file *);
static int      dev_release(struct inode *, struct file *);
static ssize_t  dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t  dev_write(struct file *, const char *, size_t, loff_t *);


// The prototype file ops functions for the mod_irqmod_irq driver
static ssize_t  sysfs_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t  sysfs_store(struct kobject *, struct kobj_attribute *,const char *, size_t);


static struct file_operations fops =
{
   .open    = dev_open,
   .read    = dev_read,
   .write   = dev_write,
   .release = dev_release,
};


struct kobj_attribute my_kobj_attr = __ATTR(dev_value, 0660, sysfs_show, sysfs_store);

//------------------------------------------------------------------------------------------------

static int dev_open(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "mod_static_workq: Device has been opened\n");

    return 0;
}


static int dev_release(struct inode *inodep, struct file *filep){
    printk(KERN_INFO "mod_static_workq: Device successfully closed\n");

    return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_static_workq: read call\n");

    asm("int $0x3A");

    return 0;
}
 

static ssize_t dev_write(struct file *filep, const char *buff, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_static_workq: write call\n");
    
    return len;
}

//------------------------------------------------------------------------------------------------

// Sysfs ops functions
//=====================
// This fuction will be called when we read the sysfs file
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        printk(KERN_INFO "mod_static_workq: sysfs show call\n");
        return sprintf(buf, "%d\n", dev_value);
}
 
// This fuction will be called when we write the sysfsfs file
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        printk(KERN_INFO "mod_static_workq: sysfs store call\n");
        sscanf(buf,"%d\n",&dev_value);
        return count;
}   

//------------------------------------------------------------------------------------------------

static int __init mod_static_workq_init(void){
    printk(KERN_INFO "mod_static_workq: Initializing the mod_static_workq LKM\n");
    
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if(majorNumber<0){
        printk(KERN_ALERT "mod_static_workq failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "mod_static_workq: registered successfully with major number %d\n", majorNumber);

    // Register the device class
    mod_static_workqClass = class_create(THIS_MODULE, CLASS_NAME);
    if(IS_ERR(mod_static_workqClass)){                
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "mod_static_workq: Failed to register device class\n");
        return PTR_ERR(mod_static_workqClass);          
    }
    printk(KERN_INFO "mod_static_workq: device class registered successfully\n");

    // Register the device driver
    mod_static_workqDevice = device_create(mod_static_workqClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if(IS_ERR(mod_static_workqDevice)){               
        class_destroy(mod_static_workqClass);    
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "mod_static_workq: Failed to create the device\n");
        return PTR_ERR(mod_static_workqDevice);
    }

    
    // Create Sysfs entry
    my_kobj = kobject_create_and_add(SYSFS_NAME, kernel_kobj);
    if(!my_kobj)
		return -ENOMEM;

    // To create a single sysfs file attribute. 
    if(sysfs_create_file(my_kobj, &my_kobj_attr.attr))
        printk(KERN_INFO"mod_static_workq: Cannot create sysfs file.\n");
    

    // Allocate interrupt resources and enable the interrupt line and IRQ handling.
    if(request_irq(IRQ_NO, my_irq_handler, IRQF_SHARED, DEVICE_NAME, (void *)(my_irq_handler)))
            printk(KERN_INFO "mod_static_workq: cannot register IRQ");
    

    printk(KERN_INFO "mod_static_workq: LKM created successfully.\n");

	return 0;
}


static void __exit mod_static_workq_exit(void){
    printk(KERN_INFO "mod_static_workq: Removing LKM\n");
    free_irq(IRQ_NO, (void *)(my_irq_handler));                 // free irq line
    kobject_put(my_kobj);                                       // To dynamically free kobj structure
    sysfs_remove_file(kernel_kobj, &my_kobj_attr.attr);         // To remove sysfs file
    device_destroy(mod_static_workqClass, MKDEV(majorNumber, 0));
    class_unregister(mod_static_workqClass);                          
    class_destroy(mod_static_workqClass);                            
    unregister_chrdev(majorNumber, DEVICE_NAME);             

    printk(KERN_INFO "mod_static_workq: LKM removed successfully.\n");
}

//------------------------------------------------------------------------------------------------

module_init(mod_static_workq_init);
module_exit(mod_static_workq_exit);