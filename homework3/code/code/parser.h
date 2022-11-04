//
// Created by bear on 11/4/2022.
//

#pragma once

#define MAXARGS 64

typedef enum command_type
{
	CMD_EXEC, CMD_REDIR, CMD_PIPE, CMD_LIST, CMD_BACK
} command_type_t;

typedef struct command
{
	command_type_t type;
} command_t;

typedef struct exec_command
{
	command_t base;
	char *argv[MAXARGS];
	char *eargv[MAXARGS];
} exec_command_t;

typedef struct redir_command
{
	command_t base;
	command_t *cmd;
	char *file;
	char *efile;
	int mode;
	int fd;
} redir_command_t;

typedef struct pipe_command
{
	command_t base;
	command_t *left;
	command_t *right;
} pipe_command_t;

typedef struct list_command
{
	command_t base;
	command_t *left;
	command_t *right;
} list_command_t;

typedef struct back_command
{
	command_t base;
	command_t *cmd;
} back_command_t;

command_t *parse_command(char *cmdline);