#include <cstring> 
#include <iostream> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h> 
#include <vector>
#include <algorithm>
#include <thread>
#include <fmt/core.h>

//when another client joins it only sends messages and joinig stuff to server so fix that to send to all current clients

using namespace std;

int main() 
{ 
    //server start socket
    bool socketConnected = false;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0); 
    if (serverSocket < 0) {
        cerr << "ERROR opening socket" << endl;
        return 1;
    }

    // specifying the address 
    sockaddr_in serverAddress; 
    serverAddress.sin_family = AF_INET; 
    serverAddress.sin_port = htons(8080); 
    serverAddress.sin_addr.s_addr = INADDR_ANY; 

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cerr << "ERROR on binding" << endl;
        close(serverSocket);
        return 1;
    }

    listen(serverSocket, 5);
    cout << "Server listening" << "\n";
    //end of servr socket staring

    //client socket start and accept
    int clientSocket;
    vector<int> connectedThreads; // store connected clients threads
    // vector<int> connectedClientsSockets; // store connected clients connected sockets

    //----------------threads---start
    while (true) {
        clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket < 0) {
            cerr << "ERROR on accept" << endl;
            continue;
        }

        // lambda thread start
        std::thread([clientSocket, &connectedThreads]() {
            char user[10] = {0};
            ssize_t userBytes = recv(clientSocket, user, sizeof(user), 0); //username declare
            if (userBytes <= 0) {
                cerr << "username must not be empty" << endl;
                close(clientSocket);
                return;
            }

            connectedThreads.push_back(clientSocket); //add client thread to vector
            string userStr(user);

            cout << fmt::format("'{}' has joined the chat", userStr) << endl;
            const string announceUserMsg = fmt::format("'{}' has joined the chat", userStr);
            for (int users=0;users<sizeof(connectedThreads);) { //send announcement msg to all threads
                send(users, announceUserMsg.c_str(), announceUserMsg.size(), 0);
            }

            bool isConnected = true;
            while (isConnected) {
                char buffer[1024] = {0};
                ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0); 

                if (bytesReceived <= 0 || strcmp(buffer, "quit") == 0) {
                    isConnected = false;

                    auto it = std::find(connectedThreads.begin(), connectedThreads.end(), clientSocket);
                    if (it != connectedThreads.end()) {
                        connectedThreads.erase(it); //delete client from vector that's leaving
                    }

                    const string exitMsg = fmt::format("'{}' has left the chat", userStr);
                    cout << exitMsg << endl;
                    for (int client : connectedThreads) {
                        send(client, exitMsg.c_str(), exitMsg.size(), 0);
                    }
                } else {
                    cout << fmt::format("Message from {}: {}", userStr, buffer) << endl;
                }
            }
            close(clientSocket);
        }).detach(); // run thread by itself
    }
    //----------------threads---end

    cout << "Server killed" << "\n";
    close(serverSocket);

    return 0; 
}
