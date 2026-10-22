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

	طالما (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		𐤀𐤌 (state != 3) {
			/* remove one level of escape characters */
			𐤀𐤌 ((c == '\\') && (prev != '\\')) {
				𐤀𐤌 (inputcnt-- == 0)
					اكسر;
				prev = c;
				c = *input++;
			}
		}

		بدل (state) {
		حالة 0:	/* Waiting for (unescaped) $    */
			𐤀𐤌 ((c == '\'') && (prev != '\\')) {
				state = 3;
				اكسر;
			}
			𐤀𐤌 ((c == '$') && (prev != '\\')) {
				state++;
			} وإلا {
				*(output++) = c;
				outputcnt--;
			}
			اكسر;
		حالة 1:	/* Waiting for (        */
			𐤀𐤌 (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} وإلا {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				𐤀𐤌 (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			اكسر;
		حالة 2:	/* Waiting for )        */
			𐤀𐤌 (c == ')' || c == '}') {
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
				𐤀𐤌 (envval != عدم)
					طالما ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			اكسر;
		حالة 3:	/* Waiting for '        */
			𐤀𐤌 ((c == '\'') && (prev != '\\')) {
				state = 0;
			} وإلا {
				*(output++) = c;
				outputcnt--;
			}
			اكسر;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	𐤀𐤌 (outputcnt) {
		*output = 0;
	} وإلا {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	أرجع ret;
}

#ifdef CONFIG_CMDLINE
صحيح cli_simple_parse_line(حرف *line, حرف *argv[])
{
	صحيح nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	طالما (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		طالما (isblank(*line))
			++line;

		𐤀𐤌 (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = عدم;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			أرجع nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		طالما (*line && !isblank(*line))
			++line;

		𐤀𐤌 (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = عدم;
			debug_parser("parse_line: nargs=%d\n", nargs);
			أرجع nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	أرجع nargs;
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
	𐤀𐤌 (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	𐤀𐤌 (!cmd || !*cmd)
		أرجع -1;	/* empty command */

	𐤀𐤌 (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		أرجع -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	طالما (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		لكل (inquotes = 0, sep = str; *sep; sep++) {
			𐤀𐤌 ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			𐤀𐤌 (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				اكسر;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		𐤀𐤌 (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} وإلا {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  حجم(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		𐤀𐤌 (argc == 0) {
			rc = -1;	/* no command at all */
			استمر;
		}

		𐤀𐤌 (cmd_process(flag, argc, argv, &repeatable, عدم))
			rc = -1;

		/* Did the user stop this? */
		𐤀𐤌 (had_ctrlc())
			أرجع -1;	/* if stopped then not repeatable */
	}

	أرجع rc ? rc : repeatable;
}

فراغ cli_simple_loop(فراغ)
{
	ساكن حرف lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	صحيح len;
	صحيح flag;
	صحيح rc = 1;

	لكل (;;) {
		𐤀𐤌 (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		𐤀𐤌 (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		وإلا 𐤀𐤌 (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		وإلا 𐤀𐤌 (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(عدم, 0, 0, عدم);
# وإلا
			أرجع;		/* retry autoboot */
# endif
		}
#endif

		𐤀𐤌 (len == -1)
			puts("<INTERRUPT>\n");
		وإلا
			rc = run_command_repeatable(lastcommand, flag);

		𐤀𐤌 (rc <= 0) {
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
	طالما (*next) {
		𐤀𐤌 (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			𐤀𐤌 (*line) {
				debug("** exec: \"%s\"\n", line);
				𐤀𐤌 (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					اكسر;
				}
			}
			line = next + 1;
		}
		++next;
	}
	𐤀𐤌 (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	أرجع rcode;
}
#endif
