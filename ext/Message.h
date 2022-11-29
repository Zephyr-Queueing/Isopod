#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <chrono>

using std::string;
using std::chrono::milliseconds;

typedef struct Message {
    int priority;
    string data;
    milliseconds enqueueTime;
    milliseconds dequeueTime;

    Message() {};
    Message(int p, const string &d, const milliseconds &en, const milliseconds &de)
        : priority(p), data(d), enqueueTime(en), dequeueTime(de) {};

    string serialize();
    static Message deserialize(const string &msg);
} Message;

#endif // MESSAGE_H
