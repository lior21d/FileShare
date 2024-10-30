#include <iostream>
#include "server.h"
#include "client.h"

int main()
{
    // Initialize the server
    Server server;
    server.start(8080);
}