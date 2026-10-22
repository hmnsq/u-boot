// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <bootretry.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_PARSER	0	/* set to 1 to debug */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_parser(fmt, ਦਲੀਲਾਂ...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

ਪੂਰਨ cli_simple_process_macros(ਅਚਲ ਅੱਖਰ *ਨਿਵੇਸ਼, ਅੱਖਰ *output, ਪੂਰਨ max_ਮਾਪ)
{
	ਅੱਖਰ c, prev;
	ਅਚਲ ਅੱਖਰ *varname_ਸ਼ੁਰੂ = ਨਿਰਾ;
	ਪੂਰਨ inputcnt = ਲੜੀ_ਲੰਬਾਈ(ਨਿਵੇਸ਼);
	ਪੂਰਨ outputcnt = max_ਮਾਪ;
	ਪੂਰਨ state = 0;		/* 0 = waiting for '$'  */
	ਪੂਰਨ ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	ਅੱਖਰ __maybe_unused *output_ਸ਼ੁਰੂ = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", ਲੜੀ_ਲੰਬਾਈ(ਨਿਵੇਸ਼),
		     ਨਿਵੇਸ਼);

	prev = '\0';		/* previous character   */

	ਜਦੋਂ (inputcnt && outputcnt) {
		c = *ਨਿਵੇਸ਼++;
		inputcnt--;

		ਜੇ (state != 3) {
			/* remove one level of escape characters */
			ਜੇ ((c == '\\') && (prev != '\\')) {
				ਜੇ (inputcnt-- == 0)
					ਰੋਕੋ;
				prev = c;
				c = *ਨਿਵੇਸ਼++;
			}
		}

		ਚੋਣ (state) {
		ਹਾਲਤ 0:	/* Waiting for (unescaped) $    */
			ਜੇ ((c == '\'') && (prev != '\\')) {
				state = 3;
				ਰੋਕੋ;
			}
			ਜੇ ((c == '$') && (prev != '\\')) {
				state++;
			} ਨਹੀਂਤਾਂ {
				*(output++) = c;
				outputcnt--;
			}
			ਰੋਕੋ;
		ਹਾਲਤ 1:	/* Waiting for (        */
			ਜੇ (c == '(' || c == '{') {
				state++;
				varname_ਸ਼ੁਰੂ = ਨਿਵੇਸ਼;
			} ਨਹੀਂਤਾਂ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ਜੇ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			ਰੋਕੋ;
		ਹਾਲਤ 2:	/* Waiting for )        */
			ਜੇ (c == ')' || c == '}') {
				ਪੂਰਨ i;
				ਅੱਖਰ envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ਪੂਰਨ envcnt = ਨਿਵੇਸ਼ - varname_ਸ਼ੁਰੂ - 1;

				/* Get the varname */
				ਲਈ (i = 0; i < envcnt; i++)
					envname[i] = varname_ਸ਼ੁਰੂ[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ਜੇ (envval != ਨਿਰਾ)
					ਜਦੋਂ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			ਰੋਕੋ;
		ਹਾਲਤ 3:	/* Waiting for '        */
			ਜੇ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} ਨਹੀਂਤਾਂ {
				*(output++) = c;
				outputcnt--;
			}
			ਰੋਕੋ;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ਜੇ (outputcnt) {
		*output = 0;
	} ਨਹੀਂਤਾਂ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     ਲੜੀ_ਲੰਬਾਈ(output_ਸ਼ੁਰੂ), output_ਸ਼ੁਰੂ);

	ਵਾਪਸ ret;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
ਪੂਰਨ cli_simple_parse_ਸਤਰ(ਅੱਖਰ *ਸਤਰ, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, ਸਤਰ);
	ਜਦੋਂ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ਜਦੋਂ (isblank(*ਸਤਰ))
			++ਸਤਰ;

		ਜੇ (*ਸਤਰ == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ਨਿਰਾ;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ਵਾਪਸ nargs;
		}

		argv[nargs++] = ਸਤਰ;	/* begin of argument string	*/

		/* find end of string */
		ਜਦੋਂ (*ਸਤਰ && !isblank(*ਸਤਰ))
			++ਸਤਰ;

		ਜੇ (*ਸਤਰ == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ਨਿਰਾ;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ਵਾਪਸ nargs;
		}

		*ਸਤਰ++ = '\0';		/* terminate current arg	 */
	}

	ਲਿਖੋ("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ਵਾਪਸ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ਪੂਰਨ cli_simple_ਲੜੀਵਾਰ_command(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ flag)
{
	ਅੱਖਰ cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	ਅੱਖਰ *token;			/* start of token in cmdbuf	*/
	ਅੱਖਰ *sep;			/* end of token (separator) in cmdbuf */
	ਅੱਖਰ finaltoken[CONFIG_SYS_CBSIZE];
	ਅੱਖਰ *str = cmdbuf;
	ਅੱਖਰ *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ਪੂਰਨ argc, inquotes;
	ਪੂਰਨ repeatable = 1;
	ਪੂਰਨ rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ਜੇ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ਜੇ (!cmd || !*cmd)
		ਵਾਪਸ -1;	/* empty command */

	ਜੇ (ਲੜੀ_ਲੰਬਾਈ(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ਵਾਪਸ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ਜਦੋਂ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		ਲਈ (inquotes = 0, sep = str; *sep; sep++) {
			ਜੇ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ਜੇ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				ਰੋਕੋ;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ਜੇ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ਨਹੀਂਤਾਂ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  ਆਕਾਰ(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_ਸਤਰ(finaltoken, argv);
		ਜੇ (argc == 0) {
			rc = -1;	/* no command at all */
			ਜਾਰੀ;
		}

		ਜੇ (cmd_process(flag, argc, argv, &repeatable, ਨਿਰਾ))
			rc = -1;

		/* Did the user stop this? */
		ਜੇ (had_ctrlc())
			ਵਾਪਸ -1;	/* if stopped then not repeatable */
	}

	ਵਾਪਸ rc ? rc : repeatable;
}

ਖਾਲੀ cli_simple_loop(ਖਾਲੀ)
{
	ਸਥਿਰ ਅੱਖਰ lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ਪੂਰਨ len;
	ਪੂਰਨ flag;
	ਪੂਰਨ rc = 1;

	ਲਈ (;;) {
		ਜੇ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ਜੇ (len > 0)
			strlcpy(lastcommand, console_ਬਫਰ,
				CONFIG_SYS_CBSIZE + 1);
		ਨਹੀਂਤਾਂ ਜੇ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BOOT_RETRY_ਸਮਾਂ
		ਨਹੀਂਤਾਂ ਜੇ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_ਮੰਜ਼ਿਲ_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ਨਿਰਾ, 0, 0, ਨਿਰਾ);
# ਨਹੀਂਤਾਂ
			ਵਾਪਸ;		/* retry autoboot */
# endif
		}
#ਜੇ_ਅੰਤ

		ਜੇ (len == -1)
			puts("<INTERRUPT>\n");
		ਨਹੀਂਤਾਂ
			rc = ਲੜੀਵਾਰ_command_repeatable(lastcommand, flag);

		ਜੇ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ਪੂਰਨ cli_simple_ਲੜੀਵਾਰ_command_list(ਅੱਖਰ *cmd, ਪੂਰਨ flag)
{
	ਅੱਖਰ *ਸਤਰ, *ਅਗਲਾ;
	ਪੂਰਨ rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	ਅਗਲਾ = cmd;
	ਸਤਰ = cmd;
	ਜਦੋਂ (*ਅਗਲਾ) {
		ਜੇ (*ਅਗਲਾ == '\n') {
			*ਅਗਲਾ = '\0';
			/* run only non-empty commands */
			ਜੇ (*ਸਤਰ) {
				debug("** exec: \"%s\"\n", ਸਤਰ);
				ਜੇ (cli_simple_ਲੜੀਵਾਰ_command(ਸਤਰ, 0) < 0) {
					rcode = 1;
					ਰੋਕੋ;
				}
			}
			ਸਤਰ = ਅਗਲਾ + 1;
		}
		++ਅਗਲਾ;
	}
	ਜੇ (rcode == 0 && *ਸਤਰ)
		rcode = (cli_simple_ਲੜੀਵਾਰ_command(ਸਤਰ, 0) < 0);

	ਵਾਪਸ rcode;
}
#ਜੇ_ਅੰਤ
