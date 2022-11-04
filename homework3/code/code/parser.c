//
// Created by bear on 11/4/2022.
//
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

command_t *parse_command(char *cmdline)
{

}