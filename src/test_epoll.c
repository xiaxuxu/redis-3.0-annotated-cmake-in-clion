#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sds.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/epoll.h"
#include "errno.h"
#include "arpa/inet.h"
#include "fcntl.h"
#include "assert.h"


int setnonblocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_opetion = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_opetion);
    return old_option;
}

void add_fd(int epoll_fd, int fd, int oneshot) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN || EPOLLET;
    if (oneshot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

void reset_oneshot(int epoll_fd, int socketfd) {
    struct epoll_event event;
    event.data.fd = socketfd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, socketfd, &event);
}


int main() {
    const char *ip = "127.0.0.1";
    int port = 8899;
    struct sockaddr_in listen_address;
    bzero(&listen_address, sizeof(listen_address));
    listen_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &listen_address.sin_addr);
    listen_address.sin_port = htons(port);
    //listen
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    int yes = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    assert(listenfd > 0);

    int ret = bind(listenfd, (const struct sockaddr_in *) &listen_address, sizeof(listen_address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);


    int epoll_fd = epoll_create(5);
    assert(epoll_fd != -1);

    // listen should not be oneshot
    add_fd(epoll_fd, listenfd, 0);

    struct epoll_event recv_events[1024];
    while (1) {
        int event_num = epoll_wait(epoll_fd, recv_events, 1024, -1);
        if (event_num < 0) {
            printf("epoll failed\n");
            break;
        }
        printf("epoll ret ....\n");
        for (int i = 0; i < event_num; ++i) {
            int sockfd = recv_events[i].data.fd;
            if (sockfd == listenfd) {
                struct sockaddr_in client_addr;
                socklen_t client_addr_len = sizeof(client_addr);
                int client_fd = accept(listenfd, (struct sockaddr *) &client_addr, &client_addr_len);
                char ip_buf[INET_ADDRSTRLEN];
                memset(ip_buf, '\0', INET_ADDRSTRLEN);
                printf("client conn:%s :%d ,soketfd:%d \n", inet_ntoa(client_addr.sin_addr),
                       ntohs(client_addr.sin_port), sockfd);
                add_fd(epoll_fd, client_fd, 1);
            } else if (recv_events[i].events & EPOLLIN) {
                char buf[4096];
                memset(buf, '\0', 4096);
                while (1) {
                    printf("==================\n");
                    int ret = recv(sockfd, buf, 4096, 0);
                    if (ret == 0) {
                        close(sockfd);
                        printf(" client close its conn,socketfd:%d \n ", sockfd);
                        break;
                    } else if (ret < 0) {
                        if (errno == EAGAIN) {
                            reset_oneshot(epoll_fd, sockfd);
                            printf("read later \n");
                            break;
                        } else {
                            printf("xxxxx\n");
                        }
                    } else {
                        printf("get: %s\n", buf);
                    }
                }
            } else {
                printf("something bad happen\n");
            }
        }
    }
    close(listenfd);
    return 0;
}
