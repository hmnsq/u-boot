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

цяло cli_simple_process_macros(const char *input, char *output, цяло max_size)
{
	char c, prev;
	const char *varname_start = NULL;
	цяло inputcnt = strlen(input);
	цяло outputcnt = max_size;
	цяло state = 0;		/* 0 = waiting for '$'  */
	цяло ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	char __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	докато (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		ако (state != 3) {
			/* remove one level of escape characters */
			ако ((c == '\\') && (prev != '\\')) {
				ако (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			ако ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			ако ((c == '$') && (prev != '\\')) {
				state++;
			} иначе {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			ако (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} иначе {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ако (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			ако (c == ')' || c == '}') {
				цяло i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				цяло envcnt = input - varname_start - 1;

				/* Get the varname */
				за (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ако (envval != NULL)
					докато ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			ако ((c == '\'') && (prev != '\\')) {
				state = 0;
			} иначе {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ако (outputcnt) {
		*output = 0;
	} иначе {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	върни ret;
}

#ifdef CONFIG_CMDLINE
цяло cli_simple_parse_line(char *line, char *argv[])
{
	цяло nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	докато (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		докато (isblank(*line))
			++line;

		ако (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			върни nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		докато (*line && !isblank(*line))
			++line;

		ако (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			върни nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	върни nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
цяло cli_simple_run_command(const char *cmd, цяло flag)
{
	char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CONFIG_SYS_CBSIZE];
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	цяло argc, inquotes;
	цяло repeatable = 1;
	цяло rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ако (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ако (!cmd || !*cmd)
		върни -1;	/* empty command */

	ако (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		върни -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	докато (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		за (inquotes = 0, sep = str; *sep; sep++) {
			ако ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ако (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ако (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} иначе {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  sizeof(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		ако (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		ако (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		ако (had_ctrlc())
			върни -1;	/* if stopped then not repeatable */
	}

	върни rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	цяло len;
	цяло flag;
	цяло rc = 1;

	за (;;) {
		ако (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ако (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		иначе ако (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		иначе ако (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# иначе
			върни;		/* retry autoboot */
# endif
		}
#endif

		ако (len == -1)
			puts("<INTERRUPT>\n");
		иначе
			rc = run_command_repeatable(lastcommand, flag);

		ако (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

цяло cli_simple_run_command_list(char *cmd, цяло flag)
{
	char *line, *next;
	цяло rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	докато (*next) {
		ако (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ако (*line) {
				debug("** exec: \"%s\"\n", line);
				ако (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					break;
				}
			}
			line = next + 1;
		}
		++next;
	}
	ако (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	върни rcode;
}
#endif
