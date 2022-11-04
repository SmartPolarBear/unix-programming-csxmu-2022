//
// Created by bear on 11/4/2022.
//
#include "apue.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#include "findcmd.h"

// search binary name in path environment variable
char *find_in_path(char *exe, char *pathlist)
{
	char *p, *q, *path;
	struct stat st;

	if (strchr(exe, '/'))
	{
		if (stat(exe, &st) < 0)
		{
			return 0;
		}
		return exe;
	}

	for (p = pathlist; *p; p = q + 1)
	{
		if ((q = strchr(p, ':')) == 0)
		{
			q = p + strlen(p);
		}
		if (q == p)
		{
			path = ".";
		}
		else
		{
			path = p;
		}
		size_t buf_len = 0;
		char *buf = path_alloc(&buf_len);
		memset(buf, 0, buf_len);

		if ((q - p) + 1 + strlen(exe) + 1 > buf_len)
		{
			free(buf);
			continue;
		}
		snprintf(buf, buf_len, "%.*s/%s", (int)(q - p), path, exe);

		int err = stat(buf, &st);
		if (err == 0)
		{
			return buf;
		}
	}
	return 0;
}