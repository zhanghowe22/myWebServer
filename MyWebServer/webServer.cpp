#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <cstring>

void handleRequest(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    if(read(clientSocket, buffer, sizeof(buffer) - 1) <= 0) {
        std::cerr << "Error: Unable to read client request. " << std::endl;
        return;
    }

    std::cout << "Recv HTTP Request. " << std::endl;

    std::cout << "Contents of buffer: " << buffer << std::endl;

    // 发送HTTP响应
    std::string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "\r\n"
                                "<html><body><h1>Hello, World!</h1></body><html>\r\n";
    write(clientSocket, response.c_str(), response.size());
}

int main()
{
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSock == -1) {
        std::cerr << "Error: Unable to create socket. " << std::endl;
        return 1;
    }

    // 设置套接字的类型、IP和端口
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // 將套接字与特定的地址和端口进行绑定
    if(bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cerr << "Error: Unable to bind to port 8080. " << std::endl;
        close(serverSock);
        return 1;
    }

    // 将套接字设置为监听状态，用于接受客户端的连接请求
    if(listen(serverSock, 5) == -1) {
        std::cerr << "Unable to listen on port 8080. " << std::endl;
        close(serverSock);
        return 1;
    }

    std::cout << "Server is listening on port 8080... " << std::endl;

    while(true) {
        sockaddr_in clientAdd;
        socklen_t clientAddLen = sizeof(clientAdd);
        int clientSock = accept(serverSock, (struct sockaddr*)&clientAdd, &clientAddLen);

        if(clientSock == -1) {
            std::cerr << "Error: Unable to accept client connection. " << std::endl;
            continue;
        }

        handleRequest(clientSock);

        close(clientSock);
    }

    close(serverSock);

    return 0;

}