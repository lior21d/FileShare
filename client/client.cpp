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
    // Generate AES key
    std::vector<unsigned char> aesKey = crypto.generateAESKey(256);
    sendKey(clientSocket, aesKey);

    // Open the file for reading
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) 
    {
        std::cerr << "Error opening file: " << filePath << std::endl;
        return;
    }

    // Send the file name
    size_t fileNameLength = filePath.size();
    int bytesSent = send(clientSocket, reinterpret_cast<char*>(&fileNameLength), sizeof(fileNameLength), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error sending file name length: " << WSAGetLastError() << std::endl;
        return;
    }

    // Read out the file
    std::string fileData((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    // Encrypt the data
    std::vector<unsigned char> encryptedFileData = crypto.encryptAES(fileData, aesKey); 

    // Send the encrypted file data 
    int bytesSent = send(clientSocket, reinterpret_cast<const char*>(encryptedFileData.data()), encryptedFileData.size(), 0);
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error sending file data: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "File sent successfully!" << std::endl;
}

void Client::receiveKey(SOCKET clientSocket)
{
     // Buffer to store received data
    char buffer[1024];
    int bytesReceived = 0;
    std::string publicKey = "";

    // Receive the public key from the server
    bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesReceived > 0)
    {
        // Store the received key
        publicKey = std::string(buffer, bytesReceived);
        std::cout << "Received public key!" << std::endl;

        // Load the public key into the Crypto object (in PEM format)
        crypto.loadPublicKey(publicKey);
    }
    else if (bytesReceived == SOCKET_ERROR)
    {
        std::cerr << "Error receiving public key: " << WSAGetLastError() << std::endl;
    }
}

void Client::sendKey(SOCKET clientSocket, const std::vector<unsigned char>& aesKey)
{
    // Encrypt the AES key using the server's public RSA key
    std::string aesKeyStr(aesKey.begin(), aesKey.end());
    std::vector<unsigned char> encryptedAESKey = crypto.encryptRSA(aesKeyStr);  // Use RSA to encrypt the AES key

    // Send the encrypted AES key to the server
    int bytesSent = send(clientSocket, reinterpret_cast<const char*>(encryptedAESKey.data()), encryptedAESKey.size(), 0);
    
    if (bytesSent == SOCKET_ERROR) {
        std::cerr << "Error sending encrypted AES key: " << WSAGetLastError() << std::endl;
        return;
    }

    std::cout << "Encrypted AES key sent to server!" << std::endl;
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
    std::cout << "Enter port: " << std::endl;
    std::cin >> port;
    std::cout << "Enter filePath: " << std::endl;
    std::cin >> filePath;


    initialize();
    connectToServer(serverIP, port);
    receiveKey(clientSocket);
    sendFile(filePath);
    cleanup();
}