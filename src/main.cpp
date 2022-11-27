#include <iostream>
#include <Message.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <vector>


using namespace std;

#define PORT 161
#define BATCH 100 // need to change this
#define MAXLINE (sizeof(Message) * BATCH)

int main(int argc, char** argv) {
    int sockfd;
    char buf[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    if( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
        perror("Socket creation failed.");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&servaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if( (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)){
        perror ("bind failed.");
        exit(EXIT_FAILURE);
    }

    // Event Loop: Poll and Process
    while(true){
        string response = 
        response = poll(sockfd, buf, cliaddr);
        vector<Message> messages = parseResponse();
        process(messages);
    }

    close(sockfd);
    return EXIT_SUCCESS;
}

string poll(int sockfd, char* buf, const struct sockaddr_in servaddr){
    // Send a message that we want work;
    int i;
    socklen_t len;
    if(recvfrom(sockfd, (char*) buf, MAXLINE, MSG_WAITALL, (struct sockaddr*) &servaddr, &len) < 0){
        perror ("Message request failed.");
        exit(EXIT_FAILURE);
    }

    buf[i] = '\0';
    return buf;
}

vector<Message> parseResponse(string response){
    vector<Message> result;
    for(int i = 0; i < sizeof(response) / sizeof(Message); i+= sizeof(Message)){
        Message msg = Message::deserialize(response.substr(i, i + sizeof(Message)));
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