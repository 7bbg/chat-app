#include <iostream>
#include <thread>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <ctime>

#ifdef _WIN32
    #include <winsock2.h>
    #pragma comment(lib, "ws2_32.lib") // link with Winsock library
#else  // Todo: Add the implementation
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <unistd.h>
#endif

#define PORT 8080
#define BUFFER_SIZE 1024

// Structure to hold client info
struct ClientInfo {
    SOCKET socket;
    std::string username;
};

std::unordered_map<std::string, ClientInfo> clients; // Map of usernames to client sockets

std:: string get_current_time() {
    time_t now = time(0);
    tm* ltm = localtime(&now);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", ltm);
    return std::string(buffer);
}


// Function to handle client communication
void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    int bytes_received;
    std::string username;
    std::ofstream log_file("server_log.txt", std::ios::app); // Open Log file in append mode


    // Receive the username from the client
    bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

    if (bytes_received <= 0) {
        std::cerr << "Failed to recieve username." << std::endl;
        closesocket(client_socket);
        return;
    }

    buffer[bytes_received] = '\0'; // Null-terminate the username
    username  = buffer;

    // Check if the username already exists in the clients map
    if (clients.find(username) != clients.end()){
        std::string error_msg = "Username already taken. Please choose a different one.";
        send(client_socket, error_msg.c_str(), error_msg.length(), 0);
        closesocket(client_socket); // Disconnect the client immediately
        return;
    }

    // Add the client to the clients map
    clients[username] = {client_socket, username};

    std::cout << "User " << username << " connected." << std::endl;

    if(!log_file) {
        std::cerr << "Error opening file";
    } else {
        log_file << "Client " << username << " connected at " << get_current_time() << std::endl;
    }

    // Start receiving messages from the client
    while((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)) > 0) {
        buffer[bytes_received] = '\0'; // Null-terminate the message
        std::cout << username << ": " << buffer << std::endl;
        

        // Check for private message
        if (buffer[0] == '@') {
            std::string message(buffer+1);
            size_t pos = message.find(' ');
            std::string recipient = message.substr(0, pos);
            std::string private_message = message.substr(pos+1);
            std::string formatted_message = "[" + get_current_time() + "] " + username + ":" + private_message + "\n";

            // Check if recipient exists
            if(clients.find(recipient) != clients.end()){
                // Send private message to the recipient
                send(clients[recipient].socket, formatted_message.c_str(), formatted_message.length(), 0);
                std::cout << "Private message from "<< username << " to " << recipient << ": " << formatted_message << std::endl;

            } else {
                std::string error_msg = "User " + recipient + " not found!";
                send(client_socket, error_msg.c_str(), error_msg.length(), 0);
            }

        } else {
            std::string message(buffer);
            std::string formatted_message = "[" + get_current_time() + "] " + username + ": " + message + "\n";

            // Broadcast message to all clients
            for(const auto &client : clients) {
                if(client.second.socket != client_socket) {
                    send(client.second.socket, formatted_message.c_str(), formatted_message.length(), 0); // Broadcast the message
                }
            }
        }
    }

    // Clean up when the client disconnects
    if(bytes_received == 0) {
        std::cout << username << " disconnected gracefully." <<std::endl;

        clients.erase(username);  // Remove client from map
        closesocket(client_socket); // Close the connection when done
    }    

    return;
}


int main(){
    WSADATA  wsaData;

    SOCKET server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);

    // Initialize Winsock
    if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
    }

    // Create a socket
    server_socket = socket(AF_INET, SOCK_STREAM,  0);
    if(server_socket == INVALID_SOCKET) {
        std::cerr << "Socket Creation failed." <<std::endl;
        #ifdef _WIN32
        WSACleanup();
        #endif
        return 1;
    }

    // Set up the server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    server_addr.sin_port = htons(PORT); // Port 8080
    
    // Bind the socker to the address
    if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed." << std::endl;
        closesocket(server_socket);

        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }

    // Listen for incoming connections
    if(listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed." << std::endl;
        closesocket(server_socket);

        #ifdef _WIN32
            WSACleanup();
        #endif
        return 1;
    }

    std::cout << "Server listening on port " << PORT << "..." << std::endl;

    // Accept incoming client connections
    while(true) {
        client_socket = accept(server_socket, (struct sockaddr*) &client_addr, &client_addr_size);
        if(client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed." << std::endl;
            continue;
        }

        // Start a new thread to handle this client
        std::thread client_thread(handle_client, client_socket);
        client_thread.detach(); // Detach the thread to handle the client asynchronous
    
    }

    // Cleanup Winsock
    closesocket(server_socket);

    #ifdef _WIN32
        WSACleanup();
    #endif
    
    std::cout << "Winsock Cleaned Up" << std::endl;
    return 0;
}