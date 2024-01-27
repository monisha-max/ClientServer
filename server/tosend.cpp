//SERVER CODE

//This is a basic client-server chat application.The application allows clients to connect to a server 
//and exchange messages.Each client can send and receive messages from each other as well as the server,and the server 
//broadcasts messages to all connected clients,or any one single client

//Features:
//Multiple clients can connect to the server simultaneously.
//Clients can send messages to the server, and the server broadcasts these messages to all other connected clients.
//Server has two features, to broadcast all connected clients, or any one single client
//Each client's messages are logged into a file named as chat_log.txt

//IMPLEMENTATION:
//The server listens for incoming connections on a specified port(12345 by default).
//Upon connection,the server receives the client's name and logs it
//Messages sent by clients are broadcasted to all other connected clients
//The server logs all received messages into a file named as chat_log.txt

#include<iostream>
#include<thread>
#include<vector>
#include<cstring>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<fstream>

//Function to handle communication with a connected client
void interactWithClient(int clientSocket, std::vector<int>& clients, std::ofstream& logFile) {
    std::cout << "client connected" << std::endl;

    char buffer[4096];
    std::string clientName;

    //Receive the client's name
    int nameLength = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (nameLength <= 0) {
        std::cerr << "Error receiving client name" << std::endl;
        close(clientSocket);
        return;
    }

    clientName = std::string(buffer, nameLength);
    std::cout << "Client's name: " << clientName << std::endl;

    while (true) {
        //Receive messages from the client
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        //If no bytes received, client disconnected
        if (bytesReceived <= 0) {
            std::cout << "client disconnected" << std::endl;
            break;
        }

        std::string message(buffer, bytesReceived);
        std::cout << "message from client " << clientName << ": " << message << std::endl;

        //Log the message into the file
        logFile << clientName << ": " << message << std::endl;
        
        //Broadcast the message to all other connected clients
        for (auto otherClient : clients) {
            if (otherClient != clientSocket) {
                send(otherClient, message.c_str(), message.length(), 0);
            }
        }
    }
    
    //Remove the disconnected client from the list
    auto it = std::find(clients.begin(), clients.end(), clientSocket);
    if (it != clients.end()) {
        clients.erase(it);
    }
    close(clientSocket);
}

//Function to broadcast messages to clients
void broadcastMessage(std::vector<int>& clients) {
    std::string message;
    //Get message and target from the server whether he wants to send message to everyone, or just one single client
    while (true) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);

        std::cout << "Select a target:\n";
        std::cout << "1. Broadcast to all clients\n";
        std::cout << "2. Send to client 1\n";
        std::cout << "3. Send to client 2\n";
        std::cout << "Choice: ";

        int choice;
        std::cin >> choice;
        std::cin.ignore();

        switch (choice) {
            case 1:
                for (auto client : clients) {
                    send(client, message.c_str(), message.length(), 0);
                }
                break;
            case 2:
                if (!clients.empty()) {
                    send(clients[0], message.c_str(), message.length(), 0);
                } else {
                    std::cout << "No clients to send to.\n";
                }
                break;
            case 3:
                if (clients.size() >= 1) {
                    send(clients[1], message.c_str(), message.length(), 0);
                } else {
                    std::cout << "Not enough clients to send to.\n";
                }
                break;
            default:
                std::cout << "Invalid choice. Try again.\n";
                break;
        }
    }
}

int main() {
    std::cout << "Server program" << std::endl;

    //Creation of socket
    int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == -1) {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    //Create address structure
    int port = 12345;
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    //Bind
    if (bind(listenSocket, reinterpret_cast<struct sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        std::cerr << "Bind failed" << std::endl;
        close(listenSocket);
        return 1;
    }

    //Listen
    if (listen(listenSocket, SOMAXCONN) == -1) {
        std::cerr << "Listen failed" << std::endl;
        close(listenSocket);
        return 1;
    }

    std::cout << "Server has started listening on port: " << port << std::endl;
    std::ofstream logFile("chat_log.txt", std::ios::app);

    //Starting a separate thread for handling message broadcasting
    std::vector<int> clients;
    std::thread broadcastThread(broadcastMessage, std::ref(clients));

    while (true) {
        //Accept a new client connection
        int clientSocket = accept(listenSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            std::cerr << "Invalid client socket" << std::endl;
        }
        
        // Add the new client to the vector
        clients.push_back(clientSocket);

        // Start a new thread to interact with the connected client
        std::thread t1(interactWithClient, clientSocket, std::ref(clients),std::ref(logFile));
        t1.detach();
    }
    logFile.close();

    close(listenSocket);

    return 0;
}
