// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <log.h>
#include <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

puur_naa_nka cli_simple_process_macros(const var_na *input, var_na *output, puur_naa_nka max_size)
{
	var_na c, prev;
	const var_na *varname_start = NULL;
	puur_naa_nka inputcnt = strlen(input);
	puur_naa_nka outputcnt = max_size;
	puur_naa_nka state = 0;		/* 0 = waiting for '$'  */
	puur_naa_nka ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	var_na __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	ᮞᮨᮓᮨᮜ (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		ᮚᮦᮔ᮪ (state != 3) {
			/* remove one level of escape characters */
			ᮚᮦᮔ᮪ ((c == '\\') && (prev != '\\')) {
				ᮚᮦᮔ᮪ (inputcnt-- == 0)
					bha_nga;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		pak_sa 0:	/* Waiting for (unescaped) $    */
			ᮚᮦᮔ᮪ ((c == '\'') && (prev != '\\')) {
				state = 3;
				bha_nga;
			}
			ᮚᮦᮔ᮪ ((c == '$') && (prev != '\\')) {
				state++;
			} ᮜᮤᮚ {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		pak_sa 1:	/* Waiting for (        */
			ᮚᮦᮔ᮪ (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} ᮜᮤᮚ {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ᮚᮦᮔ᮪ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			bha_nga;
		pak_sa 2:	/* Waiting for )        */
			ᮚᮦᮔ᮪ (c == ')' || c == '}') {
				puur_naa_nka i;
				var_na envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				puur_naa_nka envcnt = input - varname_start - 1;

				/* Get the varname */
				for (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ᮚᮦᮔ᮪ (envval != NULL)
					ᮞᮨᮓᮨᮜ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			bha_nga;
		pak_sa 3:	/* Waiting for '        */
			ᮚᮦᮔ᮪ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} ᮜᮤᮚ {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ᮚᮦᮔ᮪ (outputcnt) {
		*output = 0;
	} ᮜᮤᮚ {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ᮝᮜᮤ ret;
}

#ifdef CONFIG_CMDLINE
puur_naa_nka cli_simple_parse_line(var_na *line, var_na *argv[])
{
	puur_naa_nka nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	ᮞᮨᮓᮨᮜ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ᮞᮨᮓᮨᮜ (isblank(*line))
			++line;

		ᮚᮦᮔ᮪ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ᮝᮜᮤ nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		ᮞᮨᮓᮨᮜ (*line && !isblank(*line))
			++line;

		ᮚᮦᮔ᮪ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ᮝᮜᮤ nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ᮝᮜᮤ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
puur_naa_nka cli_simple_run_command(const var_na *cmd, puur_naa_nka flag)
{
	var_na cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	var_na *token;			/* start of token in cmdbuf	*/
	var_na *sep;			/* end of token (separator) in cmdbuf */
	var_na finaltoken[CONFIG_SYS_CBSIZE];
	var_na *str = cmdbuf;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	puur_naa_nka argc, inquotes;
	puur_naa_nka repeatable = 1;
	puur_naa_nka rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ᮚᮦᮔ᮪ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ᮚᮦᮔ᮪ (!cmd || !*cmd)
		ᮝᮜᮤ -1;	/* empty command */

	ᮚᮦᮔ᮪ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ᮝᮜᮤ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ᮞᮨᮓᮨᮜ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			ᮚᮦᮔ᮪ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ᮚᮦᮔ᮪ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				bha_nga;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ᮚᮦᮔ᮪ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ᮜᮤᮚ {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  parimaa_na(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		ᮚᮦᮔ᮪ (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		ᮚᮦᮔ᮪ (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		ᮚᮦᮔ᮪ (had_ctrlc())
			ᮝᮜᮤ -1;	/* if stopped then not repeatable */
	}

	ᮝᮜᮤ rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static var_na lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	puur_naa_nka len;
	puur_naa_nka flag;
	puur_naa_nka rc = 1;

	for (;;) {
		ᮚᮦᮔ᮪ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ᮚᮦᮔ᮪ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		ᮜᮤᮚ ᮚᮦᮔ᮪ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		ᮜᮤᮚ ᮚᮦᮔ᮪ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# ᮜᮤᮚ
			ᮝᮜᮤ;		/* retry autoboot */
# endif
		}
#endif

		ᮚᮦᮔ᮪ (len == -1)
			puts("<INTERRUPT>\n");
		ᮜᮤᮚ
			rc = run_command_repeatable(lastcommand, flag);

		ᮚᮦᮔ᮪ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

puur_naa_nka cli_simple_run_command_list(var_na *cmd, puur_naa_nka flag)
{
	var_na *line, *next;
	puur_naa_nka rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	ᮞᮨᮓᮨᮜ (*next) {
		ᮚᮦᮔ᮪ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ᮚᮦᮔ᮪ (*line) {
				debug("** exec: \"%s\"\n", line);
				ᮚᮦᮔ᮪ (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					bha_nga;
				}
			}
			line = next + 1;
		}
		++next;
	}
	ᮚᮦᮔ᮪ (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	ᮝᮜᮤ rcode;
}
#endif
