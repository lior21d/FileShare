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
    // Receive the encrypted AES key from client
    std::string decryptedAESKey;
    receiveKey(clientSocket, decryptedAESKey); 

    std::vector<unsigned char> aesKeyVector(decryptedAESKey.begin(), decryptedAESKey.end());
    // Receive the file name
    char fileNameBuffer[1024];
    int bytesReceived = recv(clientSocket, fileNameBuffer, sizeof(fileNameBuffer), 0);
    if (bytesReceived > 0)
    {
        std::string fileName(fileNameBuffer, bytesReceived);
        std::cout << "Received file name: " << fileName << std::endl;

        // Receive the encrypted file data
        std::vector<unsigned char> encryptedFileData;
        const size_t bufferSize = 4096;
        unsigned char buffer[bufferSize];
        
        while (true) {
            bytesReceived = recv(clientSocket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);
            if (bytesReceived > 0) {
                encryptedFileData.insert(encryptedFileData.end(), buffer, buffer + bytesReceived);
            } else {
                // If no more data is received, break the loop
                break;
            }
        }

        std::cout << "Received encrypted file data!" << std::endl;

        // Step 5: Decrypt the file data using the decrypted AES key (now in vector form)
        std::string decryptedFileData = crypto.decryptAES(encryptedFileData, aesKeyVector);
        
        // Step 6: Save the decrypted file to disk
        std::ofstream outFile(fileName, std::ios::binary);
        if (outFile.is_open()) {
            outFile.write(reinterpret_cast<const char*>(decryptedFileData.data()), decryptedFileData.size());
            outFile.close();
            std::cout << "Decrypted file saved as: " << fileName << std::endl;
        } else {
            std::cerr << "Error opening file for saving: " << fileName << std::endl;
        }
    }
    else if (bytesReceived == SOCKET_ERROR) {
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

    SOCKET clientSocket = acceptConnection();
    if(clientSocket != INVALID_SOCKET)
    {
        sendPublicKey(clientSocket);
        receiveFile(clientSocket);
        closeSocket(clientSocket);
       
    }
    
}

void Server::closeSocket(SOCKET &socket)
{
        if (socket != INVALID_SOCKET) {
        closesocket(socket);
        socket = INVALID_SOCKET;  
        }
}

void Server::sendPublicKey(SOCKET clientSocket)
{
    // Generate key pair
    crypto.generateRSAKeys();
    

    // Send the public key
    int bytesSent = send(clientSocket, crypto.getPublicKey().c_str(), crypto.getPublicKey().size(), 0);
    if (bytesSent == SOCKET_ERROR)
    {
        std::cerr << "Error sending public key: " << WSAGetLastError() << std::endl;
    }
    else
    {
        std::cout << "Sent public key (" << bytesSent << " bytes)" << std::endl;
        
    }
}


void Server::receiveKey(SOCKET clientSocket, std::string& decryptedAESKey)
{
    // Buffer to store received data
    char buffer[1024];
    int bytesReceived = 0;

    // Receive the encrypted AES key from the client
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0)
    {
        // Store the received encrypted AES key
        std::vector<unsigned char> encryptedAESKey(buffer, buffer + bytesReceived);
        std::cout << "Received encrypted AES key!" << std::endl;

        // Decrypt the AES key using the server's private RSA key
        decryptedAESKey = crypto.decryptRSA(encryptedAESKey); // directly assign the decrypted string
        std::cout << "Decrypted AES key!" << std::endl;
    }
    else if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "Error receiving encrypted AES key: " << WSAGetLastError() << std::endl;
    }
}
