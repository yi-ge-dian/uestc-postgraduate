#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ktime.h>
#include <linux/delay.h>

MODULE_LICENSE("Dual BSD/GPL");

extern int (*fptr_Operation)(int *, int, int, char *);

struct timeval tstart, tend;

static int call_os4_module_init(void)
{
    printk("call_os4_module_init...\n");

    do_gettimeofday(&tstart);
    const int count = 25000000;
    int result;
    int i;
    for (i = 0; i < count; i++)
    {
        char op_add = '+';
        fptr_Operation(&result, 10, 10, &op_add);
    }

    for (i = 0; i < count; i++)
    {
        char op_sub = '-';
        fptr_Operation(&result, 20, 10, &op_sub);
    }

    for (i = 0; i < count; i++)
    {
        char op_mul = '*';
        fptr_Operation(&result, 10, 10, &op_mul);
    }

    for (i = 0; i < count; i++)
    {
        char op_div = '\\';
        fptr_Operation(&result, 20, 10, &op_div);
    }

    do_gettimeofday(&tend);
    printk("total time:%ld usec\n", 1000000 * (tend.tv_sec - tstart.tv_sec) + (tend.tv_usec - tstart.tv_usec) / 1000);
    return 0;
}

static void call_os4_module_exit(void)
{
    printk("call_os4_module_exit...\n");
    return;
}
module_init(call_os4_module_init);
module_exit(call_os4_module_exit);
