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
#include<linux/sysfs.h>         // Required for sysfs functions
#include<linux/kobject.h>       // Required for struct kobject

#define  DEVICE_NAME "mod_sysfs"
#define  SYSFS_NAME "var_sysfs"
#define  CLASS_NAME  "mod_sysfs_class"


MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux sysfs driver");  
MODULE_VERSION("1.0");

static int majorNumber; 
static struct class*  modprocfsClass  = NULL;
static struct device* modprocfsDevice = NULL;
struct kobject *my_kobj;
volatile int dev_value = 0;     // This attribute file be crated under our sysfs directory to which we can read/write

// The prototype file ops functions for the procfs driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);


// The prototype file ops functions for the procfs driver
static ssize_t  sysfs_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t  sysfs_store(struct kobject *, struct kobj_attribute *,const char *, size_t);
 
struct kobj_attribute my_kobj_attr = __ATTR(dev_value, 0660, sysfs_show, sysfs_store);


// File ops structure
static struct file_operations fops =
{
	.owner      = THIS_MODULE,
	.open       = dev_open,
	.release    = dev_release,
    .read       = dev_read,
    .write      = dev_write,
};


//------------------------------------------------------------------------------------------------

static int __init mod_sysfs_init(void){
    printk(KERN_INFO "mod_sysfs: Initializing the mod_sysfs LKM\n");

    // Register major number
	majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber<0){
        printk(KERN_ALERT "mod_sysfs failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "mod_sysfs: registered correctly with major number %d\n", majorNumber);


    // Register the device class
    modprocfsClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(modprocfsClass)){                // Check for error and clean up if there is
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(modprocfsClass);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "mod_sysfs: device class registered correctly\n");


	// Register the device driver
    modprocfsDevice = device_create(modprocfsClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(modprocfsDevice)){                // Clean up if there is an error
        class_destroy(modprocfsClass);           // Repeated code but the alternative is goto statements
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(modprocfsDevice);
    }


    // Create Sysfs entry
    // If you pass kernel_kobj to the second argument, it will create the directory under /sys/kernel/
    // If you pass firmware_kobj to the second argument, it will create the directory under /sys/firmware/
    // If you pass fs_kobj to the second argument, it will create the directory under /sys/fs/
    // If you pass NULL to the second argument, it will create the directory under /sys/
    my_kobj = kobject_create_and_add(SYSFS_NAME, kernel_kobj);


    // To create a single sysfs file attribute. 
    if(sysfs_create_file(my_kobj, &my_kobj_attr.attr))
        printk(KERN_INFO"mod_sysfs: Cannot create sysfs file.\n");


    printk(KERN_INFO "mod_sysfs: device class created successfully.\n");

	return 0;
}


static void __exit mod_sysfs_exit(void){
    printk(KERN_INFO "mod_sysfs: Removing LKM\n");
    kobject_put(my_kobj);                                      // To dynamically free kobj structure
    sysfs_remove_file(kernel_kobj, &my_kobj_attr.attr);        // To remove sysfs file
    device_destroy(modprocfsClass, MKDEV(majorNumber, 0));     // remove the device
    class_unregister(modprocfsClass);                          // unregister the device class
    class_destroy(modprocfsClass);                             // remove the device class
    unregister_chrdev(majorNumber, DEVICE_NAME);               // unregister the major number
    printk(KERN_INFO "mod_sysfs: LKM removed successfully.\n");
}

//------------------------------------------------------------------------------------------------

// File ops functions
//=====================
static int dev_open(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mod_sysfs: Device has been opened.\n");
   return 0;
}


static int dev_release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "mod_sysfs: Device successfully closed\n");
   return 0;
}


static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_sysfs: read call\n");
    return 0;
}
 

static ssize_t dev_write(struct file *filep, const char *buff, size_t len, loff_t *offset){
    printk(KERN_INFO "mod_sysfs: write call\n");
    return len;
}


// Sysfs ops functions
//=====================
// This fuction will be called when we read the sysfs file
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        printk(KERN_INFO "mod_sysfs: sysfs show call\n");
        return sprintf(buf, "%d\n", dev_value);
}
 
// This fuction will be called when we write the sysfsfs file
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
        printk(KERN_INFO "mod_sysfs: sysfs store call\n");
        sscanf(buf,"%d\n",&dev_value);
        return count;
}

//------------------------------------------------------------------------------------------------

module_init(mod_sysfs_init);
module_exit(mod_sysfs_exit);