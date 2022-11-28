// TODO: 
//  - Test server connection
//  - Test client can recieve messages
//  - Test parsing of recieved message
//  - Test processing of recieved message
#include <string>
#include <main.cpp>

using std::string;

// logic being tested
vector<Message> parseResponse(string response);

int main(int argc, char** argv){
    if (!test_parsing_complete_request()) cout << "FAILED: test_parsing_complete_request()" << endl;
    return EXIT_SUCCESS;
}

bool test_parsing_complete_request(){
    Message msg1 = {"",}
    string payload ="{\"data\":\"mydata\",\"dequeueTime\":0,\"enqueueTime\":1669670169119,\"priority\":1}{\"data\":\"mydata\",\"dequeueTime\":0,\"enqueueTime\":1669670169119,\"priority\":1}{\"data\":\"mydata\",\"dequeueTime\":0,\"enqueueTime\":1669670169119,\"priority\":1}{\"data\":\"mydata\",\"dequeueTime\":0,\"enqueueTime\":1669670169119,\"priority\":1}";
    vector<Message> msgs = parseResponse(payload);
    

}

// Logic being tested

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