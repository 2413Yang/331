#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <printf.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

/**************************************************************************************************
**  函数名称:  lockfile
**  功能描述:  对文件加锁
**  输入参数:  无
**  输出参数:  无
**  返回参数:  无
**************************************************************************************************/
static int lockfile(int fd)
{
    struct flock fl;

    fl.l_type   = F_WRLCK;
    fl.l_start  = 0;
    fl.l_whence = SEEK_SET;
    fl.l_len    = 0;

    return(fcntl(fd, F_SETLK, &fl));
}

/**************************************************************************************************
**  函数名称:  proc_is_exist
**  功能描述:  判断系统中是否存在该进程
**  输入参数:  procname: 进程名
**  输出参数:  无
**  返回参数:  返回1表示系统中已经存在该进程了；返回0表示系统中不存在该进程
**  注意事项:  此处加锁完后无需对文件进行close，而是进程退出后由系统来释放；否则无法起到保护的作用
**************************************************************************************************/
int proc_is_exist(const char *procname)
{
    int  fd;
    char buf[16];
    char filename[100];

    sprintf(filename, "/var/run/%s.pid", procname);

    fd = open(filename, O_RDWR | O_CREAT, (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH));
    if (fd < 0) {
        printf("open file \"%s\" failed!!!\n", filename);
        return 1;
    }

    if (lockfile(fd) == -1) {                                                  /* 尝试对文件进行加锁 */
        printf("file \"%s\" locked. proc already exit!!!\n", filename);
        close(fd);
        return 1;
    } else {
        ftruncate(fd, 0);                                                      /* 写入运行实例的pid */
        sprintf(buf, "%ld", (long)getpid());
        write(fd, buf, strlen(buf) + 1);
        return 0;
    }
}