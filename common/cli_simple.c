// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#অন্তর্ভাব <bootretry.h>
#অন্তর্ভাব <cli.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

পূর্ণাঙ্ক cli_simple_process_macros(ধ্রুব বর্ণ *প্রবেশ, বর্ণ *output, পূর্ণাঙ্ক max_size)
{
	বর্ণ c, prev;
	ধ্রুব বর্ণ *varname_start = রিক্ত;
	পূর্ণাঙ্ক inputcnt = strlen(প্রবেশ);
	পূর্ণাঙ্ক outputcnt = max_size;
	পূর্ণাঙ্ক state = 0;		/* 0 = waiting for '$'  */
	পূর্ণাঙ্ক ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	বর্ণ __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(প্রবেশ),
		     প্রবেশ);

	prev = '\0';		/* previous character   */

	যাবত্ (inputcnt && outputcnt) {
		c = *প্রবেশ++;
		inputcnt--;

		যদি (state != 3) {
			/* remove one level of escape characters */
			যদি ((c == '\\') && (prev != '\\')) {
				যদি (inputcnt-- == 0)
					ভঙ্গ;
				prev = c;
				c = *প্রবেশ++;
			}
		}

		বিকল্প (state) {
		পক্ষ 0:	/* Waiting for (unescaped) $    */
			যদি ((c == '\'') && (prev != '\\')) {
				state = 3;
				ভঙ্গ;
			}
			যদি ((c == '$') && (prev != '\\')) {
				state++;
			} অন্যথা {
				*(output++) = c;
				outputcnt--;
			}
			ভঙ্গ;
		পক্ষ 1:	/* Waiting for (        */
			যদি (c == '(' || c == '{') {
				state++;
				varname_start = প্রবেশ;
			} অন্যথা {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				যদি (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			ভঙ্গ;
		পক্ষ 2:	/* Waiting for )        */
			যদি (c == ')' || c == '}') {
				পূর্ণাঙ্ক i;
				বর্ণ envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				পূর্ণাঙ্ক envcnt = প্রবেশ - varname_start - 1;

				/* Get the varname */
				প্রতি (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				যদি (envval != রিক্ত)
					যাবত্ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			ভঙ্গ;
		পক্ষ 3:	/* Waiting for '        */
			যদি ((c == '\'') && (prev != '\\')) {
				state = 0;
			} অন্যথা {
				*(output++) = c;
				outputcnt--;
			}
			ভঙ্গ;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	যদি (outputcnt) {
		*output = 0;
	} অন্যথা {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	প্রত্যাগম ret;
}

#ifdef CONFIG_CMDLINE
পূর্ণাঙ্ক cli_simple_parse_সারি(বর্ণ *সারি, বর্ণ *argv[])
{
	পূর্ণাঙ্ক nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, সারি);
	যাবত্ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		যাবত্ (isblank(*সারি))
			++সারি;

		যদি (*সারি == '\0') {	/* end of line, no more args	*/
			argv[nargs] = রিক্ত;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			প্রত্যাগম nargs;
		}

		argv[nargs++] = সারি;	/* begin of argument string	*/

		/* find end of string */
		যাবত্ (*সারি && !isblank(*সারি))
			++সারি;

		যদি (*সারি == '\0') {	/* end of line, no more args	*/
			argv[nargs] = রিক্ত;
			debug_parser("parse_line: nargs=%d\n", nargs);
			প্রত্যাগম nargs;
		}

		*সারি++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	প্রত্যাগম nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
পূর্ণাঙ্ক cli_simple_run_command(ধ্রুব বর্ণ *cmd, পূর্ণাঙ্ক flag)
{
	বর্ণ cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	বর্ণ *token;			/* start of token in cmdbuf	*/
	বর্ণ *sep;			/* end of token (separator) in cmdbuf */
	বর্ণ finaltoken[CONFIG_SYS_CBSIZE];
	বর্ণ *str = cmdbuf;
	বর্ণ *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	পূর্ণাঙ্ক argc, inquotes;
	পূর্ণাঙ্ক repeatable = 1;
	পূর্ণাঙ্ক rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	যদি (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	যদি (!cmd || !*cmd)
		প্রত্যাগম -1;	/* empty command */

	যদি (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		প্রত্যাগম -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	যাবত্ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		প্রতি (inquotes = 0, sep = str; *sep; sep++) {
			যদি ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			যদি (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				ভঙ্গ;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		যদি (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} অন্যথা {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  পরিমাণ(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_সারি(finaltoken, argv);
		যদি (argc == 0) {
			rc = -1;	/* no command at all */
			অনুবর্ত;
		}

		যদি (cmd_process(flag, argc, argv, &repeatable, রিক্ত))
			rc = -1;

		/* Did the user stop this? */
		যদি (had_ctrlc())
			প্রত্যাগম -1;	/* if stopped then not repeatable */
	}

	প্রত্যাগম rc ? rc : repeatable;
}

শূন্য cli_simple_loop(শূন্য)
{
	স্থির বর্ণ lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক flag;
	পূর্ণাঙ্ক rc = 1;

	প্রতি (;;) {
		যদি (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		যদি (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		অন্যথা যদি (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		অন্যথা যদি (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(রিক্ত, 0, 0, রিক্ত);
# অন্যথা
			প্রত্যাগম;		/* retry autoboot */
# endif
		}
#endif

		যদি (len == -1)
			puts("<INTERRUPT>\n");
		অন্যথা
			rc = run_command_repeatable(lastcommand, flag);

		যদি (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

পূর্ণাঙ্ক cli_simple_run_command_list(বর্ণ *cmd, পূর্ণাঙ্ক flag)
{
	বর্ণ *সারি, *next;
	পূর্ণাঙ্ক rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	সারি = cmd;
	যাবত্ (*next) {
		যদি (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			যদি (*সারি) {
				debug("** exec: \"%s\"\n", সারি);
				যদি (cli_simple_run_command(সারি, 0) < 0) {
					rcode = 1;
					ভঙ্গ;
				}
			}
			সারি = next + 1;
		}
		++next;
	}
	যদি (rcode == 0 && *সারি)
		rcode = (cli_simple_run_command(সারি, 0) < 0);

	প্রত্যাগম rcode;
}
#endif
