#include <iostream>
#include <cstring>      // memset, strerror
#include <cerrno>       // errno
#include <unistd.h>     // close
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main() {
    // ----- Phase 1: Setup -----
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket < 0) {
        std::cerr << "socket() error: " << std::strerror(errno) << "\n";
        return 1;
    }

    // Let us reuse the address immediately after exit
    int opt = 1;
    setsockopt(listenSocket,
               SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family      = AF_INET;        // IPv4
    serverAddr.sin_addr.s_addr = INADDR_ANY;     // Listen on all IPs
    serverAddr.sin_port        = htons(4221);    // Port 4221

    if (bind(listenSocket,
             (sockaddr*)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        std::cerr << "bind() error: " << std::strerror(errno) << "\n";
        close(listenSocket);
        return 1;
    }

    if (listen(listenSocket, /*backlog=*/5) < 0) {
        std::cerr << "listen() error: " << std::strerror(errno) << "\n";
        close(listenSocket);
        return 1;
    }

    std::cout << "Server is up on port 4221, waiting for one client...\n";

    // ----- Phase 2: Accept one client -----
    sockaddr_in clientAddr{};
    socklen_t clientLen = sizeof(clientAddr);

    int clientSocket = accept(
        listenSocket,
        (sockaddr*)&clientAddr,
        &clientLen
    );
    if (clientSocket < 0) {
        std::cerr << "accept() error: " << std::strerror(errno) << "\n";
        close(listenSocket);
        return 1;
    }
    std::cout << "Client connected!\n";

    // ----- Phase 3: Send HTTP/1.1 200 OK -----
    const char response[] = "HTTP/1.1 200 OK\r\n\r\n";
    ssize_t bytesSent = send(
        clientSocket,
        response,
        std::strlen(response),
        0
    );
    if (bytesSent < 0) {
        std::cerr << "send() error: " << std::strerror(errno) << "\n";
    }

    // ----- Cleanup -----
    close(clientSocket);
    close(listenSocket);
    return 0;
}
