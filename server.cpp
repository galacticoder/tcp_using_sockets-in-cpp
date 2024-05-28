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
#include <cryptopp/osrng.h>
#include <cryptopp/hex.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <sstream>
#include <boost/asio.hpp>

//g++ -o server server.cpp -lcryptopp -lfmt

using boost::asio::ip::tcp;

std::vector<int> connectedClients;
std::mutex clientsMutex;

const int TIMEOUT = 30;

using namespace std;
using namespace CryptoPP;

// const char *enter = "";

bool isPav(int port) {
    int sockfd;
    struct sockaddr_in addr;
    bool available = false;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Cannot create socket to test port availability" << std::endl;
        return false;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        available;
    } else {
        available = true;
    }

    close(sockfd);
    return available;
}

void broadcastMessage(const std::string& message, int senderSocket = -1) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (int clientSocket : connectedClients) {
        if (clientSocket != senderSocket) {
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }
}

std::string aes_decrypt(const std::string& ciphertext, const SecByteBlock& key, const SecByteBlock& iv) {
    std::string decrypted;
    CBC_Mode<AES>::Decryption decryption(key, key.size(), iv);
    StringSource(ciphertext, true, new StreamTransformationFilter(decryption, new StringSink(decrypted), StreamTransformationFilter::PKCS_PADDING));
    return decrypted;
}

void handleClient(int clientSocket) {
    char buffer[4096] = {0};

    ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
    buffer[bytesReceived] = '\0';
    std::string userStr(buffer);

    if(userStr.empty()){
        close(clientSocket);
    }
    else{
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            connectedClients.push_back(clientSocket);
        }
        std::string joinMsg = fmt::format("'{}' has joined the chat", userStr);//if empty before then dont meantiuon it
        std::cout << joinMsg << endl;
        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            for (int client : connectedClients) {
                if (client != clientSocket) {
                    send(client, joinMsg.c_str(), joinMsg.length(), 0);
                }
            }
        }

        bool isConnected = true;

        while (isConnected) {
            bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

            if (bytesReceived <= 0 || strcmp(buffer, "quit") == 0) {
                isConnected = false;
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    auto it = std::remove(connectedClients.begin(), connectedClients.end(), clientSocket);
                    connectedClients.erase(it, connectedClients.end());
                    // connectedClients.shrink_to_fit();
                    // if(connectedClients.size() == 0){
                    //     cout << "Server is empty. Shutting down" << endl;
                    //     close(serverSocket);
                    // }
                }

                std::string exitMsg = fmt::format("'{}' has left the chat", userStr);
                std::cout << exitMsg << std::endl;
                {
                    std::lock_guard<std::mutex> lock(clientsMutex);
                    for (int client : connectedClients) {
                        send(client, exitMsg.c_str(), exitMsg.length(), 0);
                    }
                }
            } else {
                buffer[bytesReceived] = '\0';
                std::string receivedData(buffer);

                std::cout << "Received data: " << receivedData << std::endl;

                size_t firstDelim = receivedData.find('|');
                size_t secondDelim = receivedData.find('|', firstDelim + 1);
                size_t thirdDelim = receivedData.find('|', secondDelim + 1);

                if (firstDelim != std::string::npos && secondDelim != std::string::npos && thirdDelim != std::string::npos) {
                    try {
                        int msgLength = std::stoi(receivedData.substr(0, firstDelim));
                        int keyLength = std::stoi(receivedData.substr(firstDelim + 1, secondDelim - firstDelim - 1));
                        int ivLength = std::stoi(receivedData.substr(secondDelim + 1, thirdDelim - secondDelim - 1));

                        std::cout << "Message length: " << msgLength << ", Key length: " << keyLength << ", IV length: " << ivLength << std::endl;

                        if (thirdDelim + 1 + msgLength + keyLength + ivLength <= receivedData.length()) {
                            std::string encryptedMessage = receivedData.substr(thirdDelim + 1, msgLength);
                            std::string receivedKeyHex = receivedData.substr(thirdDelim + 1 + msgLength, keyLength);
                            std::string receivedIvHex = receivedData.substr(thirdDelim + 1 + msgLength + keyLength, ivLength);

                            SecByteBlock decodedKey(AES::DEFAULT_KEYLENGTH);
                            StringSource(receivedKeyHex, true, new HexDecoder(new ArraySink(decodedKey, decodedKey.size())));

                            SecByteBlock decodedIv(AES::BLOCKSIZE);
                            StringSource(receivedIvHex, true, new HexDecoder(new ArraySink(decodedIv, decodedIv.size())));

                            std::string message = fmt::format("{}: ", userStr) + aes_decrypt(encryptedMessage, decodedKey, decodedIv);

                            if (message.empty()) {
                                continue;
                            } else {
                                std::cout << message << std::endl;
                            }
                            {
                                std::lock_guard<std::mutex> lock(clientsMutex);
                                for (int client : connectedClients) {
                                    if (client != clientSocket) {
                                        send(client, message.c_str(), message.length(), 0);
                                    }
                                }
                            }
                            // for (int client : connectedClients) {
                            //     if (client != clientSocket) {
                            //         broadcastMessage(enter);
                            //         broadcastMessage(enter);
                            //     }
                            // }
                        } else {
                            string errorMSG = "Couldnt send message";
                            std::cerr << errorMSG << std::endl;
                            // send(clientSocket,errorMSG.c_str(),sizeof(errorMSG),0);
                        }
                    } catch (const std::exception& e) {
                        std::cerr << "Exception: " << e.what() << std::endl;
                    }
                } else {
                    std::cerr << "Error: Malformed packet received." << std::endl;
                }
            }
        }
        close(clientSocket);
    }

}

int main() {
    //find usable port to host server on
    string local = "127.0.0.0";
    unsigned short PORT = 8080;

    thread t1([&]() {
        if (isPav(PORT) == false) {
            cout << fmt::format("Port {} is not usable searching for port to use..", PORT) << endl;
            for (unsigned short i = 49152; i <= 65535; i++) {
                if (isPav(i) != false) {
                    PORT = i;
                    break;
                }
            }
        }
    });
    t1.join();

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket < 0) {
        std::cerr << "Error opening server socket" << std::endl;
        return 1;
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Chosen port isn't available. Killing server" << endl;
        close(serverSocket);
        exit(true);
    }

    std::ofstream file("PORT.txt");
    if (file.is_open()) {
        file << PORT;
        file.close();
    } else {
        std::cout << "Warning: cannot write port to file. You may need to configure clients port manually\n";
    }

    listen(serverSocket, 5);
    std::cout << fmt::format("Server listening on port {}", PORT) << "\n";

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientLen = sizeof(clientAddress);
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientLen);

        std::thread(handleClient, clientSocket).detach();
    }
    // const string serverEnd = "Server has been shutdown";
    
    close(serverSocket);
    return 0;
}