#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/string.h>
#include<linux/proc_fs.h>   // Library for procfs functions

#include "config_ioctl.h"

#define  DEVICE_NAME "mod_procfs"
#define  PROCFS_NAME "var_procfs"
#define  CLASS_NAME  "mod_procfs_class"
#define  PROCFS_BUF_SIZE  25
 

MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux procfs driver");  
MODULE_VERSION("1.0");

char procfs_buf[PROCFS_BUF_SIZE]="init_procfs_buf\n";
int8_t ioctl_var = 0;
int8_t error_count =0;
static int buf_len = 1;


static int majorNumber; 
static struct class*  modprocfsClass  = NULL;
static struct device* modprocfsDevice = NULL;


// The prototype file ops functions for the procfs driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    static int dev_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
#else
    static long dev_ioctl(struct file *, unsigned int, unsigned long);
#endif


// The prototype file ops functions for the procfs driver
static int      proc_open(struct inode *, struct file *);
static int      proc_release(struct inode *, struct file *);
static ssize_t  proc_read(struct file *, char __user *, size_t ,loff_t * );
static ssize_t  proc_write(struct file *, const char *, size_t , loff_t * );



// File ops structure
static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_release,

    #if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
	    .ioctl = dev_ioctl
    #else
	    .unlocked_ioctl = dev_ioctl
    #endif
};


// procfs ops structure
static struct file_operations proc_ops = {
        .open = proc_open,
        .read = proc_read,
        .write = proc_write,
        .release = proc_release
};

//------------------------------------------------------------------------------------------------

static int __init mod_procfs_init(void){
    printk(KERN_INFO "mod_procfs: Initializing the mod_procfs LKM\n");

    // Register major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "mod_procfs failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "mod_procfs: registered correctly with major number %d\n", majorNumber);


    // Register the device class
    modprocfsClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(modprocfsClass)){                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(modprocfsClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "mod_procfs: device class registered correctly\n");


	// Register the device driver
    modprocfsDevice = device_create(modprocfsClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(modprocfsDevice)){               // Clean up if there is an error
        class_destroy(modprocfsClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(modprocfsDevice);
    }


    // Create Proc entry
    proc_create(PROCFS_NAME, 0666, NULL, &proc_ops);
    printk(KERN_INFO "mod_procfs: procfs entry created.\n");


    printk(KERN_INFO "mod_procfs: device class created correctly\n"); // Made it! device was initialized

	return 0;
}


static void __exit mod_procfs_exit(void){
    remove_proc_entry(PROCFS_NAME, NULL);                      // remove proc entry
    device_destroy(modprocfsClass, MKDEV(majorNumber, 0));     // remove the device
    class_unregister(modprocfsClass);                          // unregister the device class
    class_destroy(modprocfsClass);                             // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);               // unregister the major number
    printk(KERN_INFO "mod_procfs: Goodbye from the LKM!\n");
}

//------------------------------------------------------------------------------------------------

// File ops functions

static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mod_procfs: Device has been opened.\n");
   return 0;
}


static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mod_procfs: Device successfully closed\n");
   return 0;
}


// Will be called when we write IOCTL on the Device file
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int dev_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
    printk(KERN_INFO "mod_procfs: ioctl invoked\n");
	switch(cmd) {
        case WRITE_CMD:
            printk(KERN_INFO "mod_procfs: ioctl write call\n");
            copy_from_user(&ioctl_var ,(int32_t*) arg, sizeof(ioctl_var));
            break;
        case READ_CMD:
            printk(KERN_INFO "mod_procfs: ioctl read call\n");
            copy_to_user((int32_t*) arg, &ioctl_var, sizeof(ioctl_var));
            break;
    }
    return 0;
}


// Proc ops functions

static int proc_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mod_procfs: procfs has been opened.\n");
   return 0;
}


static int proc_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mod_procfs: procfs successfully closed\n");
   return 0;
}


static ssize_t proc_read(struct file *filp, char __user *buf, size_t lenn, loff_t * offset)
{
    printk(KERN_INFO "mod_procfs: Read from proc file.\n");
    error_count = 0;

    if(buf_len)
        buf_len = 0;
    else{
        buf_len = 1;
        return 0;
    }

    printk(KERN_INFO "mod_procfs: Sending %s.\n", procfs_buf);
    error_count = copy_to_user(buf, procfs_buf, strlen(procfs_buf));

    if(error_count==0){
      printk(KERN_INFO "mod_procfs: Sent procfs data %s to the user\n", procfs_buf);
      return lenn;
   }
   else{
      printk(KERN_INFO "mod_procfs: Failed to send procfs data to the user\n");
      return -EFAULT;
   }
}


static ssize_t proc_write(struct file *filp, const char *buf, size_t lenn, loff_t * off)
{
    printk(KERN_INFO "mod_procfs: Written to proc file.\n");
    memset(procfs_buf , 0 , sizeof(procfs_buf));
    copy_from_user(procfs_buf, buf, lenn);

    return lenn;
}


module_init(mod_procfs_init);
module_exit(mod_procfs_exit);