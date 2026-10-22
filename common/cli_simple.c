// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#અન્તર્ભાવ <bootretry.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

પૂર્ણાંક cli_simple_process_macros(અચલ અક્ષર *આગત, અક્ષર *output, પૂર્ણાંક max_size)
{
	અક્ષર c, prev;
	અચલ અક્ષર *varname_start = ખાલી;
	પૂર્ણાંક inputcnt = strlen(આગત);
	પૂર્ણાંક outputcnt = max_size;
	પૂર્ણાંક state = 0;		/* 0 = waiting for '$'  */
	પૂર્ણાંક ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	અક્ષર __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(આગત),
		     આગત);

	prev = '\0';		/* previous character   */

	જ્યારે (inputcnt && outputcnt) {
		c = *આગત++;
		inputcnt--;

		જો (state != 3) {
			/* remove one level of escape characters */
			જો ((c == '\\') && (prev != '\\')) {
				જો (inputcnt-- == 0)
					તોડો;
				prev = c;
				c = *આગત++;
			}
		}

		પસંદગી (state) {
		સ્થિતિ 0:	/* Waiting for (unescaped) $    */
			જો ((c == '\'') && (prev != '\\')) {
				state = 3;
				તોડો;
			}
			જો ((c == '$') && (prev != '\\')) {
				state++;
			} નહીંતર {
				*(output++) = c;
				outputcnt--;
			}
			તોડો;
		સ્થિતિ 1:	/* Waiting for (        */
			જો (c == '(' || c == '{') {
				state++;
				varname_start = આગત;
			} નહીંતર {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				જો (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			તોડો;
		સ્થિતિ 2:	/* Waiting for )        */
			જો (c == ')' || c == '}') {
				પૂર્ણાંક i;
				અક્ષર envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				પૂર્ણાંક envcnt = આગત - varname_start - 1;

				/* Get the varname */
				માટે (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				જો (envval != ખાલી)
					જ્યારે ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			તોડો;
		સ્થિતિ 3:	/* Waiting for '        */
			જો ((c == '\'') && (prev != '\\')) {
				state = 0;
			} નહીંતર {
				*(output++) = c;
				outputcnt--;
			}
			તોડો;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	જો (outputcnt) {
		*output = 0;
	} નહીંતર {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	પાછા ret;
}

#ifdef CONFIG_CMDLINE
પૂર્ણાંક cli_simple_parse_લીટી(અક્ષર *લીટી, અક્ષર *argv[])
{
	પૂર્ણાંક nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, લીટી);
	જ્યારે (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		જ્યારે (isblank(*લીટી))
			++લીટી;

		જો (*લીટી == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ખાલી;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			પાછા nargs;
		}

		argv[nargs++] = લીટી;	/* begin of argument string	*/

		/* find end of string */
		જ્યારે (*લીટી && !isblank(*લીટી))
			++લીટી;

		જો (*લીટી == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ખાલી;
			debug_parser("parse_line: nargs=%d\n", nargs);
			પાછા nargs;
		}

		*લીટી++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	પાછા nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
પૂર્ણાંક cli_simple_run_command(અચલ અક્ષર *cmd, પૂર્ણાંક flag)
{
	અક્ષર cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	અક્ષર *token;			/* start of token in cmdbuf	*/
	અક્ષર *sep;			/* end of token (separator) in cmdbuf */
	અક્ષર finaltoken[CONFIG_SYS_CBSIZE];
	અક્ષર *str = cmdbuf;
	અક્ષર *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	પૂર્ણાંક argc, inquotes;
	પૂર્ણાંક repeatable = 1;
	પૂર્ણાંક rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	જો (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	જો (!cmd || !*cmd)
		પાછા -1;	/* empty command */

	જો (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		પાછા -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	જ્યારે (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		માટે (inquotes = 0, sep = str; *sep; sep++) {
			જો ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			જો (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				તોડો;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		જો (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} નહીંતર {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  કદ(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_લીટી(finaltoken, argv);
		જો (argc == 0) {
			rc = -1;	/* no command at all */
			ચાલુ;
		}

		જો (cmd_process(flag, argc, argv, &repeatable, ખાલી))
			rc = -1;

		/* Did the user stop this? */
		જો (had_ctrlc())
			પાછા -1;	/* if stopped then not repeatable */
	}

	પાછા rc ? rc : repeatable;
}

શૂન્ય cli_simple_loop(શૂન્ય)
{
	સ્થિર અક્ષર lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	પૂર્ણાંક len;
	પૂર્ણાંક flag;
	પૂર્ણાંક rc = 1;

	માટે (;;) {
		જો (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		જો (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		નહીંતર જો (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		નહીંતર જો (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ખાલી, 0, 0, ખાલી);
# નહીંતર
			પાછા;		/* retry autoboot */
# endif
		}
#endif

		જો (len == -1)
			puts("<INTERRUPT>\n");
		નહીંતર
			rc = run_command_repeatable(lastcommand, flag);

		જો (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

પૂર્ણાંક cli_simple_run_command_list(અક્ષર *cmd, પૂર્ણાંક flag)
{
	અક્ષર *લીટી, *next;
	પૂર્ણાંક rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	લીટી = cmd;
	જ્યારે (*next) {
		જો (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			જો (*લીટી) {
				debug("** exec: \"%s\"\n", લીટી);
				જો (cli_simple_run_command(લીટી, 0) < 0) {
					rcode = 1;
					તોડો;
				}
			}
			લીટી = next + 1;
		}
		++next;
	}
	જો (rcode == 0 && *લીટી)
		rcode = (cli_simple_run_command(લીટી, 0) < 0);

	પાછા rcode;
}
#endif
