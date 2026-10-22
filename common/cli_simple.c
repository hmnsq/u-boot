// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#شامل <bootretry.h>
#شامل <cli.h>
#شامل <command.h>
#شامل <console.h>
#شامل <env.h>
#شامل <log.h>
#شامل <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

صحیح cli_simple_process_macros(ثابت نویسه *درون_داد, نویسه *output, صحیح max_size)
{
	نویسه c, prev;
	ثابت نویسه *varname_start = پوچ;
	صحیح inputcnt = strlen(درون_داد);
	صحیح outputcnt = max_size;
	صحیح state = 0;		/* 0 = waiting for '$'  */
	صحیح ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	نویسه __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(درون_داد),
		     درون_داد);

	prev = '\0';		/* previous character   */

	وقتی (inputcnt && outputcnt) {
		c = *درون_داد++;
		inputcnt--;

		اگر (state != 3) {
			/* remove one level of escape characters */
			اگر ((c == '\\') && (prev != '\\')) {
				اگر (inputcnt-- == 0)
					بشکن;
				prev = c;
				c = *درون_داد++;
			}
		}

		انتخاب (state) {
		حالت 0:	/* Waiting for (unescaped) $    */
			اگر ((c == '\'') && (prev != '\\')) {
				state = 3;
				بشکن;
			}
			اگر ((c == '$') && (prev != '\\')) {
				state++;
			} وگرنه {
				*(output++) = c;
				outputcnt--;
			}
			بشکن;
		حالت 1:	/* Waiting for (        */
			اگر (c == '(' || c == '{') {
				state++;
				varname_start = درون_داد;
			} وگرنه {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				اگر (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			بشکن;
		حالت 2:	/* Waiting for )        */
			اگر (c == ')' || c == '}') {
				صحیح i;
				نویسه envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				صحیح envcnt = درون_داد - varname_start - 1;

				/* Get the varname */
				برای (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				اگر (envval != پوچ)
					وقتی ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			بشکن;
		حالت 3:	/* Waiting for '        */
			اگر ((c == '\'') && (prev != '\\')) {
				state = 0;
			} وگرنه {
				*(output++) = c;
				outputcnt--;
			}
			بشکن;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	اگر (outputcnt) {
		*output = 0;
	} وگرنه {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	بازگشت ret;
}

#ifdef CONFIG_CMDLINE
صحیح cli_simple_parse_سطر(نویسه *سطر, نویسه *argv[])
{
	صحیح nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, سطر);
	وقتی (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		وقتی (isblank(*سطر))
			++سطر;

		اگر (*سطر == '\0') {	/* end of line, no more args	*/
			argv[nargs] = پوچ;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			بازگشت nargs;
		}

		argv[nargs++] = سطر;	/* begin of argument string	*/

		/* find end of string */
		وقتی (*سطر && !isblank(*سطر))
			++سطر;

		اگر (*سطر == '\0') {	/* end of line, no more args	*/
			argv[nargs] = پوچ;
			debug_parser("parse_line: nargs=%d\n", nargs);
			بازگشت nargs;
		}

		*سطر++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	بازگشت nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
صحیح cli_simple_run_command(ثابت نویسه *cmd, صحیح flag)
{
	نویسه cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	نویسه *token;			/* start of token in cmdbuf	*/
	نویسه *sep;			/* end of token (separator) in cmdbuf */
	نویسه finaltoken[CONFIG_SYS_CBSIZE];
	نویسه *str = cmdbuf;
	نویسه *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	صحیح argc, inquotes;
	صحیح repeatable = 1;
	صحیح rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	اگر (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	اگر (!cmd || !*cmd)
		بازگشت -1;	/* empty command */

	اگر (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		بازگشت -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	وقتی (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		برای (inquotes = 0, sep = str; *sep; sep++) {
			اگر ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			اگر (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				بشکن;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		اگر (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} وگرنه {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  اندازه(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_سطر(finaltoken, argv);
		اگر (argc == 0) {
			rc = -1;	/* no command at all */
			ادامه;
		}

		اگر (cmd_process(flag, argc, argv, &repeatable, پوچ))
			rc = -1;

		/* Did the user stop this? */
		اگر (had_ctrlc())
			بازگشت -1;	/* if stopped then not repeatable */
	}

	بازگشت rc ? rc : repeatable;
}

تهی cli_simple_loop(تهی)
{
	ایستا نویسه lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	صحیح len;
	صحیح flag;
	صحیح rc = 1;

	برای (;;) {
		اگر (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		اگر (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		وگرنه اگر (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		وگرنه اگر (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(پوچ, 0, 0, پوچ);
# وگرنه
			بازگشت;		/* retry autoboot */
# endif
		}
#endif

		اگر (len == -1)
			puts("<INTERRUPT>\n");
		وگرنه
			rc = run_command_repeatable(lastcommand, flag);

		اگر (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

صحیح cli_simple_run_command_list(نویسه *cmd, صحیح flag)
{
	نویسه *سطر, *next;
	صحیح rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	سطر = cmd;
	وقتی (*next) {
		اگر (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			اگر (*سطر) {
				debug("** exec: \"%s\"\n", سطر);
				اگر (cli_simple_run_command(سطر, 0) < 0) {
					rcode = 1;
					بشکن;
				}
			}
			سطر = next + 1;
		}
		++next;
	}
	اگر (rcode == 0 && *سطر)
		rcode = (cli_simple_run_command(سطر, 0) < 0);

	بازگشت rcode;
}
#endif
