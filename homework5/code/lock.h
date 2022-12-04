#include	<sys/types.h>
#include	<sys/stat.h>
#include	<fcntl.h>
#include	<unistd.h>

void initlock(const char *lockfile);

void lock(const char *lockfile);

void unlock(const char *lockfile);

