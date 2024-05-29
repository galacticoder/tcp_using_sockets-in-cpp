# Encrypted Chat Server
### **This script only runs on linux and is only meant for linux.**


This project is an encrypted chat server and client application implemented in C++. The communication between the server and the clients is encrypted using AES (Advanced Encryption Standard). The project consists of the following main files:

- `server.cpp`: Implements the server that handles multiple clients and manages encrypted communication.
- `client.cpp`: Implements the client that connects to the server and communicates using encrypted messages.
- `encrypt_traffic.cpp`: Contains utility functions for encrypting and decrypting traffic.
- `encry_to_server.h`: Header file for encryption and decryption utilities.

## Requirements

- C++17 or later
- Boost Asio
- Crypto++ library
- fmt library

### Installation on linux

1. **Install required packages:**
   ### **Debian based distros:**
   ```bash
   sudo apt-get update
   sudo apt-get install libboost-all-dev libcrypto++-dev libfmt-dev g++
   ```
   ### **Redhat based distros:**
   ```bash
   sudo yum check-update
   sudo yum install boost-devel crypto++-devel fmt-devel gcc-c++
   ```
      ### **On CentOS/RHEL 8 and Fedora:**
      ```bash
      sudo dnf check-update
      sudo dnf install boost-devel crypto++-devel fmt-devel gcc-c++
      ```

3. **Clone this repo:**
   ```bash
   git clone https://github.com/galacticoder/encrypted-chat-server.git
   ```

## Usage:
