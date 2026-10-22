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

ចំនួនគត់ cli_simple_process_macros(const var_na *input, var_na *output, ចំនួនគត់ max_size)
{
	var_na c, prev;
	const var_na *varname_start = NULL;
	ចំនួនគត់ inputcnt = strlen(input);
	ចំនួនគត់ outputcnt = max_size;
	ចំនួនគត់ state = 0;		/* 0 = waiting for '$'  */
	ចំនួនគត់ ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	var_na __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	ខណៈពេល (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		ប្រសិនបើ (state != 3) {
			/* remove one level of escape characters */
			ប្រសិនបើ ((c == '\\') && (prev != '\\')) {
				ប្រសិនបើ (inputcnt-- == 0)
					bha_nga;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		pak_sa 0:	/* Waiting for (unescaped) $    */
			ប្រសិនបើ ((c == '\'') && (prev != '\\')) {
				state = 3;
				bha_nga;
			}
			ប្រសិនបើ ((c == '$') && (prev != '\\')) {
				state++;
			} បើមិនដូច្នេះ {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		pak_sa 1:	/* Waiting for (        */
			ប្រសិនបើ (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} បើមិនដូច្នេះ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ប្រសិនបើ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			bha_nga;
		pak_sa 2:	/* Waiting for )        */
			ប្រសិនបើ (c == ')' || c == '}') {
				ចំនួនគត់ i;
				var_na envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ចំនួនគត់ envcnt = input - varname_start - 1;

				/* Get the varname */
				សម្រាប់ (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ប្រសិនបើ (envval != NULL)
					ខណៈពេល ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			bha_nga;
		pak_sa 3:	/* Waiting for '        */
			ប្រសិនបើ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} បើមិនដូច្នេះ {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ប្រសិនបើ (outputcnt) {
		*output = 0;
	} បើមិនដូច្នេះ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ត្រឡប់ ret;
}

#ifdef CONFIG_CMDLINE
ចំនួនគត់ cli_simple_parse_line(var_na *line, var_na *argv[])
{
	ចំនួនគត់ nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	ខណៈពេល (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ខណៈពេល (isblank(*line))
			++line;

		ប្រសិនបើ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ត្រឡប់ nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		ខណៈពេល (*line && !isblank(*line))
			++line;

		ប្រសិនបើ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ត្រឡប់ nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ត្រឡប់ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ចំនួនគត់ cli_simple_run_command(const var_na *cmd, ចំនួនគត់ flag)
{
	var_na cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	var_na *token;			/* start of token in cmdbuf	*/
	var_na *sep;			/* end of token (separator) in cmdbuf */
	var_na finaltoken[CONFIG_SYS_CBSIZE];
	var_na *str = cmdbuf;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ចំនួនគត់ argc, inquotes;
	ចំនួនគត់ repeatable = 1;
	ចំនួនគត់ rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ប្រសិនបើ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ប្រសិនបើ (!cmd || !*cmd)
		ត្រឡប់ -1;	/* empty command */

	ប្រសិនបើ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ត្រឡប់ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ខណៈពេល (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		សម្រាប់ (inquotes = 0, sep = str; *sep; sep++) {
			ប្រសិនបើ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ប្រសិនបើ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				bha_nga;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ប្រសិនបើ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} បើមិនដូច្នេះ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  parimaa_na(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		ប្រសិនបើ (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		ប្រសិនបើ (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		ប្រសិនបើ (had_ctrlc())
			ត្រឡប់ -1;	/* if stopped then not repeatable */
	}

	ត្រឡប់ rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static var_na lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ចំនួនគត់ len;
	ចំនួនគត់ flag;
	ចំនួនគត់ rc = 1;

	សម្រាប់ (;;) {
		ប្រសិនបើ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ប្រសិនបើ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		បើមិនដូច្នេះ ប្រសិនបើ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		បើមិនដូច្នេះ ប្រសិនបើ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# បើមិនដូច្នេះ
			ត្រឡប់;		/* retry autoboot */
# endif
		}
#endif

		ប្រសិនបើ (len == -1)
			puts("<INTERRUPT>\n");
		បើមិនដូច្នេះ
			rc = run_command_repeatable(lastcommand, flag);

		ប្រសិនបើ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ចំនួនគត់ cli_simple_run_command_list(var_na *cmd, ចំនួនគត់ flag)
{
	var_na *line, *next;
	ចំនួនគត់ rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	ខណៈពេល (*next) {
		ប្រសិនបើ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ប្រសិនបើ (*line) {
				debug("** exec: \"%s\"\n", line);
				ប្រសិនបើ (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					bha_nga;
				}
			}
			line = next + 1;
		}
		++next;
	}
	ប្រសិនបើ (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	ត្រឡប់ rcode;
}
#endif
