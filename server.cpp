#include "TcpServer.hpp"

int main() {
    TcpServer server(8080);
    server.start();
    return 0;
}