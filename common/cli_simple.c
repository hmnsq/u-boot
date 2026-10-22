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

ціле cli_simple_process_macros(const char *input, char *output, ціле max_size)
{
	char c, prev;
	const char *varname_start = NULL;
	ціле inputcnt = strlen(input);
	ціле outputcnt = max_size;
	ціле state = 0;		/* 0 = waiting for '$'  */
	ціле ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	char __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	поки (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		якщо (state != 3) {
			/* remove one level of escape characters */
			якщо ((c == '\\') && (prev != '\\')) {
				якщо (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			якщо ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			якщо ((c == '$') && (prev != '\\')) {
				state++;
			} інакше {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			якщо (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} інакше {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				якщо (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			якщо (c == ')' || c == '}') {
				ціле i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ціле envcnt = input - varname_start - 1;

				/* Get the varname */
				для (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				якщо (envval != NULL)
					поки ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			якщо ((c == '\'') && (prev != '\\')) {
				state = 0;
			} інакше {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	якщо (outputcnt) {
		*output = 0;
	} інакше {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	повернути ret;
}

#ifdef CONFIG_CMDLINE
ціле cli_simple_parse_line(char *line, char *argv[])
{
	ціле nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	поки (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		поки (isblank(*line))
			++line;

		якщо (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			повернути nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		поки (*line && !isblank(*line))
			++line;

		якщо (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			повернути nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	повернути nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ціле cli_simple_run_command(const char *cmd, ціле flag)
{
	char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CONFIG_SYS_CBSIZE];
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ціле argc, inquotes;
	ціле repeatable = 1;
	ціле rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	якщо (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	якщо (!cmd || !*cmd)
		повернути -1;	/* empty command */

	якщо (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		повернути -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	поки (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		для (inquotes = 0, sep = str; *sep; sep++) {
			якщо ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			якщо (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		якщо (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} інакше {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  sizeof(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		якщо (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		якщо (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		якщо (had_ctrlc())
			повернути -1;	/* if stopped then not repeatable */
	}

	повернути rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ціле len;
	ціле flag;
	ціле rc = 1;

	для (;;) {
		якщо (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		якщо (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		інакше якщо (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		інакше якщо (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# інакше
			повернути;		/* retry autoboot */
# endif
		}
#endif

		якщо (len == -1)
			puts("<INTERRUPT>\n");
		інакше
			rc = run_command_repeatable(lastcommand, flag);

		якщо (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ціле cli_simple_run_command_list(char *cmd, ціле flag)
{
	char *line, *next;
	ціле rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	поки (*next) {
		якщо (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			якщо (*line) {
				debug("** exec: \"%s\"\n", line);
				якщо (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					break;
				}
			}
			line = next + 1;
		}
		++next;
	}
	якщо (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	повернути rcode;
}
#endif
