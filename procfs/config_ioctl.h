#ifndef CONFIG_IOCTL_H
#define CONFIG_IOCTL_H

#include <linux/ioctl.h>

#define READ_CMD _IOR('a', 1, int32_t *)
#define WRITE_CMD _IOW('a', 2, int32_t *)

#endif