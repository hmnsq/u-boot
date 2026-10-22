// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#அந்தர்பாவ <bootretry.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

முழு cli_simple_process_macros(மாறிலி எழுத்து *உள்ளீடு, எழுத்து *output, முழு max_size)
{
	எழுத்து c, prev;
	மாறிலி எழுத்து *varname_start = வெற்று;
	முழு inputcnt = strlen(உள்ளீடு);
	முழு outputcnt = max_size;
	முழு state = 0;		/* 0 = waiting for '$'  */
	முழு ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	எழுத்து __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(உள்ளீடு),
		     உள்ளீடு);

	prev = '\0';		/* previous character   */

	போது (inputcnt && outputcnt) {
		c = *உள்ளீடு++;
		inputcnt--;

		எனில் (state != 3) {
			/* remove one level of escape characters */
			எனில் ((c == '\\') && (prev != '\\')) {
				எனில் (inputcnt-- == 0)
					நிறுத்து;
				prev = c;
				c = *உள்ளீடு++;
			}
		}

		தேர்வு (state) {
		நிலை 0:	/* Waiting for (unescaped) $    */
			எனில் ((c == '\'') && (prev != '\\')) {
				state = 3;
				நிறுத்து;
			}
			எனில் ((c == '$') && (prev != '\\')) {
				state++;
			} இல்லையெனில் {
				*(output++) = c;
				outputcnt--;
			}
			நிறுத்து;
		நிலை 1:	/* Waiting for (        */
			எனில் (c == '(' || c == '{') {
				state++;
				varname_start = உள்ளீடு;
			} இல்லையெனில் {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				எனில் (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			நிறுத்து;
		நிலை 2:	/* Waiting for )        */
			எனில் (c == ')' || c == '}') {
				முழு i;
				எழுத்து envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				முழு envcnt = உள்ளீடு - varname_start - 1;

				/* Get the varname */
				க்கு (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				எனில் (envval != வெற்று)
					போது ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			நிறுத்து;
		நிலை 3:	/* Waiting for '        */
			எனில் ((c == '\'') && (prev != '\\')) {
				state = 0;
			} இல்லையெனில் {
				*(output++) = c;
				outputcnt--;
			}
			நிறுத்து;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	எனில் (outputcnt) {
		*output = 0;
	} இல்லையெனில் {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	திரும்பு ret;
}

#ifdef CONFIG_CMDLINE
முழு cli_simple_parse_வரி(எழுத்து *வரி, எழுத்து *argv[])
{
	முழு nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, வரி);
	போது (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		போது (isblank(*வரி))
			++வரி;

		எனில் (*வரி == '\0') {	/* end of line, no more args	*/
			argv[nargs] = வெற்று;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			திரும்பு nargs;
		}

		argv[nargs++] = வரி;	/* begin of argument string	*/

		/* find end of string */
		போது (*வரி && !isblank(*வரி))
			++வரி;

		எனில் (*வரி == '\0') {	/* end of line, no more args	*/
			argv[nargs] = வெற்று;
			debug_parser("parse_line: nargs=%d\n", nargs);
			திரும்பு nargs;
		}

		*வரி++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	திரும்பு nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
முழு cli_simple_run_command(மாறிலி எழுத்து *cmd, முழு flag)
{
	எழுத்து cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	எழுத்து *token;			/* start of token in cmdbuf	*/
	எழுத்து *sep;			/* end of token (separator) in cmdbuf */
	எழுத்து finaltoken[CONFIG_SYS_CBSIZE];
	எழுத்து *str = cmdbuf;
	எழுத்து *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	முழு argc, inquotes;
	முழு repeatable = 1;
	முழு rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	எனில் (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	எனில் (!cmd || !*cmd)
		திரும்பு -1;	/* empty command */

	எனில் (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		திரும்பு -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	போது (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		க்கு (inquotes = 0, sep = str; *sep; sep++) {
			எனில் ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			எனில் (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				நிறுத்து;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		எனில் (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} இல்லையெனில் {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  அளவு(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_வரி(finaltoken, argv);
		எனில் (argc == 0) {
			rc = -1;	/* no command at all */
			தொடர்;
		}

		எனில் (cmd_process(flag, argc, argv, &repeatable, வெற்று))
			rc = -1;

		/* Did the user stop this? */
		எனில் (had_ctrlc())
			திரும்பு -1;	/* if stopped then not repeatable */
	}

	திரும்பு rc ? rc : repeatable;
}

வெறுமை cli_simple_loop(வெறுமை)
{
	நிலையான எழுத்து lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	முழு len;
	முழு flag;
	முழு rc = 1;

	க்கு (;;) {
		எனில் (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		எனில் (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		இல்லையெனில் எனில் (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		இல்லையெனில் எனில் (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(வெற்று, 0, 0, வெற்று);
# இல்லையெனில்
			திரும்பு;		/* retry autoboot */
# endif
		}
#endif

		எனில் (len == -1)
			puts("<INTERRUPT>\n");
		இல்லையெனில்
			rc = run_command_repeatable(lastcommand, flag);

		எனில் (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

முழு cli_simple_run_command_list(எழுத்து *cmd, முழு flag)
{
	எழுத்து *வரி, *next;
	முழு rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	வரி = cmd;
	போது (*next) {
		எனில் (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			எனில் (*வரி) {
				debug("** exec: \"%s\"\n", வரி);
				எனில் (cli_simple_run_command(வரி, 0) < 0) {
					rcode = 1;
					நிறுத்து;
				}
			}
			வரி = next + 1;
		}
		++next;
	}
	எனில் (rcode == 0 && *வரி)
		rcode = (cli_simple_run_command(வரி, 0) < 0);

	திரும்பு rcode;
}
#endif
