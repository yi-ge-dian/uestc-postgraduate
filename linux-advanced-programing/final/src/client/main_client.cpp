#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <iostream>

using namespace std;

#define BUFFER_SIZE 1024 * 1024

int main() {
    cout << "请选择A/B服务器:";
    string name;
    cin >> name;
    int money = 10000;

    // 定义客户端 sockfd
    int sock_cli = socket(AF_INET, SOCK_STREAM, 0);

    // 定义服务器 sockaddr_in
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    if (name == "A") {
        servaddr.sin_port = htons(11111); // 服务器端口
    } else if (name == "B") {
        servaddr.sin_port = htons(22222); // 服务器端口
    }
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 服务器 ip，inet_addr 用于 IPv4 的 IP 转换（十进制转换为二进制）

    // 连接服务器，成功返回0，错误返回-1
    char sendbuf[BUFFER_SIZE];
    char recvbuf[BUFFER_SIZE];
    // 如果连接正常的服务器失败，就去连接备用服务器。
    if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        servaddr.sin_port = htons(61111); // 服务器端口
        if (connect(sock_cli, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
            perror("connect");
            exit(1);
        }
        cout << "访问备用服务器" << endl;
        while (true) {
            size_t res = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); /// 接收
            if (res <= 0) {
                cout << "发送出错" << endl;
                break;
            }

            cout << recvbuf << endl;
            cout << "请输入购买的序号:";
            string buf;
            cin >> buf;
            res = send(sock_cli, buf.c_str(), strlen(buf.c_str()), 0);
            if (res <= 0) {
                cout << "发送出错" << endl;
                break;
            }

            res = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); /// 接收
            if (res <= 0) {
                cout << "发送出错" << endl;
                break;
            }

            string price = recvbuf;

            cout << "购买成功" << endl;
            money -= stof(price);
            cout << "余额：" << money << endl;

            // 接受或者发送完毕后把数组中的数据全部清空（置0）
            memset(sendbuf, 0, sizeof(sendbuf));
            memset(recvbuf, 0, sizeof(recvbuf));
        }
    }

    // 正常访问服务器
    while (true) {
        size_t res = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); /// 接收
        if (res <= 0) {
            cout << "发送出错" << endl;
            break;
        }

        cout << recvbuf << endl;
        cout << "请输入购买的序号:";
        string buf;
        cin >> buf;
        res = send(sock_cli, buf.c_str(), strlen(buf.c_str()), 0);
        if (res <= 0) {
            cout << "发送出错" << endl;
            break;
        }

        res = recv(sock_cli, recvbuf, sizeof(recvbuf), 0); /// 接收
        if (res <= 0) {
            cout << "发送出错" << endl;
            break;
        }

        string price = recvbuf;

        cout << "购买成功" << endl;
        money -= stof(price);
        cout << "余额：" << money << endl;
        // 接受或者发送完毕后把数组中的数据全部清空（置0）
        memset(sendbuf, 0, sizeof(sendbuf));
        memset(recvbuf, 0, sizeof(recvbuf));
    }
    close(sock_cli);
    return 0;
}