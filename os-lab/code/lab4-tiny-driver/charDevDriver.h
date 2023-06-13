#pragma once

#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <asm/current.h>

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "charMsgDriver"
#define CLASS_NAME "charMsg"
#define MSG_COUNT 1024

// static int majorNumber;
// static char msg[MSG_COUNT] = {0}; // 用来保存从用户空间传输的数据，用于write函数
// static short size_of_msg;         // 记录用户空间传输的数据大小
// static int DeviceOpenNumbers = 0;
// static struct class *pcharMsgDriverClass = NULL;
// static struct device *pCharMsgDriverDevice = NULL;

// aditional
#define MSG_COUNT 1024
#define BUFFER_SIZE 256
static const unsigned int MINOR_BASE = 0;
static const unsigned int MINOR_NUM = 2;
static unsigned int chardev_major;
static struct cdev chardev_cdev;
static struct class *chardev_class = NULL;
