#include <linux/module.h> //每个模块都要包括的头文件
#include <linux/kernel.h> //用到了printk()函数
#include <linux/init.h>
#include <linux/slab.h>

int num = 10;
char *name = "wenlong dong";

// 声明节点结构
typedef struct node
{
    struct node *next; // 指向直接后继元素的指针
    int elem;          // 存储整形元素
} link;

// 创建链表的函数
link *initLink(void)
{
    link *p = (link *)kmalloc(sizeof(link), GFP_KERNEL); // 创建一个头结点
    link *temp = p;                                      // 声明一个指针指向头结点，用于遍历链表
    // 生成链表
    for (int i = 1; i < 5; i++)
    {
        // 创建节点并初始化
        link *a = (link *)kmalloc(sizeof(link), GFP_KERNEL);
        a->elem = i;
        a->next = NULL;
        // 建立新节点与直接前驱节点的逻辑关系
        temp->next = a;
        temp = temp->next;
    }
    return p;
}

// p为原链表，elem表示新数据元素，add表示新元素要插入的位置
link *insertElem(link *p, int elem, int add)
{
    link *temp = p; // 创建临时结点temp
    // 首先找到要插入位置的上一个结点
    for (int i = 1; i < add; i++)
    {
        temp = temp->next;
        if (temp == NULL)
        {
            printk(KERN_WARNING "插入位置无效\n");
            return p;
        }
    }
    // 创建插入结点c
    link *c = (link *)kmalloc(sizeof(link), GFP_KERNEL);
    c->elem = elem;
    // 向链表中插入结点
    c->next = temp->next;
    temp->next = c;
    return p;
}

// p为原链表，add为要删除元素的值
link *delElem(link *p, int add)
{
    link *temp = p;
    // 遍历到被删除结点的上一个结点
    for (int i = 1; i < add; i++)
    {
        temp = temp->next;
        if (temp->next == NULL)
        {
            printk(KERN_WARNING "没有该结点\n");
            return p;
        }
    }
    link *del = temp->next;        // 单独设置一个指针指向被删除结点，以防丢失
    temp->next = temp->next->next; // 删除某个结点的方法就是更改前一个结点的指针域
    kfree(del);                    // 手动释放该结点，防止内存泄漏
    return p;
}

// p为原链表，elem表示被查找元素、
int selectElem(link *p, int elem)
{
    // 新建一个指针t，初始化为头指针 p
    link *t = p;
    int i = 1;
    // 由于头节点的存在，因此while中的判断为t->next
    while (t->next)
    {
        t = t->next;
        if (t->elem == elem)
        {
            return i;
        }
        i++;
    }
    // 程序执行至此处，表示查找失败
    return -1;
}

// 更新函数，其中，add 表示更改结点在链表中的位置，newElem 为新的数据域的值
link *updateElem(link *p, int add, int newElem)
{
    link *temp = p;
    temp = temp->next; // 在遍历之前，temp指向首元结点
    // 遍历到待更新结点
    for (int i = 1; i < add; i++)
    {
        temp = temp->next;
    }
    temp->elem = newElem;
    return p;
}

void display(link *p)
{
    link *temp = p; // 将temp指针重新指向头结点
    // 只要temp指针指向的结点的next不是Null，就执行输出语句。
    while (temp->next)
    {
        temp = temp->next;
        printk(KERN_INFO "%d ", temp->elem);
    }
    printk("\n");
}

// num 和 name 即为内核加载过程中输入的参数
static int hello_init(void) // static使得该文件以外无法访问
{
    // 只能使用内核里定义好的C函数，printk会根据日志级别将指定信息输出到控制台或日志文件中，
    // KERN_ALERT会输出到控制台
    for (int i = 0; i < num; i++)
    {
        // 注意printk和C语言中printf的区别
        printk(KERN_INFO "hello,%s\n", name);
    }

    // 初始化链表（1，2，3，4）
    printk(KERN_INFO "初始化链表为：\n");
    link *p = initLink();
    display(p);
    printk(KERN_INFO "在第4的位置插入元素5:\n");
    p = insertElem(p, 5, 4);
    display(p);
    printk(KERN_INFO "删除元素3:\n");
    p = delElem(p, 3);
    display(p);
    printk(KERN_INFO "查找元素2的位置为:\n");
    int address = selectElem(p, 2);
    if (address == -1)
    {
        printk(KERN_INFO "没有该元素");
    }
    else
    {
        printk(KERN_INFO "元素2的位置为:%d\n", address);
    }
    printk(KERN_INFO "更改第3的位置上的数据为7:\n");
    p = updateElem(p, 3, 7);
    display(p);

    return 0;
}

static void hello_exit(void)
{
    printk(KERN_NOTICE "Goodbye\n");
}

module_init(hello_init);
module_exit(hello_exit);

module_param(num, int, S_IRUGO);
module_param(name, charp, S_IRUGO);

MODULE_LICENSE("GPL"); // 没有指定license会出现error
MODULE_AUTHOR("wenlong dong");
MODULE_DESCRIPTION("Hello Kernel");