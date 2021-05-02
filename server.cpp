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


int main(int argc, char const *argv[]){
    int listenfd, connfd;
    int max_index, nready;
    int option_on = 1;
    socklen_t clilen;
    struct sockaddr_in servaddr, cliaddr;
    struct pollfd clients[FOPEN_MAX];

    
    // Socket
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        fprintf(stderr,"socket error : %s\n", strerror(errno));
        return 1;
    }

    // Socket options
    if((setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option_on, sizeof(option_on))<0))
    {
        fprintf(stderr, "socket option error : %s\n", strerror(errno));
        return 1;
    }

    // Bind
    int PORT = 55555;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0){
        fprintf(stderr, "Bind error %s\n", strerror(errno));
        return 1;
    }
    // Listen
    if(listen(listenfd, 10) < 0){
        fprintf(stderr, "Listen error %s\n", strerror(errno));
        return 1;
    }
    system("clear");    // Clear screen
    cout<<"[SERVER] Listening on: "<<servaddr.sin_addr.s_addr<<":"<<PORT<<endl;
    

    
    clients[0].fd = listenfd;               // First element is listening FD
    clients[0].events = POLLIN;              // ready to read as requested events
    for(int i=1; i<sizeof(clients); i++){
        clients[i].fd = -1;                 // -1 means this position is empty, available entry
    }
    max_index = 0;                          // max_index of descriptors in array

    while(true){
        if((nready = poll(clients, max_index+1, -1)) < 0){ // INFINITE TIMEOUT, RETURNS ONLY ON FD READY OR SIGNAL INTERRUPT
            perror("Poll error");
            return -1;
        }
        if(clients[0].revents & POLLIN){ 
            // New client connection
            clilen = sizeof(cliaddr);
            if((connfd = accept(listenfd, (SA*) &cliaddr, &clilen)) < 0){
                perror("Accept error");
                return -1;
            }
        }   
    }
}
