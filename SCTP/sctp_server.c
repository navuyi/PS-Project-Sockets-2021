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
#include 	<unistd.h>

#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>

#define SIZE 1024
char buf[SIZE];
#define CHAT_PORT 2021

int main(int argc, char *argv[]) {
    int sockfd, client_sockfd;
    int nread, nsent, len;
    struct sockaddr_in serv_addr, client_addr;
    struct sctp_sndrcvinfo sinfo;
    int flags;
    struct sctp_event_subscribe events;
    sctp_assoc_t assoc_id;

    /* create endpoint */
    if ((sockfd = socket(AF_INET, SOCK_STREAM,
                        IPPROTO_SCTP)) < 0) {
        perror("socket()");
        return -1;
    }
    /* bind address */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(CHAT_PORT);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
             sizeof(serv_addr)) < 0) {
        perror("bind()");
        close(sockfd);
        return -1;
    }

    bzero(&events, sizeof(events));
    events.sctp_data_io_event = 1;
    if (setsockopt(sockfd, IPPROTO_SCTP,
                   SCTP_EVENTS, &events, sizeof(events))) {
        perror("setsockopt()");
        close(sockfd);
        return -1;
    }

    /* specify queue */
    listen(sockfd, 5);
    printf("Listening\n");

    for (;;) {
        len = sizeof(client_addr);
        if ((client_sockfd = accept(sockfd, (struct sockaddr *)&client_addr, &len)) < 0) {
            perror("accept()");
            continue;
        }
        while (nread = read(sockfd, buf, SIZE) > 0) {
            buf[nread] = 0; /* null terminate */
            if (fputs(buf, stdout) == EOF) {
                perror("fputs()");
                return -1;
            }
        }
        if (nread < 0) {
            perror("read()");
        }

        printf("Got a read of %d\n", nread);
        write(1, buf, nread);
        /* send it back out to all associations */

        bzero(&sinfo, sizeof(sinfo));
        sinfo.sinfo_flags |= SCTP_SENDALL;

        sctp_send(sockfd, buf, nread,
                  // (struct sockaddr *) &client_addr, 1,
                  &sinfo, 0);
    }
}