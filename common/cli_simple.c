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

целое cli_simple_process_macros(const char *input, char *output, целое max_size)
{
	char c, prev;
	const char *varname_start = NULL;
	целое inputcnt = strlen(input);
	целое outputcnt = max_size;
	целое state = 0;		/* 0 = waiting for '$'  */
	целое ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	char __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	пока (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		если (state != 3) {
			/* remove one level of escape characters */
			если ((c == '\\') && (prev != '\\')) {
				если (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			если ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			если ((c == '$') && (prev != '\\')) {
				state++;
			} иначе {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			если (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} иначе {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				если (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			если (c == ')' || c == '}') {
				целое i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				целое envcnt = input - varname_start - 1;

				/* Get the varname */
				для (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				если (envval != NULL)
					пока ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			если ((c == '\'') && (prev != '\\')) {
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
	если (outputcnt) {
		*output = 0;
	} иначе {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	вернуть ret;
}

#ifdef CONFIG_CMDLINE
целое cli_simple_parse_line(char *line, char *argv[])
{
	целое nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	пока (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		пока (isblank(*line))
			++line;

		если (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			вернуть nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		пока (*line && !isblank(*line))
			++line;

		если (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			вернуть nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	вернуть nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
целое cli_simple_run_command(const char *cmd, целое flag)
{
	char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CONFIG_SYS_CBSIZE];
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	целое argc, inquotes;
	целое repeatable = 1;
	целое rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	если (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	если (!cmd || !*cmd)
		вернуть -1;	/* empty command */

	если (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		вернуть -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	пока (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		для (inquotes = 0, sep = str; *sep; sep++) {
			если ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			если (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		если (*sep) {
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
		если (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		если (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		если (had_ctrlc())
			вернуть -1;	/* if stopped then not repeatable */
	}

	вернуть rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	целое len;
	целое flag;
	целое rc = 1;

	для (;;) {
		если (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		если (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		иначе если (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		иначе если (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# иначе
			вернуть;		/* retry autoboot */
# endif
		}
#endif

		если (len == -1)
			puts("<INTERRUPT>\n");
		иначе
			rc = run_command_repeatable(lastcommand, flag);

		если (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

целое cli_simple_run_command_list(char *cmd, целое flag)
{
	char *line, *next;
	целое rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	пока (*next) {
		если (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			если (*line) {
				debug("** exec: \"%s\"\n", line);
				если (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					break;
				}
			}
			line = next + 1;
		}
		++next;
	}
	если (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	вернуть rcode;
}
#endif
