#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <thread>
#include <fcntl.h>
#include <sys/types.h>
#include <cerrno>
#include <cstring>

#define MAX_EVENTS 10

using namespace std;

vector<string> parse(const char *buffer) {
    string request(buffer);
    stringstream ss(request);
    string method , path;
    ss >> method >> path;
    return {method , path};
}

void sendResponse(int clientSocket , int contentLength , string response) {
    string fullResponse = "HTTP/1.1 200 OK\r\n"
                          "Connection: keep-alive\r\n"
                          "Content-Length: " + to_string(contentLength) + "\r\n\r\n" + response;
    send(clientSocket , fullResponse.c_str() , fullResponse.size() , MSG_NOSIGNAL);
    cout << "Response: " << response << " sent to client!" << endl;
    
}

void handleClient(int clientSocket) {
    cout << "Client connected!" << endl;
    while(true) {
        //What if the request is larger than 1024 bytes?
        //OR what if the internet connection is slow?
        //another question , what will happen to our resources after we open all this client connections without closing?
        char buffer[1024] = {0};
        int bytesRead = recv(clientSocket, buffer , sizeof(buffer) , 0);
        
        if (bytesRead == 0) {
            cout << "Client disconnected!" << endl;
            close(clientSocket);
            return;
        }

        if (bytesRead == -1) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // We read all available data for now, go back to epoll_wait
                return; 
            } else {
                cout << "Error while reading client request! Client disconnected!" << strerror(errno) << endl;
                close(clientSocket);
                return ;
            }
        }

        vector<string> tokens = parse(buffer);
        string method = tokens[0];
        string path = tokens[1];

        if (method == "GET") {

            if (path == "/") {
                int contentLength = 7;
                string responseString = "Helloo\n";
                sendResponse(clientSocket , contentLength , responseString);
                return;
            }
            if (path == "/hello") {
                int contentLength = 12;
                string responseString = "Hello World\n";
                sendResponse(clientSocket , contentLength , responseString);
                return;
            }
            if (path == "/json") {
                int contentLength = 7 ;
                string responseString = "{JSON}\n";
                sendResponse(clientSocket , contentLength , responseString);
                return;
            }
            if (path == "/doesnotexist") {
                int contentLength = 14;
                string responseString = "404 Not Found\n";
                sendResponse(clientSocket , contentLength , responseString);
                return;
            }
        }
    }
}

bool make_non_blocking(int fd) {
    int flags = fcntl(fd , F_GETFL , 0);
    if (flags == -1) {
        return false;
    }
    int isNonBlocking = fcntl(fd , F_SETFL , flags | O_NONBLOCK);
    if (isNonBlocking == -1) {
        return false;
    }
    return true;
}

void epoll(int serverSocket , sockaddr_in &address) {
    int epollFd = epoll_create1(0) ;
    if (epollFd == -1) {
        cout << "Error while creating epoll!" << endl;
        return ;
    }

    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = serverSocket;
    
    int isAdded = epoll_ctl(epollFd , EPOLL_CTL_ADD ,serverSocket  , &event);
    if (isAdded == -1) {
        cout << "Error while adding event to epoll" << endl;
        return;
    }
    
    struct epoll_event events[MAX_EVENTS];

    while (true) {
        int eventS = epoll_wait(epollFd , events, MAX_EVENTS , -1);
        for (int i = 0 ; i < eventS ; i++) {
            if (events[i].data.fd == serverSocket) {
                while(true) {
                    struct sockaddr_in clientAddress;
                    socklen_t clientLen = sizeof(clientAddress);
                    
                    char buffer[1024] = {0};
                
                    int clientSocket = accept(serverSocket , (struct sockaddr*)&clientAddress , &clientLen);
                    if (clientSocket == -1) { 
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            //Waitint room completely empty.
                            break; 
                        } else {
                            std::cerr << "Error on accept: " << std::strerror(errno) << std::endl;  
                            break;                  
                        }                  
                    }

                    if (!make_non_blocking(clientSocket)) {
                        cout << "Error while making client socket non blocking!" << endl;
                        close(clientSocket);
                        continue;
                    }
                    
                    epoll_event clientEvent;
                    clientEvent.events = EPOLLIN | EPOLLET;
                    clientEvent.data.fd = clientSocket;

                    int isClientAdded = epoll_ctl(epollFd, EPOLL_CTL_ADD, clientSocket, &clientEvent);
                    if (isClientAdded == -1) {
                        cout << "Error while adding client to epoll!" << endl;
                        close(clientSocket);
                        continue;
                    }
                    cout << "New client connected and added to epoll to watch with fd: " << clientSocket << endl;
                }
            }
            else {
                int readyClientSocket = events[i].data.fd;
                handleClient(readyClientSocket);
            }
        }
    }
}
int main() {
    //Step 1
    int serverSocket = socket(AF_INET, SOCK_STREAM , 0);
    if (serverSocket == -1) {
        cout << "Error while creating socket!" << endl;
        return 1;
    }

    int option = 1;
    int isOptionSet = setsockopt(serverSocket , SOL_SOCKET , SO_REUSEADDR | SO_REUSEPORT , &option , sizeof(option));
    if (isOptionSet == -1) {
        cout << "Error while setting socket options!" << endl;
        return 1;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    int isBinded = bind(serverSocket , (struct sockaddr*)&address , sizeof(address));
    if (isBinded == -1) {
        cout << "Error while binding socket!" << endl;
        return 1;
    }

    int isLisntened = listen(serverSocket , 10);
    if (isLisntened == -1) {
        cout << "Error while listening the socket sever" << endl;
        return 1;
    }

    if (!make_non_blocking(serverSocket)) {
        cout << "Error while changing server to non blocking" << endl;
        return 1;
    }
    
    epoll(serverSocket, address);

    return 0;
}
