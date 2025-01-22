# Chat Application

## Overview
A text-based chat application written in C++ that allows multiple users to communicate over a network. The application consists of a server and multiple clients.

## Features:
- Users can connect to the server with a unique username.
- Clients can send public messages that are broadcast to all other connected users.
- Clients can send private messages to specific users using the `@username` format.

## Requirements:
- C++17 or later
- Winsock library (Windows)

## Compiling and Running the Project:

### For Windows (Using Visual Studio)
- Navigate to the project folder via terminal where server.cpp and client.cpp are located.
`cd src`

- Compile the server and client files using g++
    ```bash
    $ g++ server.cpp -o server -lws2_32
    $ g++ client.cpp -o client -lws2_32
    ```
    `-lws2_32` links the necessary Winsock library.
- Start the Server:

    - Navigate to the folder where server.exe is located.
    `cd build`
    - Run the server
    `./server`

- Start the Clients:
    - Open new terminals for each client you want to connect.
    - Navigate to the folder where `client.exe` is located.
    `cd build`
    - Run the client
    `./client`
    - Each client will prompt for a username, and then it will connect to the server. After successfully connecting, send messages, either publicly to all clients or privately to a specific user by typing a message starting with `@username`.
    

## Communication Protocol
- Public messages: Send any message directly (Example: `Hello everyone!`).
- Private messages: Send a message to a specific user by typing `@username` followed by the message (Example: `@john How are you?`).
