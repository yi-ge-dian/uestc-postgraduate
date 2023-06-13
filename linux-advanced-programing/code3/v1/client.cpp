#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>

typedef void (* TCPClient)(int nConnectedSocket);

int RunTCPClient(TCPClient ClientFunction, int nServerPort, const char *strServerIP)
{
    // 1. 创建 socket 对象
    int nClientSocket = ::socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == nClientSocket)
    {
	std::cout << "socket error" << std::endl;
	return -1;
    }

    // 2. 初始化，定义一个套接口地址对象，完成结构内字段的赋值
    sockaddr_in ServerAddress;
    // 2.1. 将该地址所占据的内存区域清 0
    memset(&ServerAddress, 0, sizeof(sockaddr_in));

    // 2.2. 协议族
    ServerAddress.sin_family = AF_INET;
    // 2.3. 将点分十进制的 IP 地址转换为二进制格式
    if(::inet_pton(AF_INET, strServerIP, &ServerAddress.sin_addr) != 1)
    {
	std::cout << "inet_pton error" << std::endl;
	::close(nClientSocket);
	return -1;
    }
    // 2.4. 主机字节序和网络字节序的转换
    ServerAddress.sin_port = htons(nServerPort);

    // 3. 建立客户端套接字和服务器端套接字之间的连接
    if(::connect(nClientSocket, (sockaddr*)&ServerAddress, sizeof(ServerAddress)) == -1)
    {
	std::cout << "connect error" << std::endl;
	::close(nClientSocket);
	return -1;
    }
    
    // 4. 函数指针调用 MyClient 函数
    ClientFunction(nClientSocket);
    
    ::close(nClientSocket);
    
    return 0;
}

void MyClient(int nConnectedSocket)
{
    char buf[13];
    
    ::read(nConnectedSocket, buf, 13);

    std::cout << buf << std::endl;
}

int main()
{
    RunTCPClient(MyClient, 4000, "127.0.0.1");

    return 0;
}

