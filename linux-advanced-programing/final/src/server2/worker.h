#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "server.h"
#include "food.h"
#include "../../others/json/json.h"

// 点菜的线程与客户端交互
void *order(void *args) {
    Server *s = (Server *)args;

    // 创建一个 socket
    int ss = socket(AF_INET, SOCK_STREAM, 0);

    // 解决在close之后会有一个WAIT_TIME，导致bind失败的问题
    int val = 1;
    int ret = setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
    if (ret == -1) {
        perror("setsockopt");
        exit(1);
    }

    // printf("%d\n",ss);

    // 设置 sockaddr_in 结构体中相关参数
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(s->port_);          //将一个无符号短整型数值转换为网络字节序，即大端模式(big-endian)　
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //将主机的无符号长整形数转换成网络字节顺序

    // bind
    if (bind(ss, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // listen
    if (listen(ss, 21) == -1) {
        perror("listen");
        exit(1);
    }

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);
    //成功返回非负描述字，出错返回-1
    int conn;
    while (true) {
        conn = accept(ss, (struct sockaddr *)&client_addr, &length);
        // 如果accpet成功，那么其返回值是由内核自动生成的一个全新描述符，代表与所返回客户的TCP连接。
        // accpet之后就会用新的套接字conn
        if (conn < 0) {
            perror("connect");
            exit(1);
        }

        char buffer[1024 * 1024];
        while (true) {
            memset(buffer, 0, sizeof(buffer));
            string menu;

            for (int i = 0; i < s->foods_.size(); i++) {
                menu += to_string(i) + ":" + s->foods_[i].name + " 价钱:" + to_string(s->foods_[i].price) + " 库存:" + to_string(s->foods_[i].stock) + "\n";
            }

            // 发送菜单给客户端
            size_t res = send(conn, menu.c_str(), strlen(menu.c_str()), 0);
            if (res <= 0) {
                cout << "发送出错" << endl;
                break;
            }

            // 从客户端接收数据
            size_t len = recv(conn, buffer, sizeof(buffer), 0);
            if (len <= 0) {
                cout << "接收错误" << endl;
                break;
            }

            string number = buffer;
            int num = stoi(number);
            s->foods_[num].stock--;

            // 将处理后的数据发送给客户端
            res = send(conn, to_string(s->foods_[num].price).c_str(), strlen(to_string(s->foods_[num].price).c_str()), 0);
            if (res <= 0) {
                cout << "发送出错" << endl;
                break;
            }
        }
        close(conn); // accpet 函数连接成功后还会生成一个新的套接字描述符，结束后也需要关闭
    }
    close(ss); //关闭 socket 套接字描述符
    return 0;
}

// 发送本服务器的菜单信息到对方服务器,防止出现异常
void *sendData(void *args) {
    Server *s = (Server *)args;

    // 创建一个 socket
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    int val = 1;
    int ret = setsockopt(sock_cli, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
    if (ret == -1) {
        printf("setsockopt");
        exit(1);
    }

    // 定义sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(33333);                     //服务器端口
    servaddr.sin_addr.s_addr = inet_addr(s->ip_.c_str()); //服务器ip，inet_addr用于IPv4的IP转换（十进制转换为二进制）

    while (true) {
        if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) > -1) {
            break;
        }
    }

    // cout << "启动发送" << endl;
    while (true) {
        if (s->flag_ == 1) {
            s->flag_ = 0;

            // cout << "发送" << endl;
            size_t res = send(sock_cli, s->data_str_.c_str(), strlen(s->data_str_.c_str()) + 1, 0);
            if (res <= 0) {
                cout << "发送出错" << endl;
                break;
            }
        }
        usleep(56);
    }
    close(sock_cli);
    return 0;
}

// 接收对方服务器的数据
void *recvData(void *args) {
    Server *s = (Server *)args;

    // 创建一个 socket
    int ss = socket(AF_INET, SOCK_STREAM, 0);

    // 解决在close之后会有一个WAIT_TIME，导致bind失败的问题
    int val = 1;
    int ret = setsockopt(ss, SOL_SOCKET, SO_REUSEADDR, (void *)&val, sizeof(int));
    if (ret == -1) {
        perror("setsockopt");
        exit(1);
    }

    // printf("%d\n",ss);

    // 设置 sockaddr_in 结构体中相关参数
    struct sockaddr_in server_sockaddr;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(44444);             //将一个无符号短整型数值转换为网络字节序，即大端模式(big-endian)　
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY); //将主机的无符号长整形数转换成网络字节顺序

    // bind
    if (bind(ss, (struct sockaddr *)&server_sockaddr, sizeof(server_sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    // listen
    if (listen(ss, 21) == -1) {
        perror("listen");
        exit(1);
    }

    struct sockaddr_in client_addr;
    socklen_t length = sizeof(client_addr);

    ///成功返回非负描述字，出错返回-1
    int conn = accept(ss, (struct sockaddr *)&client_addr, &length);
    //如果accpet成功，那么其返回值是由内核自动生成的一个全新描述符，代表与所返回客户的TCP连接。
    // accpet之后就会用新的套接字conn
    if (conn < 0) {
        perror("connect");
        exit(1);
    }

    char buffer[1024 * 1024 * 6];
    // cout << "启动接收" << endl;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int len = recv(conn, buffer, sizeof(buffer), 0);
        if (len <= 0) {
            cout << "对方出现异常" << endl;
            s->flag2_ = 1;
            break;
        }

        s->writeFileFromString("another-server.json", buffer);
        // cout << buffer << endl;
    }

    close(conn); //因为accpet函数连接成功后还会生成一个新的套接字描述符，结束后也需要关闭
    close(ss);   //关闭socket套接字描述符
    return 0;
}
