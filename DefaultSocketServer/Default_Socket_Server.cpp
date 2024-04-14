#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstring>
#include <unistd.h> // for close()

int main() {
    int server_fd, new_socket, yes = -1;
    long valread;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    // SO_REUSEADDR 옵션 설정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt"); // 오류 발생 시 오류 메시지 출력
        exit(1); // 비정상 종료
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(80); // Port 80 for HTTP
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port 80" << std::endl;

    while(true) {
        std::cout << "Waiting for connections ..." << std::endl;

        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("In accept");
            exit(EXIT_FAILURE);
        }

        char buffer[3000] = {0};// string parsing needed
        valread = read(new_socket, buffer, 3000);// depending on buffer size
        //std::cout << buffer << std::endl;
        std::string request(buffer);

        //request parsing
        size_t method_end = request.find(" ");
        size_t path_end = request.find(" ", method_end + 1);
        std::string method = request.substr(0, method_end);
        std::string path = request.substr(method_end + 1, path_end - method_end - 1);

        std::cout << "Method: " << method << "\n";
        std::cout << "Path: " << path << "\n";

        // Only handling GET request in this example
        const char *hello = "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!";
        write(new_socket, hello, strlen(hello));
        std::cout << "Hello message sent\n";
        close(new_socket);
    }

    return 0;
}
