#include "DriverFileOperation.h"

struct data
{
    unsigned char buffer[BUFFER_SIZE];
};

int DriverOpen(struct inode *inode, struct file *file)
{
    // DeviceOpenNumbers++;
    // printk(DEVICE_NAME ": device opened %d times.\n", DeviceOpenNumbers);
    // return 0;

    char *str = "";
    int ret;

    struct data *p = kmalloc(sizeof(struct data), GFP_KERNEL);

    printk(KERN_ALERT "The DriverOpen() function has been called.\n");

    if (p == NULL)
    {
        printk(KERN_ERR "kmalloc - Null");
        return -ENOMEM;
    }

    ret = strlcpy(p->buffer, str, sizeof(p->buffer));
    if (ret > strlen(str))
    {
        printk(KERN_ERR "strlcpy - too long (%d)", ret);
    }

    file->private_data = p;
    return 0;
}

int DriverRelease(struct inode *inode, struct file *file)
{
    // printk(DEVICE_NAME ": close invoked.\n");
    // return 0;

    printk(KERN_ALERT "The DriverRelease() function has been called.\n");
    if (file->private_data)
    {
        kfree(file->private_data);
        file->private_data = NULL;
    }
    return 0;
}

ssize_t DriverRead(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    // int error_count = 0;
    // error_count = copy_to_user(pBuffer, msg, size_of_msg);
    // if(error_count==0)
    //{
    //     printk(DEVICE_NAME ": send %d characters to user.\n", size_of_msg);
    //     return (size_of_msg=0);
    // }
    // else
    //{
    //     printk(DEVICE_NAME ": failed to read from device.\n");
    //     return -EFAULT;
    // }
    struct data *p = filp->private_data;

    printk(KERN_ALERT "The DriverRead() function has been called.\n");

    if (count > BUFFER_SIZE)
    {
        count = BUFFER_SIZE;
    }

    if (copy_to_user(buf, p->buffer, count) != 0)
    {
        return -EFAULT;
    }

    return count;
}

ssize_t DriverWrite(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
    // sprintf(msg, "%s(%d letters)", pBuffer, count);
    // size_of_msg = strlen(msg);
    // printk(DEVICE_NAME " : received %d characters from the user.\n " , count);
    // return 0;

    struct data *p = filp->private_data;

    printk(KERN_ALERT "The DriverWrite() function has been called.\n");
    // printk("Before calling the copy_from_user() function : %p, %s", p->buffer, p->buffer);
    if (copy_from_user(p->buffer, buf, count) != 0)
    {
        return -EFAULT;
    }
    // printk("After calling the copy_from_user() function : %p, %s", p->buffer, p->buffer);
    return count;
}
