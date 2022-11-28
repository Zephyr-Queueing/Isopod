#include <Message.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <json.hpp>

#include <iostream>
#include <vector>

#define PORT 0
#define MAXLINE 2048
#define TIMEOUT 100000  // 100 ms

using namespace std;
using nlohmann::json;

// Requests batch of work from server
// Args: 
//  - sockfd: socket file descriptor
//  - buf: the buffer to write into
// Returns:
//  - String: the payload returned from server (unparsed)
string poll(int sockfd, char* buf);

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
  struct sockaddr_in servaddr, cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
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
  while (true) {
    string response = poll(sockfd, buf);
    vector<Message> messages = parseResponse(response);
    process(messages);
  }

  close(sockfd);
  return EXIT_SUCCESS;
}

string poll(int sockfd, char* buf) {
  // set timeout
  struct timeval tv;
  tv.tv_sec = 0;
  tv.tv_usec = TIMEOUT;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    perror("Error setting up timeout.");
    exit(EXIT_FAILURE);
  }

  int i;
  socklen_t len;
  string buffer_;

  size_t find_position = buffer_.find('\0');
  while (find_position == string::npos) {
    int num_read;
    if ((num_read = recv(sockfd, (char*) buf, MAXLINE, MSG_WAITALL)) < 0) {
        perror("Timeout Error: Failure to Receive Messages.");
        exit(EXIT_FAILURE);
    }

    if (num_read == 0) {
      break;
    } else if (num_read < 0) {
        perror("Timeout Error: Failure to Receive Messages.");
        exit(EXIT_FAILURE);
    }
    buffer_ += std::string(reinterpret_cast<char*>(buf), num_read);
    find_position =  buffer_.find('\0');
  }

  return buffer_;
}

vector<Message> parseResponse(string response) {
  vector<Message> result;
  
   // "{}{}{}\0"
   int close_bracket = response.find("}");

   while(close_bracket != string::npos){
        Message m = Message::deserialize(response.substr(0, close_bracket + 1));
        result.push_back(m);
        
        response = response.substr(close_bracket + 1);
        int close_bracket = response.find("}");
   }

  return result;
}

bool process(vector<Message> messages) {
  for (int i = 0; i < messages.size(); i++) {
    cout << messages[i].data << endl;
  }
  messages.clear();
  return true;
}
