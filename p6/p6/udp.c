#include "udp.h"
#include <sys/select.h>
#include <sys/time.h>


// create a socket and bind it to a port on the current machine
// used to listen for incoming packets
int UDP_Open(int port) {

    int fd;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
	perror("socket");
	return 0;
    }

    // set up the bind
    struct sockaddr_in myaddr;
    bzero(&myaddr, sizeof(myaddr));

    myaddr.sin_family      = AF_INET;
    myaddr.sin_port        = htons(port);
    myaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(fd, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
	perror("bind");
	close(fd);
	return -1;
    }

    // give back descriptor
    return fd;
}

// fill sockaddr_in struct with proper goodies
int UDP_FillSockAddr(struct sockaddr_in *addr, char *hostName, int port) {
    bzero(addr, sizeof(struct sockaddr_in));
    if (hostName == NULL) {
	return 0; // it's OK just to clear the address
    }
    
    addr->sin_family = AF_INET;          // host byte order
    addr->sin_port   = htons(port);      // short, network byte order

    struct in_addr *inAddr;
    struct hostent *hostEntry;
    if ((hostEntry = gethostbyname(hostName)) == NULL) {
	perror("gethostbyname");
	return -1;
    }
    inAddr = (struct in_addr *) hostEntry->h_addr;
    addr->sin_addr = *inAddr;

    // all is good
    return 0;
}

int UDP_Read(int fd, struct sockaddr_in *addr, char *buffer, int n) {
    int len = sizeof(struct sockaddr_in); 
    int rc = recvfrom(fd, buffer, n, 0, (struct sockaddr *) addr, (socklen_t *) &len);
    // assert(len == sizeof(struct sockaddr_in)); 
    return rc;
}

int UDP_Write(int fd, struct sockaddr_in *addr, char *buffer, int n) {
    int addrLen = sizeof(struct sockaddr_in);
    int rc      = sendto(fd, buffer, n, 0, (struct sockaddr *) addr, addrLen);
    return rc;
}

// udp_read with timeout feature:
// returns TIME_OUT if not able to read something within 5 seconds
int Safe_UDP_Read(int fd, struct sockaddr_in *addr, char *buffer, int n) {

    // parameters needed for select
	fd_set rset;
	FD_ZERO(&rset);
	FD_SET(fd,&rset);

    //initialize timeout
    struct timeval timeout;
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;

	int rv = select(FD_SETSIZE,&rset,NULL,NULL,&timeout);

	if(rv == -1){ //select error
		fprintf(stderr, "select error!");
		exit(-1);
	}else if(rv == 0){//time out
        printf("  TIME_OUT == %d\n", TIME_OUT );
		return TIME_OUT; // read return failure if time out
	}

    int len = sizeof(struct sockaddr_in); 
    int rc = recvfrom(fd, buffer, n, 0, (struct sockaddr *) addr, 
                        (socklen_t *) &len);
    return rc;
}

int UDP_Close(int fd) {
    return close(fd);
}
