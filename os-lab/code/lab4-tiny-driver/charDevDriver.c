#include "charDevDriver.h"
#include "DriverFileOperation.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("wenlongdong");
MODULE_DESCRIPTION("a simple linux char driver example");

/**
 * linux操作系统中一切都是文件,设备也是文件。
 * linux/fs.h定义file_operations结构体
 * 这里定义文件操作的相关联的回调函数
 * 字符设备的基本功能包括打开,关闭,读取,写入
 */

struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = DriverOpen,
    .release = DriverRelease,
    .read = DriverRead,
    .write = DriverWrite,
};

static int __init charMsgDriverInit(void)
{
    int alloc_ret = 0;
    int cdev_err = 0;
    int minor;
    dev_t dev;

    printk(KERN_INFO "The charMsgDriverInit() function has been called.\n");

    alloc_ret = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUM, DEVICE_NAME);
    if (alloc_ret != 0)
    {
        printk(KERN_ERR "alloc_chrdev_region = %d\n", alloc_ret);
        return -1;
    }
    // Get the major number value in dev.
    chardev_major = MAJOR(dev);
    dev = MKDEV(chardev_major, MINOR_BASE);

    // initialize a cdev structure
    cdev_init(&chardev_cdev, &fops);
    chardev_cdev.owner = THIS_MODULE;

    // add a char device to the system
    cdev_err = cdev_add(&chardev_cdev, dev, MINOR_NUM);
    if (cdev_err != 0)
    {
        printk(KERN_ERR "cdev_add = %d\n", alloc_ret);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    chardev_class = class_create(THIS_MODULE, "chardev");
    if (IS_ERR(chardev_class))
    {
        printk(KERN_ERR "class_create\n");
        cdev_del(&chardev_cdev);
        unregister_chrdev_region(dev, MINOR_NUM);
        return -1;
    }

    for (minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++)
    {
        device_create(chardev_class, NULL, MKDEV(chardev_major, minor), NULL, "chardev%d", minor);
    }

    return 0;
}

static void __exit charMsgDriverExit(void)
{
    int minor;
    dev_t dev = MKDEV(chardev_major, MINOR_BASE);

    printk(KERN_INFO "The charMsgDriverExit() function has been called.\n");

    for (minor = MINOR_BASE; minor < MINOR_BASE + MINOR_NUM; minor++)
    {
        device_destroy(chardev_class, MKDEV(chardev_major, minor));
    }

    class_destroy(chardev_class);
    cdev_del(&chardev_cdev);
    unregister_chrdev_region(dev, MINOR_NUM);
}

module_init(charMsgDriverInit);
module_exit(charMsgDriverExit);
