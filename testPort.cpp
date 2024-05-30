#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

bool isPortOpen(const char* ip, int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return false;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return false;
    }

    close(sockfd);
    return true;
}

int main() {
    const char* ip = "YOUR_PUBLIC_IP";
    int port = 46568;

    if (isPortOpen(ip, port)) {
        std::cout << "Port " << port << " is open on " << ip << std::endl;
    } else {
        std::cout << "Port " << port << " is closed on " << ip << std::endl;
    }

    return 0;
}
