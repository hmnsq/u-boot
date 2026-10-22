// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#समावेश <bootretry.h>
#समावेश <cli.h>
#समावेश <command.h>
#समावेश <console.h>
#समावेश <env.h>
#समावेश <log.h>
#समावेश <linux/ctype.h>

#घोषणा DEBUG_PARSER	0	/* set to 1 to debug */

#घोषणा debug_parser(fmt, args...)		\
	debug_cond(DEBUG_PARSER, fmt, ##args)

पूर्णांक cli_simple_process_macros(स्थिर अक्षर *निवेश, अक्षर *output, पूर्णांक max_आकार)
{
	अक्षर c, पूर्व;
	स्थिर अक्षर *varname_start = शून्य;
	पूर्णांक inputcnt = म_माप(निवेश);
	पूर्णांक outputcnt = max_आकार;
	पूर्णांक state = 0;		/* 0 = waiting for '$'  */
	पूर्णांक ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	अक्षर __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", म_माप(निवेश),
		     निवेश);

	पूर्व = '\0';		/* previous character   */

	जबतक (inputcnt && outputcnt) {
		c = *निवेश++;
		inputcnt--;

		अगर (state != 3) {
			/* remove one level of escape characters */
			अगर ((c == '\\') && (पूर्व != '\\')) {
				अगर (inputcnt-- == 0)
					अवरोध;
				पूर्व = c;
				c = *निवेश++;
			}
		}

		चयन (state) {
		हाल 0:	/* Waiting for (unescaped) $    */
			अगर ((c == '\'') && (पूर्व != '\\')) {
				state = 3;
				अवरोध;
			}
			अगर ((c == '$') && (पूर्व != '\\')) {
				state++;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			अवरोध;
		हाल 1:	/* Waiting for (        */
			अगर (c == '(' || c == '{') {
				state++;
				varname_start = निवेश;
			} अन्यथा {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				अगर (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			अवरोध;
		हाल 2:	/* Waiting for )        */
			अगर (c == ')' || c == '}') {
				पूर्णांक i;
				अक्षर envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				पूर्णांक envcnt = निवेश - varname_start - 1;

				/* Get the varname */
				क्रम (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				अगर (envval != शून्य)
					जबतक ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			अवरोध;
		हाल 3:	/* Waiting for '        */
			अगर ((c == '\'') && (पूर्व != '\\')) {
				state = 0;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			अवरोध;
		}
		पूर्व = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	अगर (outputcnt) {
		*output = 0;
	} अन्यथा {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     म_माप(output_start), output_start);

	वापस ret;
}

#अगर_घोषित CONFIG_CMDLINE
पूर्णांक cli_simple_parse_पंक्ति(अक्षर *पंक्ति, अक्षर *argv[])
{
	पूर्णांक nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, पंक्ति);
	जबतक (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		जबतक (isblank(*पंक्ति))
			++पंक्ति;

		अगर (*पंक्ति == '\0') {	/* end of line, no more args	*/
			argv[nargs] = शून्य;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			वापस nargs;
		}

		argv[nargs++] = पंक्ति;	/* begin of argument string	*/

		/* find end of string */
		जबतक (*पंक्ति && !isblank(*पंक्ति))
			++पंक्ति;

		अगर (*पंक्ति == '\0') {	/* end of line, no more args	*/
			argv[nargs] = शून्य;
			debug_parser("parse_line: nargs=%d\n", nargs);
			वापस nargs;
		}

		*पंक्ति++ = '\0';		/* terminate current arg	 */
	}

	म_लिखो("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	वापस nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
पूर्णांक cli_simple_run_command(स्थिर अक्षर *cmd, पूर्णांक flag)
{
	अक्षर cmdbuf[CONFIG_SYS_CBSIZE];	/* working copy of cmd		*/
	अक्षर *token;			/* start of token in cmdbuf	*/
	अक्षर *sep;			/* end of token (separator) in cmdbuf */
	अक्षर finaltoken[CONFIG_SYS_CBSIZE];
	अक्षर *str = cmdbuf;
	अक्षर *argv[CONFIG_SYS_MAXARGS + 1];	/* NULL terminated	*/
	पूर्णांक argc, inquotes;
	पूर्णांक repeatable = 1;
	पूर्णांक rc = 0;

	debug_parser("[RUN_COMMAND] cmd[%p]=\"", cmd);
	अगर (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		माला_दो(cmd ? cmd : "NULL");
		माला_दो("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	अगर (!cmd || !*cmd)
		वापस -1;	/* empty command */

	अगर (म_माप(cmd) >= CONFIG_SYS_CBSIZE) {
		माला_दो("## Command too long!\n");
		वापस -1;
	}

	म_नकल(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	जबतक (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		क्रम (inquotes = 0, sep = str; *sep; sep++) {
			अगर ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			अगर (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				अवरोध;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		अगर (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} अन्यथा {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  माप(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_पंक्ति(finaltoken, argv);
		अगर (argc == 0) {
			rc = -1;	/* no command at all */
			जारी;
		}

		अगर (cmd_process(flag, argc, argv, &repeatable, शून्य))
			rc = -1;

		/* Did the user stop this? */
		अगर (had_ctrlc())
			वापस -1;	/* if stopped then not repeatable */
	}

	वापस rc ? rc : repeatable;
}

व्योम cli_simple_loop(व्योम)
{
	अटल अक्षर lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	पूर्णांक len;
	पूर्णांक flag;
	पूर्णांक rc = 1;

	क्रम (;;) {
		अगर (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		अगर (len > 0)
			strlcpy(lastcommand, console_बफर,
				CONFIG_SYS_CBSIZE + 1);
		अन्यथा अगर (len == 0)
			flag |= CMD_FLAG_REPEAT;
#अगर_घोषित CONFIG_BOOT_RETRY_TIME
		अन्यथा अगर (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			माला_दो("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(शून्य, 0, 0, शून्य);
# अन्यथा
			वापस;		/* retry autoboot */
# endif
		}
#पूर्ण_अगर

		अगर (len == -1)
			माला_दो("<INTERRUPT>\n");
		अन्यथा
			rc = run_command_repeatable(lastcommand, flag);

		अगर (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

पूर्णांक cli_simple_run_command_list(अक्षर *cmd, पूर्णांक flag)
{
	अक्षर *पंक्ति, *अग्र;
	पूर्णांक rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	अग्र = cmd;
	पंक्ति = cmd;
	जबतक (*अग्र) {
		अगर (*अग्र == '\n') {
			*अग्र = '\0';
			/* run only non-empty commands */
			अगर (*पंक्ति) {
				debug("** exec: \"%s\"\n", पंक्ति);
				अगर (cli_simple_run_command(पंक्ति, 0) < 0) {
					rcode = 1;
					अवरोध;
				}
			}
			पंक्ति = अग्र + 1;
		}
		++अग्र;
	}
	अगर (rcode == 0 && *पंक्ति)
		rcode = (cli_simple_run_command(पंक्ति, 0) < 0);

	वापस rcode;
}
#पूर्ण_अगर
