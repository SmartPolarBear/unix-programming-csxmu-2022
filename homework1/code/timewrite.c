#include <sys/times.h>
#include <unistd.h>
#include <fcntl.h>

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


int main(int argc, char** argv)
{
    assert(argc == 2 || argc == 3);

    if (argc == 3 && strcmp(argv[2], "sync") != 0)
    {
        printf("usage: %s <pathname> [sync]", argv[0]);
        return 1;
    }

    const int mode = (argc == 3) ? O_RDWR | O_CREAT | O_TRUNC | O_SYNC : O_RDWR | O_CREAT | O_TRUNC;

    int out_file = open(argv[1], mode, FILE_MODE);
    if (out_file < 0)
    {
        printf("open error");
        return 1;
    }

    int length = lseek(STDIN_FILENO, 0, SEEK_END);
    if (length < 0)
    {
        printf("lseek error");
        return 1;
    }

    if (lseek(STDIN_FILENO, 0, SEEK_SET) < 0)
    {
        printf("lseek error");
        return 1;
    }

    char* buff = malloc(length);
    if (buff == NULL)
    {
        printf("malloc error");
        return 1;
    }

    if (read(STDIN_FILENO, buff, length) != length)
    {
        printf("read error");
        return 1;
    }

    const int lock_per_second = sysconf(_SC_CLK_TCK);

    for (int bufsize = 256;bufsize <= 131072;bufsize <<= 1)
    {
        lseek(out_file, 0, SEEK_SET);

        int g = length / bufsize, res = length % bufsize;
        struct tms start, end;
        clock_t start_clock, end_clock;
        start_clock = times(&start);
        
        for (int i = 0;i < g;i++)
        {
            if (write(out_file, buff + i * bufsize, bufsize) != bufsize)
            {
                printf("write error");
                return 1;
            }
        }

        if (res != 0 && write(out_file, buff + g * bufsize, res) != res)
        {
            printf("write error");
            return 1;
        }
        end_clock = times(&end);

        const int loop = g + (res != 0);
        const double real_time = (double)(end_clock - start_clock) / lock_per_second;
        const double user_time = (double)(end.tms_utime - start.tms_utime) / lock_per_second;
        const double sys_time = (double)(end.tms_stime - start.tms_stime) / lock_per_second;

        printf("%d\t%7.4lf\t%7.4lf\t%7.4lf\t%d\n", bufsize, user_time, sys_time,real_time, loop);

    }

    return 0;
}