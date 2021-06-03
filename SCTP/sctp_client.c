#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 1024
char buf[SIZE];
char *msg = "hello\n";
#define ECHO_PORT 2021

int main(int argc, char *argv[]) {
    int sockfd;
    int nread, nsent;
    int flags, len;
    struct sockaddr_in serv_addr;
    struct sctp_sndrcvinfo sinfo;
    fd_set readfds;

    if (argc != 2) {
        fprintf(stderr, "usage: %s IPaddr\n", argv[0]);
        exit(1);
    }
    /* create endpoint using  SCTP */
    sockfd = socket(AF_INET, SOCK_SEQPACKET,
                    IPPROTO_SCTP);
    if (sockfd < 0) {
        perror("socket creation failed");
        exit(2);
    }
    /* connect to server */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(ECHO_PORT);

    if (connect(sockfd,
                (struct sockaddr *)&serv_addr,
                sizeof(serv_addr)) < 0) {
        perror("connect to server failed");
        exit(3);
    }
    printf("Connected\n");

    while (1) {
        // printf(">\t");
        /* we need to select between messages FROM the user
                   on the console and messages TO the user from the
                   socket
                */
        FD_CLR(sockfd, &readfds);
        FD_SET(sockfd, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (select(sockfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select()");
            close(sockfd);
            return -1;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if ((nread = read(STDIN_FILENO, buf, SIZE)) < 0) {
                perror("read() form stdin");
                close(sockfd);
                return -1;
            }
            buf[nread] = 0;
            if (sendto(sockfd, buf, nread, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
                perror("send() to sockfd");
                close(sockfd);
                return -1;
            }
        } else if (FD_ISSET(sockfd, &readfds)) {
            len = sizeof(serv_addr);
            if ((nread = sctp_recvmsg(sockfd, buf, SIZE, (struct sockaddr *)&serv_addr, &len, &sinfo, &flags)) < 0) {
                perror("read() from sockfd");
                close(sockfd);
                return -1;
            }
            printf("Received >\t%s", buf);
        }
    }
    close(sockfd);
    exit(0);
}