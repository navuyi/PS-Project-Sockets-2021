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
#include 	    <unistd.h>
#include        <ostream>
#include        <iostream>

using namespace std;

#define MAXLINE 2048
#define PORT 55555
#define SA struct sockaddr
#define MAXCLIENTS FD_SETSIZE           // CAN NOT BE LARGER THEN 1024

int main(int argc, char *argv[]){
    int opt = 1;
    int listenfd, connfd, sockfd;
    int i, n, max_fd, nready;
    int clients[MAXCLIENTS];
    struct sockaddr_in servaddr, cliaddr, tmp_addr;
    socklen_t addrlen;
    
    
    char addr_buf[INET_ADDRSTRLEN+1];
    char buf[MAXLINE];
    char hellomsg[] = "Welcome to chat.";

    fd_set master_set;


    for(i=0; i<MAXCLIENTS; i++){
        clients[i] = -1;                // -1 indicates that this element is available for new descriptor
    }

    // Create listening socket
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        fprintf(stderr,"socket error : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    // Set socket options
    if(setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("Setsockopt error");
        exit(EXIT_FAILURE);
    }

    // Fill address structure
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    // Overwriting previous line in order to bind to loopback
    inet_pton(AF_INET, "127.0.0.1", &(servaddr.sin_addr.s_addr)); 
    servaddr.sin_port = htons(PORT);

    // Bind listening socket to local PORT
    if(bind(listenfd, (SA*) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr,"bind error : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if(listen(listenfd, 5) < 0){
        fprintf(stderr,"listen error : %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }




    while(true){
        // Clear the set
        FD_ZERO(&master_set);

        // Add listening socket to the set
        FD_SET(listenfd, &master_set);
        max_fd = listenfd;


        // Add child sockets to the set
        for(i=0; i<MAXCLIENTS; i++){
            // Socket descriptor
            sockfd = clients[i];

            // If socket descriptor is valid then add it to the set
            if(sockfd > 0){
                FD_SET(sockfd, &master_set);
            }
            // Highest socket descriptor number, for select function
            if(sockfd > max_fd){
                max_fd = sockfd;
            }
        }


        // Wait for activity on sockets
        if((nready = select(max_fd+1, &master_set, NULL, NULL, NULL)) < 0){
            perror("Select error");
            exit(EXIT_FAILURE);
        }


        // Handle action on listening socket
        if(FD_ISSET(listenfd, &master_set)){
            // Accept new connection
            addrlen = sizeof(cliaddr);
            if((connfd = accept(listenfd, (SA*) &cliaddr, &addrlen)) < 0){
                perror("Accept error");
                exit(EXIT_FAILURE);
            }
            cout<<"New connection from: "<<inet_ntoa(cliaddr.sin_addr)<<":"<<ntohs(cliaddr.sin_port)<<endl;
            
            // Send greeting message to client
            if(send(connfd, hellomsg, strlen(hellomsg), 0) < 0){
                perror("Send error");
                exit(EXIT_FAILURE);
            }

            // Add new socket to clients array
            for(i=0; i< MAXCLIENTS; i++){
                // Check if position is available
                if(clients[i] < 0){
                    clients[i] = connfd;
                    break;
                }
            }
        }


        // Handle input/output operations on sockets
        for(i=0; i<MAXCLIENTS; i++){
            sockfd = clients[i];

            // Check if and what event occured on socket
            if(FD_ISSET(sockfd, &master_set)){
                // Check for closing event
                if((n = read(sockfd, buf, MAXLINE)) == 0){
                    // User closed connection
                    addrlen = sizeof(tmp_addr);
                    getpeername(sockfd, (SA*) &tmp_addr, &addrlen);
                    cout<<"User disconnected: "<<inet_ntoa(tmp_addr.sin_addr)<<":"<<ntohs(tmp_addr.sin_port)<<endl;

                    // Close socket and release its position
                    close(sockfd);
                    clients[i] = -1;    // set to avaiable
                }
                else if((n = read(sockfd, buf, MAXLINE)) < 0){
                     if(errno == ECONNRESET){
                        // Connection reset by client
                        cout<<"User unexpectedly disconnected"<<endl;

                        // Close socket and release its position
                        close(sockfd);
                        clients[i] = -1;
                    }
                    else{
                        perror("Read error");
                        exit(1);
                    }
                }
                else{
                    // Write to the socket
                }
            }
        }
    }
}
