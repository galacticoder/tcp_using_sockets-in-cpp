#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

bool isPortAvailable(const char* ip, int port) {
    int sockfd;
    struct sockaddr_in addr;
    bool available = false;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error opening socket." << std::endl;
        return false;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        if (errno == EADDRINUSE) {
            std::cerr << "Port " << port << " is already in use." << std::endl;
        } else {
            std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
        }
    } else {
        std::cout << "Port " << port << " is available." << std::endl;
        available = true;
    }

    close(sockfd);
    return available;
}

int main(int argc, char *argv[]) {
    const char *ip = "24.236.68.122";
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    int port = std::stoi(argv[1]);
    if (port < 1 || port > 65535) {
        std::cerr << "Invalid port number. Port must be between 1 and 65535." << std::endl;
        return 1;
    }

    if (isPortAvailable(ip,port)) {
        std::cout << "You can use port " << port << " to host your server." << std::endl;
    } else {
        std::cerr << "You cannot use port " << port << " to host your server." << std::endl;
    }

    return 0;
}
