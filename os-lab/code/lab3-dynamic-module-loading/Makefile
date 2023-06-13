obj-m:=os4_module.o
CFLAGS_os4_module.o := -std=gnu11 -Wno-declaration-after-statement

CURRENT_PATH :=$(shell pwd)
VERSION_NUM :=$(shell uname -r)
LINUX_PATH :=/usr/src/linux-headers-$(VERSION_NUM)
all :
	make -C $(LINUX_PATH) M=$(CURRENT_PATH) modules -Wdeclaration-after-statement
	rm *.order *.mod.c *.o *.symvers
clean :
	make -C $(LINUX_PATH) M=$(CURRENT_PATH) clean

 