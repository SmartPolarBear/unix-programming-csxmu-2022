#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>
#include	"apue.h"

void initlock(const char *lockfile)
{
	int	i;

	unlink(lockfile);
}

void lock(const char *lockfile)
{
	int	fd;
	while ( (fd = open(lockfile, O_RDONLY | O_CREAT | O_EXCL, FILE_MODE)) < 0)
		sleep(1);
	close(fd);
}

void unlock(const char *lockfile)
{
	unlink(lockfile);
}

