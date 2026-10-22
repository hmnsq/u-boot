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

#परिभाषय DEBUG_PARSER	0	/* set to 1 to debug */

#परिभाषय debug_parser(fmt, तर्काः...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

पूर्णाङ्क cli_simple_process_macros(ध्रुव वर्ण *निवेशः, वर्ण *output, पूर्णाङ्क max_आकार)
{
	वर्ण c, पूर्व;
	ध्रुव वर्ण *varname_start = रिक्त;
	पूर्णाङ्क inputcnt = वाक्यदैर्घ्यम्(निवेशः);
	पूर्णाङ्क outputcnt = max_आकार;
	पूर्णाङ्क state = 0;		/* 0 = waiting for '$'  */
	पूर्णाङ्क ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	वर्ण __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", वाक्यदैर्घ्यम्(निवेशः),
		     निवेशः);

	पूर्व = '\0';		/* previous character   */

	यावत् (inputcnt && outputcnt) {
		c = *निवेशः++;
		inputcnt--;

		यदि (state != 3) {
			/* remove one level of escape characters */
			यदि ((c == '\\') && (पूर्व != '\\')) {
				यदि (inputcnt-- == 0)
					भङ्ग;
				पूर्व = c;
				c = *निवेशः++;
			}
		}

		विकल्प (state) {
		पक्ष 0:	/* Waiting for (unescaped) $    */
			यदि ((c == '\'') && (पूर्व != '\\')) {
				state = 3;
				भङ्ग;
			}
			यदि ((c == '$') && (पूर्व != '\\')) {
				state++;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			भङ्ग;
		पक्ष 1:	/* Waiting for (        */
			यदि (c == '(' || c == '{') {
				state++;
				varname_start = निवेशः;
			} अन्यथा {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				यदि (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			भङ्ग;
		पक्ष 2:	/* Waiting for )        */
			यदि (c == ')' || c == '}') {
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
				यदि (envval != रिक्त)
					यावत् ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			भङ्ग;
		पक्ष 3:	/* Waiting for '        */
			यदि ((c == '\'') && (पूर्व != '\\')) {
				state = 0;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			भङ्ग;
		}
		पूर्व = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	यदि (outputcnt) {
		*output = 0;
	} अन्यथा {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     वाक्यदैर्घ्यम्(output_start), output_start);

	प्रत्यागम ret;
}

#यदि_परिभाषितम् CONFIG_CMDLINE
पूर्णाङ्क cli_simple_parse_पङ्क्तिः(वर्ण *पङ्क्तिः, वर्ण *argv[])
{
	पूर्णाङ्क nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, पङ्क्तिः);
	यावत् (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		यावत् (isblank(*पङ्क्तिः))
			++पङ्क्तिः;

		यदि (*पङ्क्तिः == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			प्रत्यागम nargs;
		}

		argv[nargs++] = पङ्क्तिः;	/* begin of argument string	*/

		/* find end of string */
		यावत् (*पङ्क्तिः && !isblank(*पङ्क्तिः))
			++पङ्क्तिः;

		यदि (*पङ्क्तिः == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("parse_line: nargs=%d\n", nargs);
			प्रत्यागम nargs;
		}

		*पङ्क्तिः++ = '\0';		/* terminate current arg	 */
	}

	प्रारूप_लिख("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

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
पूर्णाङ्क cli_simple_सततम्_command(ध्रुव वर्ण *cmd, पूर्णाङ्क flag)
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
		पङ्क्तिं_लिख(cmd ? cmd : "NULL");
		पङ्क्तिं_लिख("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	यदि (!cmd || !*cmd)
		प्रत्यागम -1;	/* empty command */

	यदि (वाक्यदैर्घ्यम्(cmd) >= CONFIG_SYS_CBSIZE) {
		पङ्क्तिं_लिख("## Command too long!\n");
		प्रत्यागम -1;
	}

	वाक्यं_प्रतिलिख(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	यावत् (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		प्रति (inquotes = 0, sep = str; *sep; sep++) {
			यदि ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			यदि (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				भङ्ग;
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
		argc = cli_simple_parse_पङ्क्तिः(finaltoken, argv);
		यदि (argc == 0) {
			rc = -1;	/* no command at all */
			अनुवर्त;
		}

		यदि (cmd_process(flag, argc, argv, &repeatable, रिक्त))
			rc = -1;

		/* Did the user stop this? */
		यदि (had_ctrlc())
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
		यदि (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		यदि (len > 0)
			strlcpy(lastcommand, console_बफर,
				CONFIG_SYS_CBSIZE + 1);
		अन्यथा यदि (len == 0)
			flag |= CMD_FLAG_REPEAT;
#यदि_परिभाषितम् CONFIG_BOOT_RETRY_TIME
		अन्यथा यदि (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			पङ्क्तिं_लिख("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_गन्तव्यम्_RETRY
			/* Reinit board to run initialization code again */
			do_reset(रिक्त, 0, 0, रिक्त);
# अन्यथा
			प्रत्यागम;		/* retry autoboot */
# endif
		}
#यदि_अन्तः

		यदि (len == -1)
			पङ्क्तिं_लिख("<INTERRUPT>\n");
		अन्यथा
			rc = सततम्_command_repeatable(lastcommand, flag);

		यदि (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

पूर्णाङ्क cli_simple_सततम्_command_list(वर्ण *cmd, पूर्णाङ्क flag)
{
	वर्ण *पङ्क्तिः, *अग्र;
	पूर्णाङ्क rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	अग्र = cmd;
	पङ्क्तिः = cmd;
	यावत् (*अग्र) {
		यदि (*अग्र == '\n') {
			*अग्र = '\0';
			/* run only non-empty commands */
			यदि (*पङ्क्तिः) {
				debug("** exec: \"%s\"\n", पङ्क्तिः);
				यदि (cli_simple_सततम्_command(पङ्क्तिः, 0) < 0) {
					rcode = 1;
					भङ्ग;
				}
			}
			पङ्क्तिः = अग्र + 1;
		}
		++अग्र;
	}
	यदि (rcode == 0 && *पङ्क्तिः)
		rcode = (cli_simple_सततम्_command(पङ्क्तिः, 0) < 0);

	प्रत्यागम rcode;
}
#यदि_अन्तः
