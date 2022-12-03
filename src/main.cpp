#include <Message.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <thread>
#include <chrono>


#include <iostream>
#include <vector>

#define INTERVAL 500
#define BATCH_SIZE "10"
#define BATCH_DELIM '*'
#define PORT 51711
#define BUF_SIZE 8192
#define TIMEOUT 5000  // 50 ms
#define DEBUGGING true

using namespace std;

struct addrinfo hints;

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


// Resolve DNS Name
// Args:
//  - char* name: Name of the server
//  - unsigned short port: the port #
//  - struct sockaddr_storage *ret_addr
//  -   size_t *ret_addrlen
// Returns:
//  - bool: true on success, false otherwise.
bool LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen);


// Read that handles trying again
// Args: 
//  - int fd: socket descriptor
//  - unsigned char *buf: space to read into
//  - int readlen: num of bytes to read ( <= buflen)
// Returns:
//  - int: num bytes read
int WrappedRead(int fd, unsigned char *buf, int readlen);

// Accepts server ip address as first arg
int main(int argc, char** argv) {
  if (argc != 2) {
    cerr << "argc != 2" << endl;
    exit(EXIT_FAILURE);
  }

  // Get an appropriate sockaddr structure.
  struct sockaddr_storage addr;
  size_t addrlen;
  if (!LookupName(argv[1], PORT, &addr, &addrlen)) {
    cerr << "LookupName() failed" << endl;
    exit(EXIT_FAILURE);
  }
  
  int sockfd;
  if ((sockfd = socket(addr.ss_family, SOCK_DGRAM, 0)) < 0) {
    cerr << "socket() failed:" << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  // connect to the server
  if ((connect(sockfd, reinterpret_cast<sockaddr *>(&addr), addrlen) < 0)) {
    cerr << "connect() failed:" << strerror(errno) << endl;
    exit(EXIT_FAILURE);
  }

  // set timeout
  struct timeval tv;
  tv.tv_sec = 1;
  tv.tv_usec = TIMEOUT;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
    cerr << "setsockopt() failed: timeout error" << endl;
    exit(EXIT_FAILURE);
  }

  // Event Loop: Poll and Process
  char buf[BUF_SIZE];
  while (true) {
    this_thread::sleep_for(chrono::milliseconds(static_cast<int>(INTERVAL)));
    string response = poll(sockfd, buf);
    cout << response << endl;
    vector<Message> messages = parseResponse(response);
    process(messages);
  }

  close(sockfd);
  return EXIT_SUCCESS;
}

string poll(int sockfd, char* buf) {
  // send request packet containing batch size
  unsigned char requestPacket[sizeof(BATCH_SIZE)];
  strcpy((char*) requestPacket, BATCH_SIZE);

  while (true) {
    int rd_val = send(sockfd, requestPacket, sizeof(requestPacket), 0);
    if (rd_val == sizeof(requestPacket)) {
      break;
    } else {
      if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
        continue;
      }
      cerr << "send() failed" << endl;
      exit(EXIT_FAILURE);
    }
  }

  int i;
  socklen_t len;
  string buffer_;

  size_t findPos = buffer_.find(BATCH_DELIM);
  while (findPos == string::npos) {
    int num_read = WrappedRead(sockfd, (unsigned char*) buf, BUF_SIZE);

    if (num_read == 0) {
      break;
    } else if (num_read < 0) {
      if (errno == EAGAIN || errno == EINTR || errno == EWOULDBLOCK) {
        continue;
      }
      cerr << "WrappedRead() failed: " << strerror(errno) << endl;;
      exit(EXIT_FAILURE);

    }
    buffer_.append(string((char*) buf, num_read));
    findPos = buffer_.find(BATCH_DELIM);
  }
  return buffer_.substr(0, findPos);  // read as {.}{.}\0, returned as {.}{.}
}

vector<Message> parseResponse(string response) {
  vector<Message> batch;
  int front = 0;
  for (int i = 0; i < response.size(); i++) {
    if (response[i] == '}') {
      cout << response.substr(front, i + 1 - front) << endl;
      batch.push_back(Message::deserialize(response.substr(front, i + 1 - front)));
      front = i + 1;
    }
  }
  return batch;
}

bool process(vector<Message> messages) {
  for (int i = 0; i < messages.size(); i++) {
    cout << messages[i].data << endl;
  }
  messages.clear();
  return true;
}


bool LookupName(char *name,
                unsigned short port,
                struct sockaddr_storage *ret_addr,
                size_t *ret_addrlen) {
  // struct addrinfo hints, *results;
  struct addrinfo *results;
  int retval;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;

  // Do the lookup by invoking getaddrinfo().
  if ((retval = getaddrinfo(name, nullptr, &hints, &results)) != 0) {
    cerr << "getaddrinfo failed: ";
    cerr << gai_strerror(retval) << std::endl;
    return false;
  }

  // Set the port in the first result.
  if (results->ai_family == AF_INET) {
    struct sockaddr_in *v4addr = (struct sockaddr_in *) results->ai_addr;
    v4addr->sin_port = htons(port);
  } else if (results->ai_family == AF_INET6) {
    struct sockaddr_in6 *v6addr = (struct sockaddr_in6 *) results->ai_addr;
    v6addr->sin6_port = htons(port);
  } else {
    cerr << "getaddrinfo() failed to provide an IPv4 or IPv6 address";
    cerr << std::endl;
    freeaddrinfo(results);
    return false;
  }

  // Return the first result.
  assert(results != nullptr);
  memcpy(ret_addr, results->ai_addr, results->ai_addrlen);
  *ret_addrlen = results->ai_addrlen;

  // Clean up.
  freeaddrinfo(results);
  return true;
}

int WrappedRead(int fd, unsigned char *buf, int readlen) {
  int res;
  while (1) {
    res = read(fd, buf, readlen);
    if (res == -1) {
      if ((errno == EAGAIN) || (errno == EINTR))
        continue;
    }
    break;
  }
  return res;
}