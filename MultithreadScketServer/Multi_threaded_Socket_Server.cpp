#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define PORT 80

void* handle_connection(void* socket) {
    int sock = *(int*)socket;
    char buffer[3000] = {0};

    long valread = read(sock, buffer, 3000);
    std::string request(buffer);

    size_t method_end = request.find(" ");
    size_t path_end = request.find(" ", method_end + 1);
    std::string method = request.substr(0, method_end);
    std::string path = request.substr(method_end + 1, path_end - method_end - 1);
    
    std::cout << "Method: " << method << "\n";
    std::cout << "Path: " << path << "\n";

    char *response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 70\n\n<html><body><h1>Hello, World!</h1></body></html>";
    write(sock, response, strlen(response));
    close(sock);

    delete (int*)socket;

    return NULL;
}

int main() {

    int server_fd, client_socket, yes = -1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Socket creation and options
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("In socket");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) {
        perror("setsockopt"); // 오류 발생 시 오류 메시지 출력
        exit(1); // 비정상 종료
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    memset(address.sin_zero, '\0', sizeof(address.sin_zero));


    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("In bind");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 10) < 0) {
        perror("In listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port 80" << PORT << std::endl;

    while (true) {
        std::cout << "Waiting for connections ..." << std::endl;

        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("In accept");
            continue;
        }

        pthread_t thread_id;
        int* new_socket = new int;
        *new_socket = client_socket;

        if (pthread_create(&thread_id, NULL, handle_connection, (void*)new_socket) != 0) {
            perror("could not create thread");
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
