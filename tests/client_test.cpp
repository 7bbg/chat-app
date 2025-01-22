#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <cstdlib>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib") // link with Winsock library
#else  // Todo: For Linux/MacOS
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024

void send_message(SOCKET sock, const std::string& msg) {
    send(sock, msg.c_str(), msg.length(), 0);
}

void receive_message(SOCKET server_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;

    while((bytes_received = recv(server_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the received message
        std::cout << "\nReceived: " << buffer << std::endl;
    }
}

void test_client(const std::string & username) {
    WSADATA wsaData;
    SOCKET client_socket;
    sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Initialize Winsock
    #ifdef _WIN32
        if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
            std::cerr << "WSASartup failed." << std::endl;
        }
    #endif

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(client_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed." << std::endl;
        #ifdef _WIN32
            WSACleanup();
        #endif
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Connect to server
    if(connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Connection failed." << std::endl;
        closesocket(client_socket);

        #ifdef _WIN32
            WSACleanup();
        #endif
        return;
    }

    // Send the username to the server
    send_message(client_socket, username);
    std::cout << username << " connected to server.\n";

    // Start a thread  to receive  messages
    std::thread receiver_thread(receive_message, client_socket);

    // Simulate sending public message 1
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_message(client_socket, "Hello everyone!");

    // Simulate sending a private message
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_message(client_socket, "@testuser2 Hi, private message!");

    // Simulate sending  public message 2
    std::this_thread::sleep_for(std::chrono::seconds(1));
    send_message(client_socket, "This is another public message!");

    // Wait to see responses for 5 secs
    std::this_thread::sleep_for(std::chrono::seconds(5));

    // Close  the client
    closesocket(client_socket);

    #ifdef _WIN32
        WSACleanup();
    #endif
}

int main () {
    // Start multiple test clients
    std::vector<std::thread> clients;

    // Simulate 10 clients
    for (size_t i = 0; i < 10; ++i) {
        clients.push_back(std::thread(test_client, "testuser" + std::to_string(i+1)));
    }

    // Wait for all clients to finish
    for(auto& client: clients) {
        client.join();
    }
    
    return 0;
}