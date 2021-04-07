/* 获取当前的进程ID, 进程组ID */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
 
int main() {
    printf("real uid : %d, real gid: %d\n", getuid(), getgid());
    printf("effective uid: %d, effective gid: %d\n", geteuid(), getegid());
    exit(0);
    return 0;
}