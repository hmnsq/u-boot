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

पूर्णांक cli_simple_process_macros(स्थिरांक अक्षर *आदान, अक्षर *output, पूर्णांक max_size)
{
	अक्षर c, prev;
	स्थिरांक अक्षर *varname_start = रिक्त;
	पूर्णांक inputcnt = म_माप(आदान);
	पूर्णांक outputcnt = max_size;
	पूर्णांक state = 0;		/* 0 = waiting for '$'  */
	पूर्णांक ret;

	/* 1 = waiting for '(' or '{' */
	/* 2 = waiting for ')' or '}' */
	/* 3 = waiting for '''  */
	अक्षर __maybe_unused *output_start = output;

	debug_parser("[PROCESS_MACROS] INPUT len %zd: \"%s\"\n", म_माप(आदान),
		     आदान);

	prev = '\0';		/* previous character   */

	पर्यंत (inputcnt && outputcnt) {
		c = *आदान++;
		inputcnt--;

		जर (state != 3) {
			/* remove one level of escape characters */
			जर ((c == '\\') && (prev != '\\')) {
				जर (inputcnt-- == 0)
					थांबा;
				prev = c;
				c = *आदान++;
			}
		}

		निवड (state) {
		स्थिती 0:	/* Waiting for (unescaped) $    */
			जर ((c == '\'') && (prev != '\\')) {
				state = 3;
				थांबा;
			}
			जर ((c == '$') && (prev != '\\')) {
				state++;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			थांबा;
		स्थिती 1:	/* Waiting for (        */
			जर (c == '(' || c == '{') {
				state++;
				varname_start = आदान;
			} अन्यथा {
				state = 0;
				*(output++) = '$';
				outputcnt--;

				जर (outputcnt) {
					*(output++) = c;
					outputcnt--;
				}
			}
			थांबा;
		स्थिती 2:	/* Waiting for )        */
			जर (c == ')' || c == '}') {
				पूर्णांक i;
				अक्षर envname[CONFIG_SYS_CBSIZE], *envval;
				/* Varname # of chars */
				पूर्णांक envcnt = आदान - varname_start - 1;

				/* Get the varname */
				साठी (i = 0; i < envcnt; i++)
					envname[i] = varname_start[i];
				envname[i] = 0;

				/* Get its value */
				envval = env_get(envname);

				/* Copy into the line if it exists */
				जर (envval != रिक्त)
					पर्यंत ((*envval) && outputcnt) {
						*(output++) = *(envval++);
						outputcnt--;
					}
				/* Look for another '$' */
				state = 0;
			}
			थांबा;
		स्थिती 3:	/* Waiting for '        */
			जर ((c == '\'') && (prev != '\\')) {
				state = 0;
			} अन्यथा {
				*(output++) = c;
				outputcnt--;
			}
			थांबा;
		}
		prev = c;
	}

	ret = inputcnt ? -ENOSPC : 0;
	जर (outputcnt) {
		*output = 0;
	} अन्यथा {
		*(output - 1) = 0;
		ret = -ENOSPC;
	}

	debug_parser("[PROCESS_MACROS] OUTPUT len %zd: \"%s\"\n",
		     म_माप(output_start), output_start);

	परत ret;
}

#ifdef CONFIG_CMDLINE
पूर्णांक cli_simple_parse_ओळ(अक्षर *ओळ, अक्षर *argv[])
{
	पूर्णांक nargs = 0;

	debug_parser("%s: \"%s\"\n", __func__, ओळ);
	पर्यंत (nargs < CONFIG_SYS_MAXARGS) {
		/* skip any white space */
		पर्यंत (isblank(*ओळ))
			++ओळ;

		जर (*ओळ == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("%s: nargs=%d\n", __func__, nargs);
			परत nargs;
		}

		argv[nargs++] = ओळ;	/* begin of argument string	*/

		/* find end of string */
		पर्यंत (*ओळ && !isblank(*ओळ))
			++ओळ;

		जर (*ओळ == '\0') {	/* end of line, no more args	*/
			argv[nargs] = रिक्त;
			debug_parser("parse_line: nargs=%d\n", nargs);
			परत nargs;
		}

		*ओळ++ = '\0';		/* terminate current arg	 */
	}

	म_लिखो("** Too many args (max. %d) **\n", CONFIG_SYS_MAXARGS);

	debug_parser("%s: nargs=%d\n", __func__, nargs);
	परत nargs;
}

 /*
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from env_get(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */
पूर्णांक cli_simple_run_command(स्थिरांक अक्षर *cmd, पूर्णांक flag)
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
	जर (DEBUG_PARSER) {
		/* use puts - string may be loooong */
		माला_दो(cmd ? cmd : "NULL");
		माला_दो("\"\n");
	}
	clear_ctrlc();		/* forget any previous Control C */

	जर (!cmd || !*cmd)
		परत -1;	/* empty command */

	जर (म_माप(cmd) >= CONFIG_SYS_CBSIZE) {
		माला_दो("## Command too long!\n");
		परत -1;
	}

	म_नकल(cmdbuf, cmd);

	/* Process separators and check for invalid
	 * repeatable commands
	 */

	debug_parser("[PROCESS_SEPARATORS] %s\n", cmd);
	पर्यंत (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		साठी (inquotes = 0, sep = str; *sep; sep++) {
			जर ((*sep == '\'') &&
			    (*(sep - 1) != '\\'))
				inquotes = !inquotes;

			जर (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    (sep != str) &&	/* past string start	*/
			    (*(sep - 1) != '\\'))	/* and NOT escaped */
				थांबा;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		जर (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		} अन्यथा {
			str = sep;	/* no more commands for next pass */
		}
		debug_parser("token: \"%s\"\n", token);

		/* find macros in this token and replace them */
		cli_simple_process_macros(token, finaltoken,
					  आकार(finaltoken));

		/* Extract arguments */
		argc = cli_simple_parse_ओळ(finaltoken, argv);
		जर (argc == 0) {
			rc = -1;	/* no command at all */
			सुरूठेवा;
		}

		जर (cmd_process(flag, argc, argv, &repeatable, रिक्त))
			rc = -1;

		/* Did the user stop this? */
		जर (had_ctrlc())
			परत -1;	/* if stopped then not repeatable */
	}

	परत rc ? rc : repeatable;
}

शून्य cli_simple_loop(शून्य)
{
	स्थिर अक्षर lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0, };

	पूर्णांक len;
	पूर्णांक flag;
	पूर्णांक rc = 1;

	साठी (;;) {
		जर (rc >= 0) {
			/* Saw enough of a valid command to
			 * restart the timeout.
			 */
			bootretry_reset_cmd_timeout();
		}
		len = cli_readline(CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		जर (len > 0)
			strlcpy(lastcommand, console_buffer,
				CONFIG_SYS_CBSIZE + 1);
		अन्यथा जर (len == 0)
			flag |= CMD_FLAG_REPEAT;
#ifdef CONFIG_BOOT_RETRY_TIME
		अन्यथा जर (len == -2) {
			/* -2 means timed out, retry autoboot
			 */
			माला_दो("\nTimed out waiting for command\n");
# ifdef CONFIG_RESET_TO_RETRY
			/* Reinit board to run initialization code again */
			do_reset(रिक्त, 0, 0, रिक्त);
# अन्यथा
			परत;		/* retry autoboot */
# endif
		}
#endif

		जर (len == -1)
			माला_दो("<INTERRUPT>\n");
		अन्यथा
			rc = run_command_repeatable(lastcommand, flag);

		जर (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
}

पूर्णांक cli_simple_run_command_list(अक्षर *cmd, पूर्णांक flag)
{
	अक्षर *ओळ, *next;
	पूर्णांक rcode = 0;

	/*
	 * Break into individual lines, and execute each line; terminate on
	 * error.
	 */
	next = cmd;
	ओळ = cmd;
	पर्यंत (*next) {
		जर (*next == '\n') {
			*next = '\0';
			/* run only non-empty commands */
			जर (*ओळ) {
				debug("** exec: \"%s\"\n", ओळ);
				जर (cli_simple_run_command(ओळ, 0) < 0) {
					rcode = 1;
					थांबा;
				}
			}
			ओळ = next + 1;
		}
		++next;
	}
	जर (rcode == 0 && *ओळ)
		rcode = (cli_simple_run_command(ओळ, 0) < 0);

	परत rcode;
}
#endif
