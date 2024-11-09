#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>
#include "../crypto/crypto.hpp"

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library


class Server
{
public:
    Server() : serverSocket(INVALID_SOCKET) {}
    ~Server() { cleanup(); }

    void receiveFile(SOCKET clientSocket);
    void cleanup();
    void start();

    

private:
    SOCKET serverSocket;
    Crypto crypto;
    

    void initialize();
    void bindAndListen(int port);
    SOCKET acceptConnection();
    void closeSocket(SOCKET& socket);
    void sendPublicKey(SOCKET clientSocket);
    void receiveKey(SOCKET clientSocket, std::string& decryptedAESKey);
    
};




#endif // SERVER_H
