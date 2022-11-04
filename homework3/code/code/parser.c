//
// Created by bear on 11/4/2022.
//
#include "apue.h"

#include <stdlib.h>

#include "parser.h"

static inline exec_command_t *make_exec_command(void)
{
	exec_command_t *cmd = malloc(sizeof(exec_command_t));
	cmd->base.type = CMD_EXEC;
	return cmd;
}

static inline redir_command_t *make_redir_command(command_t *subcmd, char *file, char *efile, int mode, int fd)
{
	redir_command_t *cmd = malloc(sizeof(redir_command_t));
	cmd->base.type = CMD_REDIR;
	cmd->cmd = subcmd;
	cmd->file = file;
	cmd->efile = efile;
	cmd->mode = mode;
	cmd->fd = fd;
	return cmd;
}

static inline pipe_command_t *make_pipe_command(command_t *left, command_t *right)
{
	pipe_command_t *cmd = malloc(sizeof(pipe_command_t));
	cmd->base.type = CMD_PIPE;
	cmd->left = left;
	cmd->right = right;
	return cmd;
}

static inline list_command_t *make_list_command(command_t *left, command_t *right)
{
	list_command_t *cmd = malloc(sizeof(list_command_t));
	cmd->base.type = CMD_LIST;
	cmd->left = left;
	cmd->right = right;
	return cmd;
}

static inline back_command_t *make_back_command(command_t *subcmd)
{
	back_command_t *cmd = malloc(sizeof(back_command_t));
	cmd->base.type = CMD_BACK;
	cmd->cmd = subcmd;
	return cmd;
}

char *begins[MAXARGS] = {NULL};
char *ends[MAXARGS] = {NULL};

command_t *parse_command(char *cmdline)
{
//	char *cbeg = cmdline, *cend = cmdline;
//	while (*cend)
//		cend++;
//
//	command_t *cmd = parse_line(&cbeg, cend);
//	peek_token(&cbeg, cend, "");
//	if (cbeg != cend)
//	{
//		err_quit("syntax error with leftovers: %s\n", cbeg);
//	}
//	nulterminate(cmd);
//	return cmd;

	int argc = 0;
	char *cbeg = cmdline, *cend = cmdline + strlen(cmdline);
	begins[argc] = cmdline;
	for (char *p = cbeg; p < cend; p++)
	{
		if (p != cbeg && *(p - 1) == ' ' && *p != ' ')
		{
			ends[argc] = p - 1;
			while (ends[argc] > begins[argc] && ends[argc][-1] == ' ')
				ends[argc]--;

			begins[++argc] = p;
		}
	}
	ends[argc] = cend;
	argc++;

	exec_command_t *cmd = make_exec_command();
	for (int i = 0; i < argc; i++)
	{
		cmd->argv[i] = calloc(ends[i] - begins[i] + 1, sizeof(char));
		strncpy(cmd->argv[i], begins[i], ends[i] - begins[i]);
	}
	cmd->argv[argc - 1][strlen(cmd->argv[argc - 1]) - 1] = '\0';
	cmd->argv[argc] = NULL;
	return cmd;
}