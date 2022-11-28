#include <iostream>
#include <vector>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <Message.h>
#include <json.hpp>


using namespace std;
using nlohmann::json;

#define PORT 0  // Connects to first, open port
#define BATCH 1024  // TODO
#define MAXLINE (sizeof(Message) * BATCH)
#define TIMEOUT_USECONDS 100000 // 100 ms

// Requests batch of work from server
// Args: 
//  - sockfd: socket file descriptor
//  - buf: the buffer to write into
//  - servaddr: the servaddr struct for the server to connect to
// Returns:
//  - String: the payload returned from server (unparsed)
string poll(int sockfd, char* buf, const struct sockaddr_in servaddr);

// Parses the response from the server
// Args: 
//  - response: the string response from the server
// Returns:
//  - vector<Message>: the parsed messages in a vector
vector<Message> parseResponse(string response);

// Parses the response from the server
// Args: 
//  - vector<Message> messages: the messages to process (print payload).
// Returns:
//  - bool: true on success, false otherwise.
bool process(vector<Message> messages);

// Accepts server ip address as first arg
int main(int argc, char** argv) {
    if(argc != 2){
        perror("Error: Incorrect number of arguments");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    char buf[MAXLINE];
    struct sockaddr_in servaddr;

    if( (sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0){
        perror("Error: Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    // Clear and set memory
    bzero(&servaddr, sizeof(servaddr));
    in_addr_t ip = inet_addr(argv[1]);
    servaddr.sin_family = AF_INET6;
    servaddr.sin_addr.s_addr = htonl(ip);
    servaddr.sin_port = htons(PORT);

    // connect to the server
    if((connect(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)){
        perror ("Error: Connection failed.");
        exit(EXIT_FAILURE);
    }

    // Event Loop: Poll and Process
    for(;;){
        string response;
        response = poll(sockfd, buf, servaddr);
        vector<Message> messages = parseResponse(response);
        process(messages);
    }

    close(sockfd);
    return EXIT_SUCCESS;
}

string poll(int sockfd, char* buf, const struct sockaddr_in servaddr){
    // set timeout
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = TIMEOUT_USECONDS;

    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting up timeout.");
        exit(EXIT_FAILURE);
    }

    int i;
    socklen_t len;

    // TODO: Try again if error is recoverable
    if(recvfrom(sockfd, (char*) buf, MAXLINE, MSG_WAITALL, (struct sockaddr*) &servaddr, &len) < 0){
        perror ("Timeout Error: Failure to Receive Messages.");
        exit(EXIT_FAILURE);
    }

    buf[i] = '\0';
    return buf;
}

vector<Message> parseResponse(string response){
    json j = json::parse(response);

    // TODO Parse the form server sends and add messages to vector
    int MAGIC_NUMBER = 3;  // Change 

    vector<Message> result;
    for(int i = 0; i < MAGIC_NUMBER; i++){
        Message msg = Message::deserialize("message");
        result.push_back(msg);
    }
    return result;
}

bool process(vector<Message> messages){
    for(int i = 0; i < messages.size(); i++){
        cout << messages[i].data << endl;
    }

    messages.clear();
    return true;
}