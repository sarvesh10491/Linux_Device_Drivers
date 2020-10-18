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

#define  SYSFS_NAME "var_sysfs"
#define  CLASS_NAME  "mod_sysfs_class"


MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux sysfs driver");  
MODULE_VERSION("1.0");

static struct class*  mod_sysfs_class  = NULL;

struct kobject *my_kobj;
volatile int dev_value = 0;     // This attribute file be crated under our sysfs directory to which we can read/write


// The prototype file ops functions for the sysfs driver
static ssize_t  sysfs_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t  sysfs_store(struct kobject *, struct kobj_attribute *,const char *, size_t);
 
struct kobj_attribute my_kobj_attr = __ATTR(dev_value, 0660, sysfs_show, sysfs_store);

//------------------------------------------------------------------------------------------------

static int __init mod_sysfs_init(void){
    printk(KERN_INFO "mod_sysfs: Initializing the mod_sysfs LKM\n");


    // Register the device class
    mod_sysfs_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mod_sysfs_class)){                // Check for error and clean up if there is
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(mod_sysfs_class);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "mod_sysfs: device class registered correctly\n");


    // Create Sysfs entry
    // If you pass kernel_kobj to the second argument, it will create the directory under /sys/kernel/
    // If you pass firmware_kobj to the second argument, it will create the directory under /sys/firmware/
    // If you pass fs_kobj to the second argument, it will create the directory under /sys/fs/
    // If you pass NULL to the second argument, it will create the directory under /sys/
    my_kobj = kobject_create_and_add(SYSFS_NAME, kernel_kobj);
    if(!my_kobj)
		return -ENOMEM;

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
    class_unregister(mod_sysfs_class);                         // unregister the device class
    class_destroy(mod_sysfs_class);                            // remove the device class
    printk(KERN_INFO "mod_sysfs: LKM removed successfully.\n");
}

//------------------------------------------------------------------------------------------------

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