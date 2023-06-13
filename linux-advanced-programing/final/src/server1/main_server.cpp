#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "server.h"

using namespace std;

Server *s2 = 0;

//检测对方服务器是否异常
void *start(void *args) {
    Server *s = (Server *)args;
    while (1) {
        if (s->flag2_) {
            if (s->name_ == "A") {
                s2 = new Server("B", "another-server.json");
            } else {
                s2 = new Server("A", "another-server.json");
            }
            break;
        }
        usleep(56);
    }
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        cout << "Usage-----./server1 A or ./server2 B" << endl;
        return 0;
    }

    Server *s1 = new Server(argv[1], "");
    Server *temp = 0;

    unsigned int number;
    unsigned char ch;
    string server_name;

    // 启动检测异常线程
    static pthread_t pid;
    int ret = pthread_create(&pid, NULL, start, s1);

    while (true) {
        if (s2 != 0) {
            cout << "请输入A/B来选择任一服务器";
            cin >> server_name;
            if (s1->name_ == server_name) {
                temp = s1;
            } else if (s2->name_ == server_name) {
                temp = s2;
            }
        } else {
            temp = s1; //切换成当前服务器
        }

        cout << "1:删除一个商品" << endl;
        cout << "2:增加一个商品" << endl;
        cout << "3:修改一个商品" << endl;
        cout << "4:显示所有商品" << endl;
        cout << "请输入操作序号:";
        cin >> ch;       //读取序号
        cout << "\033c"; //清屏

        switch (ch) {
        case '1':
            temp->deleteFood();
            break;
        case '2':
            temp->addFood();
            break;
        case '3':
            temp->updateFood();
            break;
        case '4':
            temp->show();
            break;
        default:
            std::cout << "error input" << std::endl;
        }
        // system("cls");
        temp->saveData();
        cout << "\033c"; //清屏
    }
    return 0;
}