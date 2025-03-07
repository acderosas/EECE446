// EECE 446
// Spring 2025
// Assignment 1
// Jake Baartman
// Andre de Rosas

/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */

// Using the starter code from lab and for lec for this assignment

#define _GNU_SOURCE //For memmem
#include <string.h> //FOr memmem

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> //To get ssize_t
#include <sys/socket.h> //To get send
#include <netdb.h>
#include <unistd.h> //To get close

#define MAX_CHUNK_SIZE 1000

/*
 * Lookup a host IP address and connect to it using service. Arguments match the first two
 * arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing
 * the returned socket.
 */
int lookup_and_connect( const char *host, const char *service );

//From the book
int sendall(int s, const char *buf, int *len);

//With a sendall comes a recvall
int recvall(int s, char *buf, int chunk_size, ssize_t *total_received);

// For c command line, argc is the number of arguements entered by the user while argv actually goes about indexing those values
int main(int argc, char *argv[]) {
    int s;
    const char *host = "www.ecst.csuchico.edu";
    const char *port = "80";
    const char *request = "GET /~kkredo/file.html HTTP/1.0\r\n\r\n";

    // Command line for getting chunk size
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <chunk_size>\n", argv[0]);
        exit(1);
    }

    int chunk_size = atoi(argv[1]);
    if (chunk_size <= 0 || chunk_size > MAX_CHUNK_SIZE) {
        fprintf(stderr, "Chunk size must be between 1 and %d.\n", MAX_CHUNK_SIZE);
        exit(1);
    }

    // 1) connects to www.ecst.csuchico.edu on port 80 (mostly done above)
    if ((s = lookup_and_connect(host, port)) < 0) {
        perror("Failed to connect to server");
        exit(1);
    }

    // 2) sends "GET /~kkredo/file.html HTTP/1.0\r\n\r\n" to the server
    int request_len = strlen(request);
    if (sendall(s, request, &request_len) == -1) {
        perror("sendall");
        close(s);
        exit(1);
    }

    //? need to malloc?
    char buf[chunk_size];
    ssize_t bytes_received;
    ssize_t total_bytes = 0;
    int h1_count = 0;

    // 3) receives all the data sent by the server (HINT: "orderly shutdown" in recv(2))
    while (recvall(s, buf, chunk_size, &bytes_received) == 0 && bytes_received > 0) {
        total_bytes += bytes_received;

        // Count <h1> tags for given chunk with a pointer to be used for strstr
        void *tag_position = buf;
        //Cannot assume null termination with strstr()
        while ((tag_position = memmem(tag_position, bytes_received - ((char *)tag_position - buf), "<h1>", 4)) != NULL) {
            h1_count++;
            tag_position = (char *)tag_position + 4; // Move past the found tag
        }
    }

    // 4) prints the total number of bytes received
    if (bytes_received == 0) {
        printf("Number of <h1> tags: %d\n", h1_count);
        printf("Total bytes received: %zd\n", total_bytes);
    } else {
        perror("recv");
    }

	close( s );

	return 0;
}


// Same as the starter code
int lookup_and_connect( const char *host, const char *service ) {
	struct addrinfo hints;
	struct addrinfo *rp, *result;
	int s;

	/* Translate host name into peer's IP address */
	memset( &hints, 0, sizeof( hints ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if ( ( s = getaddrinfo( host, service, &hints, &result ) ) != 0 ) {
		fprintf( stderr, "stream-talk-client: getaddrinfo: %s\n", gai_strerror( s ) );
		return -1;
	}

	/* Iterate through the address list and try to connect */
	for ( rp = result; rp != NULL; rp = rp->ai_next ) {
		if ( ( s = socket( rp->ai_family, rp->ai_socktype, rp->ai_protocol ) ) == -1 ) {
			continue;
		}

		if ( connect( s, rp->ai_addr, rp->ai_addrlen ) != -1 ) {
			break;
		}

		close( s );
	}
	if ( rp == NULL ) {
		perror( "stream-talk-client: connect" );
		return -1;
	}
	freeaddrinfo( result );

	return s;
}


int sendall(int s, const char *buf, int *len)
{
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