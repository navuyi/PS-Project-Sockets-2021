#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include	    <limits.h>		/* for OPEN_MAX */
#include 	    <poll.h>
#include 	    <unistd.h>
#include        <sys/types.h>   /* basic system data types */
#include        <sys/socket.h>  /* basic socket definitions */
#include        <sys/time.h>    /* timeval{} for select() */
#include        <time.h>                /* timespec{} for pselect() */
#include        <netinet/in.h>  /* sockaddr_in{} and other Internet defns */
#include        <arpa/inet.h>   /* inet(3) functions */
#include        <errno.h>
#include        <fcntl.h>               /* for nonblocking */
#include        <netdb.h>
#include        <signal.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include	    <limits.h>		/* for OPEN_MAX */
#include 	    <poll.h>
#include 	    <unistd.h>
#include        <ostream>
#include        <iostream>

using namespace std;

#define MAXLINE 2048
#define SA struct sockaddr






int main(){
    int sockfd;
    struct sockaddr_in servaddr;

    int SERVER_PORT = 55555;
    char SERVER_ADDR[] = "127.0.0.1";


    // Create client socket descriptor
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Socket error: %s\n", strerror(errno));
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET, SERVER_ADDR, &servaddr.sin_addr.s_addr) <= 0){
        perror("Addres string to bytes conversion error");
        return 1;
    }


    // Connect to server
    if(connect(sockfd, (SA*) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr,"connect error : %s \n", strerror(errno));
		return 1;
    }

    sleep(5);
    close(sockfd);
}


