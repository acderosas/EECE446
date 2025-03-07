// EECE 446
// Spring 2025
// Assignment 2
// Jake Baartman
// Andre de Rosas

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
#include <errno.h>

// Action code definitions
enum P2P_ACTION {
    ACTION_JOIN = 0,
    ACTION_PUBLISH = 1,
    ACTION_SEARCH = 2
};

// sendall function from program 1
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

// recvall function from program 1
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

// Perform JOIN request to registry
int join_request(int socket_fd, uint32_t peer_id) {
    // buffer for the request
    unsigned char packet[5];
    
    // action code (first byte)
    packet[0] = ACTION_JOIN;
    
    // convert peer_id to network byte order
    uint32_t network_id = htonl(peer_id);
    
    // copy the network byte order ID to the packet (bytes 1-4)
    memcpy(packet + 1, &network_id, 4);
    
    // send packet
    int len = 5;
    return sendall(socket_fd, (const char *)packet, &len);
}

// SEARCH request
int search_request(int socket_fd, const char *filename) {
    // calculate full packet size
    size_t filename_len = strlen(filename);
    size_t packet_size = 1 + filename_len + 1; // action + filename + null terminator
    
    // allocate memory for the request
    uint8_t *request_buffer = (uint8_t *)malloc(packet_size);
    if (!request_buffer) {
        fprintf(stderr, "Memory allocation failed for search request\n");
        return -1;
    }
    
    // create request
    request_buffer[0] = ACTION_SEARCH;
    strcpy((char *)request_buffer + 1, filename);
    
    // send the request
    int len = packet_size;
    int send_result = sendall(socket_fd, (const char *)request_buffer, &len);
    free(request_buffer);
    
    if (send_result < 0) {
        return -1;
    }
    
    unsigned char response_buffer[10]; // 4B peer ID + 4B IP + 2B port
    
    // receive response
    ssize_t total_received;
    if (recvall(socket_fd, (char *)response_buffer, sizeof(response_buffer), &total_received) < 0) {
        fprintf(stderr, "Failed to receive SEARCH response\n");
        return -1;
    }
    
    // take values from response
    uint32_t peer_id;
    uint32_t ip_address;
    uint16_t port;
    
    // First 4 bytes: peer ID
    memcpy(&peer_id, response_buffer, 4);
    
    // Next 4 bytes: IP address
    memcpy(&ip_address, response_buffer + 4, 4);
    
    // Last 2 bytes: port
    memcpy(&port, response_buffer + 8, 2);
    
    // convert from network byte order 
    peer_id = ntohl(peer_id);
    port = ntohs(port);
    
    // display results
    if (peer_id > 0) {
        // convert IP address to string
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_address, ip_str, INET_ADDRSTRLEN);
        
        printf("File found at\n");
        printf("\tPeer %u\n", peer_id);
        printf("%s:%u\n", ip_str, port);
    } else {
        printf("File not indexed by registry\n");
    }
    
    return 0;
}

// PUBLISH request
int publish_request(int socket_fd) {
    // open the SharedFiles directory
    DIR *dir = opendir("SharedFiles");
    if (!dir) {
        fprintf(stderr, "Could not open SharedFiles directory: %s\n", strerror(errno));
        return -1;
    }
    
    // calculate total size
    struct dirent *entry;
    uint32_t file_count = 0;
    size_t total_size = 1 + 4; // action byte + file count (4 bytes)
    
    // storage for filenames
    char **filenames = NULL;
    size_t *filename_lengths = NULL;
    
    // count files
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) { // only regular files
            file_count++;
            
            // resize arrays 
            /* 
            reallocates the given area of memory, 
            if ptr is not NULL, it must be previously allocated by malloc, calloc or realloc,
            and not yet freed with a call to free or realloc. Otherwise, the results are undefined.
            takes a ptr and new size of array as arguments
            */
            filenames = realloc(filenames, file_count * sizeof(char *)); 
            filename_lengths = realloc(filename_lengths, file_count * sizeof(size_t));
            
            if (!filenames || !filename_lengths) {
                fprintf(stderr, "Memory allocation failed\n");
                closedir(dir);
                return -1;
            }
            
            // store filename and length
            size_t name_len = strlen(entry->d_name);
            filenames[file_count-1] = strdup(entry->d_name);
            filename_lengths[file_count-1] = name_len;
            
            // update total size (filename + null terminator)
            total_size += name_len + 1;
        }
    }
    
    // check if packet is too large
    if (total_size > 1200) {
        fprintf(stderr, "Publish packet would exceed maximum size (1200 bytes)\n");
        closedir(dir);
        
        // allocated memory
        // need to use 'free' because of remalloc and strdup to prevent memory leaks
        for (uint32_t i = 0; i < file_count; i++) {
            free(filenames[i]);
        }
        free(filenames);
        free(filename_lengths);
        
        return -1;
    }
    
    // create publish packet
    uint8_t *packet = malloc(total_size);
    if (!packet) {
        fprintf(stderr, "Memory allocation failed for publish packet\n");
        closedir(dir);
        
        // need to use 'free' on each filename 
        for (uint32_t i = 0; i < file_count; i++) {
            free(filenames[i]);
        }
        free(filenames);
        free(filename_lengths);
        
        // return -1 if memory allocation fails
        return -1;
    }
    
    // set action code
    packet[0] = ACTION_PUBLISH;
    
    // set file count (network byte order)
    uint32_t network_file_count = htonl(file_count);
    memcpy(packet + 1, &network_file_count, 4);
    
    // add each filename to the packet
    size_t offset = 5; // start after action and count
    for (uint32_t i = 0; i < file_count; i++) {
        // copy filename
        strcpy((char *)packet + offset, filenames[i]);
        offset += filename_lengths[i] + 1; // +1 for null terminator
        
        // free the filename string
        free(filenames[i]);
    }
    
    // need to use 'free' on each filename to prevent memory leak
    free(filenames);
    free(filename_lengths);
    closedir(dir);
    
    // send PUBLISH request
    int len = total_size;
    int result = sendall(socket_fd, (const char *)packet, &len);
    free(packet);
    
    return result;
}

// lookup_and_connect from program 1
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

int main(int argc, char *argv[]) {
    // command line arguments
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <registry_host> <registry_port> <peer_id>\n", argv[0]);
        return 1;
    }
    
    // parse command line arguments
    const char *registry_host = argv[1];
    const char *registry_port = argv[2];
    uint32_t peer_id = strtoul(argv[3], NULL, 10);
    
    // connect to registry
    int socket_fd = lookup_and_connect(registry_host, registry_port);
    if (socket_fd < 0) {
        fprintf(stderr, "Failed to connect to registry at %s:%s\n", registry_host, registry_port);
        return 1;
    }
    
    // command loop
    char command[20];
    printf("Enter a command: ");
    
    while (scanf("%s", command) != EOF) {
        if (strcmp(command, "JOIN") == 0) {
            join_request(socket_fd, peer_id);
        }
        else if (strcmp(command, "SEARCH") == 0) {
            char filename[100];
            printf("Enter a file name: ");
            scanf("%s", filename);
            search_request(socket_fd, filename);
        }
        else if (strcmp(command, "PUBLISH") == 0) {
            publish_request(socket_fd);
        }
        else if (strcmp(command, "EXIT") == 0) {
            break;
        }
        else {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
        
        printf("Enter a command: ");
    }
    
    // close socket and exit
    close(socket_fd);
    return 0;
}