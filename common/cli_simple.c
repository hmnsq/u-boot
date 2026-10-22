// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ಅನ್ತರ್ಭಾವ <bootretry.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

ಪೂರ್ಣಾಙ್ಕ cli_simple_process_macros(ಧ್ರುವ ವರ್ಣ *ಒಳಹರಿವು, ವರ್ಣ *output, ಪೂರ್ಣಾಙ್ಕ max_size)
{
	ವರ್ಣ c, prev;
	ಧ್ರುವ ವರ್ಣ *varname_start = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ inputcnt = strlen(ಒಳಹರಿವು);
	ಪೂರ್ಣಾಙ್ಕ outputcnt = max_size;
	ಪೂರ್ಣಾಙ್ಕ state = 0;		/* 0 = waiting for '$'  */
	ಪೂರ್ಣಾಙ್ಕ ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	ವರ್ಣ __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(ಒಳಹರಿವು),
		     ಒಳಹರಿವು);

	prev = '\0';		/* previous character   */

	ಯಾವತ್ (inputcnt && outputcnt) {
		c = *ಒಳಹರಿವು++;
		inputcnt--;

		ಯದಿ (state != 3) {
			/* remove one level of escape characters */
			ಯದಿ ((c == '\\') && (prev != '\\')) {
				ಯದಿ (inputcnt-- == 0)
					ಭಙ್ಗ;
				prev = c;
				c = *ಒಳಹರಿವು++;
			}
		}

		ವಿಕಲ್ಪ (state) {
		ಪಕ್ಷ 0:	/* Waiting for (unescaped) $    */
			ಯದಿ ((c == '\'') && (prev != '\\')) {
				state = 3;
				ಭಙ್ಗ;
			}
			ಯದಿ ((c == '$') && (prev != '\\')) {
				state++;
			} ಅನ್ಯಥಾ {
				*(output++) = c;
				outputcnt--;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ 1:	/* Waiting for (        */
			ಯದಿ (c == '(' || c == '{') {
				state++;
				varname_start = ಒಳಹರಿವು;
			} ಅನ್ಯಥಾ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ಯದಿ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ 2:	/* Waiting for )        */
			ಯದಿ (c == ')' || c == '}') {
				ಪೂರ್ಣಾಙ್ಕ i;
				ವರ್ಣ envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ಪೂರ್ಣಾಙ್ಕ envcnt = ಒಳಹರಿವು - varname_start - 1;

				/* Get the varname */
				ಪ್ರತಿ (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ಯದಿ (envval != ರಿಕ್ತ)
					ಯಾವತ್ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ 3:	/* Waiting for '        */
			ಯದಿ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} ಅನ್ಯಥಾ {
				*(output++) = c;
				outputcnt--;
			}
			ಭಙ್ಗ;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ಯದಿ (outputcnt) {
		*output = 0;
	} ಅನ್ಯಥಾ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ಪ್ರತ್ಯಾಗಮ ret;
}

#ifdef CONFIG_CMDLINE
ಪೂರ್ಣಾಙ್ಕ cli_simple_parse_ಸಾಲು(ವರ್ಣ *ಸಾಲು, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, ಸಾಲು);
	ಯಾವತ್ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ಯಾವತ್ (isblank(*ಸಾಲು))
			++ಸಾಲು;

		ಯದಿ (*ಸಾಲು == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ರಿಕ್ತ;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ಪ್ರತ್ಯಾಗಮ nargs;
		}

		argv[nargs++] = ಸಾಲು;	/* begin of argument string	*/

		/* find end of string */
		ಯಾವತ್ (*ಸಾಲು && !isblank(*ಸಾಲು))
			++ಸಾಲು;

		ಯದಿ (*ಸಾಲು == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ರಿಕ್ತ;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ಪ್ರತ್ಯಾಗಮ nargs;
		}

		*ಸಾಲು++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ಪ್ರತ್ಯಾಗಮ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ಪೂರ್ಣಾಙ್ಕ cli_simple_run_command(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ flag)
{
	ವರ್ಣ cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	ವರ್ಣ *token;			/* start of token in cmdbuf	*/
	ವರ್ಣ *sep;			/* end of token (separator) in cmdbuf */
	ವರ್ಣ finaltoken[CONFIG_SYS_CBSIZE];
	ವರ್ಣ *str = cmdbuf;
	ವರ್ಣ *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ಪೂರ್ಣಾಙ್ಕ argc, inquotes;
	ಪೂರ್ಣಾಙ್ಕ repeatable = 1;
	ಪೂರ್ಣಾಙ್ಕ rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ಯದಿ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ಯದಿ (!cmd || !*cmd)
		ಪ್ರತ್ಯಾಗಮ -1;	/* empty command */

	ಯದಿ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ಪ್ರತ್ಯಾಗಮ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ಯಾವತ್ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		ಪ್ರತಿ (inquotes = 0, sep = str; *sep; sep++) {
			ಯದಿ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ಯದಿ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				ಭಙ್ಗ;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ಯದಿ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ಅನ್ಯಥಾ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  ಪರಿಮಾಣ(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_ಸಾಲು(finaltoken, argv);
		ಯದಿ (argc == 0) {
			rc = -1;	/* no command at all */
			ಅನುವರ್ತ;
		}

		ಯದಿ (cmd_process(flag, argc, argv, &repeatable, ರಿಕ್ತ))
			rc = -1;

		/* Did the user stop this? */
		ಯದಿ (had_ctrlc())
			ಪ್ರತ್ಯಾಗಮ -1;	/* if stopped then not repeatable */
	}

	ಪ್ರತ್ಯಾಗಮ rc ? rc : repeatable;
}

ಶೂನ್ಯ cli_simple_loop(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ವರ್ಣ lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ಪೂರ್ಣಾಙ್ಕ len;
	ಪೂರ್ಣಾಙ್ಕ flag;
	ಪೂರ್ಣಾಙ್ಕ rc = 1;

	ಪ್ರತಿ (;;) {
		ಯದಿ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ಯದಿ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		ಅನ್ಯಥಾ ಯದಿ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		ಅನ್ಯಥಾ ಯದಿ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ರಿಕ್ತ, 0, 0, ರಿಕ್ತ);
# ಅನ್ಯಥಾ
			ಪ್ರತ್ಯಾಗಮ;		/* retry autoboot */
# endif
		}
#endif

		ಯದಿ (len == -1)
			puts("<INTERRUPT>\n");
		ಅನ್ಯಥಾ
			rc = run_command_repeatable(lastcommand, flag);

		ಯದಿ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ಪೂರ್ಣಾಙ್ಕ cli_simple_run_command_list(ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ flag)
{
	ವರ್ಣ *ಸಾಲು, *next;
	ಪೂರ್ಣಾಙ್ಕ rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	ಸಾಲು = cmd;
	ಯಾವತ್ (*next) {
		ಯದಿ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ಯದಿ (*ಸಾಲು) {
				debug("** exec: \"%s\"\n", ಸಾಲು);
				ಯದಿ (cli_simple_run_command(ಸಾಲು, 0) < 0) {
					rcode = 1;
					ಭಙ್ಗ;
				}
			}
			ಸಾಲು = next + 1;
		}
		++next;
	}
	ಯದಿ (rcode == 0 && *ಸಾಲು)
		rcode = (cli_simple_run_command(ಸಾಲು, 0) < 0);

	ಪ್ರತ್ಯಾಗಮ rcode;
}
#endif
