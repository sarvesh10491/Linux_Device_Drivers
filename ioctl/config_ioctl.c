#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/init.h>

#include "config_ioctl.h"

#define  DEVICE_NAME "config_ioctl"
#define  CLASS_NAME  "config_ioctl_class"
 

MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux ioctl driver");  
MODULE_VERSION("1.0");


// Config variables used from user space
int madeup_dev_id;
int some_config;
int another_config;


static int majorNumber; 
static struct class*  configioctlClass  = NULL;
static struct device* configioctlDevice = NULL;

// The prototype functions for the ioctl character driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
    static int dev_ioctl(struct inode *, struct file *, unsigned int, unsigned long);
#else
    static long dev_ioctl(struct file *, unsigned int, unsigned long);
#endif


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




static int __init config_ioctl_init(void){
    printk(KERN_INFO "config_ioctl: Initializing the config_ioctl LKM\n");

    // Register major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "config_ioctl failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "config_ioctl: registered correctly with major number %d\n", majorNumber);


    // Register the device class
    configioctlClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(configioctlClass)){                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(configioctlClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "config_ioctl: device class registered correctly\n");


	// Register the device driver
    configioctlDevice = device_create(configioctlClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(configioctlDevice)){               // Clean up if there is an error
        class_destroy(configioctlClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(configioctlDevice);
    }
    printk(KERN_INFO "config_ioctl: device class created correctly\n"); // Made it! device was initialized

	return 0;
}


static void __exit config_ioctl_exit(void){
   device_destroy(configioctlClass, MKDEV(majorNumber, 0));     // remove the device
   class_unregister(configioctlClass);                          // unregister the device class
   class_destroy(configioctlClass);                             // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);                 // unregister the major number
   printk(KERN_INFO "config_ioctl: Goodbye from the LKM!\n");
}


static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "config_ioctl: Device has been opened.\n");
   return 0;
}


static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "config_ioctl: Device successfully closed\n");
   return 0;
}


#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,35))
static int dev_ioctl(struct inode *i, struct file *f, unsigned int cmd, unsigned long arg)
#else
static long dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg)
#endif
{
	config_dev_t cfgdata;

	switch (cmd){
		case GET_CONFIG_VAR:
			cfgdata.madeup_dev_id = madeup_dev_id;
            cfgdata.some_config = some_config;
            cfgdata.another_config = another_config;

			if(copy_to_user((config_dev_t *)arg, &cfgdata, sizeof(config_dev_t))){
				return -EACCES;
			}
			break;

		case CLR_CONFIG_VAR:
			madeup_dev_id = -1;
			some_config = -1;
			another_config = -1;
			break;

		case SET_CONFIG_VAR:
			if(copy_from_user(&cfgdata, (config_dev_t *)arg, sizeof(config_dev_t))){
				return -EACCES;
			}
			madeup_dev_id = cfgdata.madeup_dev_id;
			some_config = cfgdata.some_config;
			another_config = cfgdata.another_config;
			break;
		default:
			return -EINVAL;
	}

	return 0;
}

module_init(config_ioctl_init);
module_exit(config_ioctl_exit);