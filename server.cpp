#include "server.hpp"

int main()
{
    // Initialize the server
    Server server;
    server.start();
}

void Server::initialize()
{
    // Initialize WSA
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        exit(1);
    }
}

void Server::bindAndListen(int port)
{
    // Create the socket
    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) 
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        cleanup();
        exit(1);
    }

    // Setting up the server address structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on any available interface
    serverAddr.sin_port = htons(port); // Convert port to network byte order

    // Binding the socket
    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) 
    {
        std::cerr << "Binding failed: " << WSAGetLastError() << std::endl;
        cleanup();
        exit(1);
    }

    // Listening for connections
    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) 
    {
        std::cerr << "Listening failed: " << WSAGetLastError() << std::endl;
        cleanup();
        exit(1);
    }

    std::cout << "Server is listening on port " << port << std::endl;
}

SOCKET Server::acceptConnection()
{
    // Accept connection from client
    sockaddr_in clientAddr;
    int clientAddrSize = sizeof(clientAddr);

    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientAddrSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
        return INVALID_SOCKET;
    }

    std::cout << "Client connected from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;

    return clientSocket;
}

void Server::receiveFile(SOCKET clientSocket)
{
    const std::size_t bufferSize = 1024; // 1 KB
    char buffer[bufferSize]; // Buffer that will recieve the data

    // Recieving file name
    std::size_t recievedBytes = recv(clientSocket, buffer, bufferSize, 0);
    if (recievedBytes > 0)
    {
        std::string fileName(buffer, recievedBytes);
        std::cout << "Receiving file: " << fileName << std::endl;

        std::ofstream outFile(fileName, std::ios::binary);
        if (!outFile)
        {
            std::cerr << "Could not create file: " << fileName << std::endl;
        }
    

        // Reading the file data in chunks
        while ((recievedBytes = recv(clientSocket, buffer, bufferSize, 0)) > 0)
        {
            outFile.write(buffer, recievedBytes);
        }

        if (recievedBytes < 0)
        {
            std::cerr << "Error receiving file data: " << WSAGetLastError() << std::endl;
        }
        
        // Finished
        outFile.close();
        std::cout << "File received successfully!" << std::endl;

    }
    else 
    {
        std::cerr << "Error receiving file name: " << WSAGetLastError() << std::endl; 
    }
    closeSocket(clientSocket);
}


void Server::cleanup()
{
    if (serverSocket != INVALID_SOCKET) 
    {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    closeSocket(serverSocket);
    WSACleanup();
}

void Server::start()
{
    int port;
    std::cout << "Enter port" << std::endl;
    std::cin >> port;
    initialize();
    bindAndListen(port);
    while(true)
    {
        SOCKET clientSocket = acceptConnection();
        if(clientSocket != INVALID_SOCKET)
        {
            receiveFile(clientSocket);
            closeSocket(clientSocket);
        }
    }
}

void Server::closeSocket(SOCKET &socket)
{
        if (socket != INVALID_SOCKET) {
        closesocket(socket);
        socket = INVALID_SOCKET;  
        }
}
