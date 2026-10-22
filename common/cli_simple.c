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

จำนวนเต็ม cli_simple_process_macros(const var_na *input, var_na *output, จำนวนเต็ม max_size)
{
	var_na c, prev;
	const var_na *varname_start = NULL;
	จำนวนเต็ม inputcnt = strlen(input);
	จำนวนเต็ม outputcnt = max_size;
	จำนวนเต็ม state = 0;		/* 0 = waiting for '$'  */
	จำนวนเต็ม ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	var_na __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	ขณะ (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		ถ้า (state != 3) {
			/* remove one level of escape characters */
			ถ้า ((c == '\\') && (prev != '\\')) {
				ถ้า (inputcnt-- == 0)
					bha_nga;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		pak_sa 0:	/* Waiting for (unescaped) $    */
			ถ้า ((c == '\'') && (prev != '\\')) {
				state = 3;
				bha_nga;
			}
			ถ้า ((c == '$') && (prev != '\\')) {
				state++;
			} มิฉะนั้น {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		pak_sa 1:	/* Waiting for (        */
			ถ้า (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} มิฉะนั้น {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ถ้า (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			bha_nga;
		pak_sa 2:	/* Waiting for )        */
			ถ้า (c == ')' || c == '}') {
				จำนวนเต็ม i;
				var_na envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				จำนวนเต็ม envcnt = input - varname_start - 1;

				/* Get the varname */
				สำหรับ (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ถ้า (envval != NULL)
					ขณะ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			bha_nga;
		pak_sa 3:	/* Waiting for '        */
			ถ้า ((c == '\'') && (prev != '\\')) {
				state = 0;
			} มิฉะนั้น {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ถ้า (outputcnt) {
		*output = 0;
	} มิฉะนั้น {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	คืน ret;
}

#ifdef CONFIG_CMDLINE
จำนวนเต็ม cli_simple_parse_line(var_na *line, var_na *argv[])
{
	จำนวนเต็ม nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	ขณะ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ขณะ (isblank(*line))
			++line;

		ถ้า (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			คืน nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		ขณะ (*line && !isblank(*line))
			++line;

		ถ้า (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			คืน nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	คืน nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
จำนวนเต็ม cli_simple_run_command(const var_na *cmd, จำนวนเต็ม flag)
{
	var_na cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	var_na *token;			/* start of token in cmdbuf	*/
	var_na *sep;			/* end of token (separator) in cmdbuf */
	var_na finaltoken[CONFIG_SYS_CBSIZE];
	var_na *str = cmdbuf;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	จำนวนเต็ม argc, inquotes;
	จำนวนเต็ม repeatable = 1;
	จำนวนเต็ม rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ถ้า (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ถ้า (!cmd || !*cmd)
		คืน -1;	/* empty command */

	ถ้า (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		คืน -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ขณะ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		สำหรับ (inquotes = 0, sep = str; *sep; sep++) {
			ถ้า ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ถ้า (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				bha_nga;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ถ้า (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} มิฉะนั้น {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  parimaa_na(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		ถ้า (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		ถ้า (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		ถ้า (had_ctrlc())
			คืน -1;	/* if stopped then not repeatable */
	}

	คืน rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static var_na lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	จำนวนเต็ม len;
	จำนวนเต็ม flag;
	จำนวนเต็ม rc = 1;

	สำหรับ (;;) {
		ถ้า (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ถ้า (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		มิฉะนั้น ถ้า (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		มิฉะนั้น ถ้า (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# มิฉะนั้น
			คืน;		/* retry autoboot */
# endif
		}
#endif

		ถ้า (len == -1)
			puts("<INTERRUPT>\n");
		มิฉะนั้น
			rc = run_command_repeatable(lastcommand, flag);

		ถ้า (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

จำนวนเต็ม cli_simple_run_command_list(var_na *cmd, จำนวนเต็ม flag)
{
	var_na *line, *next;
	จำนวนเต็ม rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	ขณะ (*next) {
		ถ้า (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ถ้า (*line) {
				debug("** exec: \"%s\"\n", line);
				ถ้า (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					bha_nga;
				}
			}
			line = next + 1;
		}
		++next;
	}
	ถ้า (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	คืน rcode;
}
#endif
