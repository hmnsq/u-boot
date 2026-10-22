// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <log.h>
#include <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

정수 cli_simple_process_macros(const char *input, char *output, 정수 max_size)
{
	char c, prev;
	const char *varname_start = NULL;
	정수 inputcnt = strlen(input);
	정수 outputcnt = max_size;
	정수 state = 0;		/* 0 = waiting for '$'  */
	정수 ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	char __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	동안 (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		만약 (state != 3) {
			/* remove one level of escape characters */
			만약 ((c == '\\') && (prev != '\\')) {
				만약 (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			만약 ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			만약 ((c == '$') && (prev != '\\')) {
				state++;
			} 아니면 {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			만약 (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} 아니면 {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				만약 (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			만약 (c == ')' || c == '}') {
				정수 i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				정수 envcnt = input - varname_start - 1;

				/* Get the varname */
				위해 (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				만약 (envval != NULL)
					동안 ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			만약 ((c == '\'') && (prev != '\\')) {
				state = 0;
			} 아니면 {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	만약 (outputcnt) {
		*output = 0;
	} 아니면 {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	반환 ret;
}

#ifdef CONFIG_CMDLINE
정수 cli_simple_parse_line(char *line, char *argv[])
{
	정수 nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	동안 (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		동안 (isblank(*line))
			++line;

		만약 (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			반환 nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		동안 (*line && !isblank(*line))
			++line;

		만약 (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			반환 nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	반환 nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
정수 cli_simple_run_command(const char *cmd, 정수 flag)
{
	char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CONFIG_SYS_CBSIZE];
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	정수 argc, inquotes;
	정수 repeatable = 1;
	정수 rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	만약 (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	만약 (!cmd || !*cmd)
		반환 -1;	/* empty command */

	만약 (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		반환 -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	동안 (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		위해 (inquotes = 0, sep = str; *sep; sep++) {
			만약 ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			만약 (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		만약 (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} 아니면 {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  sizeof(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		만약 (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		만약 (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		만약 (had_ctrlc())
			반환 -1;	/* if stopped then not repeatable */
	}

	반환 rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	정수 len;
	정수 flag;
	정수 rc = 1;

	위해 (;;) {
		만약 (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		만약 (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		아니면 만약 (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		아니면 만약 (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# 아니면
			반환;		/* retry autoboot */
# endif
		}
#endif

		만약 (len == -1)
			puts("<INTERRUPT>\n");
		아니면
			rc = run_command_repeatable(lastcommand, flag);

		만약 (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

정수 cli_simple_run_command_list(char *cmd, 정수 flag)
{
	char *line, *next;
	정수 rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	동안 (*next) {
		만약 (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			만약 (*line) {
				debug("** exec: \"%s\"\n", line);
				만약 (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					break;
				}
			}
			line = next + 1;
		}
		++next;
	}
	만약 (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	반환 rcode;
}
#endif
