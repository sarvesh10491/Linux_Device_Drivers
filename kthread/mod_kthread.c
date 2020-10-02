#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/kthread.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/sched.h>

#define NUM_KTHREAD 2

MODULE_LICENSE("GPL v2");                             
MODULE_AUTHOR("Sarvesh");                          
MODULE_DESCRIPTION("A simple Linux kthread driver");
MODULE_VERSION("1.0");

static struct task_struct *(ts[NUM_KTHREAD]);
static char ktstr[10];
int err;
int i;
int j;

static int kthread_func(void *arg)
{
/* Every kthread has a struct task_struct associated with it which is it's identifier.
* Whenever a thread is schedule for execution, the kernel sets "current" pointer to it's struct task_struct.
* current->comm is the name of the command that caused creation of this thread
* current->pid is the process of currently executing thread 
*/
    printk(KERN_INFO "This is thread : %s[PID = %d]\n", current->comm, current->pid);
    return 0;
}

//------------------------------------------------------------------------------------------------

static int __init mod_kthread_init(void){
    printk(KERN_INFO "mod_kthread: Initializing the mod_kthread LKM\n");
    
    for(i=0; i<NUM_KTHREAD; i++){
        printk(KERN_INFO "Starting Thread %d\n",i);
        snprintf(ktstr, 10, "Thread-%d", i);

        ts[i] = kthread_run(kthread_func, NULL, ktstr);
        if (IS_ERR(ts[i])) {
            printk(KERN_INFO "ERROR: Cannot create thread %d\n",i);
            err = PTR_ERR(ts[i]);
            ts[i] = NULL;
            return err;
        }
    }

    printk(KERN_INFO "mod_kthread: LKM created correctly\n");

	return 0;
}


static void __exit mod_kthread_exit(void){
    for(j=0; j<NUM_KTHREAD; j++){
        printk(KERN_INFO "Stopping Thread %d\n",j);
        kthread_stop(ts[j]);
    }

    printk(KERN_INFO "mod_kthread: Goodbye from the LKM!\n");
}

//------------------------------------------------------------------------------------------------

module_init(mod_kthread_init);
module_exit(mod_kthread_exit);