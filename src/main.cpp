#include <Message.h>
#include <arpa/inet.h>
#include <netinet/in.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 

#include <iostream>
#include <vector>

using namespace std;

#define PORT 161
#define BATCH 100  // batch size
#define MAXLINE (sizeof(Message) * BATCH)
#define TIMEOUT 100000  // 100 ms

int main(int argc, char** argv) {
  int sockfd;
  char buf[MAXLINE];
  struct sockaddr_in servaddr, cliaddr;

  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("Socket creation failed.");
    exit(EXIT_FAILURE);
  }

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&servaddr, 0, sizeof(cliaddr));

  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(PORT);

  if ((bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)) {
    perror("Bind failed.");
    exit(EXIT_FAILURE);
  }

  // Event Loop: Poll and Process
  while (true) {
    string response = poll(sockfd, buf, cliaddr);
    vector<Message> messages = parseResponse(response);
    process(messages);
  }

  close(sockfd);
  return EXIT_SUCCESS;
}

string poll(int sockfd, char* buf, const struct sockaddr_in servaddr) {
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
  if (recvfrom(sockfd, (char*)buf, MAXLINE, MSG_WAITALL,
               (struct sockaddr*)&servaddr, &len) < 0) {
    perror("Timeout Error: Failure to Receive Messages.");
    exit(EXIT_FAILURE);
  }

  buf[i] = '\0';
  return buf;
}

vector<Message> parseResponse(string response) {
  vector<Message> result;
  for (int i = 0; i < sizeof(response); i += sizeof(Message)) {
    Message msg = Message::deserialize(response.substr(i, i + sizeof(Message)));
    result.push_back(msg);
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
