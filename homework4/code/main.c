#include "apue.h"
#include <dirent.h>
#include <limits.h>

#include <libgen.h> // basename

typedef int (Callback)(const char *, const struct stat *, int);

static Callback simple_statistic;

static int myftw(char *, Callback *);

static int dopath(Callback *);

static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot, nless4k;

#define NARGS 16

int main(int argc, char *argv[])
{
	int ret;
	if (!(argc == 2))
	{
		err_quit("usage: myfind <params>");
	}

	if (argc == 2)
	{
		ret = myftw(argv[1], simple_statistic);
		ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
		if (ntot == 0)
		{
			ntot = 1;
		}

		printf("%ld\n", nless4k);
	}

	return ret;
}

#define FTW_F 1
#define FTW_D 2
#define FTW_DNR 3
#define FTW_NS 4

static char *fullpath;
static size_t pathlen;

static int myftw(char *pathname, Callback *func)
{
	fullpath = path_alloc(&pathlen);

	if (pathlen <= strlen(pathname))
	{
		pathlen = strlen(pathname) * 2;
		if ((fullpath = realloc(fullpath, pathlen)) == NULL)
		{
			err_sys("realloc failed");
		}
	}
	strcpy(fullpath, pathname);
	return (dopath(func));
}

static int dopath(Callback *func)
{
	struct stat statbuf;
	struct dirent *dirp;
	DIR *dp;
	int ret, n;
	if (lstat(fullpath, &statbuf) < 0)
	{
		return (func(fullpath, &statbuf, FTW_NS));
	}
	if (S_ISDIR(statbuf.st_mode) == 0)
	{
		return (func(fullpath, &statbuf, FTW_F));
	}

	if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
	{
		return (ret);
	}
	n = strlen(fullpath);
	if (n + NAME_MAX + 2 > pathlen)
	{
		pathlen *= 2;
		if ((fullpath = realloc(fullpath, pathlen)) == NULL)
		{
			err_sys("realloc failed");
		}
	}
	fullpath[n++] = '/';
	fullpath[n] = 0;
	if ((dp = opendir(fullpath)) == NULL)
	{
		return (func(fullpath, &statbuf, FTW_DNR));
	}
	while ((dirp = readdir(dp)) != NULL)
	{
		if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0)
		{
			continue;
		}
		strcpy(&fullpath[n], dirp->d_name);
		if ((ret = dopath(func)) != 0)
		{
			break;
		}
	}
	fullpath[n - 1] = 0;
	if (closedir(dp) < 0)
	{
		err_ret("can't close directory %s", fullpath);
	}
	return (ret);
}

static int simple_statistic(const char *pathname, const struct stat *statptr, int type)
{
	switch (type)
	{
	case FTW_F:
		switch (statptr->st_mode & S_IFMT)
		{
		case S_IFREG:
			nreg++;
			if (statptr->st_size > 4096)
			{
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
