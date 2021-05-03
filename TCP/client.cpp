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
#define STDIN 0





int main(){
    int sockfd;
    struct sockaddr_in servaddr;
    int n, max_fd, nready;
    int SERVER_PORT = 55555;
    char SERVER_ADDR[] = "127.0.0.1";

    char recv_buf[MAXLINE];
    char send_buf[MAXLINE];
    fd_set master_set;

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

    max_fd = (sockfd > STDIN) ? sockfd : STDIN;

    // Client program loop
    while(true){
        // Clear the set
        FD_ZERO(&master_set);
        
        // Add connected socked and standard input to the set
        FD_SET(sockfd, &master_set);
        FD_SET(STDIN, &master_set);

        if((nready = select(max_fd+1, &master_set, NULL, NULL , NULL))< 0){
            perror("Select error");
            exit(EXIT_FAILURE);
        }

        // Handle action on connected sockets
        if(FD_ISSET(sockfd, &master_set)){
            if((n = read(sockfd, recv_buf, MAXLINE)) == 0){
                cout<<"Server closed"<<endl;
                close(sockfd);
                exit(1);
            }
            else if(n < 0){
                if(errno == ECONNRESET){
                    // Server shutdown
                    cout<<"Server shutdown"<<endl;
                    // Close socket and end program
                    close(sockfd);
                    exit(1);
                }
                else{
                    perror("Read error");
                    exit(1);
                }
            }
            else{
                recv_buf[n] = 0;     // Get rid of end of line character
                cout<<recv_buf<<endl;
            }
        }
        if(FD_ISSET(STDIN, &master_set)){
            cout<<endl;
            // Read data from standard input to send buffor
            if((n = read(STDIN, send_buf, MAXLINE)) < 0){ 
                perror("Read error");
            }
            send_buf[n] = 0;
            // Send data to the server
            if((send(sockfd, send_buf, MAXLINE, 0)) < 0){ 
                perror("Send error");
            }
        }
    }

    
}


