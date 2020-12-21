#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sds.h"
#include "unistd.h"
#include "fcntl.h"
#include "sys/time.h"
#include <stdio.h>
#include <signal.h>
#include <sys/time.h>

#define TEST_LOG "/home/x/gg.log"


void read_stdin() {
    size_t len = 1024;
    char buffer[len];
    for (int i = 0; i < 4; ++i) {
        int read_len = -1;
        if ((read_len = read(STDIN_FILENO, buffer, len)) > 0) {
            buffer[read_len] = '\0';
            printf("print stdin:%s\n", buffer);
        }
    }
}

void signalAlarmHandler(int signo) {
    struct timeval timeval;
    gettimeofday(&timeval, NULL);
    switch (signo) {
        case SIGALRM:
            printf("cron ,%ld,%ld \n", timeval.tv_sec, timeval.tv_usec);
            break;
    }
}

void testTimer() {
    struct timeval begin;
    gettimeofday(&begin, NULL);
    printf("begin time , %ld,%ld \n", begin.tv_sec, begin.tv_usec);
    signal(SIGALRM, signalAlarmHandler);
    struct itimerval new_value, old_value;
    //value 是延时
    new_value.it_value.tv_sec = 3;
    new_value.it_value.tv_usec = 0;
    // interval 是时间间隔
    new_value.it_interval.tv_sec = 1;
    new_value.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &new_value, &old_value);
    for (;;) {
        sleep(1);
    }
}

/**
 * dup2的用法
 */
void testDup() {
    int fp = open(TEST_LOG, O_RDWR | O_APPEND);
    dup2(fp, STDIN_FILENO);
    dup2(fp, STDOUT_FILENO);
    close(fp);
    printf("sout ---=-==-=-= \n");
    read_stdin();
}

/**
 * 怎么让一个任务成为后台任务
 */
void HowDaemonize() {
    printf("parent ....\n");
    if (fork() != 0) {
        printf("parent exit\n");
        exit(0);
    }
    printf("child running...\n");
    setsid();
    printf("child setsid ...\n");
    char *msg = "file append.....\n";
    while (1) {
        int fp = open(TEST_LOG, O_RDWR | O_APPEND);
        write(fp, msg, strlen(msg));
        close(fp);
        sleep(1);
    }
}

int main() {
    testTimer();
    return 0;
}