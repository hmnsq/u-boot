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

#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_PARSER	0	/* set to 1 to debug */

#ವ್ಯಾಖ್ಯಾನಿಸು debug_parser(fmt, ವಾದಗಳು...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

ಪೂರ್ಣಾಂಕ cli_simple_process_macros(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಒಳಹರಿವು, ಅಕ್ಷರ *output, ಪೂರ್ಣಾಂಕ max_ಗಾತ್ರ)
{
	ಅಕ್ಷರ c, prev;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *varname_ಆರಂಭ = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ inputcnt = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಒಳಹರಿವು);
	ಪೂರ್ಣಾಂಕ outputcnt = max_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ state = 0;		/* 0 = waiting for '$'  */
	ಪೂರ್ಣಾಂಕ ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	ಅಕ್ಷರ __maybe_unused *output_ಆರಂಭ = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಒಳಹರಿವು),
		     ಒಳಹರಿವು);

	prev = '\0';		/* previous character   */

	ಆಗ (inputcnt && outputcnt) {
		c = *ಒಳಹರಿವು++;
		inputcnt--;

		ಆದರೆ (state != 3) {
			/* remove one level of escape characters */
			ಆದರೆ ((c == '\\') && (prev != '\\')) {
				ಆದರೆ (inputcnt-- == 0)
					ನಿಲ್ಲಿಸು;
				prev = c;
				c = *ಒಳಹರಿವು++;
			}
		}

		ಆಯ್ಕೆ (state) {
		ಸ್ಥಿತಿ 0:	/* Waiting for (unescaped) $    */
			ಆದರೆ ((c == '\'') && (prev != '\\')) {
				state = 3;
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ ((c == '$') && (prev != '\\')) {
				state++;
			} ಇಲ್ಲದಿದ್ದರೆ {
				*(output++) = c;
				outputcnt--;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 1:	/* Waiting for (        */
			ಆದರೆ (c == '(' || c == '{') {
				state++;
				varname_ಆರಂಭ = ಒಳಹರಿವು;
			} ಇಲ್ಲದಿದ್ದರೆ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ಆದರೆ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 2:	/* Waiting for )        */
			ಆದರೆ (c == ')' || c == '}') {
				ಪೂರ್ಣಾಂಕ i;
				ಅಕ್ಷರ envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ಪೂರ್ಣಾಂಕ envcnt = ಒಳಹರಿವು - varname_ಆರಂಭ - 1;

				/* Get the varname */
				ಗಾಗಿ (i = 0; i < envcnt; i++)
					envname[i] = varname_ಆರಂಭ[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ಆದರೆ (envval != ಖಾಲಿ)
					ಆಗ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 3:	/* Waiting for '        */
			ಆದರೆ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} ಇಲ್ಲದಿದ್ದರೆ {
				*(output++) = c;
				outputcnt--;
			}
			ನಿಲ್ಲಿಸು;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ಆದರೆ (outputcnt) {
		*output = 0;
	} ಇಲ್ಲದಿದ್ದರೆ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(output_ಆರಂಭ), output_ಆರಂಭ);

	ಹಿಂತಿರುಗಿ ret;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
ಪೂರ್ಣಾಂಕ cli_simple_parse_ಸಾಲು(ಅಕ್ಷರ *ಸಾಲು, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, ಸಾಲು);
	ಆಗ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ಆಗ (isblank(*ಸಾಲು))
			++ಸಾಲು;

		ಆದರೆ (*ಸಾಲು == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ಖಾಲಿ;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ಹಿಂತಿರುಗಿ nargs;
		}

		argv[nargs++] = ಸಾಲು;	/* begin of argument string	*/

		/* find end of string */
		ಆಗ (*ಸಾಲು && !isblank(*ಸಾಲು))
			++ಸಾಲು;

		ಆದರೆ (*ಸಾಲು == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ಖಾಲಿ;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ಹಿಂತಿರುಗಿ nargs;
		}

		*ಸಾಲು++ = '\0';		/* terminate current arg	 */
	}

	ಬರೆ("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ಹಿಂತಿರುಗಿ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ಪೂರ್ಣಾಂಕ cli_simple_ಸತತ_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ flag)
{
	ಅಕ್ಷರ cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	ಅಕ್ಷರ *token;			/* start of token in cmdbuf	*/
	ಅಕ್ಷರ *sep;			/* end of token (separator) in cmdbuf */
	ಅಕ್ಷರ finaltoken[CONFIG_SYS_CBSIZE];
	ಅಕ್ಷರ *str = cmdbuf;
	ಅಕ್ಷರ *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ಪೂರ್ಣಾಂಕ argc, inquotes;
	ಪೂರ್ಣಾಂಕ repeatable = 1;
	ಪೂರ್ಣಾಂಕ rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ಆದರೆ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ಆದರೆ (!cmd || !*cmd)
		ಹಿಂತಿರುಗಿ -1;	/* empty command */

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ಹಿಂತಿರುಗಿ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ಆಗ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		ಗಾಗಿ (inquotes = 0, sep = str; *sep; sep++) {
			ಆದರೆ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ಆದರೆ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				ನಿಲ್ಲಿಸು;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ಆದರೆ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ಇಲ್ಲದಿದ್ದರೆ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  ಗಾತ್ರ(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_ಸಾಲು(finaltoken, argv);
		ಆದರೆ (argc == 0) {
			rc = -1;	/* no command at all */
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (cmd_process(flag, argc, argv, &repeatable, ಖಾಲಿ))
			rc = -1;

		/* Did the user stop this? */
		ಆದರೆ (had_ctrlc())
			ಹಿಂತಿರುಗಿ -1;	/* if stopped then not repeatable */
	}

	ಹಿಂತಿರುಗಿ rc ? rc : repeatable;
}

ಶೂನ್ಯ cli_simple_loop(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ ಅಕ್ಷರ lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ಪೂರ್ಣಾಂಕ len;
	ಪೂರ್ಣಾಂಕ flag;
	ಪೂರ್ಣಾಂಕ rc = 1;

	ಗಾಗಿ (;;) {
		ಆದರೆ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ಆದರೆ (len > 0)
			strlcpy(lastcommand, console_ಬಫರ್,
				CONFIG_SYS_CBSIZE + 1);
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BOOT_RETRY_ಸಮಯ
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_ಗಮ್ಯ_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ಖಾಲಿ, 0, 0, ಖಾಲಿ);
# ಇಲ್ಲದಿದ್ದರೆ
			ಹಿಂತಿರುಗಿ;		/* retry autoboot */
# endif
		}
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ (len == -1)
			puts("<INTERRUPT>\n");
		ಇಲ್ಲದಿದ್ದರೆ
			rc = ಸತತ_command_repeatable(lastcommand, flag);

		ಆದರೆ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ಪೂರ್ಣಾಂಕ cli_simple_ಸತತ_command_list(ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ flag)
{
	ಅಕ್ಷರ *ಸಾಲು, *ಮುಂದಿನ;
	ಪೂರ್ಣಾಂಕ rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	ಮುಂದಿನ = cmd;
	ಸಾಲು = cmd;
	ಆಗ (*ಮುಂದಿನ) {
		ಆದರೆ (*ಮುಂದಿನ == '\n') {
			*ಮುಂದಿನ = '\0';
			/* run only non-empty commands */
			ಆದರೆ (*ಸಾಲು) {
				debug("** exec: \"%s\"\n", ಸಾಲು);
				ಆದರೆ (cli_simple_ಸತತ_command(ಸಾಲು, 0) < 0) {
					rcode = 1;
					ನಿಲ್ಲಿಸು;
				}
			}
			ಸಾಲು = ಮುಂದಿನ + 1;
		}
		++ಮುಂದಿನ;
	}
	ಆದರೆ (rcode == 0 && *ಸಾಲು)
		rcode = (cli_simple_ಸತತ_command(ಸಾಲು, 0) < 0);

	ಹಿಂತಿರುಗಿ rcode;
}
#ಆದರೆ_ಅಂತ್ಯ
