obj-m := hello.o

CFLAGS_hello.o := -std=gnu11 -Wno-declaration-after-statement

# 指定内核源码
KERNEL_DIR := /lib/modules/$(shell uname -r)/build
# 指向当前目录
PWD := $(shell pwd)

all:
	make -C $(KERNEL_DIR) M=$(PWD) modules
clean:
	make -C $(KERNEL_DIR) M=$(PWD) clean

 