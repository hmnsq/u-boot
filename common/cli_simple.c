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

#تعریف_کرو DEBUG_PARSER	0	/* set to 1 to debug */

#تعریف_کرو debug_parser(fmt, دلائل...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

صحیح cli_simple_process_macros(ثابت حرف *داخلہ, حرف *output, صحیح max_سائز)
{
	حرف c, پچھلا;
	ثابت حرف *varname_آغاز = تہی;
	صحیح inputcnt = لڑی_لمبائی(داخلہ);
	صحیح outputcnt = max_سائز;
	صحیح state = 0;		/* 0 = waiting for '$'  */
	صحیح ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	حرف __maybe_unused *output_آغاز = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", لڑی_لمبائی(داخلہ),
		     داخلہ);

	پچھلا = '\0';		/* previous character   */

	جب (inputcnt && outputcnt) {
		c = *داخلہ++;
		inputcnt--;

		اگر (state != 3) {
			/* remove one level of escape characters */
			اگر ((c == '\\') && (پچھلا != '\\')) {
				اگر (inputcnt-- == 0)
					توڑو;
				پچھلا = c;
				c = *داخلہ++;
			}
		}

		انتخاب (state) {
		حالت 0:	/* Waiting for (unescaped) $    */
			اگر ((c == '\'') && (پچھلا != '\\')) {
				state = 3;
				توڑو;
			}
			اگر ((c == '$') && (پچھلا != '\\')) {
				state++;
			} ورنہ {
				*(output++) = c;
				outputcnt--;
			}
			توڑو;
		حالت 1:	/* Waiting for (        */
			اگر (c == '(' || c == '{') {
				state++;
				varname_آغاز = داخلہ;
			} ورنہ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				اگر (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			توڑو;
		حالت 2:	/* Waiting for )        */
			اگر (c == ')' || c == '}') {
				صحیح i;
				حرف envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				صحیح envcnt = داخلہ - varname_آغاز - 1;

				/* Get the varname */
				برائے (i = 0; i < envcnt; i++)
					envname[i] = varname_آغاز[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				اگر (envval != تہی)
					جب ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			توڑو;
		حالت 3:	/* Waiting for '        */
			اگر ((c == '\'') && (پچھلا != '\\')) {
				state = 0;
			} ورنہ {
				*(output++) = c;
				outputcnt--;
			}
			توڑو;
		}
		پچھلا = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	اگر (outputcnt) {
		*output = 0;
	} ورنہ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     لڑی_لمبائی(output_آغاز), output_آغاز);

	واپس ret;
}

#اگر_تعریف_شدہ CONFIG_CMDLINE
صحیح cli_simple_parse_سطر(حرف *سطر, حرف *argv[])
{
	صحیح nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, سطر);
	جب (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		جب (isblank(*سطر))
			++سطر;

		اگر (*سطر == '\0') {	/* end of line, no more args	*/
			argv[nargs] = تہی;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			واپس nargs;
		}

		argv[nargs++] = سطر;	/* begin of argument string	*/

		/* find end of string */
		جب (*سطر && !isblank(*سطر))
			++سطر;

		اگر (*سطر == '\0') {	/* end of line, no more args	*/
			argv[nargs] = تہی;
			debug_parser("parse_line: nargs=%d\n", nargs);
			واپس nargs;
		}

		*سطر++ = '\0';		/* terminate current arg	 */
	}

	لکھو("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	واپس nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
صحیح cli_simple_تسلسل_command(ثابت حرف *cmd, صحیح flag)
{
	حرف cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	حرف *token;			/* start of token in cmdbuf	*/
	حرف *sep;			/* end of token (separator) in cmdbuf */
	حرف finaltoken[CONFIG_SYS_CBSIZE];
	حرف *str = cmdbuf;
	حرف *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
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
		واپس -1;	/* empty command */

	اگر (لڑی_لمبائی(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		واپس -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	جب (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		برائے (inquotes = 0, sep = str; *sep; sep++) {
			اگر ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			اگر (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				توڑو;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		اگر (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ورنہ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  حجم(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_سطر(finaltoken, argv);
		اگر (argc == 0) {
			rc = -1;	/* no command at all */
			جاری;
		}

		اگر (cmd_process(flag, argc, argv, &repeatable, تہی))
			rc = -1;

		/* Did the user stop this? */
		اگر (had_ctrlc())
			واپس -1;	/* if stopped then not repeatable */
	}

	واپس rc ? rc : repeatable;
}

خالی cli_simple_loop(خالی)
{
	ساکن حرف lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	صحیح len;
	صحیح flag;
	صحیح rc = 1;

	برائے (;;) {
		اگر (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		اگر (len > 0)
			strlcpy(lastcommand, console_بفر,
				CONFIG_SYS_CBSIZE + 1);
		ورنہ اگر (len == 0)
			flag |= CMD_FLAG_REPEAT;
#اگر_تعریف_شدہ CONFIG_BOOT_RETRY_وقت
		ورنہ اگر (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_مقصد_RETRY
			/* Reinit board to run initialization code again */
			do_reset(تہی, 0, 0, تہی);
# ورنہ
			واپس;		/* retry autoboot */
# endif
		}
#اگر_ختم

		اگر (len == -1)
			puts("<INTERRUPT>\n");
		ورنہ
			rc = تسلسل_command_repeatable(lastcommand, flag);

		اگر (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

صحیح cli_simple_تسلسل_command_list(حرف *cmd, صحیح flag)
{
	حرف *سطر, *اگلا;
	صحیح rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	اگلا = cmd;
	سطر = cmd;
	جب (*اگلا) {
		اگر (*اگلا == '\n') {
			*اگلا = '\0';
			/* run only non-empty commands */
			اگر (*سطر) {
				debug("** exec: \"%s\"\n", سطر);
				اگر (cli_simple_تسلسل_command(سطر, 0) < 0) {
					rcode = 1;
					توڑو;
				}
			}
			سطر = اگلا + 1;
		}
		++اگلا;
	}
	اگر (rcode == 0 && *سطر)
		rcode = (cli_simple_تسلسل_command(سطر, 0) < 0);

	واپس rcode;
}
#اگر_ختم
