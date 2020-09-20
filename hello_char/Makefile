obj-m+=hello_char.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) user_hello_char.c -o user_hello_char
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm user_hello_char