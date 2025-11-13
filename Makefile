obj-m += invisibl_cl.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

install:
	sudo insmod invisibl_cl.ko

uninstall:
	sudo rmmod invisibl_cl

test:
	sudo dmesg -c
	sudo insmod invisibl_cl.ko
	dmesg