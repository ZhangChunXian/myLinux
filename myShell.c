/* 头文件 */
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

/* 宏定义 */
#define IN 1
#define OUT 0
#define MAX_CMD 10
#define BUFFSIZE 100
#define MAX_CMD_LEN 100


/* 全局变量 */
int argc;                                       // 有效参数个数
char* argv[MAX_CMD];                            // 参数数组
char command[MAX_CMD][MAX_CMD_LEN];             // 参数数组
char buf[BUFFSIZE];                             // 接受输入的参数数组
char backupBuf[BUFFSIZE];                       // 参数数组的备份
char curPath[BUFFSIZE];                         // 当前shell工作路径
int i, j;                                       // 循环变量
int commandNum;                                 // 已经输入指令数目
char history[MAX_CMD][BUFFSIZE];                // 存放历史命令


/* 函数声明 */
int get_input(char buf[]);                      // 输入指令并存入buf数组
void parse(char* buf);                          // 解析字符串
void do_cmd(int argc, char* argv[]);            
    int callCd(int argc);                       // 执行cd指令
    int printHistory(char command[MAX_CMD][MAX_CMD_LEN]);   // 打印历史指令
    // 重定向指令
    int commandWithOutputRedi(char buf[BUFFSIZE]);          // 执行输出重定向
    int commandWithInputRedi(char buf[BUFFSIZE]);           // 执行输入重定向命令
    int commandWithReOutputRedi(char buf[BUFFSIZE]);        // 执行输出重定向追加写


/* 函数定义 */
/* get_input接受输入的字符并存入buf数组中 */
int get_input(char buf[]) {
    // buf数组初始化
    memset(buf, 0x00, BUFFSIZE);
    memset(backupBuf, 0x00, BUFFSIZE);        

    fgets(buf, BUFFSIZE, stdin);
    // 去除fgets带来的末尾\n字符
    buf[strlen(buf) - 1] = '\0';
    return strlen(buf);
}

void parse(char* buf) {
    // 初始化argv数组和argc
    for (i = 0; i < MAX_CMD; i++) {
        argv[i] = NULL;
        for (j = 0; j < MAX_CMD_LEN; j++)
            command[i][j] = '\0';
    }
    argc = 0;
    // 下列操作改变了buf数组, 为buf数组做个备份
    strcpy(backupBuf, buf);
    /** 构建command数组
     *  即若输入为"ls -a"
     *  strcmp(command[0], "ls") == 0 成立且
     *  strcmp(command[1], "-a") == 0 成立
     */  
    int len = strlen(buf);
    for (i = 0, j = 0; i < len; ++i) {
        if (buf[i] != ' ') {
            command[argc][j++] = buf[i];
        } else {
            if (j != 0) {
                command[argc][j] = '\0';
                ++argc;
                j = 0;
            }
        }
    }
    if (j != 0) {
        command[argc][j] = '\0';
    }

    /** 构建argv数组
     *  即若输入buf为"ls -a"
     *  strcmp(argv[0], "ls") == 0 成立且
     *  strcmp(argv[1], "-a") == 0 成立*/
    argc = 0;
    int flg = OUT;
    for (i = 0; buf[i] != '\0'; i++) {
        if (flg == OUT && !isspace(buf[i])) {
            flg = IN;
            argv[argc++] = buf + i;
        } else if (flg == IN && isspace(buf[i])) {
            flg = OUT;
            buf[i] = '\0';
        }
    }
    argv[argc] = NULL;
}

void do_cmd(int argc, char* argv[]) {
    pid_t pid;
    /* 识别program命令 */
    // 识别重定向输出命令
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], ">") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithOutputRedi(buf);
            return;
        }
    }
    // 识别输入重定向
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[i], "<") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithInputRedi(buf);
            return;
        }
    }
    // 识别追加写重定向
    for (j = 0; j < MAX_CMD; j++) {
        if (strcmp(command[j], ">>") == 0) {
            strcpy(buf, backupBuf);
            int sample = commandWithReOutputRedi(buf);
            return;
        }
    }





    /* 识别shell内置命令 */
    if (strcmp(command[0], "cd") == 0) {
        int res = callCd(argc);
        if (!res) printf("cd指令输入错误!");
    } else if (strcmp(command[0], "history") == 0) {
        printHistory(command);
    } else if (strcmp(command[0], "mytop") == 0) {

    } else if (strcmp(command[0], "exit") == 0) {
        exit(0);
    } else {
        switch(pid = fork()) {
            // fork子进程失败
            case -1:
                printf("创建子进程未成功");
                return;
            // 处理子进程
            case 0:
                {   /* 函数说明：execvp()会从PATH 环境变量所指的目录中查找符合参数file 的文件名, 找到后便执行该文件, 
                     * 然后将第二个参数argv 传给该欲执行的文件。
                     * 返回值：如果执行成功则函数不会返回, 执行失败则直接返回-1, 失败原因存于errno 中. 
                     * */
                    execvp(argv[0], argv);
                    // 代码健壮性: 如果子进程未被成功执行, 则报错
                    printf("%s: 命令输入错误\n", argv[0]);
                    // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
                    exit(1);
                }
            default: {
                    int status;
                    waitpid(pid, &status, 0);      // 等待子进程返回
                    int err = WEXITSTATUS(status); // 读取子进程的返回码

                    if (err) { 
                        printf("Error: %s\n", strerror(err));
                    }                    
            }
        }
    }
}

int callCd(int argc) {
    // result为1代表执行成功, 为0代表执行失败
    int result = 1;
    if (argc != 2) {
        printf("指令数目错误!");
    } else {
        int ret = chdir(command[1]);
        if (ret) return 0;
    }

    if (result) {
        char* res = getcwd(curPath, BUFFSIZE);
        if (res == NULL) {
            printf("文件路径不存在!");
        }
        return result;
    }
    return 0;
}


int printHistory(char command[MAX_CMD][MAX_CMD_LEN]) {
    int n = atoi(command[1]);

    for (i = n; i > 0 && commandNum - i >= 0; i--) {
        printf("%d\t%s\n", n - i + 1, history[commandNum - i]);
    }
    return 0;
}

int commandWithOutputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char outFile[BUFFSIZE];
    memset(outFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 1 < strlen(buf); i++) {
        if (buf[i] == '>') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("输出重定向指令输入有误!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], ">") == 0) {
            if (i + 1 < argc) {
                strcpy(outFile, command[i + 1]);
            } else {
                printf("缺少输出文件!");
                return 0;
            }
        }
    }

    // 指令分割, outFile为输出文件, buf为重定向符号前的命令
    for (j = 0; j < strlen(buf); j++) {
        if (buf[j] == '>') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    // 解析指令
    parse(buf);
    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("创建子进程未成功");
            return 0;
        }
        // 处理子进程:
        case 0: {
            // 完成输出重定向
            int fd;
            fd = open(outFile, O_RDONLY, 7777);
            // 文件打开失败
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  
            execvp(argv[0], argv);
            // 代码健壮性: 如果子进程未被成功执行, 则报错
            printf("%s: 命令输入错误\n", argv[0]);
            // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // 等待子进程返回
            int err = WEXITSTATUS(status);  // 读取子进程的返回码
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }

}

int commandWithInputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char inFile[BUFFSIZE];
    memset(inFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 1< strlen(buf); i++) {
        if (buf[i] == '<' && buf[i + 1] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("输入重定向指令输入有误!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], "<") == 0) {
            if (i + 1 < argc) {
                strcpy(inFile, command[i + 1]);
            } else {
                printf("缺少输入指令!");
                return 0;
            }
        }
    }

    // 指令分割, InFile为输出文件, buf为重定向符号前的命令
    for (j = 0; j < strlen(buf); j++) {
        if (buf[j] == '<') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';

    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("创建子进程未成功");
            return 0;
        }
        // 处理子进程:
        case 0: {
            // 完成输出重定向
            int fd;
            fd = open(inFile, O_WRONLY|O_CREAT, 7777);
            // 文件打开失败
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDIN_FILENO);  
            parse(buf);
            execvp(argv[0], argv);
            // 代码健壮性: 如果子进程未被成功执行, 则报错
            printf("%s: 命令输入错误\n", argv[0]);
            // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // 等待子进程返回
            int err = WEXITSTATUS(status);  // 读取子进程的返回码
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }

}


int commandWithReOutputRedi(char buf[BUFFSIZE]) {
    strcpy(buf, backupBuf);
    char reOutFile[BUFFSIZE];
    memset(reOutFile, 0x00, BUFFSIZE);
    int RediNum = 0;
    for ( i = 0; i + 2 < strlen(buf); i++) {
        if (buf[i] == '>' && buf[i + 1] == '>' && buf[i + 2] == ' ') {
            RediNum++;
            break;
        }
    }
    if (RediNum != 1) {
        printf("输出重定向指令输入有误!");
        return 0;
    }

    for (i = 0; i < argc; i++) {
        if (strcmp(command[i], ">>") == 0) {
            if (i + 1 < argc) {
                strcpy(reOutFile, command[i + 1]);
            } else {
                printf("缺少输出文件!");
                return 0;
            }
        }
    }

    // 指令分割, outFile为输出文件, buf为重定向符号前的命令
    for (j = 0; j + 2< strlen(buf); j++) {
        if (buf[j] == '>' && buf[j + 1] == '>') {
            break;
        }
    }
    buf[j - 1] = '\0';
    buf[j] = '\0';
    // 解析指令

    pid_t pid;
    switch(pid = fork()) {
        case -1: {
            printf("创建子进程未成功");
            return 0;
        }
        // 处理子进程:
        case 0: {
            // 完成输出重定向
            int fd;
            fd = open(reOutFile, O_WRONLY|O_CREAT|O_APPEND, 7777);
            // 文件打开失败
            if (fd < 0) {
                exit(1);
            }
            dup2(fd, STDOUT_FILENO);  
            parse(buf);
            execvp(argv[0], argv);
            // 代码健壮性: 如果子进程未被成功执行, 则报错
            printf("%s: 命令输入错误\n", argv[0]);
            // exit函数终止当前进程, 括号内参数为1时, 会像操作系统报告该进程因异常而终止
            exit(1);
        }
        default: {
            int status;
            waitpid(pid, &status, 0);       // 等待子进程返回
            int err = WEXITSTATUS(status);  // 读取子进程的返回码
            if (err) { 
                printf("Error: %s\n", strerror(err));
            } 
        }                        
    }   
}



/* main函数 */
int main() {
        // while循环
    while(1) {
        printf("[myshell]$ ");
        // 输入字符存入buf数组, 如果输入字符数为0, 则跳过此次循环
        if (get_input(buf) == 0)
            continue;
        strcpy(history[commandNum++], buf);
        strcpy(backupBuf, buf);
        parse(buf);
        do_cmd(argc, argv);
        argc = 0;
    }
}