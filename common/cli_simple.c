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

පූර්ණ_සංඛ්‍යා cli_simple_process_macros(const var_na *input, var_na *output, පූර්ණ_සංඛ්‍යා max_size)
{
	var_na c, prev;
	const var_na *varname_start = NULL;
	පූර්ණ_සංඛ්‍යා inputcnt = strlen(input);
	පූර්ණ_සංඛ්‍යා outputcnt = max_size;
	පූර්ණ_සංඛ්‍යා state = 0;		/* 0 = waiting for '$'  */
	පූර්ණ_සංඛ්‍යා ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	var_na __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	අතරතුර (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		නම් (state != 3) {
			/* remove one level of escape characters */
			නම් ((c == '\\') && (prev != '\\')) {
				නම් (inputcnt-- == 0)
					bha_nga;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		pak_sa 0:	/* Waiting for (unescaped) $    */
			නම් ((c == '\'') && (prev != '\\')) {
				state = 3;
				bha_nga;
			}
			නම් ((c == '$') && (prev != '\\')) {
				state++;
			} නැතහොත් {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		pak_sa 1:	/* Waiting for (        */
			නම් (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} නැතහොත් {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				නම් (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			bha_nga;
		pak_sa 2:	/* Waiting for )        */
			නම් (c == ')' || c == '}') {
				පූර්ණ_සංඛ්‍යා i;
				var_na envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				පූර්ණ_සංඛ්‍යා envcnt = input - varname_start - 1;

				/* Get the varname */
				සඳහා (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				නම් (envval != NULL)
					අතරතුර ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			bha_nga;
		pak_sa 3:	/* Waiting for '        */
			නම් ((c == '\'') && (prev != '\\')) {
				state = 0;
			} නැතහොත් {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	නම් (outputcnt) {
		*output = 0;
	} නැතහොත් {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ආපසු ret;
}

#ifdef CONFIG_CMDLINE
පූර්ණ_සංඛ්‍යා cli_simple_parse_line(var_na *line, var_na *argv[])
{
	පූර්ණ_සංඛ්‍යා nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	අතරතුර (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		අතරතුර (isblank(*line))
			++line;

		නම් (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ආපසු nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		අතරතුර (*line && !isblank(*line))
			++line;

		නම් (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ආපසු nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ආපසු nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
පූර්ණ_සංඛ්‍යා cli_simple_run_command(const var_na *cmd, පූර්ණ_සංඛ්‍යා flag)
{
	var_na cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	var_na *token;			/* start of token in cmdbuf	*/
	var_na *sep;			/* end of token (separator) in cmdbuf */
	var_na finaltoken[CONFIG_SYS_CBSIZE];
	var_na *str = cmdbuf;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	පූර්ණ_සංඛ්‍යා argc, inquotes;
	පූර්ණ_සංඛ්‍යා repeatable = 1;
	පූර්ණ_සංඛ්‍යා rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	නම් (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	නම් (!cmd || !*cmd)
		ආපසු -1;	/* empty command */

	නම් (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ආපසු -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	අතරතුර (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		සඳහා (inquotes = 0, sep = str; *sep; sep++) {
			නම් ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			නම් (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				bha_nga;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		නම් (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} නැතහොත් {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  parimaa_na(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		නම් (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		නම් (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		නම් (had_ctrlc())
			ආපසු -1;	/* if stopped then not repeatable */
	}

	ආපසු rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static var_na lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	පූර්ණ_සංඛ්‍යා len;
	පූර්ණ_සංඛ්‍යා flag;
	පූර්ණ_සංඛ්‍යා rc = 1;

	සඳහා (;;) {
		නම් (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		නම් (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		නැතහොත් නම් (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		නැතහොත් නම් (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# නැතහොත්
			ආපසු;		/* retry autoboot */
# endif
		}
#endif

		නම් (len == -1)
			puts("<INTERRUPT>\n");
		නැතහොත්
			rc = run_command_repeatable(lastcommand, flag);

		නම් (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

පූර්ණ_සංඛ්‍යා cli_simple_run_command_list(var_na *cmd, පූර්ණ_සංඛ්‍යා flag)
{
	var_na *line, *next;
	පූර්ණ_සංඛ්‍යා rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	අතරතුර (*next) {
		නම් (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			නම් (*line) {
				debug("** exec: \"%s\"\n", line);
				නම් (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					bha_nga;
				}
			}
			line = next + 1;
		}
		++next;
	}
	නම් (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	ආපසු rcode;
}
#endif
