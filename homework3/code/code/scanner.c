//
// Created by bear on 11/4/2022.
//

#include <string.h>

#include "scanner.h"

static const char whitespace[] = " \t\r\n\v";
static const char symbols[] = "<|>&;()";

int get_token(char **ps, char *es, char **q, char **eq)
{
	char *s = *ps;

	while (s < es && strchr(whitespace, *s))
		s++;


	if (q)
	{
		*q = s;
	}

	int ret = *s;
	switch (*s)
	{
	case 0:
		break;
	case '|':
	case '(':
	case ')':
	case ';':
	case '&':
	case '<':
		s++;
		break;
	case '>':
		s++;
		if (*s == '>')
		{
			ret = '+';
			s++;
		}
		break;
	default:
		ret = 'a';
		while (s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
			s++;
		break;
	}
	if (eq)
	{
		*eq = s;
	}

	while (s < es && strchr(whitespace, *s))
		s++;

	*ps = s;
	return ret;
}

int peek(char **ps, char *es, char *toks)
{
	char *s;

	s = *ps;
	while (s < es && strchr(whitespace, *s))
		s++;
	*ps = s;
	return *s && strchr(toks, *s);
}
