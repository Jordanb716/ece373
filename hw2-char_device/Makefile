KERNEL_DIR = /lib/modules/$(shell uname -r)/build
PWD:= $(shell pwd)

obj-m += myCharDev.o

default: test
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) modules

test:
	gcc test.c -o test

clean:
	$(MAKE) -C $(KERNEL_DIR) SUBDIRS=$(PWD) clean
	rm test