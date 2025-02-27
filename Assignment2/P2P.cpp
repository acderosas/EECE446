// EECE 446
// Spring 2025
// Assignment 2
// Jake Baartman
// Andre de Rosas

#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <dirent.h>

// Constants for request types
// Got from PDF
#define JOIN_ACTION 0
#define PUBLISH_ACTION 1
#define SEARCH_ACTION 2

// Structure to represent a P2P peer
typedef struct {
    int sock_fd;
    uint32_t peer_id;
} P2PPeer;

// Function declarations
int lookup_and_connect(const char* host, const char* service);

int connect_to_registry(P2PPeer* peer, const char* host, uint16_t port);

char** get_shared_files(int* num_files);

int send_join_request(P2PPeer* peer);

int send_publish_request(P2PPeer* peer);

int send_search_request(P2PPeer* peer, const char* filename);

int receive_search_response(P2PPeer* peer);

int initialize_peer(P2PPeer* peer, uint32_t id, const char* reg_host, uint16_t reg_port);

void command_loop(P2PPeer* peer);

void cleanup_peer(P2PPeer* peer);

int main(int argc, char* argv[]) {
    // Parse arguments and create peer
    
    return 0;
}

// Function definitions 

// Initialize peer
int initialize_peer(P2PPeer* peer, uint32_t id, const char* reg_host, uint16_t reg_port) {
    return 0;
}

// Connect to registry using lookup_and_connect
int connect_to_registry(P2PPeer* peer, const char* host, uint16_t port) {
    return 0;
}

// Get list of files from SharedFiles directory
char** get_shared_files(int* num_files) {
    return NULL;
}

// Send JOIN request
int send_join_request(P2PPeer* peer) {
    return 0;
}

// Send PUBLISH request
int send_publish_request(P2PPeer* peer) {
    return 0;
}

// Send SEARCH request
int send_search_request(P2PPeer* peer, const char* filename) {
    return 0;
}

// Receive and handle SEARCH response
int receive_search_response(P2PPeer* peer) {
    return 0;
}

// Command loop to handle user interaction
void command_loop(P2PPeer* peer) {
}

// Clean up resources
void cleanup_peer(P2PPeer* peer) {
}

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