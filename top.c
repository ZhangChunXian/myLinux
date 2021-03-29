#include <stdio.h>
#include <dirent.h>            // readdir函数
#include <string.h>            // strcmp
#include <sys/stat.h>


void myTop() {
    FILE *fp = NULL;                    // 文件指针
    char buff[255];

    /* 获取内容一: 
       总体内存大小，
       空闲内存大小，
       缓存大小。 */
    fp = fopen("/proc/meminfo", "r");   // 以只读方式打开meminfo文件
    fgets(buff, 255, (FILE*)fp);        // 读取meminfo文件内容进buff
    fclose(fp);

    // 获取 pagesize
    int i = 0, pagesize = 0;
    while (buff[i] != ' ') {
        pagesize = 10 * pagesize + buff[i] - 48;
        i++;
    }

    // 获取 页总数 total
    i++;
    int total = 0;
    while (buff[i] != ' ') {
        total = 10 * total + buff[i] - 48;
        i++;
    }

    // 获取空闲页数 free
    i++;
    int free = 0;
    while (buff[i] != ' ') {
        free = 10 * free + buff[i] - 48;
        i++;
    }

    // 获取最大页数量largest
    i++;
    int largest = 0;
    while (buff[i] != ' ') {
        largest = 10 * largest + buff[i] - 48;
        i++;
    }

    // 获取缓存页数量 cached
    i++;
    int cached = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        cached = 10 * cached + buff[i] - 48;
        i++;
    }

    // 总体内存大小 = (pagesize * total) / 1024 单位 KB
    int totalMemory  = pagesize / 1024 * total;
    // 空闲内存大小 = pagesize * free) / 1024 单位 KB
    int freeMemory   = pagesize / 1024 * free;
    // 缓存大小    = (pagesize * cached) / 1024 单位 KB
    int cachedMemory = pagesize / 1024 * cached;

    printf("totalMemory  is %d KB\n", totalMemory);
    printf("freeMemory   is %d KB\n", freeMemory);
    printf("cachedMemory is %d KB\n", cachedMemory);

    /* 2. 获取内容2
        进程和任务数量
     */
    fp = fopen("/proc/kinfo", "r");     // 以只读方式打开meminfo文件
    memset(buff, 0x00, 255);            // 格式化buff字符串
    fgets(buff, 255, (FILE*)fp);        // 读取meminfo文件内容进buff
    fclose(fp);

    // 获取进程数量
    int processNumber = 0;
    i = 0;
    while (buff[i] != ' ') {
        processNumber = 10 * processNumber + buff[i] - 48;
        i++;
    }
    printf("processNumber = %d\n", processNumber);

    // 获取任务数量
    i++;
    int tasksNumber = 0;
    while (buff[i] >= '0' && buff[i] <= '9') {
        tasksNumber = 10 * tasksNumber + buff[i] - 48;
        i++;
    }
    printf("tasksNumber = %d\n", tasksNumber);


    // /* 3. 获取psinfo中的内容 */
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc");
    int totalTicks = 0, freeTicks = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {                   // 遍历proc文件夹
            if (strcmp(dir->d_name, ".") != 0 && 
                strcmp(dir->d_name, "..") != 0) {
                    char path[255];
                    memset(path, 0x00, 255);
                    strcpy(path, "/proc/");
                    strcat(path, dir->d_name);          // 连接成为完成路径名
                    struct stat s;
                    if (stat (path, &s) == 0) {
                        if (S_ISDIR(s.st_mode)) {       // 判断为目录
                            strcat(path, "/psinfo");

                            FILE* fp = fopen(path, "r");
                            char buf[255];
                            memset(buf, 0x00, 255);
                            fgets(buf, 255, (FILE*)fp);
                            fclose(fp);

                            // 获取ticks和进程状态
                            int j = 0;
                            for (i = 0; i < 4;) {
                                for (j = 0; j < 255; j++) {
                                    if (i >= 4) break;
                                    if (buf[j] == ' ') i++;
                                }
                            }
                            // 循环结束, buf[j]为进程的状态, 共有S, W, R三种状态.
                            int k = j + 1;
                            for (i = 0; i < 3;) {               // 循环结束后k指向ticks位置
                                for (k = j + 1; k < 255; k++) {
                                    if (i >= 3) break;
                                    if (buf[k] == ' ') i++;
                                }
                            }
                            int processTick = 0;
                            while (buf[k] != ' ') {
                                processTick = 10 * processTick + buff[k] - 48;
                                k++;
                            }
                            totalTicks += processTick;
                            if (buf[j] != 'R') {
                                freeTicks += processTick;
                            }                                                             
                        }else continue;
                    }else continue;
                }
        }
    }
    printf("CPU states: %.2lf%% used,\t%.2lf%% idle",
           (double)((totalTicks - freeTicks) * 100) / (double)totalTicks,
           (double)(freeTicks * 100) / (double)totalTicks);
    return;

}

int main() {
    myTop();
    return 0;
}