#include "apue.h"

#include <stdio.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/wait.h>

#include "parser.h"
#include "scanner.h"
#include "findcmd.h"

extern char **environ;

#define BUFMAX 4096

char cmd_buf[BUFMAX];

int get_command(char *buf, int nbuf);
void run_command(command_t *cmd);

int fork_panic()
{
	int pid = fork();
	if (pid < 0)
	{
		err_exit(pid, "fork");
	}
	return pid;
}

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
				if (WEXITSTATUS(status) != 0)
				{
					printf("Process exit with status %d.\n", WEXITSTATUS(status));
				}
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
	int p[2];
	int err;
	back_command_t *bcmd;
	exec_command_t *ecmd;
	list_command_t *lcmd;
	pipe_command_t *pcmd;
	redir_command_t *rcmd;

	if (cmd == 0)
	{
		exit(0);
	}

	switch (cmd->type)
	{
	default:
		err_quit("run_command");

	case CMD_EXEC:
		ecmd = (exec_command_t *)cmd;
		if (ecmd->argv[0] == 0)
		{
			exit(0);
		}
		char *name = find_in_path(ecmd->argv[0], getenv("PATH"));
		err = execve(name, ecmd->argv, environ);
		err_exit(err, "exec %s failed\n", ecmd->argv[0]);
		break;

	case CMD_REDIR:
		rcmd = (redir_command_t *)cmd;
		close(rcmd->fd);
		err = open(rcmd->file, rcmd->mode);
		if (err < 0)
		{
			err_exit(err, "open %s failed\n", rcmd->file);
		}
		run_command(rcmd->cmd);
		break;

	case CMD_LIST:
		lcmd = (list_command_t *)cmd;
		if (fork_panic() == 0)
		{
			run_command(lcmd->left);
		}
		wait(NULL);
		run_command(lcmd->right);
		break;

	case CMD_PIPE:
		pcmd = (pipe_command_t *)cmd;
		if (pipe(p) < 0)
		{
			err_quit("pipe");
		}
		if (fork_panic() == 0)
		{
			close(1);
			dup(p[1]);
			close(p[0]);
			close(p[1]);
			run_command(pcmd->left);
		}
		if (fork_panic() == 0)
		{
			close(0);
			dup(p[0]);
			close(p[0]);
			close(p[1]);
			run_command(pcmd->right);
		}
		close(p[0]);
		close(p[1]);
		wait(NULL);
		wait(NULL);
		break;

	case CMD_BACK:
		bcmd = (back_command_t *)cmd;
		if (fork_panic() == 0)
		{
			run_command(bcmd->cmd);
		}
		break;
	}
	exit(0);
}