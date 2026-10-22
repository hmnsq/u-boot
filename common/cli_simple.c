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

पूर्णाङ्क cli_simple_process_macros(अचर वर्ण *आगत, वर्ण *output, पूर्णाङ्क max_size)
{
	वर्ण c, prev;
	अचर वर्ण *varname_start = रिक्त;
	पूर्णाङ्क inputcnt = म_माप(आगत);
	पूर्णाङ्क outputcnt = max_size;
	पूर्णाङ्क state = 0;		/* 0 = waiting for '$'  */
	पूर्णाङ्क ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	वर्ण __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", म_माप(आगत),
		     आगत);

	prev = '\0';		/* previous character   */

	जबसम्म (inputcnt && outputcnt) {
		c = *आगत++;
		inputcnt--;

		यदि (state != 3) {
			/* remove one level of escape characters */
			यदि ((c == '\\') && (prev != '\\')) {
				यदि (inputcnt-- == 0)
					रोक;
				prev = c;
				c = *आगत++;
			}
		}

		विकल्प (state) {
		पक्ष 0:	/* Waiting for (unescaped) $    */
			यदि ((c == '\'') && (prev != '\\')) {
				state = 3;
				रोक;
			}
			यदि ((c == '$') && (prev != '\\')) {
				state++;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			रोक;
		पक्ष 1:	/* Waiting for (        */
			यदि (c == '(' || c == '{') {
				state++;
				varname_start = आगत;
			} अन्यथा {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				यदि (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			रोक;
		पक्ष 2:	/* Waiting for )        */
			यदि (c == ')' || c == '}') {
				पूर्णाङ्क i;
				वर्ण envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				पूर्णाङ्क envcnt = आगत - varname_start - 1;

				/* Get the varname */
				लागि (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				यदि (envval != रिक्त)
					जबसम्म ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			रोक;
		पक्ष 3:	/* Waiting for '        */
			यदि ((c == '\'') && (prev != '\\')) {
				state = 0;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			रोक;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	यदि (outputcnt) {
		*output = 0;
	} अन्यथा {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     म_माप(output_start), output_start);

	फर्काऊ ret;
}

#ifdef CONFIG_CMDLINE
पूर्णाङ्क cli_simple_parse_पङ्क्ति(वर्ण *पङ्क्ति, वर्ण *argv[])
{
	पूर्णाङ्क nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, पङ्क्ति);
	जबसम्म (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		जबसम्म (isblank(*पङ्क्ति))
			++पङ्क्ति;

		यदि (*पङ्क्ति == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			फर्काऊ nargs;
		}

		argv[nargs++] = पङ्क्ति;	/* begin of argument string	*/

		/* find end of string */
		जबसम्म (*पङ्क्ति && !isblank(*पङ्क्ति))
			++पङ्क्ति;

		यदि (*पङ्क्ति == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("parse_line: nargs=%d\n", nargs);
			फर्काऊ nargs;
		}

		*पङ्क्ति++ = '\0';		/* terminate current arg	 */
	}

	म_लिखो("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	फर्काऊ nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
पूर्णाङ्क cli_simple_run_command(अचर वर्ण *cmd, पूर्णाङ्क flag)
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
	यदि (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		माला_दो(cmd ? cmd : "NULL");
		माला_दो("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	यदि (!cmd || !*cmd)
		फर्काऊ -1;	/* empty command */

	यदि (म_माप(cmd) >= CONFIG_SYS_CBSIZE) {
		माला_दो("## Command too long!\n");
		फर्काऊ -1;
	}

	म_नकल(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	जबसम्म (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		लागि (inquotes = 0, sep = str; *sep; sep++) {
			यदि ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			यदि (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				रोक;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		यदि (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} अन्यथा {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  परिमाण(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_पङ्क्ति(finaltoken, argv);
		यदि (argc == 0) {
			rc = -1;	/* no command at all */
			जारी;
		}

		यदि (cmd_process(flag, argc, argv, &repeatable, रिक्त))
			rc = -1;

		/* Did the user stop this? */
		यदि (had_ctrlc())
			फर्काऊ -1;	/* if stopped then not repeatable */
	}

	फर्काऊ rc ? rc : repeatable;
}

शून्य cli_simple_loop(शून्य)
{
	स्थिर वर्ण lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	पूर्णाङ्क len;
	पूर्णाङ्क flag;
	पूर्णाङ्क rc = 1;

	लागि (;;) {
		यदि (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		यदि (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		अन्यथा यदि (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		अन्यथा यदि (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			माला_दो("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(रिक्त, 0, 0, रिक्त);
# अन्यथा
			फर्काऊ;		/* retry autoboot */
# endif
		}
#endif

		यदि (len == -1)
			माला_दो("<INTERRUPT>\n");
		अन्यथा
			rc = run_command_repeatable(lastcommand, flag);

		यदि (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

पूर्णाङ्क cli_simple_run_command_list(वर्ण *cmd, पूर्णाङ्क flag)
{
	वर्ण *पङ्क्ति, *next;
	पूर्णाङ्क rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	पङ्क्ति = cmd;
	जबसम्म (*next) {
		यदि (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			यदि (*पङ्क्ति) {
				debug("** exec: \"%s\"\n", पङ्क्ति);
				यदि (cli_simple_run_command(पङ्क्ति, 0) < 0) {
					rcode = 1;
					रोक;
				}
			}
			पङ्क्ति = next + 1;
		}
		++next;
	}
	यदि (rcode == 0 && *पङ्क्ति)
		rcode = (cli_simple_run_command(पङ्क्ति, 0) < 0);

	फर्काऊ rcode;
}
#endif
