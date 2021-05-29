#include <arpa/inet.h> /* inet(3) functions */
#include <errno.h>
#include <fcntl.h>  /* for nonblocking */
#include <limits.h> /* for OPEN_MAX */
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h> /* sockaddr_in{} and other Internet defns */
#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h> /* basic socket definitions */
#include <sys/time.h>   /* timeval{} for select() */
#include <sys/types.h>  /* basic system data types */
#include <time.h>       /* timespec{} for pselect() */
#include <unistd.h>

#define PORT 2021
#define MAXLINE 2048

int mcast_join(int sockfd, const struct sockaddr* grp, socklen_t grplen, const char* ifname, u_int ifindex) {
    struct group_req req;
    if (ifindex > 0) {
        req.gr_interface = ifindex;
    } else if (ifname != NULL) {
        if ((req.gr_interface = if_nametoindex(ifname)) == 0) {
            errno = ENXIO; /* if name not found */
            perror("Bad interface");
            return (-1);
        }
    } else
        req.gr_interface = 0;
    if (grplen > sizeof(req.gr_group)) {
        errno = EINVAL;
        return -1;
    }
    memcpy(&req.gr_group, grp, grplen);
    return (setsockopt(sockfd, IPPROTO_IP,
                       MCAST_JOIN_GROUP, &req, sizeof(req)));
}

void recv_all(int recvfd, socklen_t addr_len) {
    int n;
    char line[MAXLINE + 1];
    struct sockaddr_in* sender_addr;

    sender_addr = malloc(addr_len);

    while (1) {
        if ((n = recvfrom(recvfd, line, MAXLINE, 0, (struct sockaddr*)sender_addr, &addr_len)) < 0)
            perror("recvfrom()");

        line[n] = 0; /* null terminate */

        sender_addr = (struct sockaddr_in*)sender_addr;

        printf("%s\n", line);
        fflush(stdout);
    }
}

int main(int argc, char** argv) {
    int sockfd;
    int so_reuseaddr = 1;
    struct sockaddr_in rcv_addr, mcast_grp;
    char rcv_buff[MAXLINE];
    char snd_buff[MAXLINE];
    int n;
    char mcast_grp_str[INET_ADDRSTRLEN]; 
    char if_name[7];
    char if_addr[INET_ADDRSTRLEN];

    if (argc == 3) {
        strcpy(if_name, "enp0s8");
        strcpy(if_addr, "192.168.56.31");
        strcpy(mcast_grp_str, "224.0.0.1");
    }
    else {
        strcpy(if_name, argv[1]);
        strcpy(if_addr, argv[2]);
        strcpy(mcast_grp_str, argv[3]);
    }

    struct in_addr if_struct;
    if_struct.s_addr = inet_addr(if_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }

    // Enable reuseaddr
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &so_reuseaddr, sizeof(so_reuseaddr)) < 0) {
        perror("SO_REUSEADDR");
        close(sockfd);
        exit(-1);
    }

    // Bind to device
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, if_name, strlen(if_name))) {
        perror("SO_BINDTODEVICE");
        close(sockfd);
        exit(-1);
    }

    // Enable multicast
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &if_struct, sizeof(if_struct)) < 0) {
        perror("IP_MULTICAST_IF");
        close(sockfd);
        exit(-1);
    }

    // Declare receive address
    bzero(&rcv_addr, sizeof(rcv_addr));
    rcv_addr.sin_family = AF_INET;
    rcv_addr.sin_port = htons(PORT);
    rcv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind multicast receive address
    if (bind(sockfd, (struct sockaddr*)&rcv_addr, sizeof(rcv_addr))) {
        perror("binding datagram socket");
        close(sockfd);
        exit(-1);
    }

    // Declare multicast group
    bzero(&mcast_grp, sizeof(mcast_grp));
    mcast_grp.sin_family = AF_INET;
    mcast_grp.sin_port = htons(PORT);
    inet_pton(AF_INET, mcast_grp_str, &mcast_grp.sin_addr);

    // Join multicast group
    if (mcast_join(sockfd, (const struct sockaddr*)&mcast_grp, sizeof(mcast_grp), if_name, 0) < 0) {
        perror("mcast_join()");
        close(sockfd);
        exit(-1);
    }

    // Receive
    recv_all(sockfd, sizeof(rcv_addr));
}