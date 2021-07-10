#include <arpa/inet.h> /* inet(3) functions */
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h> /* sockaddr_in{} and other Internet defns */
#include <netinet/sctp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h> /* basic socket definitions */
#include <sys/time.h>   /* timeval{} for select() */
#include <sys/types.h>  /* basic system data types */
#include <time.h>       /* timespec{} for pselect() */
#include <unistd.h>

#define SIZE 1024
char buf[SIZE];
#define CHAT_PORT 2021

int main(int argc, char *argv[]) {
    int sockfd, client_sockfd, nsent, len, flags = 0;
    size_t nread;
    struct sockaddr_in serv_addr, client_addr;
    struct sctp_sndrcvinfo sinfo;
    struct sctp_event_subscribe events;
    sctp_assoc_t assoc_id;

    /* create endpoint */
    if ((sockfd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP)) < 0) {
        perror("socket()");
        return -1;
    }
    /* bind address */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(CHAT_PORT);
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("bind()");
        close(sockfd);
        return -1;
    }

    bzero(&events, sizeof(events));
    events.sctp_data_io_event = 1;
    if (setsockopt(sockfd, IPPROTO_SCTP, SCTP_EVENTS, &events, sizeof(events))) {
        perror("setsockopt()");
        close(sockfd);
        return -1;
    }

    /* specify queue */
    if (listen(sockfd, 5) < 0) {
        perror("listen()");
        close(sockfd);
        return -1;
    }
    printf("Listening\n");

    for (;;) {

	for (int i=0; i<sizeof(buf); i++){
		buf[i]=0;
	}	
        len = sizeof(client_addr);
        if ((nread = sctp_recvmsg(sockfd, buf, sizeof(buf), (struct sockaddr *)&client_addr, &len, &sinfo, &flags) < 0)) {
            perror("sctp_recvmsg()");
            close(sockfd);
            return -1;
        }

        printf("%s", buf);
        /* send it back out to all associations */

        bzero(&sinfo, sizeof(sinfo));
        sinfo.sinfo_flags |= SCTP_SENDALL;

        if (sctp_send(sockfd, buf, strlen(buf), &sinfo, 0) < 0) {
            perror("sctp_send()");
            close(sockfd);
            return -1;
        }
    }
}
