#include <linux/init.h>             // Macros used to mark up functions e.g., __init __exit
#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel
 
MODULE_LICENSE("GPL v2");                     // The license type -- this affects runtime behavior
MODULE_AUTHOR("Sarvesh");                     // The author -- visible when you use modinfo
MODULE_DESCRIPTION("A simple Linux driver");  // The description -- see modinfo
MODULE_VERSION("1.0");                        // The version of the module
 
static char *name = "world";           // An example LKM argument -- default value is "world". To use other, insmod hello.ko name=someOtherName
module_param(name, charp, S_IRUGO);    // Parameter desc. charp = char ptr, S_IRUGO can be read/not changed
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");  // Parameter description
 

/* LKM initialization function
*  The static keyword restricts the visibility of the function to within this C file. 
*  The __init macro means that for a built-in driver (not a LKM) the function is only used at initialization
*  time and that it can be discarded and its memory freed up after that point.
*  returns 0 if successful
*/
static int __init hello_init(void){
   printk(KERN_INFO "Hello %s. Logging from the LKM!\n", name); 

   return 0;
}
 

/* LKM cleanup function
*  Similar to the initialization function, it is static. The __exit macro notifies that if this
*  code is used for a built-in driver (not a LKM) that this function is not required.
*/
static void __exit hello_exit(void){
   printk(KERN_INFO "Goodbye %s. Logging out from the LKM!\n", name);
}
 

/* A module must use the module_init() module_exit() macros from linux/init.h, which
*  identify the initialization function at insertion time and the cleanup function (as listed above)
*/
module_init(hello_init);
module_exit(hello_exit);