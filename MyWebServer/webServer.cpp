#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <string>
#include <cstring>
#include <sys/epoll.h>
#include <cassert>
#include <arpa/inet.h>
#include <cstdlib>

const int MAX_EVENTS = 10;
const int PORT = 8080;

void handleRequest(int clientSocket) {
    // char buffer[1024];
    // memset(buffer, 0, sizeof(buffer));

    // if(read(clientSocket, buffer, sizeof(buffer) - 1) <= 0) {
    //     std::cerr << "Error: Unable to read client request. " << std::endl;
    //     return;
    // }

    // std::cout << "Recv HTTP Request. " << std::endl;

    // std::cout << "Contents of buffer: " << buffer << std::endl;

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
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    int flag = 1;
    setsockopt(serverSock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));

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

    // 创建epoll实例
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    // 添加监听套接字到epoll实例
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = serverSock;

    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSock, &event) == -1) {
        std::cerr << "Epoll control failed" << std::endl;
        return 1;
    }

    while(true) {
        struct epoll_event events[MAX_EVENTS];
        int numEvents = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if(numEvents == -1) {
            std::cerr << "Epoll wait failed. " << std::endl;
            return 1;
        }

        for(int i = 0; i < numEvents; ++i) {
            int currentFd = events[i].data.fd;

            if(currentFd == serverSock) {
                int clientSock = accept(serverSock, nullptr, nullptr);
                if(clientSock == -1) {
                    std::cerr << "Accept failed. " << std::endl;
                    continue;
                }

                std::cout << "Accepted new connection. " << std::endl;

                event.events = EPOLLIN || EPOLLET;
                event.data.fd = clientSock;
                if(epoll_ctl(epollfd, EPOLL_CTL_ADD, clientSock, &event) == -1) {
                    std::cerr << "Epoll control failed. " << std::endl;
                    return 1;
                }
            }

            else {
                char buffer[1024];
                ssize_t byteRead = recv(currentFd, buffer, sizeof(buffer), 0);
                if(byteRead <= 0) {
                    if(byteRead == 0) {
                        std::cout << "Connection closed by client. " << std::endl;
                    } else {
                        std::cerr << " Recv failed. " << std::endl;
                    }

                    close(currentFd);
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, currentFd, nullptr);
                } else {
                    handleRequest(currentFd);
                    close(currentFd);
                }
            }
        }

    }

    close(serverSock);

    return 0;

}