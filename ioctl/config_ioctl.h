#ifndef CONFIG_IOCTL_H
#define CONFIG_IOCTL_H

#include <linux/ioctl.h>


// Config structure which will hold all required data structures, pointer to which will be used to pass data in ioctl.
typedef struct
{
    int madeup_dev_id;
    int some_config;
    int another_config;

} config_dev_t;


/*

_IO		an	ioctl with no parameters
_IOW	an	ioctl with write parameters (copy_from_user)
_IOR	an	ioctl with read parameters (copy_to_user)
_IOWR	an	ioctl with both write and read parameters

The first argument : 8-bit magic number [bits 15:8] – to render the commands unique enough for identifying with some unique typically an ASCII character letter or number. 
Because of the large number of drivers, many drivers share a partial letter with other drivers.

The second argument is a sequence number to distinguish ioctls from each other. 
Its an original command number [bits 7:0] – the actual command number (1, 2, 3, …), defined as per our requirement.

The third argument is the type of the data going into the kernel or coming out of the kernel. 
Size of the command argument [bits 29:16] – computed using sizeof() with the command argument’s type.

*/

#define GET_CONFIG_VAR _IOR('q', 1, config_dev_t *)
#define CLR_CONFIG_VAR _IO('q', 2)
#define SET_CONFIG_VAR _IOW('q', 3, config_dev_t *)

#endif