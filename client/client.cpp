#include "client.hpp"


int main() 
{
    Client client;
    client.start();  // Connect to server and send file
    return 0;
}

void Client::initialize()
{
    // Initialize WSA
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2 ), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        exit(1);
    }
}

void Client::connectToServer(const std::string& serverIP, int port)
{
    // Create client socket
    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        cleanup();
        exit(1);
    }

    // Connect to server and set up client adress structure
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, serverIP.c_str(), &serverAddr.sin_addr);

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Connection to server failed: " << WSAGetLastError() << std::endl;
        cleanup();
        exit(1);
    }

    std::cout << "Connected to server at " << serverIP << ":" << port << std::endl;
}

void Client::sendFile(const std::string& filePath)
{
    // Open the file
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile) 
    {
        std::cerr << "Could not open file: " << filePath << std::endl;
        return;
    }

    // Send file name to server
    std::string fileName = filePath.substr(filePath.find_last_of("/\\") + 1);
    send(clientSocket, fileName.c_str(), fileName.size(), 0);

    // Send file in chunks
    std::size_t bufferSize = 1024; // 1KB
    char buffer[bufferSize];

    while (inFile.read(buffer, bufferSize) || inFile.gcount() > 0)
    {
        int bytesToSend = static_cast<int>(inFile.gcount());
        int bytesSent = send(clientSocket, buffer, bytesToSend, 0);

        if (bytesSent == SOCKET_ERROR)
        {
            std::cerr << "Failed to send file data: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    std::cout << "File sent successfully!" << std::endl;
    inFile.close();
}

void Client::cleanup() {
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    WSACleanup();
}

void Client::start() {

    int port;
    std::string serverIP, filePath;
    std::cout << "Enter server IP: " << std::endl;
    std::cin >> serverIP;
    std::cout << "Enter filePath: " << std::endl;
    std::cin >> filePath;
    std::cout << "Enter port: " << std::endl;
    std::cin >> port;
    

    initialize();
    connectToServer(serverIP, port);
    sendFile(filePath);
    cleanup();
}