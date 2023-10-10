#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

void handleRequest(int clientSocket) {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    // 读取客户端的HTTP请求
    if (read(clientSocket, buffer, sizeof(buffer) - 1) <= 0) {
        std::cerr << "Error: Unable to read client request." << std::endl;
        return;
    }

    // 解析HTTP请求，这里可以根据需求实现更复杂的请求解析逻辑

    // 发送HTTP响应
    std::string response = "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n"
                           "\r\n"
                           "<html><body><h1>Hello, World!</h1></body></html>\r\n";
    write(clientSocket, response.c_str(), response.size());
}

int main() {
    // 创建套接字
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        std::cerr << "Error: Unable to create socket." << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080); // 设置服务器端口
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1) {
        std::cerr << "Error: Unable to bind to port 8080." << std::endl;
        close(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == -1) {
        std::cerr << "Error: Unable to listen on port 8080." << std::endl;
        close(serverSocket);
        return 1;
    }

    std::cout << "Server is listening on port 8080..." << std::endl;

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);

        if (clientSocket == -1) {
            std::cerr << "Error: Unable to accept client connection." << std::endl;
            continue;
        }

        handleRequest(clientSocket);

        close(clientSocket);
    }

    close(serverSocket);

    return 0;
}
