#include <cstring> 
#include <iostream> 
#include <netinet/in.h> 
#include <sys/socket.h> 
#include <unistd.h>
#include <stdbool.h>
#include <fmt/core.h>
#include <algorithm>
//make unique id

std::string t_w(std::string strIp);

int main() 
{
    std::string user;
	int clientSocket = socket(AF_INET, SOCK_STREAM, 0); 

	sockaddr_in serverAddress; 
	serverAddress.sin_family = AF_INET; 
	serverAddress.sin_port = htons(8080); 
	serverAddress.sin_addr.s_addr = INADDR_ANY; 

	// connect(clientSocket, (struct sockaddr*)&serverAddress, 
	// 		sizeof(serverAddress)); 

    if(connect(clientSocket, (struct sockaddr*)&serverAddress,sizeof(serverAddress)) < 0){
        const std::string noConnection = "No connection to server exists";
        std::cout << noConnection << "\n"; // add formatted string of ip an dport or server address;
	    close(clientSocket);
        exit(true);
    }

    std::cout << "found connection to seeverr" << "\n";
    std::cout << "Enter a username to go by: ";
    std::getline(std::cin,user);

    user = t_w(user);

    if(user.length()<=0){ // if usrame emtpy
        std::cout << "Username cannot be empty. Disconnecteing from sevrer\n";
	    close(clientSocket);
        exit(true);
    }

    connect(clientSocket, (struct sockaddr*)&serverAddress,sizeof(serverAddress));

    send(clientSocket, user.c_str(), user.length(), 0);

    bool socketIn = true;

    while(socketIn){
    	std::string message;
        std::cout << fmt::format("{}(Me): ", user);
        std::getline(std::cin, message);

        message = t_w(message);

        if(message=="quit"){
            send(clientSocket, message.c_str(), message.length(), 0);
	        // close(clientSocket);
            socketIn = false;
        } else{
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }


	close(clientSocket);
	return 0; 
}

std::string t_w(std::string strIp){
    strIp.erase(strIp.begin(), std::find_if(strIp.begin(), strIp.end(), [](unsigned char ch) {
    return !std::isspace(ch);
    }));

    strIp.erase(std::find_if(strIp.rbegin(), strIp.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), strIp.end());

    return strIp;
}