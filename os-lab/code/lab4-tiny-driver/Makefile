obj-m := charMsgModule.o
charMsgModule-objs :=charDevDriver.o DriverFileOperation.o
CFLAGS_charMsgModule.o := -std=gnu11 -Wno-declaration-after-statement
ONFIG_MODULE_SIG=n

# 指定内核源码
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
# 指向当前目录
PWD := $(shell pwd)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
	rm *.order *.mod.c *.o *.symvers
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

 
