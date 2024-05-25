#include <iostream>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <algorithm>
#include <fstream>
#include <cstring>
#include <mutex>
#include <fmt/core.h>
//try adding tls encryption thru traffic and add admin mode try to maybe

std::vector<int> connectedClients;
std::mutex clientsMutex;

using namespace std;

void handleClient(int clientSocket) {
    char buffer[1024] = {0};

    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    buffer[bytesReceived] = '\0'; //nullterm
    std::string userStr(buffer);

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        connectedClients.push_back(clientSocket);
    }
    std::string joinMsg = fmt::format("'{}' has joined the chat",userStr);
    std::cout << joinMsg << std::endl;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        for (int client : connectedClients) {
            if(client == clientSocket){
                continue;
            }
            send(client, joinMsg.c_str(), joinMsg.length(), 0);
        }
    }

    bool isConnected = true;

    while (isConnected) {
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

        if (bytesReceived <= 0 || strcmp(buffer, "quit") == 0) {
            isConnected = false;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                auto it = std::remove(connectedClients.begin(), connectedClients.end(), clientSocket);
                connectedClients.erase(it, connectedClients.end());
            }

            std::string exitMsg = fmt::format("'{}' has left the chat",userStr);
            std::cout << exitMsg << std::endl;
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                for (int client : connectedClients) {
                    send(client, exitMsg.c_str(), exitMsg.length(), 0);
                }
            }
        } else {
            buffer[bytesReceived] = '\0';
            std::string message = fmt::format("{}: ",userStr) + buffer;
            // std::string messageTemp = fmt::format("{}: ",userStr);

            if(message.empty()){
                continue;
            } else{
                std::cout << message << std::endl;
            }
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                for (int client : connectedClients) {
                    if (client != clientSocket) {
                        send(client, message.c_str(), message.length(), 0);
                        // send(client, messageTemp.c_str(), messageTemp.length(), 0);
                    }
                }
            }
        }
    }

    close(clientSocket);
}

int main() {
    int PORT=8080;
    //server start socket
    bool socketConnected = false;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (serverSocket < 0) {
        std::cerr << "ERROR opening socket" << std::endl;
        return 1;
    }

    // specifying the address 
    sockaddr_in serverAddress; 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(PORT); 
    serverAddress.sin_addr.s_addr = INADDR_ANY; 

    while (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        PORT++;
    }
    //write port to file
    std::ofstream file("PORT.txt");

    if(file.is_open()){
        int PORTWRITE = PORT;
        file << PORT;
        file.close();
    } else{
        cout << "Warning: cannot write port to file. You may need to configure clients port manually\n";
    }

    listen(serverSocket, 5);
    std::cout << fmt::format("Server listening on port {}", PORT) << "\n";
    //end of servr socket staring

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);

        std::thread(handleClient, clientSocket).detach();
    }

    std::cout << "Server killed\n";
    close(serverSocket);
    return 0;
}
