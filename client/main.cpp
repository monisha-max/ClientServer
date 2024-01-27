//CLIENT CODE #1

//This is a basic client-server chat application.The application allows clients to connect to a server 
//and exchange messages.Each client can send and receive messages from each other as well as the server,and the server 
//broadcasts messages to all connected clients,or any one single client

//Features:
//Multiple clients can connect to the server simultaneously.
//Clients can send messages to the server, and the server broadcasts these messages to all other connected clients.
//Server has two features, to broadcast all connected clients, or any one single client
//Each client's messages are logged into a file named as chat_log.txt

//IMPLEMENTATION
//The client connects to the server using the server's IP address(127.0.0.1) and port number(12345 by default)
//The client enters a chat name,which is sent to the server
//Clients can send messages to the server,and the server broadcasts these messages to all other connected clients
//The client logs all received messages into a file (chat_log.txt)


#include<iostream>
#include<thread>
#include<string>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<fstream>

using namespace std;

bool Initialise() {
    return true;
}
//Function to send messages to the server
void SendMsg(int s,std::ofstream& logFile) {
    //Getting the user's chat name
    cout << "Enter your chat name: ";
    string name;
    getline(cin, name);
    //Sending the client's name to the server
    int bytesent = send(s, name.c_str(), name.length(), 0);
    if (bytesent == -1) {
        cout << "Error sending name to server" << endl;
        close(s);
        return;
    }

    string message;
    while (1) {
        //Getting user input and sending the message to the server
        getline(cin, message);
        string msg = name + ": " + message;
        int bytesent = send(s, msg.c_str(), msg.length(), 0);
        if (bytesent == -1) {
            cout << "Error sending message" << endl;
            break;
        }
        //Log the message into the file
        logFile << name << ": " << message << std::endl;
        //Checking if the user wants to quit
        if (message == "quit") {
            cout << "Stopping the application" << endl;
            break;
        }
    }
    close(s);
}
//Function to receive messages from the server
void ReceiveMsg(int s,std::ofstream& logFile) {
    char buffer[4096];
    int recvlength;
    string msg = "";
    while (1) {
        //Receive messages from the server
        recvlength = recv(s, buffer, sizeof(buffer), 0);
        if (recvlength <= 0) {
            cout << "disconnected from the server" << endl;
            break;
        } else {
            msg = string(buffer, recvlength);
            cout << msg << endl;
            //Log the received message into the file
            logFile << "Server: " << msg << std::endl;
        }
    }
    close(s);
}

int main() {
    if (!Initialise()) {
        cerr << "Failed to initialize.\n";
        return 1;
    }
    //Creation of socket
    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        cout << "invalid socket created" << endl;
        return 1;
    }
    //Setting server details
    int port = 12345;
    string serveraddress = "127.0.0.1";
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));
    std::ofstream logFile("chat_log.txt", std::ios::app);
    //Connect to the server
    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == -1) {
        cout << "not able to connect to server" << endl;
        perror("Error");
        close(s);
        return 1;
    }

    cout << "successfully connected to server" << endl;
    //Start sender and receiver threads
    thread senderThread(SendMsg, s, std::ref(logFile));
    thread receiverThread(ReceiveMsg, s, std::ref(logFile));

    senderThread.join();
    receiverThread.join();
    logFile.close(); 

    return 0;
}