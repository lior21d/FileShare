#ifndef CLIENT_H
#define CLIENT_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../crypto/crypto.hpp"
#pragma comment(lib, "ws2_32.lib") // Link with Winsock library


class Client
{
    public:
    Client() : clientSocket(INVALID_SOCKET) {}
    ~Client() { cleanup(); }

    void start();
    void cleanup();

    private:
    void initialize();
    void connectToServer(const std::string& serverIP, int port);
    void sendFile(const std::string& filePath);
    void receiveKey(SOCKET clientSocket);

    SOCKET clientSocket;
    std::string publicKey;
    
};

#endif // CLIENT_H