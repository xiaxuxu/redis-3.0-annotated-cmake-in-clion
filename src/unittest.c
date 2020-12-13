#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "sds.h"
#include "unistd.h"
#include "fcntl.h"

struct ccdd {
    int a;
    char ccc[];
};

int main() {
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
        int fp = open("/home/d/gg.log", O_RDWR | O_APPEND);
        write(fp, msg, strlen(msg));
        close(fp);
        sleep(1);
    }

    return 0;
}