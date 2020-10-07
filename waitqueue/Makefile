ifeq ($(mode),0)
	obj-m += mod_static_wtq.o
else
	obj-m += mod_dynamic_wtq.o
endif

all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean