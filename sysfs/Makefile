ifeq ($(mode),0)
	obj-m += mod_sysfs.o
else
	obj-m += mod_sysfs_grp.o
endif

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean