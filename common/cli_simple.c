// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#അന്തര്ഭാവ <bootretry.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

പൂർണ്ണസംഖ്യ cli_simple_process_macros(സ്ഥിരം അക്ഷരം *നിവേശം, അക്ഷരം *output, പൂർണ്ണസംഖ്യ max_size)
{
	അക്ഷരം c, prev;
	സ്ഥിരം അക്ഷരം *varname_start = ശൂന്യ;
	പൂർണ്ണസംഖ്യ inputcnt = strlen(നിവേശം);
	പൂർണ്ണസംഖ്യ outputcnt = max_size;
	പൂർണ്ണസംഖ്യ state = 0;		/* 0 = waiting for '$'  */
	പൂർണ്ണസംഖ്യ ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	അക്ഷരം __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(നിവേശം),
		     നിവേശം);

	prev = '\0';		/* previous character   */

	ആകുമ്പോൾ (inputcnt && outputcnt) {
		c = *നിവേശം++;
		inputcnt--;

		എങ്കിൽ (state != 3) {
			/* remove one level of escape characters */
			എങ്കിൽ ((c == '\\') && (prev != '\\')) {
				എങ്കിൽ (inputcnt-- == 0)
					നിർത്തുക;
				prev = c;
				c = *നിവേശം++;
			}
		}

		തിരഞ്ഞെടുപ്പ് (state) {
		അവസ്ഥ 0:	/* Waiting for (unescaped) $    */
			എങ്കിൽ ((c == '\'') && (prev != '\\')) {
				state = 3;
				നിർത്തുക;
			}
			എങ്കിൽ ((c == '$') && (prev != '\\')) {
				state++;
			} അല്ലെങ്കിൽ {
				*(output++) = c;
				outputcnt--;
			}
			നിർത്തുക;
		അവസ്ഥ 1:	/* Waiting for (        */
			എങ്കിൽ (c == '(' || c == '{') {
				state++;
				varname_start = നിവേശം;
			} അല്ലെങ്കിൽ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				എങ്കിൽ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			നിർത്തുക;
		അവസ്ഥ 2:	/* Waiting for )        */
			എങ്കിൽ (c == ')' || c == '}') {
				പൂർണ്ണസംഖ്യ i;
				അക്ഷരം envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				പൂർണ്ണസംഖ്യ envcnt = നിവേശം - varname_start - 1;

				/* Get the varname */
				വേണ്ടി (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				എങ്കിൽ (envval != ശൂന്യ)
					ആകുമ്പോൾ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			നിർത്തുക;
		അവസ്ഥ 3:	/* Waiting for '        */
			എങ്കിൽ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} അല്ലെങ്കിൽ {
				*(output++) = c;
				outputcnt--;
			}
			നിർത്തുക;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	എങ്കിൽ (outputcnt) {
		*output = 0;
	} അല്ലെങ്കിൽ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	തിരികെ ret;
}

#ifdef CONFIG_CMDLINE
പൂർണ്ണസംഖ്യ cli_simple_parse_വരി(അക്ഷരം *വരി, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, വരി);
	ആകുമ്പോൾ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ആകുമ്പോൾ (isblank(*വരി))
			++വരി;

		എങ്കിൽ (*വരി == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ശൂന്യ;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			തിരികെ nargs;
		}

		argv[nargs++] = വരി;	/* begin of argument string	*/

		/* find end of string */
		ആകുമ്പോൾ (*വരി && !isblank(*വരി))
			++വരി;

		എങ്കിൽ (*വരി == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ശൂന്യ;
			debug_parser("parse_line: nargs=%d\n", nargs);
			തിരികെ nargs;
		}

		*വരി++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	തിരികെ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
പൂർണ്ണസംഖ്യ cli_simple_run_command(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ flag)
{
	അക്ഷരം cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	അക്ഷരം *token;			/* start of token in cmdbuf	*/
	അക്ഷരം *sep;			/* end of token (separator) in cmdbuf */
	അക്ഷരം finaltoken[CONFIG_SYS_CBSIZE];
	അക്ഷരം *str = cmdbuf;
	അക്ഷരം *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	പൂർണ്ണസംഖ്യ argc, inquotes;
	പൂർണ്ണസംഖ്യ repeatable = 1;
	പൂർണ്ണസംഖ്യ rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	എങ്കിൽ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	എങ്കിൽ (!cmd || !*cmd)
		തിരികെ -1;	/* empty command */

	എങ്കിൽ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		തിരികെ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ആകുമ്പോൾ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		വേണ്ടി (inquotes = 0, sep = str; *sep; sep++) {
			എങ്കിൽ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			എങ്കിൽ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				നിർത്തുക;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		എങ്കിൽ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} അല്ലെങ്കിൽ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  വലുപ്പം(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_വരി(finaltoken, argv);
		എങ്കിൽ (argc == 0) {
			rc = -1;	/* no command at all */
			തുടരുക;
		}

		എങ്കിൽ (cmd_process(flag, argc, argv, &repeatable, ശൂന്യ))
			rc = -1;

		/* Did the user stop this? */
		എങ്കിൽ (had_ctrlc())
			തിരികെ -1;	/* if stopped then not repeatable */
	}

	തിരികെ rc ? rc : repeatable;
}

ശൂന്യം cli_simple_loop(ശൂന്യം)
{
	സ്ഥിതം അക്ഷരം lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	പൂർണ്ണസംഖ്യ len;
	പൂർണ്ണസംഖ്യ flag;
	പൂർണ്ണസംഖ്യ rc = 1;

	വേണ്ടി (;;) {
		എങ്കിൽ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		എങ്കിൽ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		അല്ലെങ്കിൽ എങ്കിൽ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		അല്ലെങ്കിൽ എങ്കിൽ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ശൂന്യ, 0, 0, ശൂന്യ);
# അല്ലെങ്കിൽ
			തിരികെ;		/* retry autoboot */
# endif
		}
#endif

		എങ്കിൽ (len == -1)
			puts("<INTERRUPT>\n");
		അല്ലെങ്കിൽ
			rc = run_command_repeatable(lastcommand, flag);

		എങ്കിൽ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

പൂർണ്ണസംഖ്യ cli_simple_run_command_list(അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ flag)
{
	അക്ഷരം *വരി, *next;
	പൂർണ്ണസംഖ്യ rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	വരി = cmd;
	ആകുമ്പോൾ (*next) {
		എങ്കിൽ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			എങ്കിൽ (*വരി) {
				debug("** exec: \"%s\"\n", വരി);
				എങ്കിൽ (cli_simple_run_command(വരി, 0) < 0) {
					rcode = 1;
					നിർത്തുക;
				}
			}
			വരി = next + 1;
		}
		++next;
	}
	എങ്കിൽ (rcode == 0 && *വരി)
		rcode = (cli_simple_run_command(വരി, 0) < 0);

	തിരികെ rcode;
}
#endif
