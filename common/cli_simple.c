// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#অন্তর্ভাৱ <bootretry.h>
#অন্তর্ভাৱ <cli.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

পূর্ণাঙ্ক cli_simple_process_macros(ধ্রুৱ ৱর্ণ *প্ৰবেশ, ৱর্ণ *output, পূর্ণাঙ্ক max_size)
{
	ৱর্ণ c, prev;
	ধ্রুৱ ৱর্ণ *varname_start = রিক্ত;
	পূর্ণাঙ্ক inputcnt = strlen(প্ৰবেশ);
	পূর্ণাঙ্ক outputcnt = max_size;
	পূর্ণাঙ্ক state = 0;		/* 0 = waiting for '$'  */
	পূর্ণাঙ্ক ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	ৱর্ণ __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(প্ৰবেশ),
		     প্ৰবেশ);

	prev = '\0';		/* previous character   */

	যেতিয়ালৈ (inputcnt && outputcnt) {
		c = *প্ৰবেশ++;
		inputcnt--;

		যদি (state != 3) {
			/* remove one level of escape characters */
			যদি ((c == '\\') && (prev != '\\')) {
				যদি (inputcnt-- == 0)
					ভঙ্গ;
				prev = c;
				c = *প্ৰবেশ++;
			}
		}

		ৱিকল্প (state) {
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
				varname_start = প্ৰবেশ;
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
				ৱর্ণ envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				পূর্ণাঙ্ক envcnt = প্ৰবেশ - varname_start - 1;

				/* Get the varname */
				বাবে (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				যদি (envval != রিক্ত)
					যেতিয়ালৈ ((*envval) && outputcnt) {
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

	ঘূৰাই ret;
}

#ifdef CONFIG_CMDLINE
পূর্ণাঙ্ক cli_simple_parse_সাৰি(ৱর্ণ *সাৰি, ৱর্ণ *argv[])
{
	পূর্ণাঙ্ক nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, সাৰি);
	যেতিয়ালৈ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		যেতিয়ালৈ (isblank(*সাৰি))
			++সাৰি;

		যদি (*সাৰি == '\0') {	/* end of line, no more args	*/
			argv[nargs] = রিক্ত;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ঘূৰাই nargs;
		}

		argv[nargs++] = সাৰি;	/* begin of argument string	*/

		/* find end of string */
		যেতিয়ালৈ (*সাৰি && !isblank(*সাৰি))
			++সাৰি;

		যদি (*সাৰি == '\0') {	/* end of line, no more args	*/
			argv[nargs] = রিক্ত;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ঘূৰাই nargs;
		}

		*সাৰি++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ঘূৰাই nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
পূর্ণাঙ্ক cli_simple_run_command(ধ্রুৱ ৱর্ণ *cmd, পূর্ণাঙ্ক flag)
{
	ৱর্ণ cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	ৱর্ণ *token;			/* start of token in cmdbuf	*/
	ৱর্ণ *sep;			/* end of token (separator) in cmdbuf */
	ৱর্ণ finaltoken[CONFIG_SYS_CBSIZE];
	ৱর্ণ *str = cmdbuf;
	ৱর্ণ *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
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
		ঘূৰাই -1;	/* empty command */

	যদি (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ঘূৰাই -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	যেতিয়ালৈ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		বাবে (inquotes = 0, sep = str; *sep; sep++) {
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
		argc = cli_simple_parse_সাৰি(finaltoken, argv);
		যদি (argc == 0) {
			rc = -1;	/* no command at all */
			অনুৱর্ত;
		}

		যদি (cmd_process(flag, argc, argv, &repeatable, রিক্ত))
			rc = -1;

		/* Did the user stop this? */
		যদি (had_ctrlc())
			ঘূৰাই -1;	/* if stopped then not repeatable */
	}

	ঘূৰাই rc ? rc : repeatable;
}

শূন্য cli_simple_loop(শূন্য)
{
	স্থির ৱর্ণ lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক flag;
	পূর্ণাঙ্ক rc = 1;

	বাবে (;;) {
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
			ঘূৰাই;		/* retry autoboot */
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

পূর্ণাঙ্ক cli_simple_run_command_list(ৱর্ণ *cmd, পূর্ণাঙ্ক flag)
{
	ৱর্ণ *সাৰি, *next;
	পূর্ণাঙ্ক rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	সাৰি = cmd;
	যেতিয়ালৈ (*next) {
		যদি (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			যদি (*সাৰি) {
				debug("** exec: \"%s\"\n", সাৰি);
				যদি (cli_simple_run_command(সাৰি, 0) < 0) {
					rcode = 1;
					ভঙ্গ;
				}
			}
			সাৰি = next + 1;
		}
		++next;
	}
	যদি (rcode == 0 && *সাৰি)
		rcode = (cli_simple_run_command(সাৰি, 0) < 0);

	ঘূৰাই rcode;
}
#endif
