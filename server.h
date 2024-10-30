#ifndef SERVER_H
#define SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib") // Link with Winsock library


class Server
{
public:
    Server() : serverSocket(INVALID_SOCKET) {}
    ~Server() { cleanup(); }

    void initialize();
    void bindAndListen(int port);
    SOCKET acceptConnection();
    void receiveFile(SOCKET clientSocket);
    void cleanup();
    void start(int port);
    void closeSocket(SOCKET& socket);
    

private:
    SOCKET serverSocket;
};




#endif // SERVER_H
