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
#define MAXNICKLEN 1024


int main(int argc, char *argv[]){
    int sockfd;
    struct sockaddr_in servaddr;
    int n, max_fd, nready;
    int SERVER_PORT = 55555;
    char SERVER_ADDR[INET_ADDRSTRLEN];

    char recv_buf[MAXLINE];
    char send_buf[MAXLINE];
    fd_set master_set;

    char nickname[MAXNICKLEN];
    char msg[MAXLINE + MAXNICKLEN];

    // Check for input parameters
    if(argc<3){
        cout<<"Not enough arguments"<<endl;
        return 1;
    }
    if(argc>3){
        cout<<"Too many arguments"<<endl;
        return 1;
    }
    
    // Get the nickname string
    strcpy(nickname, argv[2]);
    cout<<nickname<<endl;
    
    
    

    // Create client socket descriptor
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr, "Socket error: %s\n", strerror(errno));
        return 1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    if(inet_pton(AF_INET, argv[1], &servaddr.sin_addr.s_addr) <= 0){
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
        // Clear buffers
        for(int i=0; i<MAXLINE; i++){
            recv_buf[i] = 0;
            send_buf[i] = 0;
        }

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
            // Clear msg buffer
            for(int i=0; i<sizeof(msg); i++){
                msg[i] = 0;
            }
            
            // Print empty line
            cout<<endl;

            // Read data from standard input to send buffer
            if((n = read(STDIN, send_buf, MAXLINE)) < 0){ 
                perror("Read error");
            }
            // Create the message
            strcat(msg, nickname);
            strcat(msg, ": ");
            strcat(msg, send_buf);

            send_buf[n] = 0;
            // Send data to the server
            if((send(sockfd, msg, strlen(msg), 0)) < 0){ 
                perror("Send error");
            }
        }
    }

    
}


