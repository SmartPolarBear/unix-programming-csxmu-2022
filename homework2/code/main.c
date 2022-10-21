#include "apue.h"
#include <dirent.h>
#include <limits.h>


typedef int (Callback)(const char*, const struct stat*, int);

static Callback simple_statistic;
static Callback content_compare;
static Callback name_compare;

static int myftw(char*, Callback*);

static int dopath(Callback*);

static int compare_file(const char* file1,const char* file2);

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot, nless4k;

#define NARGS 16

static char* names[NARGS];
static int name_count = 0;

static char* comp_filename = NULL;
static struct stat comp_stat;

int main(int argc, char* argv[]) {
    int ret;
    if (!(argc == 2 || argc >= 4)) {
        err_quit("usage: myfind <params>");
    }

    if (argc == 2)
    {
        ret = myftw(argv[1], simple_statistic);
        ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
        if (ntot == 0) {
            ntot = 1;
        }

        printf("regular files  = %7ld, %5.2f %%\n", nreg, nreg * 100.0 / ntot);
        printf("directories    = %7ld, %5.2f %%\n", ndir, ndir * 100.0 / ntot);
        printf("block special  = %7ld, %5.2f %%\n", nblk, nblk * 100.0 / ntot);
        printf("char special   = %7ld, %5.2f %%\n", nchr, nchr * 100.0 / ntot);
        printf("FIFOs          = %7ld, %5.2f %%\n", nfifo, nfifo * 100.0 / ntot);
        printf("symbolic links = %7ld, %5.2f %%\n", nslink, nslink * 100.0 / ntot);
        printf("sockets        = %7ld, %5.2f %%\n", nsock, nsock * 100.0 / ntot);
        printf("smaller than 4k= %7ld, %5.2f %%\n", nless4k, nless4k * 100.0 / ntot);

    }
    else if (argc >= 4)
    {
        char* pathname = argv[1];
        if (strcmp(argv[2], "-comp") == 0)
        {
            comp_filename = argv[3];
            if (lstat(comp_filename, &comp_stat) < 0)
            {
                err_sys("lstat error for %s", comp_filename);
            }
            ret = myftw(pathname, content_compare);
        }
        else if (strcmp(argv[2], "-name") == 0)
        {
            for (int i = 3; i < argc; i++)
            {
                names[i - 3] = argv[i];
                name_count++;
            }
            ret = myftw(pathname, name_compare);
        }
        else
        {
            err_quit("usage: myfind <params>");
        }
    }

    return ret;
}

#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4

static char* fullpath;
static size_t pathlen;

static int myftw(char* pathname, Callback* func) {
    fullpath = path_alloc(&pathlen);

    if (pathlen <= strlen(pathname)) {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL) {
            err_sys("realloc failed");
        }
    }
    strcpy(fullpath, pathname);
    return (dopath(func));
}

static int dopath(Callback* func) {
    struct stat statbuf;
    struct dirent* dirp;
    DIR* dp;
    int ret, n;
    if (lstat(fullpath, &statbuf) < 0) {
        return (func(fullpath, &statbuf, FTW_NS));
    }
    if (S_ISDIR(statbuf.st_mode) == 0) {
        return (func(fullpath, &statbuf, FTW_F));
    }

    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0) {
        return (ret);
    }
    n = strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) {
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL) {
            err_sys("realloc failed");
        }
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;
    if ((dp = opendir(fullpath)) == NULL) {
        return (func(fullpath, &statbuf, FTW_DNR));
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
            continue;
        }
        strcpy(&fullpath[n], dirp->d_name);
        if ((ret = dopath(func)) != 0) {
            break;
        }
    }
    fullpath[n - 1] = 0;
    if (closedir(dp) < 0) {
        err_ret("can't close directory %s", fullpath);
    }
    return (ret);
}

static int simple_statistic(const char* pathname, const struct stat* statptr, int type) {
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:
            nreg++;
            if (statptr->st_size <= 4096) {
                nless4k++;
            }
            break;
        case S_IFBLK:
            nblk++;
            break;
        case S_IFCHR:
            nchr++;
            break;
        case S_IFIFO:
            nfifo++;
            break;
        case S_IFLNK:
            nslink++;
            break;
        case S_IFSOCK:
            nsock++;
            break;
        case S_IFDIR:
            err_dump("for S_IFDIR for %s", pathname);
        }
        break;
    case FTW_D:
        ndir++;
        break;
    case FTW_DNR:
        err_ret("can't read directory %s", pathname);
        break;
    case FTW_NS:
        err_ret("stat error for %s", pathname);
        break;
    default:
        err_dump("unknown type %d for pathname %s", type, pathname);
    }
    return 0;
}

static int content_compare(const char* pathname, const struct stat* statptr, int type) {
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:
        case S_IFBLK:
        case S_IFCHR:
        case S_IFIFO:
        case S_IFLNK:
        case S_IFSOCK:
            if (statptr->st_size > 0) {
                if (statptr->st_size == comp_stat.st_size &&
                    compare_file(pathname, comp_filename)) {
                    printf("%s", pathname);
                }
            }
            break;
        case S_IFDIR:
            err_dump("for S_IFDIR for %s", pathname);
        }
        break;
    case FTW_D:
        break;
    case FTW_DNR:
        err_ret("can't read directory %s", pathname);
        break;
    case FTW_NS:
        err_ret("stat error for %s", pathname);
        break;
    default:
        err_dump("unknown type %d for pathname %s", type, pathname);
    }
    return 0;
}

static int name_compare(const char* pathname, const struct stat* statptr, int type) {
    switch (type) {
    case FTW_F:
        switch (statptr->st_mode & S_IFMT) {
        case S_IFREG:
        case S_IFBLK:
        case S_IFCHR:
        case S_IFIFO:
        case S_IFLNK:
        case S_IFSOCK:
            for (int i = 0;i < name_count;i++)
            {
                if (strcmp(pathname, names[i]) == 0)
                {
                    printf("%s", pathname);

                }
            }
            break;
        case S_IFDIR:
            err_dump("for S_IFDIR for %s", pathname);
        }
        break;
    case FTW_D:
        break;
    case FTW_DNR:
        err_ret("can't read directory %s", pathname);
        break;
    case FTW_NS:
        err_ret("stat error for %s", pathname);
        break;
    default:
        err_dump("unknown type %d for pathname %s", type, pathname);
    }
    return 0;
}

static int compare_file(const char* file1, const char* file2)
{
    FILE* fp1 = fopen(file1, "r");
    FILE* fp2 = fopen(file2, "r");
    if (fp1 == NULL || fp2 == NULL)
    {
        return 0;
    }

    char* buf1 = malloc(4096);
    if (!buf1)
    {
        return 0;
    }

    char* buf2 = malloc(4096);
    if (!buf2)
    {
        return 0;
    }

    int ret = 1;
    while (1)
    {
        int n1 = fread(buf1, 1, 4096, fp1);
        int n2 = fread(buf2, 1, 4096, fp2);
        if (n1 != n2)
        {
            ret = 0;
            break;
        }
        if (n1 == 0)
        {
            break;
        }
        if (memcmp(buf1, buf2, n1) != 0)
        {
            ret = 0;
            break;
        }
    }
    return ret;
}
