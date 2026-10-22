// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ଅନ୍ତର୍ଭାଵ <bootretry.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_simple_process_macros(ଅଚଳ ଅକ୍ଷର *ନିବେଶ, ଅକ୍ଷର *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_size)
{
	ଅକ୍ଷର c, prev;
	ଅଚଳ ଅକ୍ଷର *varname_start = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା inputcnt = strlen(ନିବେଶ);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା outputcnt = max_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା state = 0;		/* 0 = waiting for '$'  */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	ଅକ୍ଷର __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(ନିବେଶ),
		     ନିବେଶ);

	prev = '\0';		/* previous character   */

	ଯେତେବେଳେ (inputcnt && outputcnt) {
		c = *ନିବେଶ++;
		inputcnt--;

		ଯଦି (state != 3) {
			/* remove one level of escape characters */
			ଯଦି ((c == '\\') && (prev != '\\')) {
				ଯଦି (inputcnt-- == 0)
					ଭାଙ୍ଗ;
				prev = c;
				c = *ନିବେଶ++;
			}
		}

		ବାଛ (state) {
		ସ୍ଥିତି 0:	/* Waiting for (unescaped) $    */
			ଯଦି ((c == '\'') && (prev != '\\')) {
				state = 3;
				ଭାଙ୍ଗ;
			}
			ଯଦି ((c == '$') && (prev != '\\')) {
				state++;
			} ଅନ୍ୟଥା {
				*(output++) = c;
				outputcnt--;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 1:	/* Waiting for (        */
			ଯଦି (c == '(' || c == '{') {
				state++;
				varname_start = ନିବେଶ;
			} ଅନ୍ୟଥା {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				ଯଦି (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 2:	/* Waiting for )        */
			ଯଦି (c == ')' || c == '}') {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
				ଅକ୍ଷର envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା envcnt = ନିବେଶ - varname_start - 1;

				/* Get the varname */
				ପାଇଁ (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				ଯଦି (envval != ଖାଲି)
					ଯେତେବେଳେ ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 3:	/* Waiting for '        */
			ଯଦି ((c == '\'') && (prev != '\\')) {
				state = 0;
			} ଅନ୍ୟଥା {
				*(output++) = c;
				outputcnt--;
			}
			ଭାଙ୍ଗ;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	ଯଦି (outputcnt) {
		*output = 0;
	} ଅନ୍ୟଥା {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	ଫେରନ୍ତୁ ret;
}

#ifdef CONFIG_CMDLINE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_simple_parse_ଧାଡ଼ି(ଅକ୍ଷର *ଧାଡ଼ି, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, ଧାଡ଼ି);
	ଯେତେବେଳେ (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		ଯେତେବେଳେ (isblank(*ଧାଡ଼ି))
			++ଧାଡ଼ି;

		ଯଦି (*ଧାଡ଼ି == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ଖାଲି;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			ଫେରନ୍ତୁ nargs;
		}

		argv[nargs++] = ଧାଡ଼ି;	/* begin of argument string	*/

		/* find end of string */
		ଯେତେବେଳେ (*ଧାଡ଼ି && !isblank(*ଧାଡ଼ି))
			++ଧାଡ଼ି;

		ଯଦି (*ଧାଡ଼ି == '\0') {	/* end of line, no more args	*/
			argv[nargs] = ଖାଲି;
			debug_parser("parse_line: nargs=%d\n", nargs);
			ଫେରନ୍ତୁ nargs;
		}

		*ଧାଡ଼ି++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	ଫେରନ୍ତୁ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_simple_run_command(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
{
	ଅକ୍ଷର cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	ଅକ୍ଷର *token;			/* start of token in cmdbuf	*/
	ଅକ୍ଷର *sep;			/* end of token (separator) in cmdbuf */
	ଅକ୍ଷର finaltoken[CONFIG_SYS_CBSIZE];
	ଅକ୍ଷର *str = cmdbuf;
	ଅକ୍ଷର *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, inquotes;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା repeatable = 1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	ଯଦି (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	ଯଦି (!cmd || !*cmd)
		ଫେରନ୍ତୁ -1;	/* empty command */

	ଯଦି (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		ଫେରନ୍ତୁ -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	ଯେତେବେଳେ (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		ପାଇଁ (inquotes = 0, sep = str; *sep; sep++) {
			ଯଦି ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			ଯଦି (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				ଭାଙ୍ଗ;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		ଯଦି (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} ଅନ୍ୟଥା {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  ଆକାର(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_ଧାଡ଼ି(finaltoken, argv);
		ଯଦି (argc == 0) {
			rc = -1;	/* no command at all */
			ଜାରି;
		}

		ଯଦି (cmd_process(flag, argc, argv, &repeatable, ଖାଲି))
			rc = -1;

		/* Did the user stop this? */
		ଯଦି (had_ctrlc())
			ଫେରନ୍ତୁ -1;	/* if stopped then not repeatable */
	}

	ଫେରନ୍ତୁ rc ? rc : repeatable;
}

ଶୂନ୍ୟ cli_simple_loop(ଶୂନ୍ୟ)
{
	ସ୍ଥିର ଅକ୍ଷର lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc = 1;

	ପାଇଁ (;;) {
		ଯଦି (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		ଯଦି (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		ଅନ୍ୟଥା ଯଦି (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		ଅନ୍ୟଥା ଯଦି (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(ଖାଲି, 0, 0, ଖାଲି);
# ଅନ୍ୟଥା
			ଫେରନ୍ତୁ;		/* retry autoboot */
# endif
		}
#endif

		ଯଦି (len == -1)
			puts("<INTERRUPT>\n");
		ଅନ୍ୟଥା
			rc = run_command_repeatable(lastcommand, flag);

		ଯଦି (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_simple_run_command_list(ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
{
	ଅକ୍ଷର *ଧାଡ଼ି, *next;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	ଧାଡ଼ି = cmd;
	ଯେତେବେଳେ (*next) {
		ଯଦି (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			ଯଦି (*ଧାଡ଼ି) {
				debug("** exec: \"%s\"\n", ଧାଡ଼ି);
				ଯଦି (cli_simple_run_command(ଧାଡ଼ି, 0) < 0) {
					rcode = 1;
					ଭାଙ୍ଗ;
				}
			}
			ଧାଡ଼ି = next + 1;
		}
		++next;
	}
	ଯଦି (rcode == 0 && *ଧାଡ଼ି)
		rcode = (cli_simple_run_command(ଧାଡ଼ି, 0) < 0);

	ଫେରନ୍ତୁ rcode;
}
#endif
