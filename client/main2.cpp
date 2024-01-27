//CLIENT CODE #2

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

void SendMsg(int s,std::ofstream& logFile) {
    cout << "Enter your chat name: ";
    string name;
    getline(cin, name);

    int bytesent = send(s, name.c_str(), name.length(), 0);
    if (bytesent == -1) {
        cout << "Error sending name to server" << endl;
        close(s);
        return;
    }

    string message;
    while (1) {
        getline(cin, message);
        string msg = name + ": " + message;
        int bytesent = send(s, msg.c_str(), msg.length(), 0);
        if (bytesent == -1) {
            cout << "Error sending message" << endl;
            break;
        }
        logFile << name << ": " << message << std::endl;

        if (message == "quit") {
            cout << "Stopping the application" << endl;
            break;
        }
    }
    close(s);
}

void ReceiveMsg(int s,std::ofstream& logFile) {
    char buffer[4096];
    int recvlength;
    string msg = "";
    while (1) {
        recvlength = recv(s, buffer, sizeof(buffer), 0);
        if (recvlength <= 0) {
            cout << "disconnected from the server" << endl;
            break;
        } else {
            msg = string(buffer, recvlength);
            cout << msg << endl;

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

    int s;
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        cout << "invalid socket created" << endl;
        return 1;
    }

    int port = 12345;
    string serveraddress = "127.0.0.1";
    sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(port);
    inet_pton(AF_INET, serveraddress.c_str(), &(serveraddr.sin_addr));
    std::ofstream logFile("chat_log.txt", std::ios::app);

    if (connect(s, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == -1) {
        cout << "not able to connect to server" << endl;
        perror("Error");
        close(s);
        return 1;
    }

    cout << "successfully connected to server" << endl;
    thread senderThread(SendMsg, s, std::ref(logFile));
    thread receiverThread(ReceiveMsg, s, std::ref(logFile));

    senderThread.join();
    receiverThread.join();
    logFile.close(); 

    return 0;
}