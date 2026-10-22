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

পূর্ণসংখ্যা cli_simple_process_macros(ধ্রুবক অক্ষর *প্রবেশ, অক্ষর *output, পূর্ণসংখ্যা max_আকার)
{
	অক্ষর c, পূর্ব;
	ধ্রুবক অক্ষর *varname_start = ফাঁকা;
	পূর্ণসংখ্যা inputcnt = strlen(প্রবেশ);
	পূর্ণসংখ্যা outputcnt = max_আকার;
	পূর্ণসংখ্যা state = 0;		/* 0 = waiting for '$'  */
	পূর্ণসংখ্যা ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	অক্ষর __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(প্রবেশ),
		     প্রবেশ);

	পূর্ব = '\0';		/* previous character   */

	যখন (inputcnt && outputcnt) {
		c = *প্রবেশ++;
		inputcnt--;

		যদি (state != 3) {
			/* remove one level of escape characters */
			যদি ((c == '\\') && (পূর্ব != '\\')) {
				যদি (inputcnt-- == 0)
					বিরতি;
				পূর্ব = c;
				c = *প্রবেশ++;
			}
		}

		নির্বাচন (state) {
		ক্ষেত্র 0:	/* Waiting for (unescaped) $    */
			যদি ((c == '\'') && (পূর্ব != '\\')) {
				state = 3;
				বিরতি;
			}
			যদি ((c == '$') && (পূর্ব != '\\')) {
				state++;
			} অন্যথা {
				*(output++) = c;
				outputcnt--;
			}
			বিরতি;
		ক্ষেত্র 1:	/* Waiting for (        */
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
			বিরতি;
		ক্ষেত্র 2:	/* Waiting for )        */
			যদি (c == ')' || c == '}') {
				পূর্ণসংখ্যা i;
				অক্ষর envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				পূর্ণসংখ্যা envcnt = প্রবেশ - varname_start - 1;

				/* Get the varname */
				জন্য (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				যদি (envval != ফাঁকা)
					যখন ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			বিরতি;
		ক্ষেত্র 3:	/* Waiting for '        */
			যদি ((c == '\'') && (পূর্ব != '\\')) {
				state = 0;
			} অন্যথা {
				*(output++) = c;
				outputcnt--;
			}
			বিরতি;
		}
		পূর্ব = c;
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

	ফেরত ret;
}

#ifdef CONFIG_CMDLINE
পূর্ণসংখ্যা cli_simple_parse_সারি(অক্ষর *সারি, অক্ষর *argv[])
{
	পূর্ণসংখ্যা nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, সারি);
	যখন (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		যখন (isblank(*সারি))
			++সারি;

		যদি (*সারি == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ফাঁকা;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ফেরত nargs;
		}

		argv[nargs++] = সারি;	/* begin of argument string	*/

		/* find end of string */
		যখন (*সারি && !isblank(*সারি))
			++সারি;

		যদি (*সারি == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ফাঁকা;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ফেরত nargs;
		}

		*সারি++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ফেরত nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
পূর্ণসংখ্যা cli_simple_run_command(ধ্রুবক অক্ষর *cmd, পূর্ণসংখ্যা flag)
{
	অক্ষর cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	অক্ষর *token;			/* start of token in cmdbuf	*/
	অক্ষর *sep;			/* end of token (separator) in cmdbuf */
	অক্ষর finaltoken[CONFIG_SYS_CBSIZE];
	অক্ষর *str = cmdbuf;
	অক্ষর *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	পূর্ণসংখ্যা argc, inquotes;
	পূর্ণসংখ্যা repeatable = 1;
	পূর্ণসংখ্যা rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	যদি (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	যদি (!cmd || !*cmd)
		ফেরত -1;	/* empty command */

	যদি (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ফেরত -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	যখন (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		জন্য (inquotes = 0, sep = str; *sep; sep++) {
			যদি ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			যদি (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				বিরতি;
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
					  আকার(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_সারি(finaltoken, argv);
		যদি (argc == 0) {
			rc = -1;	/* no command at all */
			চালু;
		}

		যদি (cmd_process(flag, argc, argv, &repeatable, ফাঁকা))
			rc = -1;

		/* Did the user stop this? */
		যদি (had_ctrlc())
			ফেরত -1;	/* if stopped then not repeatable */
	}

	ফেরত rc ? rc : repeatable;
}

শূন্য cli_simple_loop(শূন্য)
{
	স্থির অক্ষর lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	পূর্ণসংখ্যা len;
	পূর্ণসংখ্যা flag;
	পূর্ণসংখ্যা rc = 1;

	জন্য (;;) {
		যদি (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		যদি (len > 0)
			strlcpy(lastcommand, console_বাফার,
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
			do_reset(ফাঁকা, 0, 0, ফাঁকা);
# অন্যথা
			ফেরত;		/* retry autoboot */
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

পূর্ণসংখ্যা cli_simple_run_command_list(অক্ষর *cmd, পূর্ণসংখ্যা flag)
{
	অক্ষর *সারি, *পরবর্তী;
	পূর্ণসংখ্যা rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	পরবর্তী = cmd;
	সারি = cmd;
	যখন (*পরবর্তী) {
		যদি (*পরবর্তী == '\n') {
			*পরবর্তী = '\0';
			/* run only non-empty commands */
			যদি (*সারি) {
				debug("** exec: \"%s\"\n", সারি);
				যদি (cli_simple_run_command(সারি, 0) < 0) {
					rcode = 1;
					বিরতি;
				}
			}
			সারি = পরবর্তী + 1;
		}
		++পরবর্তী;
	}
	যদি (rcode == 0 && *সারি)
		rcode = (cli_simple_run_command(সারি, 0) < 0);

	ফেরত rcode;
}
#endif
