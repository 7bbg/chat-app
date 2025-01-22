#include <iostream>
#include <cstring> // For memset
#include <string>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib") // link with Winsock library
#else  // TODO: For Linux/MacOS 
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void send_message(SOCKET sock, const std::string& message) {
    send(sock, message.c_str(), message.length(), 0);
}

int main() {
    WSADATA wsaData;
    SOCKET client_socket;
    sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    std::string username;

    // Initialize Winsock (Windows only)
    #ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cerr << "WSAStartup failed." << std::endl;
            return 1;
        }
    #endif

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); // Set server IP

    // Connect to the server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(client_socket);
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    std::cout << "Enter your username: ";
    std::getline(std::cin, username);
    
    // Send the username to the server
    send_message(client_socket, username);
    std::cout << "Connected to server.\n";

    // Chat loop: send and receive messages
    while (true) {
        std::cout << "Enter message (or type '@username message' for private message): ";
        std::string message;
        std::getline(std::cin, message);

        // Skip empty messages
        if (message.empty()) {
            continue;  
        }

        // Send the message to the server
        send_message(client_socket, message);

        // Wait for the server's response
        int byte_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (byte_received <= 0) {
            std::cerr << "Error receiving message from server." << std::endl;
            break;
        }
        buffer[byte_received] = '\0'; // Null-terminate the received message
        std::cout << "Server: " << buffer << std::endl;

        // Handle server disconnect
        if (byte_received == 0) {
            std::cerr << "Server disconnected." << std::endl;
            break;
        }
    }

    // Close the socket and clean up
    closesocket(client_socket);

    #ifdef _WIN32
        WSACleanup();
    #endif

    return 0;
}
