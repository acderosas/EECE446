// EECE 446
// Spring 2025
// Assignment 2
// Jake Baartman
// Andre de Rosas

#define _GNU_SOURCE 

// including every library I found cuz why not
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <iostream>
#include <string>
#include <cstdint>
#include <sys/socket.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <filesystem>
#include <vector>
#include <cstring>

// functions
int lookup_and_connect(const char* host, const char* service);

class P2PPeer {
private:
    int sock_fd;
    uint32_t peer_id;
    
    // got this from the pdf
    // These values must be sent as the first byte of each request
    // to tell the registry what type of action you're performing
    static const uint8_t JOIN_ACTION = 0;
    static const uint8_t PUBLISH_ACTION = 1;
    static const uint8_t SEARCH_ACTION = 2;

    bool connectToRegistry(const std::string& host, uint16_t port);

    std::vector<std::string> getSharedFiles();

    bool sendJoinRequest();

    bool sendPublishRequest();

    bool sendSearchRequest(const std::string& filename);

    bool receiveSearchResponse();

public:
    P2PPeer(uint32_t id);

    ~P2PPeer();

    bool initialize(const std::string& reg_host, uint16_t reg_port);

    void commandLoop();
};

int main(int argc, char* argv[]) {

    // Parse arguments and create peer

    return 0;
}

// Function definitions
int lookup_and_connect(const char* host, const char* service) {
    struct addrinfo hints;
    struct addrinfo *rp, *result;
    int s;

    /* Translate host name into peer's IP address */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = 0;
    hints.ai_protocol = 0;

    if ((s = getaddrinfo(host, service, &hints, &result)) != 0) {
        fprintf(stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror(s));
        return -1;
    }

    /* Iterate through the address list and try to connect */
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if ((s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol)) == -1) {
            continue;
        }

        if (connect(s, rp->ai_addr, rp->ai_addrlen) != -1) {
            break;
        }

        close(s);
    }
    if (rp == NULL) {
        perror("stream-talk-client: connect");
        return -1;
    }
    freeaddrinfo(result);

    return s;
}


// initialize member variables
P2PPeer::P2PPeer(uint32_t id) {
}


// clean stuff up
P2PPeer::~P2PPeer() {
}


// initialize peer and connect to registry
bool P2PPeer::initialize(const std::string& reg_host, uint16_t reg_port) {
    return false;
}


// connect to registry using lookup_and_connect
bool P2PPeer::connectToRegistry(const std::string& host, uint16_t port) {
    return false;
}


// get list of files from SharedFiles directory
std::vector<std::string> P2PPeer::getSharedFiles() {
    return std::vector<std::string>();
}


// send JOIN request
bool P2PPeer::sendJoinRequest() {
    return false;
}


// send PUBLISH request
bool P2PPeer::sendPublishRequest() {
    return false;
}


 // send SEARCH request
bool P2PPeer::sendSearchRequest(const std::string& filename) {
    return false;
}


// receive and handle SEARCH response
bool P2PPeer::receiveSearchResponse() {
    return false;
}


// make command loop to handle user interaction
// send JOIN request
// send PUBLISH request (registry)
// asks user for a filename
// send SEARCH request
// hopefully displays message or data
void P2PPeer::commandLoop() {
}


// maybe another thing idk