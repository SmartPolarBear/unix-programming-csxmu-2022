#include "apue.h"

#include <stdio.h>
#include <dirent.h>
#include <limits.h>
#include <sys/wait.h>

#include "parser.h"

#define BUFMAX 4096

char cmd_buf[BUFMAX];

int get_command(char *buf, int nbuf);
void run_command(command_t *cmd);

int main()
{
	while (get_command(cmd_buf, BUFMAX) >= 0)
	{
		if (cmd_buf[0] == 'c' && cmd_buf[1] == 'd' && cmd_buf[2] == ' ')
		{
			cmd_buf[strlen(cmd_buf) - 1] = 0;  // chop \n
			if (chdir(cmd_buf + 3) < 0)
			{
				printf("cannot cd %s\n", cmd_buf + 3);
			}
			goto cleanbuf;
		}

		int pid = fork();
		if (pid < 0)
		{
			printf("fork error\n");
			goto cleanbuf;
		}
		else if (pid == 0)
		{
			command_t *cmd = parse_command(cmd_buf);
			run_command(cmd);
		}
		else
		{
			int status = 0;
			if (waitpid(pid, &status, 0) < 0)
			{
				printf("waitpid error\n");
			}

			if (WIFEXITED(status))
			{
				printf("Process exit with status %d.\n", WEXITSTATUS(status));
			}
			else if (WIFSIGNALED(status))
			{
				printf("Process killed by signal %d.\n", WTERMSIG(status));
			}
		}

	cleanbuf:
		memset(cmd_buf, 0, BUFMAX);
	}
	return 0;
}

int get_command(char *buf, int nbuf)
{
	if (isatty(fileno(stdin)))
	{
		printf("%% ");
	}
	fflush(stdout);
	if (fgets(buf, nbuf, stdin) == NULL)
	{
		return -1;
	}
	return 0;
}

void run_command(command_t *cmd)
{
	exec_command_t *exec_cmd = NULL;
	switch (cmd->type)
	{
	case CMD_EXEC:
		exec_cmd = (exec_command_t *)cmd;
		if (exec_cmd->argv[0] == NULL)
		{
			exit(0);
		}
		int err = execve(exec_cmd->argv[0], exec_cmd->argv, NULL);
		err_exit(err, "execve error");
		// not breakthrough
	case CMD_REDIR:
		break;
	case CMD_PIPE:
		break;
	case CMD_LIST:
		break;
	case CMD_BACK:
		break;
	}
}