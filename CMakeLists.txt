cmake_minimum_required(VERSION 3.10)
project(FileTransferApp)

add_subdirectory(server)
add_subdirectory(client)

# cmake -B build -S . ; cmake --build build ; .\build\server\server.exe
# cmake -B build -S . ; cmake --build build ; .\build\client\client.exe