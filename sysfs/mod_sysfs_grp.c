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

#define  SYSFS_NAME  "var_sysfs_grp"
#define  CLASS_NAME  "mod_sysfs_grp_class"


MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux sysfs driver");  
MODULE_VERSION("1.0");

static struct class*  mod_sysfs_grp_class  = NULL;

struct kobject *my_kobj;
// This attribute file group be crated under our sysfs directory to which we can read/write
volatile int one_attr = 0;
volatile int two_attr = 1;


// The prototype file ops functions for the procfs driver
static ssize_t  sysfs_show(struct kobject *, struct kobj_attribute *, char *);
static ssize_t  sysfs_store(struct kobject *, struct kobj_attribute *,const char *, size_t);
 

struct kobj_attribute my_kobj_attr_1 = __ATTR(one_attr, 0660, sysfs_show, sysfs_store);
struct kobj_attribute my_kobj_attr_2 = __ATTR(two_attr, 0660, sysfs_show, sysfs_store);


static struct attribute *my_attrs[] = {
	&my_kobj_attr_1.attr,
	&my_kobj_attr_2.attr,
	NULL,                   // NULL required to terminate the list of attributes
};

static struct attribute_group my_attr_group = {
	.attrs = my_attrs,
};

int var = -1;

//------------------------------------------------------------------------------------------------

static int __init mod_sysfs_grp_init(void){
    printk(KERN_INFO "mod_sysfs_grp: Initializing the mod_sysfs_grp LKM\n");

    // Register the device class
    mod_sysfs_grp_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mod_sysfs_grp_class)){                 // Check for error and clean up if there is
        printk(KERN_ALERT "mod_sysfs_grp: Failed to register device class\n");
        return PTR_ERR(mod_sysfs_grp_class);          // Correct way to return an error on a pointer
    }
    printk(KERN_INFO "mod_sysfs_grp: device class registered correctly\n");


    // Create Sysfs entry
    // If you pass kernel_kobj to the second argument, it will create the directory under /sys/kernel/
    // If you pass firmware_kobj to the second argument, it will create the directory under /sys/firmware/
    // If you pass fs_kobj to the second argument, it will create the directory under /sys/fs/
    // If you pass NULL to the second argument, it will create the directory under /sys/
    my_kobj = kobject_create_and_add(SYSFS_NAME, kernel_kobj);
    if(!my_kobj)
		return -ENOMEM;


    // To create a sysfs file attribute group. 
    if(sysfs_create_group(my_kobj, &my_attr_group))
        printk(KERN_INFO"mod_sysfs_grp: Cannot create sysfs file.\n");


    printk(KERN_INFO "mod_sysfs_grp: device class created successfully.\n");

	return 0;
}


static void __exit mod_sysfs_grp_exit(void){
    printk(KERN_INFO "mod_sysfs_grp: Removing LKM\n");
    kobject_put(my_kobj);                                      // To dynamically free kobj structure
    sysfs_remove_group(kernel_kobj, &my_attr_group);           // To remove sysfs file group
    class_unregister(mod_sysfs_grp_class);                     // unregister the device class
    class_destroy(mod_sysfs_grp_class);                        // remove the device class
    printk(KERN_INFO "mod_sysfs_grp: LKM removed successfully.\n");
}

//------------------------------------------------------------------------------------------------

// Sysfs ops functions
//=====================
// This fuction will be called when we read the sysfs file
static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    printk(KERN_INFO "mod_sysfs_grp: sysfs show call\n");
    var = -1;

	if(strcmp(attr->attr.name, "one_attr") == 0)
		var = one_attr;
	else if(strcmp(attr->attr.name, "two_attr") == 0)
		var = two_attr;

	return sprintf(buf, "%d\n", var);
}
 
 
// This fuction will be called when we write the sysfsfs file
static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
    printk(KERN_INFO "mod_sysfs_grp: sysfs store call\n");

    sscanf(buf, "%du", &var);
	if(strcmp(attr->attr.name, "one_attr") == 0)
		one_attr = var;
	else if(strcmp(attr->attr.name, "two_attr") == 0)
		two_attr = var;

    return count;
}

//------------------------------------------------------------------------------------------------

module_init(mod_sysfs_grp_init);
module_exit(mod_sysfs_grp_exit);