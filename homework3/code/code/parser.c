//
// Created by bear on 11/4/2022.
//
#include "apue.h"

#include <stdlib.h>
#include <fcntl.h>

#include "parser.h"
#include "scanner.h"

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

command_t *parse_line(char **ps, char *es);
command_t *parse_pipe(char **ps, char *es);
command_t *parse_exec(char **ps, char *es);
command_t *parse_redirs(command_t *cmd, char **ps, char *es);
command_t *ensure_nulterm(command_t *cmd);

command_t *
parse_line(char **ps, char *es)
{
	command_t *cmd;

	cmd = parse_pipe(ps, es);
	while (peek(ps, es, "&"))
	{
		get_token(ps, es, 0, 0);
		cmd = (command_t *)make_back_command(cmd);
	}
	if (peek(ps, es, ";"))
	{
		get_token(ps, es, 0, 0);
		cmd = (command_t *)make_list_command(cmd, parse_line(ps, es));
	}
	return cmd;
}

command_t *
parse_pipe(char **ps, char *es)
{
	command_t *cmd;

	cmd = parse_exec(ps, es);
	if (peek(ps, es, "|"))
	{
		get_token(ps, es, 0, 0);
		cmd = (command_t *)make_pipe_command(cmd, parse_pipe(ps, es));
	}
	return cmd;
}

command_t *
parse_redirs(command_t *cmd, char **ps, char *es)
{
	int tok;
	char *q, *eq;

	while (peek(ps, es, "<>"))
	{
		tok = get_token(ps, es, 0, 0);
		if (get_token(ps, es, &q, &eq) != 'a')
		{
			err_quit("missing file for redirection");
		}
		switch (tok)
		{
		case '<':
			cmd = (command_t *)make_redir_command(cmd, q, eq, O_RDONLY, STDIN_FILENO);
			break;
		case '>':
			cmd = (command_t *)make_redir_command(cmd, q, eq, O_WRONLY | O_CREAT, STDOUT_FILENO);
			break;
		case '+':  // >>
			cmd = (command_t *)make_redir_command(cmd, q, eq, O_WRONLY | O_CREAT, STDOUT_FILENO);
			break;
		}
	}
	return cmd;
}

command_t *
parse_paren(char **ps, char *es)
{
	command_t *cmd;

	if (!peek(ps, es, "("))
	{
		err_quit("parse_paren");
	}
	get_token(ps, es, 0, 0);
	cmd = parse_line(ps, es);
	if (!peek(ps, es, ")"))
	{
		err_quit("syntax - missing )");
	}
	get_token(ps, es, 0, 0);
	cmd = parse_redirs(cmd, ps, es);
	return cmd;
}

command_t *
parse_exec(char **ps, char *es)
{
	char *q, *eq;
	int tok, argc;
	exec_command_t *cmd;
	command_t *ret;

	if (peek(ps, es, "("))
	{
		return parse_paren(ps, es);
	}

	ret = (command_t *)make_exec_command();
	cmd = (exec_command_t *)ret;

	argc = 0;
	ret = parse_redirs(ret, ps, es);
	while (!peek(ps, es, "|)&;"))
	{
		if ((tok = get_token(ps, es, &q, &eq)) == 0)
		{
			break;
		}
		if (tok != 'a')
		{
			err_quit("syntax");
		}
		cmd->argv[argc] = q;
		cmd->eargv[argc] = eq;
		argc++;
		if (argc >= MAXARGS)
		{
			err_quit("too many args");
		}
		ret = parse_redirs(ret, ps, es);
	}
	cmd->argv[argc] = 0;
	cmd->eargv[argc] = 0;
	return ret;
}

// NUL-terminate all the counted strings.
command_t *
ensure_nulterm(command_t *cmd)
{
	int i;
	back_command_t *bcmd;
	exec_command_t *ecmd;
	list_command_t *lcmd;
	pipe_command_t *pcmd;
	redir_command_t *rcmd;

	if (cmd == 0)
	{
		return 0;
	}

	switch (cmd->type)
	{
	case CMD_EXEC:
		ecmd = (exec_command_t *)cmd;
		for (i = 0; ecmd->argv[i]; i++)
			*ecmd->eargv[i] = 0;
		break;

	case CMD_REDIR:
		rcmd = (redir_command_t *)cmd;
		ensure_nulterm(rcmd->cmd);
		*rcmd->efile = 0;
		break;

	case CMD_PIPE:
		pcmd = (pipe_command_t *)cmd;
		ensure_nulterm(pcmd->left);
		ensure_nulterm(pcmd->right);
		break;

	case CMD_LIST:
		lcmd = (list_command_t *)cmd;
		ensure_nulterm(lcmd->left);
		ensure_nulterm(lcmd->right);
		break;

	case CMD_BACK:
		bcmd = (back_command_t *)cmd;
		ensure_nulterm(bcmd->cmd);
		break;
	}
	return cmd;
}

command_t *parse_command(char *cmdline)
{
	char *cbeg = cmdline, *cend = cmdline;
	while (*cend)
		cend++;

	command_t *cmd = parse_line(&cbeg, cend);
	peek(&cbeg, cend, "");
	if (cbeg != cend)
	{
		err_quit("syntax error with leftovers: %s\n", cbeg);
	}
	ensure_nulterm(cmd);
	return cmd;
}
