// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#అన్తర్భావ <bootretry.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <linux/ctype.h>

#నిర్వచించు DEBUG_PARSER	0	/* set to 1 to debug */

#నిర్వచించు debug_parser(fmt, వాదనలు...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

పూర్ణాంకం cli_simple_process_macros(స్థిరరాశి అక్షరం *ఆదానం, అక్షరం *output, పూర్ణాంకం max_కొలత)
{
	అక్షరం c, prev;
	స్థిరరాశి అక్షరం *varname_ప్రారంభం = ఖాళీ;
	పూర్ణాంకం inputcnt = అక్షరశ్రేణి_పొడవు(ఆదానం);
	పూర్ణాంకం outputcnt = max_కొలత;
	పూర్ణాంకం state = 0;		/* 0 = waiting for '$'  */
	పూర్ణాంకం ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	అక్షరం __maybe_unused *output_ప్రారంభం = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", అక్షరశ్రేణి_పొడవు(ఆదానం),
		     ఆదానం);

	prev = '\0';		/* previous character   */

	అయితే (inputcnt && outputcnt) {
		c = *ఆదానం++;
		inputcnt--;

		ఒకవేళ (state != 3) {
			/* remove one level of escape characters */
			ఒకవేళ ((c == '\\') && (prev != '\\')) {
				ఒకవేళ (inputcnt-- == 0)
					ఆపు;
				prev = c;
				c = *ఆదానం++;
			}
		}

		ఎంపిక (state) {
		స్థితి 0:	/* Waiting for (unescaped) $    */
			ఒకవేళ ((c == '\'') && (prev != '\\')) {
				state = 3;
				ఆపు;
			}
			ఒకవేళ ((c == '$') && (prev != '\\')) {
				state++;
			} లేకపోతే {
				*(output++) = c;
				outputcnt--;
			}
			ఆపు;
		స్థితి 1:	/* Waiting for (        */
			ఒకవేళ (c == '(' || c == '{') {
				state++;
				varname_ప్రారంభం = ఆదానం;
			} లేకపోతే {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ఒకవేళ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			ఆపు;
		స్థితి 2:	/* Waiting for )        */
			ఒకవేళ (c == ')' || c == '}') {
				పూర్ణాంకం i;
				అక్షరం envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				పూర్ణాంకం envcnt = ఆదానం - varname_ప్రారంభం - 1;

				/* Get the varname */
				కొరకు (i = 0; i < envcnt; i++)
					envname[i] = varname_ప్రారంభం[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ఒకవేళ (envval != ఖాళీ)
					అయితే ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			ఆపు;
		స్థితి 3:	/* Waiting for '        */
			ఒకవేళ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} లేకపోతే {
				*(output++) = c;
				outputcnt--;
			}
			ఆపు;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ఒకవేళ (outputcnt) {
		*output = 0;
	} లేకపోతే {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     అక్షరశ్రేణి_పొడవు(output_ప్రారంభం), output_ప్రారంభం);

	తిరిగి ret;
}

#నిర్వచితమైతే CONFIG_CMDLINE
పూర్ణాంకం cli_simple_parse_పంక్తి(అక్షరం *పంక్తి, అక్షరం *argv[])
{
	పూర్ణాంకం nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, పంక్తి);
	అయితే (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		అయితే (isblank(*పంక్తి))
			++పంక్తి;

		ఒకవేళ (*పంక్తి == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ఖాళీ;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			తిరిగి nargs;
		}

		argv[nargs++] = పంక్తి;	/* begin of argument string	*/

		/* find end of string */
		అయితే (*పంక్తి && !isblank(*పంక్తి))
			++పంక్తి;

		ఒకవేళ (*పంక్తి == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ఖాళీ;
			debug_parser("parse_line: nargs=%d\n", nargs);
			తిరిగి nargs;
		}

		*పంక్తి++ = '\0';		/* terminate current arg	 */
	}

	రాయి("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	తిరిగి nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
పూర్ణాంకం cli_simple_నిరంతరం_command(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం flag)
{
	అక్షరం cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	అక్షరం *token;			/* start of token in cmdbuf	*/
	అక్షరం *sep;			/* end of token (separator) in cmdbuf */
	అక్షరం finaltoken[CONFIG_SYS_CBSIZE];
	అక్షరం *str = cmdbuf;
	అక్షరం *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	పూర్ణాంకం argc, inquotes;
	పూర్ణాంకం repeatable = 1;
	పూర్ణాంకం rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ఒకవేళ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ఒకవేళ (!cmd || !*cmd)
		తిరిగి -1;	/* empty command */

	ఒకవేళ (అక్షరశ్రేణి_పొడవు(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		తిరిగి -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	అయితే (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		కొరకు (inquotes = 0, sep = str; *sep; sep++) {
			ఒకవేళ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ఒకవేళ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				ఆపు;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ఒకవేళ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} లేకపోతే {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  పరిమాణం(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_పంక్తి(finaltoken, argv);
		ఒకవేళ (argc == 0) {
			rc = -1;	/* no command at all */
			కొనసాగు;
		}

		ఒకవేళ (cmd_process(flag, argc, argv, &repeatable, ఖాళీ))
			rc = -1;

		/* Did the user stop this? */
		ఒకవేళ (had_ctrlc())
			తిరిగి -1;	/* if stopped then not repeatable */
	}

	తిరిగి rc ? rc : repeatable;
}

శూన్య cli_simple_loop(శూన్య)
{
	స్థిర అక్షరం lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	పూర్ణాంకం len;
	పూర్ణాంకం flag;
	పూర్ణాంకం rc = 1;

	కొరకు (;;) {
		ఒకవేళ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ఒకవేళ (len > 0)
			strlcpy(lastcommand, console_బఫరు,
				CONFIG_SYS_CBSIZE + 1);
		లేకపోతే ఒకవేళ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#నిర్వచితమైతే CONFIG_BOOT_RETRY_సమయం
		లేకపోతే ఒకవేళ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_గమ్యం_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ఖాళీ, 0, 0, ఖాళీ);
# లేకపోతే
			తిరిగి;		/* retry autoboot */
# endif
		}
#అయితే_ముగింపు

		ఒకవేళ (len == -1)
			puts("<INTERRUPT>\n");
		లేకపోతే
			rc = నిరంతరం_command_repeatable(lastcommand, flag);

		ఒకవేళ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

పూర్ణాంకం cli_simple_నిరంతరం_command_list(అక్షరం *cmd, పూర్ణాంకం flag)
{
	అక్షరం *పంక్తి, *తదుపరి;
	పూర్ణాంకం rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	తదుపరి = cmd;
	పంక్తి = cmd;
	అయితే (*తదుపరి) {
		ఒకవేళ (*తదుపరి == '\n') {
			*తదుపరి = '\0';
			/* run only non-empty commands */
			ఒకవేళ (*పంక్తి) {
				debug("** exec: \"%s\"\n", పంక్తి);
				ఒకవేళ (cli_simple_నిరంతరం_command(పంక్తి, 0) < 0) {
					rcode = 1;
					ఆపు;
				}
			}
			పంక్తి = తదుపరి + 1;
		}
		++తదుపరి;
	}
	ఒకవేళ (rcode == 0 && *పంక్తి)
		rcode = (cli_simple_నిరంతరం_command(పంక్తి, 0) < 0);

	తిరిగి rcode;
}
#అయితే_ముగింపు
