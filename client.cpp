#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <locale>
#include <thread>
#include <fmt/core.h>

#define GREEN_TEXT "\033[32m"
#define RESET_TEXT "\033[0m"

std::string t_w(std::string strIp) {
    // Trim start
    strIp.erase(strIp.begin(), std::find_if(strIp.begin(), strIp.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    // Trim end
    strIp.erase(std::find_if(strIp.rbegin(), strIp.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), strIp.end());
    return strIp;
}

void receiveMessages(int clientSocket) {
    char buffer[1024];
    // char tmp[1024];
    while (true) {
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
        // ssize_t tmpB = recv(clientSocket, tmp, sizeof(tmp)-1, 0);

        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0'; // term cuz you gonn a have porebelms
            // tmp[tmpB] = '\0';
            std::cout << buffer << std::endl;
            // std::cout << tmp << std::endl;
        }
    }
}

int main() {
    std::ifstream file("PORT.txt");
    std::string PORTSTR;
    std::getline(file, PORTSTR);
    int PORT;
    std::istringstream(PORTSTR) >> PORT;

    std::string user;
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cout << "cant connection to server exists\n";
        close(clientSocket);
        return 1;
    }

    std::cout << "Found connection to server on port " << PORT << "\n";
    std::cout << "Enter a username to go by: ";
    std::getline(std::cin, user);

    user = t_w(user);

    if (user.empty()) {
        std::cout << "Username cannot be empty. Disconnecting from server\n";
        close(clientSocket);
        return 1;
    }

    send(clientSocket, user.c_str(), user.length(), 0);
    // std::cout << fmt::format("You have joined the chat as '{}'",user) << std::endl;

    std::thread receiver(receiveMessages, clientSocket);
    receiver.detach();
    std::cout << GREEN_TEXT << fmt::format("You have joined the chat as '{}'\n",user) << RESET_TEXT;

    std::string message;
    while (true) {
        std::getline(std::cin, message);
        if (message == "quit") {
            break;
        } else if(message.empty()){
            continue;
        }
        send(clientSocket, message.c_str(), message.length(), 0);
    }

    close(clientSocket);
    return 0;
}
