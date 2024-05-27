#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <thread>
#include <fmt/core.h>
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>
#include <cryptopp/filters.h>
#include <netinet/ip_icmp.h>
#include <cryptopp/hex.h>
#include <cryptopp/secblock.h>
#include <netinet/in.h>
#include "encry_to_server.h"
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <arpa/inet.h>

#define GREEN_TEXT "\033[32m"
#define RESET_TEXT "\033[0m"
#define PACKET_SIZE     64
#define PING_TIMEOUT    2   //secs

using namespace std;
using namespace CryptoPP;

std::string t_w(std::string strIp) {
    strIp.erase(strIp.begin(), std::find_if(strIp.begin(), strIp.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    strIp.erase(std::find_if(strIp.rbegin(), strIp.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), strIp.end());
    return strIp;
}

void receiveMessages(int clientSocket) {
    char buffer[1024];
    while (true) {
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer)-1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            std::cout << buffer << std::endl;
        }
    }
}

int main() {
    ifstream file("PORT.txt");
    string PORTSTR;
    getline(file, PORTSTR);
    int PORT;
    istringstream(PORTSTR) >> PORT;

    string user;
    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(PORT);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Cannot connect to server\n";
        close(clientSocket);
        return 1;
    }

    cout << "Found connection to server on port " << PORT << "\n";
    cout << "Enter a username to go by: ";
    getline(cin, user);

    if (user.empty()) {
        cout << "Username cannot be empty. Disconnecting from server\n";
        close(clientSocket);
        return 1;
    }

    send(clientSocket, user.c_str(), user.length(), 0);
    cout << fmt::format("You have joined the chat as '{}'\n", user) << endl;

    SecByteBlock key(AES::DEFAULT_KEYLENGTH);
    SecByteBlock iv(AES::BLOCKSIZE);
    generate_key_iv(key, iv);

    std::string keyHex, ivHex;

    StringSource(key.data(), key.size(), true, new HexEncoder(new StringSink(keyHex)));
    StringSource(iv.data(), iv.size(), true, new HexEncoder(new StringSink(ivHex)));

    thread receiver(receiveMessages, clientSocket);
    receiver.detach();

    // string socketStr = sockaddrToString(serverAddress).c_str();
    // socketStr.append((fmt::format(":{}",PORT))); port is already appended
    // cout << socketStr << endl;

    string message;
    while (true) {
        getline(cin, message);
        if (message == "quit") {
            break;
        } else if (message.empty()) {
            continue;
        }
        message = t_w(message);
        std::string encryptedMessage = aes_encrypt(message, key, iv);

        std::ostringstream oss;
        oss << encryptedMessage.length() << '|' << keyHex.length() << '|' << ivHex.length() << '|'
            << encryptedMessage << keyHex << ivHex;
        std::string formattedMessage = oss.str();
        //need to send key, iv, and message all at once because of data loss
        send(clientSocket, formattedMessage.c_str(), formattedMessage.length(), 0);
    }

    close(clientSocket);
    return 0;
}
