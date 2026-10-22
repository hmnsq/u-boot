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

số_nguyên cli_simple_process_macros(const char *input, char *output, số_nguyên max_size)
{
	char c, prev;
	const char *varname_start = NULL;
	số_nguyên inputcnt = strlen(input);
	số_nguyên outputcnt = max_size;
	số_nguyên state = 0;		/* 0 = waiting for '$'  */
	số_nguyên ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	char __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	trong_khi (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		nếu (state != 3) {
			/* remove one level of escape characters */
			nếu ((c == '\\') && (prev != '\\')) {
				nếu (inputcnt-- == 0)
					break;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		case 0:	/* Waiting for (unescaped) $    */
			nếu ((c == '\'') && (prev != '\\')) {
				state = 3;
				break;
			}
			nếu ((c == '$') && (prev != '\\')) {
				state++;
			} không_thì {
				*(output++) = c;
				outputcnt--;
			}
			break;
		case 1:	/* Waiting for (        */
			nếu (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} không_thì {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				nếu (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			break;
		case 2:	/* Waiting for )        */
			nếu (c == ')' || c == '}') {
				số_nguyên i;
				char envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				số_nguyên envcnt = input - varname_start - 1;

				/* Get the varname */
				cho (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				nếu (envval != NULL)
					trong_khi ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			break;
		case 3:	/* Waiting for '        */
			nếu ((c == '\'') && (prev != '\\')) {
				state = 0;
			} không_thì {
				*(output++) = c;
				outputcnt--;
			}
			break;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	nếu (outputcnt) {
		*output = 0;
	} không_thì {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	trả_về ret;
}

#ifdef CONFIG_CMDLINE
số_nguyên cli_simple_parse_line(char *line, char *argv[])
{
	số_nguyên nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	trong_khi (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		trong_khi (isblank(*line))
			++line;

		nếu (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			trả_về nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		trong_khi (*line && !isblank(*line))
			++line;

		nếu (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			trả_về nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	trả_về nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
số_nguyên cli_simple_run_command(const char *cmd, số_nguyên flag)
{
	char cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CONFIG_SYS_CBSIZE];
	char *str = cmdbuf;
	char *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	số_nguyên argc, inquotes;
	số_nguyên repeatable = 1;
	số_nguyên rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	nếu (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	nếu (!cmd || !*cmd)
		trả_về -1;	/* empty command */

	nếu (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		trả_về -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	trong_khi (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		cho (inquotes = 0, sep = str; *sep; sep++) {
			nếu ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			nếu (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		nếu (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} không_thì {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  sizeof(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		nếu (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		nếu (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		nếu (had_ctrlc())
			trả_về -1;	/* if stopped then not repeatable */
	}

	trả_về rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	số_nguyên len;
	số_nguyên flag;
	số_nguyên rc = 1;

	cho (;;) {
		nếu (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		nếu (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		không_thì nếu (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		không_thì nếu (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# không_thì
			trả_về;		/* retry autoboot */
# endif
		}
#endif

		nếu (len == -1)
			puts("<INTERRUPT>\n");
		không_thì
			rc = run_command_repeatable(lastcommand, flag);

		nếu (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

số_nguyên cli_simple_run_command_list(char *cmd, số_nguyên flag)
{
	char *line, *next;
	số_nguyên rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	trong_khi (*next) {
		nếu (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			nếu (*line) {
				debug("** exec: \"%s\"\n", line);
				nếu (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					break;
				}
			}
			line = next + 1;
		}
		++next;
	}
	nếu (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	trả_về rcode;
}
#endif
