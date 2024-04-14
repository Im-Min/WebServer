#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdlib> 
#include <ctime>    
#include <iomanip>  
#include <sstream>


#define PORT 80

void* handle_connection(void* socket);
void updateBackgroundColor(std::string& htmlContent, const std::string& newColor);
std::string readHtmlFile(std::string file_path);
std::string generateRandomColor();

struct SocketData {
    int socket;
    std::string pageData;
};



int main() {

    int server_fd, client_socket, yes = -1;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    
    std::string file_path = "/home/min/WebServer/DynamicContentsServer/mypage.html";
    std::string html_content = readHtmlFile(file_path);
    std::string color;

    
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

    std::cout << "Server started on port" << PORT << std::endl;

    while (true) {
        std::cout << "Waiting for connections ..." << std::endl;

        if ((client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("In accept");
            continue;
        }

        color = generateRandomColor();
        std::string client_html = html_content;
        updateBackgroundColor(client_html, color);

        std::cout << client_html <<"\n";


        SocketData* data = new SocketData;
        data->socket = client_socket;
        data->pageData = client_html;

        pthread_t thread_id;


        if (pthread_create(&thread_id, NULL, handle_connection, data) != 0) {
            perror("could not create thread");
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}


void* handle_connection(void* socket) {
    SocketData* data = static_cast<SocketData*>(socket);
    int sock = data->socket;
    std::string page_html = data->pageData;
    char buffer[3000] = {0};

    long valread = read(sock, buffer, 3000);
    std::string request(buffer);

    size_t method_end = request.find(" ");
    size_t path_end = request.find(" ", method_end + 1);
    std::string method = request.substr(0, method_end);
    std::string path = request.substr(method_end + 1, path_end - method_end - 1);
    
    std::cout << "Method: " << method << "\n";
    std::cout << "Path: " << path << "\n";

    std::string content_length = std::to_string(page_html.length());

    std::string response = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " + content_length + "\n\n" + page_html;

    write(sock, response.c_str(), response.length());
    close(sock);

    delete data;

    return NULL;
}

std::string readHtmlFile(std::string file_path) {

    int size = 0;
    std::ifstream file_stream(file_path);

    if (!file_stream.is_open()) {
        std::cerr << "\nNo such file: " << file_path << std::endl;
        return "";
    } 

    std::string html_contents = std::string(
        (std::istreambuf_iterator<char>(file_stream)),
        std::istreambuf_iterator<char>()
    );

    file_stream.close();

    return html_contents;
}

void updateBackgroundColor(std::string& htmlContent, const std::string& newColor) {
    std::string searchString = "background-color:";
    size_t start = htmlContent.find(searchString);

    if (start != std::string::npos) {
        htmlContent.replace(start + searchString.length(), newColor.length(), newColor);
    }
}

std::string generateRandomColor() {
    std::stringstream color;

    srand(time(0)); 
    
    int red = rand() % 256;
    int green = rand() % 256;
    int blue = rand() % 256;
    
    color << "#" << std::hex << std::setfill('0') << std::setw(2) << red << std::setw(2) << green << std::setw(2) << blue;
    
    return color.str();
}