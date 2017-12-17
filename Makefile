obj-m+=chardevice.o
 
all:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) modules
	$(CC) testchardevice.c -o testchardevice
clean:
	make -C /lib/modules/$(shell uname -r)/build/ M=$(PWD) clean
	rm testchardevice