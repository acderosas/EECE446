/* This code is an updated version of the sample code from "Computer Networks: A Systems
 * Approach," 5th Edition by Larry L. Peterson and Bruce S. Davis. Some code comes from
 * man pages, mostly getaddrinfo(3). */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT "5432" // This must match on client and server
#define BUF_SIZE 256 // This can be smaller. What size?

/*
 * Lookup a host IP address and connect to it using service. Arguments match the first two
 * arguments to getaddrinfo(3).
 *
 * Returns a connected socket descriptor or -1 on error. Caller is responsible for closing
 * the returned socket.
 */
int lookup_and_connect( const char *host, const char *service );

int main( int argc, char *argv[] ) {
	char *host;
	char buf[BUF_SIZE];
	int s;
	int len;
	uint32_t a, b;
	uint32_t answer;

	if ( argc == 2 ) {
		host = argv[1];
	}
	else {
		fprintf( stderr, "usage: %s host\n", argv[0] );
		exit( 1 );
	}

	/* Lookup IP and connect to server */
	if ( ( s = lookup_and_connect( host, SERVER_PORT ) ) < 0 ) {
		exit( 1 );
	}

	while(1) {

		// Get two numbers (a and b) from the user

		// Copy the numbers into a buffer (buf)

		// Send the buffer to the server using the connected socket. Only send the bytes for a and b!

		// Receive the sum from the server into a buffer

		// Copy the sum out of the buffer into a variable (answer)

		// Print the sum

		printf("Enter two numbers:");
		if(scanf("%u, %u", &a, &b) != 2){
			fprintf(stderr, "Didn't enter two ints.\n");
			while (getchar() != "\n");
			continue;
		}

		uint32_t net_a = htonl(a);
		uint32_t net_b = htonl(b);

		memcpy(buf, &net_a, sizeof(net_a));
        memcpy(buf + sizeof(net_a), &net_b, sizeof(net_b));

		if (send(s, buf, BUF_SIZE, 0) != BUF_SIZE) {
			perror("send");
			break;
		}

		/*
		if (send(s, buf, BUF_SIZE, 0) != BUF_SIZE) {
            perror("send");
            break;
        }

        if (recv(s, &answer, sizeof(answer), 0) != sizeof(answer)) {
            perror("recv");
            break;
        }

		answer = ntohl(answer);
		printf("Sum received from server: %u\n", answer);
		*/
	}

	close( s );

	return 0;
}

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
