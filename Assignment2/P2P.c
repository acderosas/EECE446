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

void sendJoin(int s, int id);

void sendSearch(int s);

void sendPublish(int s);

int sendall(int s, const char *buf, int *len);

int recvall(int s, char *buf, int chunk_size, ssize_t *total_received);

int lookup_and_connect(const char* host, const char* service);




int main(int argc, char* argv[]) {
    int s;
    const char *host;
    const char *port;
    
}


// Function definitions 

void sendJoin(int s, int id) {

}

void sendSearch(int s) {

}

void sendPublish(int s) {
    
}

int sendall(int s, const char *buf, int *len) {
    int total = 0;        // how many bytes we've sent
    int bytesleft = *len; // how many we have left to send
    int n;

    while(total < *len) {
        n = send(s, buf+total, bytesleft, 0);
        if (n == -1) { break; }
        total += n;
        bytesleft -= n;
    }

    *len = total; // return number actually sent here

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 


int recvall(int s, char *buf, int chunk_size, ssize_t *total_received) {
    ssize_t bytes_received;
    ssize_t total = 0;

    while (total < chunk_size) {
        bytes_received = recv(s, buf + total, chunk_size - total, 0);
        if (bytes_received == 0) {
            break; // Connection closed
        }
        if (bytes_received < 0) {
            perror("recv failed");
            return -1; // Error
        }
        total += bytes_received;
    }

    *total_received = total;
    return 0;
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