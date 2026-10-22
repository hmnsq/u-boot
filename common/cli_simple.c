// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <command.h>
#ضمن <console.h>
#ضمن <env.h>
#ضمن <log.h>
#ضمن <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

صحيح cli_simple_process_macros(ثابت حرف *input, حرف *output, صحيح max_size)
{
	حرف c, prev;
	ثابت حرف *varname_start = عدم;
	صحيح inputcnt = strlen(input);
	صحيح outputcnt = max_size;
	صحيح state = 0;		/* 0 = waiting for '$'  */
	صحيح ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	حرف __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	ܟܡܐ (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		ܐܢ (state != 3) {
			/* remove one level of escape characters */
			ܐܢ ((c == '\\') && (prev != '\\')) {
				ܐܢ (inputcnt-- == 0)
					اكسر;
				prev = c;
				c = *input++;
			}
		}

		بدل (state) {
		حالة 0:	/* Waiting for (unescaped) $    */
			ܐܢ ((c == '\'') && (prev != '\\')) {
				state = 3;
				اكسر;
			}
			ܐܢ ((c == '$') && (prev != '\\')) {
				state++;
			} ܐܚܪܢܐ {
				*(output++) = c;
				outputcnt--;
			}
			اكسر;
		حالة 1:	/* Waiting for (        */
			ܐܢ (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} ܐܚܪܢܐ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ܐܢ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			اكسر;
		حالة 2:	/* Waiting for )        */
			ܐܢ (c == ')' || c == '}') {
				صحيح i;
				حرف envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				صحيح envcnt = input - varname_start - 1;

				/* Get the varname */
				لكل (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ܐܢ (envval != عدم)
					ܟܡܐ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			اكسر;
		حالة 3:	/* Waiting for '        */
			ܐܢ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} ܐܚܪܢܐ {
				*(output++) = c;
				outputcnt--;
			}
			اكسر;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ܐܢ (outputcnt) {
		*output = 0;
	} ܐܚܪܢܐ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ܦܢܐ ret;
}

#ifdef CONFIG_CMDLINE
صحيح cli_simple_parse_line(حرف *line, حرف *argv[])
{
	صحيح nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	ܟܡܐ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ܟܡܐ (isblank(*line))
			++line;

		ܐܢ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = عدم;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ܦܢܐ nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		ܟܡܐ (*line && !isblank(*line))
			++line;

		ܐܢ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = عدم;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ܦܢܐ nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ܦܢܐ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
صحيح cli_simple_run_command(ثابت حرف *cmd, صحيح flag)
{
	حرف cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	حرف *token;			/* start of token in cmdbuf	*/
	حرف *sep;			/* end of token (separator) in cmdbuf */
	حرف finaltoken[CONFIG_SYS_CBSIZE];
	حرف *str = cmdbuf;
	حرف *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	صحيح argc, inquotes;
	صحيح repeatable = 1;
	صحيح rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ܐܢ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ܐܢ (!cmd || !*cmd)
		ܦܢܐ -1;	/* empty command */

	ܐܢ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ܦܢܐ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ܟܡܐ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		لكل (inquotes = 0, sep = str; *sep; sep++) {
			ܐܢ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ܐܢ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				اكسر;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ܐܢ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ܐܚܪܢܐ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  حجم(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		ܐܢ (argc == 0) {
			rc = -1;	/* no command at all */
			استمر;
		}

		ܐܢ (cmd_process(flag, argc, argv, &repeatable, عدم))
			rc = -1;

		/* Did the user stop this? */
		ܐܢ (had_ctrlc())
			ܦܢܐ -1;	/* if stopped then not repeatable */
	}

	ܦܢܐ rc ? rc : repeatable;
}

فراغ cli_simple_loop(فراغ)
{
	ساكن حرف lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	صحيح len;
	صحيح flag;
	صحيح rc = 1;

	لكل (;;) {
		ܐܢ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ܐܢ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		ܐܚܪܢܐ ܐܢ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		ܐܚܪܢܐ ܐܢ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(عدم, 0, 0, عدم);
# ܐܚܪܢܐ
			ܦܢܐ;		/* retry autoboot */
# endif
		}
#endif

		ܐܢ (len == -1)
			puts("<INTERRUPT>\n");
		ܐܚܪܢܐ
			rc = run_command_repeatable(lastcommand, flag);

		ܐܢ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

صحيح cli_simple_run_command_list(حرف *cmd, صحيح flag)
{
	حرف *line, *next;
	صحيح rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	ܟܡܐ (*next) {
		ܐܢ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ܐܢ (*line) {
				debug("** exec: \"%s\"\n", line);
				ܐܢ (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					اكسر;
				}
			}
			line = next + 1;
		}
		++next;
	}
	ܐܢ (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	ܦܢܐ rcode;
}
#endif
