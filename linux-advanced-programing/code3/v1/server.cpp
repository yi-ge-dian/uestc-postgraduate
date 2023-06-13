#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <unistd.h>

#include <iostream>

typedef void (*TCPServer)(int nConnectedSocket, int nListenSocket);

int RunTCPServer(TCPServer ServerFunction, int nPort, int nLengthOfQueueOfListen = 100, const char *strBoundIP = NULL)
{
    // 1. 创建一个 Socket 对象
    int nListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == nListenSocket)
    {
        std::cout << "socket error" << std::endl;
        return -1;
    }
    // 2. 初始化
    sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(sockaddr_in));
    ServerAddress.sin_family = AF_INET;
    // 2.1 静态绑定IP。如果没有，则为本地IP
    if (NULL == strBoundIP)
    {
        ServerAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    // 如果有，则绑定
    else
    {
        if (::inet_pton(AF_INET, strBoundIP, &ServerAddress.sin_addr) != 1)
        {
            std::cout << "inet_pton error" << std::endl;
            ::close(nListenSocket);
            return -1;
        }
    }
    // 2.2 绑定端口
    ServerAddress.sin_port = htons(nPort);

    // 3. 套接字与ip和端口号的绑定
    if (::bind(nListenSocket, (sockaddr *)&ServerAddress, sizeof(sockaddr_in)) == -1)
    {
        std::cout << "bind error" << std::endl;
        ::close(nListenSocket);
        return -1;
    }

    // 4. 监听端口
    if (::listen(nListenSocket, nLengthOfQueueOfListen) == -1)
    {
        std::cout << "listen error" << std::endl;
        ::close(nListenSocket);
        return -1;
    }

    // 5. 接受客户端的申请
    sockaddr_in ClientAddress;
    socklen_t LengthOfClientAddress = sizeof(sockaddr_in);
    int nConnectedSocket = ::accept(nListenSocket, (sockaddr *)&ClientAddress, &LengthOfClientAddress);
    if (-1 == nConnectedSocket)
    {
        std::cout << "accept error" << std::endl;
        ::close(nListenSocket);
        return -1;
    }

    // 6. 函数指针调用 MyServer 函数
    ServerFunction(nConnectedSocket, nListenSocket);

    ::close(nConnectedSocket);
    ::close(nListenSocket);

    return 0;
}

void MyServer(int nConnectedSocket, int nListenSocket)
{
    ::write(nConnectedSocket, "Hello World\n", 13);
}

int main()
{
    RunTCPServer(MyServer, 4000);
    while (true)
    {
        /* code */
    }

    return 0;
}
