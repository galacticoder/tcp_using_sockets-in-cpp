# Encrypted Chat Server

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

## Installation

### Linux

1. **Install dependencies:**

   ```bash
   sudo apt-get update
   sudo apt-get install libboost-all-dev libcrypto++-dev libfmt-dev g++
