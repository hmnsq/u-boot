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

ကိန်းပြည့် cli_simple_process_macros(const var_na *input, var_na *output, ကိန်းပြည့် max_size)
{
	var_na c, prev;
	const var_na *varname_start = NULL;
	ကိန်းပြည့် inputcnt = strlen(input);
	ကိန်းပြည့် outputcnt = max_size;
	ကိန်းပြည့် state = 0;		/* 0 = waiting for '$'  */
	ကိန်းပြည့် ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	var_na __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(input),
		     input);

	prev = '\0';		/* previous character   */

	နေစဉ် (inputcnt && outputcnt) {
		c = *input++;
		inputcnt--;

		အကယ်၍ (state != 3) {
			/* remove one level of escape characters */
			အကယ်၍ ((c == '\\') && (prev != '\\')) {
				အကယ်၍ (inputcnt-- == 0)
					bha_nga;
				prev = c;
				c = *input++;
			}
		}

		switch (state) {
		pak_sa 0:	/* Waiting for (unescaped) $    */
			အကယ်၍ ((c == '\'') && (prev != '\\')) {
				state = 3;
				bha_nga;
			}
			အကယ်၍ ((c == '$') && (prev != '\\')) {
				state++;
			} မဟုတ်ပါက {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		pak_sa 1:	/* Waiting for (        */
			အကယ်၍ (c == '(' || c == '{') {
				state++;
				varname_start = input;
			} မဟုတ်ပါက {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				အကယ်၍ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			bha_nga;
		pak_sa 2:	/* Waiting for )        */
			အကယ်၍ (c == ')' || c == '}') {
				ကိန်းပြည့် i;
				var_na envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ကိန်းပြည့် envcnt = input - varname_start - 1;

				/* Get the varname */
				အတွက် (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				အကယ်၍ (envval != NULL)
					နေစဉ် ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			bha_nga;
		pak_sa 3:	/* Waiting for '        */
			အကယ်၍ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} မဟုတ်ပါက {
				*(output++) = c;
				outputcnt--;
			}
			bha_nga;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	အကယ်၍ (outputcnt) {
		*output = 0;
	} မဟုတ်ပါက {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ပြန်ပေး ret;
}

#ifdef CONFIG_CMDLINE
ကိန်းပြည့် cli_simple_parse_line(var_na *line, var_na *argv[])
{
	ကိန်းပြည့် nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, line);
	နေစဉ် (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		နေစဉ် (isblank(*line))
			++line;

		အကယ်၍ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ပြန်ပေး nargs;
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		နေစဉ် (*line && !isblank(*line))
			++line;

		အကယ်၍ (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ပြန်ပေး nargs;
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ပြန်ပေး nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ကိန်းပြည့် cli_simple_run_command(const var_na *cmd, ကိန်းပြည့် flag)
{
	var_na cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	var_na *token;			/* start of token in cmdbuf	*/
	var_na *sep;			/* end of token (separator) in cmdbuf */
	var_na finaltoken[CONFIG_SYS_CBSIZE];
	var_na *str = cmdbuf;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ကိန်းပြည့် argc, inquotes;
	ကိန်းပြည့် repeatable = 1;
	ကိန်းပြည့် rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	အကယ်၍ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	အကယ်၍ (!cmd || !*cmd)
		ပြန်ပေး -1;	/* empty command */

	အကယ်၍ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ပြန်ပေး -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	နေစဉ် (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		အတွက် (inquotes = 0, sep = str; *sep; sep++) {
			အကယ်၍ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			အကယ်၍ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				bha_nga;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		အကယ်၍ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} မဟုတ်ပါက {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  parimaa_na(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_line(finaltoken, argv);
		အကယ်၍ (argc == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		အကယ်၍ (cmd_process(flag, argc, argv, &repeatable, NULL))
			rc = -1;

		/* Did the user stop this? */
		အကယ်၍ (had_ctrlc())
			ပြန်ပေး -1;	/* if stopped then not repeatable */
	}

	ပြန်ပေး rc ? rc : repeatable;
}

void cli_simple_loop(void)
{
	static var_na lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ကိန်းပြည့် len;
	ကိန်းပြည့် flag;
	ကိန်းပြည့် rc = 1;

	အတွက် (;;) {
		အကယ်၍ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		အကယ်၍ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		မဟုတ်ပါက အကယ်၍ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		မဟုတ်ပါက အကယ်၍ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(NULL, 0, 0, NULL);
# မဟုတ်ပါက
			ပြန်ပေး;		/* retry autoboot */
# endif
		}
#endif

		အကယ်၍ (len == -1)
			puts("<INTERRUPT>\n");
		မဟုတ်ပါက
			rc = run_command_repeatable(lastcommand, flag);

		အကယ်၍ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ကိန်းပြည့် cli_simple_run_command_list(var_na *cmd, ကိန်းပြည့် flag)
{
	var_na *line, *next;
	ကိန်းပြည့် rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	line = cmd;
	နေစဉ် (*next) {
		အကယ်၍ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			အကယ်၍ (*line) {
				debug("** exec: \"%s\"\n", line);
				အကယ်၍ (cli_simple_run_command(line, 0) < 0) {
					rcode = 1;
					bha_nga;
				}
			}
			line = next + 1;
		}
		++next;
	}
	အကယ်၍ (rcode == 0 && *line)
		rcode = (cli_simple_run_command(line, 0) < 0);

	ပြန်ပေး rcode;
}
#endif
