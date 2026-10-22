// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <linux/ctype.h>

#define DEBUG_PARSER	0	/* set to 1 to debug */

#define debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

पूर्णाङ्क cli_simple_process_macros(ध्रुव वर्ण *निवेशः, वर्ण *output, पूर्णाङ्क max_size)
{
	वर्ण c, prev;
	ध्रुव वर्ण *varname_start = रिक्त;
	पूर्णाङ्क inputcnt = strlen(निवेशः);
	पूर्णाङ्क outputcnt = max_size;
	पूर्णाङ्क state = 0;		/* 0 = waiting for '$'  */
	पूर्णाङ्क ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	वर्ण __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", strlen(निवेशः),
		     निवेशः);

	prev = '\0';		/* previous character   */

	जाव (inputcnt && outputcnt) {
		c = *निवेशः++;
		inputcnt--;

		जइ (state != 3) {
			/* remove one level of escape characters */
			जइ ((c == '\\') && (prev != '\\')) {
				जइ (inputcnt-- == 0)
					भङ्ग;
				prev = c;
				c = *निवेशः++;
			}
		}

		विकल्प (state) {
		पक्ष 0:	/* Waiting for (unescaped) $    */
			जइ ((c == '\'') && (prev != '\\')) {
				state = 3;
				भङ्ग;
			}
			जइ ((c == '$') && (prev != '\\')) {
				state++;
			} अण्णहा {
				*(output++) = c;
				outputcnt--;
			}
			भङ्ग;
		पक्ष 1:	/* Waiting for (        */
			जइ (c == '(' || c == '{') {
				state++;
				varname_start = निवेशः;
			} अण्णहा {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				जइ (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			भङ्ग;
		पक्ष 2:	/* Waiting for )        */
			जइ (c == ')' || c == '}') {
				पूर्णाङ्क i;
				वर्ण envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				पूर्णाङ्क envcnt = निवेशः - varname_start - 1;

				/* Get the varname */
				प्रति (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				जइ (envval != रिक्त)
					जाव ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			भङ्ग;
		पक्ष 3:	/* Waiting for '        */
			जइ ((c == '\'') && (prev != '\\')) {
				state = 0;
			} अण्णहा {
				*(output++) = c;
				outputcnt--;
			}
			भङ्ग;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	जइ (outputcnt) {
		*output = 0;
	} अण्णहा {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     strlen(output_start), output_start);

	प्रत्यागम ret;
}

#ifdef CONFIG_CMDLINE
पूर्णाङ्क cli_simple_parse_पङ्क्तिः(वर्ण *पङ्क्तिः, वर्ण *argv[])
{
	पूर्णाङ्क nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, पङ्क्तिः);
	जाव (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		जाव (isblank(*पङ्क्तिः))
			++पङ्क्तिः;

		जइ (*पङ्क्तिः == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			प्रत्यागम nargs;
		}

		argv[nargs++] = पङ्क्तिः;	/* begin of argument string	*/

		/* find end of string */
		जाव (*पङ्क्तिः && !isblank(*पङ्क्तिः))
			++पङ्क्तिः;

		जइ (*पङ्क्तिः == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("parse_line: nargs=%d\n", nargs);
			प्रत्यागम nargs;
		}

		*पङ्क्तिः++ = '\0';		/* terminate current arg	 */
	}

	printf("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	प्रत्यागम nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
पूर्णाङ्क cli_simple_run_command(ध्रुव वर्ण *cmd, पूर्णाङ्क flag)
{
	वर्ण cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	वर्ण *token;			/* start of token in cmdbuf	*/
	वर्ण *sep;			/* end of token (separator) in cmdbuf */
	वर्ण finaltoken[CONFIG_SYS_CBSIZE];
	वर्ण *str = cmdbuf;
	वर्ण *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	पूर्णाङ्क argc, inquotes;
	पूर्णाङ्क repeatable = 1;
	पूर्णाङ्क rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	जइ (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		puts(cmd ? cmd : "NULL");
		puts("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	जइ (!cmd || !*cmd)
		प्रत्यागम -1;	/* empty command */

	जइ (strlen(cmd) >= CONFIG_SYS_CBSIZE) {
		puts("## Command too long!\n");
		प्रत्यागम -1;
	}

	strcpy(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	जाव (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		प्रति (inquotes = 0, sep = str; *sep; sep++) {
			जइ ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			जइ (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				भङ्ग;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		जइ (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} अण्णहा {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  परिमाण(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_पङ्क्तिः(finaltoken, argv);
		जइ (argc == 0) {
			rc = -1;	/* no command at all */
			अनुवर्त;
		}

		जइ (cmd_process(flag, argc, argv, &repeatable, रिक्त))
			rc = -1;

		/* Did the user stop this? */
		जइ (had_ctrlc())
			प्रत्यागम -1;	/* if stopped then not repeatable */
	}

	प्रत्यागम rc ? rc : repeatable;
}

शून्य cli_simple_loop(शून्य)
{
	स्थिर वर्ण lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	पूर्णाङ्क len;
	पूर्णाङ्क flag;
	पूर्णाङ्क rc = 1;

	प्रति (;;) {
		जइ (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		जइ (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		अण्णहा जइ (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		अण्णहा जइ (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			puts("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(रिक्त, 0, 0, रिक्त);
# अण्णहा
			प्रत्यागम;		/* retry autoboot */
# endif
		}
#endif

		जइ (len == -1)
			puts("<INTERRUPT>\n");
		अण्णहा
			rc = run_command_repeatable(lastcommand, flag);

		जइ (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

पूर्णाङ्क cli_simple_run_command_list(वर्ण *cmd, पूर्णाङ्क flag)
{
	वर्ण *पङ्क्तिः, *next;
	पूर्णाङ्क rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	पङ्क्तिः = cmd;
	जाव (*next) {
		जइ (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			जइ (*पङ्क्तिः) {
				debug("** exec: \"%s\"\n", पङ्क्तिः);
				जइ (cli_simple_run_command(पङ्क्तिः, 0) < 0) {
					rcode = 1;
					भङ्ग;
				}
			}
			पङ्क्तिः = next + 1;
		}
		++next;
	}
	जइ (rcode == 0 && *पङ्क्तिः)
		rcode = (cli_simple_run_command(पङ्क्तिः, 0) < 0);

	प्रत्यागम rcode;
}
#endif
