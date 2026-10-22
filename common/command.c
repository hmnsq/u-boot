// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <compiler.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

पूर्णाङ्क _do_help(संरचना cmd_tbl *cmd_start, पूर्णाङ्क cmd_वस्तुएँ, संरचना cmd_tbl *cmdtp,
	     पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i;
	पूर्णाङ्क rcode = 0;

	यदि (argc == 1) {	/* show list of commands */
		संरचना cmd_tbl *cmd_array[cmd_वस्तुएँ];
		पूर्णाङ्क i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		प्रति (i = 0; i < cmd_वस्तुएँ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		प्रति (i = cmd_वस्तुएँ - 1; i > 0; --i) {
			swaps = 0;
			प्रति (j = 0; j < i; ++j) {
				यदि (म_भेद(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					संरचना cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			यदि (!swaps)
				भङ्ग;
		}

		/* print short help (usage) */
		प्रति (i = 0; i < cmd_वस्तुएँ; i++) {
			ध्रुव वर्ण *usage = cmd_array[i]->usage;

			/* allow user abort */
			यदि (ctrlc())
				प्रत्यागम 1;
			यदि (usage == रिक्त)
				अनुवर्त;
			म_लिखो("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		प्रत्यागम 0;
	}
	/*
	 * command help (long version)
	 */
	प्रति (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_वस्तुएँ);
		यदि (cmdtp != रिक्त) {
			rcode |= cmd_usage(cmdtp);
		} अन्यथा {
			म_लिखो("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	प्रत्यागम rcode;
}

/* find command table entry for a command */
संरचना cmd_tbl *find_cmd_tbl(ध्रुव वर्ण *cmd, संरचना cmd_tbl *table,
			     पूर्णाङ्क table_len)
{
#ifdef CONFIG_CMDLINE
	संरचना cmd_tbl *cmdtp;
	संरचना cmd_tbl *cmdtp_temp = table;	/* Init value */
	ध्रुव वर्ण *p;
	पूर्णाङ्क len;
	पूर्णाङ्क n_found = 0;

	यदि (!cmd)
		प्रत्यागम रिक्त;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = म_अक्षर(cmd, '.')) == रिक्त) ? म_माप (cmd) : (p - cmd);

	प्रति (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		यदि (म_भेदन(cmd, cmdtp->name, len) == 0) {
			यदि (len == म_माप(cmdtp->name))
				प्रत्यागम cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	यदि (n_found == 1) {			/* exactly one match */
		प्रत्यागम cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	प्रत्यागम रिक्त;	/* not found or ambiguous command */
}

संरचना cmd_tbl *find_cmd(ध्रुव वर्ण *cmd)
{
	संरचना cmd_tbl *start = ll_entry_start(संरचना cmd_tbl, cmd);
	ध्रुव पूर्णाङ्क len = ll_entry_गिनती(संरचना cmd_tbl, cmd);
	प्रत्यागम find_cmd_tbl(cmd, start, len);
}

पूर्णाङ्क cmd_usage(ध्रुव संरचना cmd_tbl *cmdtp)
{
	म_लिखो("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	म_लिखो("Usage:\n%s ", cmdtp->name);

	यदि (!cmdtp->help) {
		माला_दो ("- No additional help available.\n");
		प्रत्यागम 1;
	}

	माला_दो(cmdtp->help);
	अ_दो('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	प्रत्यागम 1;
}

#ifdef CONFIG_AUTO_COMPLETE
स्थिर वर्ण env_complete_buf[512];

पूर्णाङ्क var_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char, पूर्णाङ्क maxv,
		 वर्ण *cmdv[])
{
	पूर्णाङ्क space;

	space = last_char == '\0' || isblank(last_char);

	यदि (space && argc == 1)
		प्रत्यागम env_complete("", maxv, cmdv, परिमाण(env_complete_buf),
				    env_complete_buf, false);

	यदि (!space && argc == 2)
		प्रत्यागम env_complete(argv[1], maxv, cmdv,
				    परिमाण(env_complete_buf),
				    env_complete_buf, false);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क dollar_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char,
			   पूर्णाङ्क maxv, वर्ण *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	यदि (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		प्रत्यागम 0;

	प्रत्यागम env_complete(argv[argc - 1], maxv, cmdv, परिमाण(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

पूर्णाङ्क complete_subcmdv(संरचना cmd_tbl *cmdtp, पूर्णाङ्क गिनती, पूर्णाङ्क argc,
		     वर्ण *ध्रुव argv[], वर्ण last_char,
		     पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ध्रुव संरचना cmd_tbl *cmdend = cmdtp + गिनती;
	ध्रुव वर्ण *p;
	पूर्णाङ्क len, clen;
	पूर्णाङ्क n_found = 0;
	ध्रुव वर्ण *cmd;

	/* sanity? */
	यदि (maxv < 2)
		प्रत्यागम -2;

	cmdv[0] = रिक्त;

	यदि (argc == 0) {
		/* output full list of commands */
		प्रति (; cmdtp != cmdend; cmdtp++) {
			यदि (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				भङ्ग;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = रिक्त;
		प्रत्यागम n_found;
	}

	/* more than one arg or one but the start of the next */
	यदि (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, गिनती);
		यदि (cmdtp == रिक्त || cmdtp->complete == रिक्त) {
			cmdv[0] = रिक्त;
			प्रत्यागम 0;
		}
		प्रत्यागम (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = म_अक्षर(cmd, '.');
	यदि (p == रिक्त)
		len = म_माप(cmd);
	अन्यथा
		len = p - cmd;

	/* return the partial matches */
	प्रति (; cmdtp != cmdend; cmdtp++) {

		clen = म_माप(cmdtp->name);
		यदि (clen < len)
			अनुवर्त;

		यदि (स_भेद(cmd, cmdtp->name, len) != 0)
			अनुवर्त;

		/* too many! */
		यदि (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			भङ्ग;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = रिक्त;
	प्रत्यागम n_found;
#अन्यथा
	प्रत्यागम 0;
#endif
}

स्थिर पूर्णाङ्क complete_cmdv(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char,
			 पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#ifdef CONFIG_CMDLINE
	प्रत्यागम complete_subcmdv(ll_entry_start(संरचना cmd_tbl, cmd),
				ll_entry_गिनती(संरचना cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#अन्यथा
	प्रत्यागम 0;
#endif
}

स्थिर पूर्णाङ्क make_argv(वर्ण *s, पूर्णाङ्क argvsz, वर्ण *argv[])
{
	पूर्णाङ्क argc = 0;

	/* split into argv */
	यावत् (argc < argvsz - 1) {

		/* skip any white space */
		यावत् (isblank(*s))
			++s;

		यदि (*s == '\0')	/* end of s, no more args	*/
			भङ्ग;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		यावत् (*s && !isblank(*s))
			++s;

		यदि (*s == '\0')		/* end of s, no more args	*/
			भङ्ग;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = रिक्त;

	प्रत्यागम argc;
}

स्थिर शून्य print_argv(ध्रुव वर्ण *banner, ध्रुव वर्ण *leader, ध्रुव वर्ण *sep,
		       पूर्णाङ्क linemax, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क ll = leader != रिक्त ? म_माप(leader) : 0;
	पूर्णाङ्क sl = sep != रिक्त ? म_माप(sep) : 0;
	पूर्णाङ्क len, i;

	यदि (banner) {
		माला_दो("\n");
		माला_दो(banner);
	}

	i = linemax;	/* force leader and newline */
	यावत् (*argv != रिक्त) {
		len = म_माप(*argv) + sl;
		यदि (i + len >= linemax) {
			माला_दो("\n");
			यदि (leader)
				माला_दो(leader);
			i = ll - sl;
		} अन्यथा यदि (sep)
			माला_दो(sep);
		माला_दो(*argv++);
		i += len;
	}
	म_लिखो("\n");
}

स्थिर पूर्णाङ्क find_common_prefix(वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i, len;
	वर्ण *anchor, *s, *t;

	यदि (*argv == रिक्त)
		प्रत्यागम 0;

	/* begin with max */
	anchor = *argv++;
	len = म_माप(anchor);
	यावत् ((t = *argv++) != रिक्त) {
		s = anchor;
		प्रति (i = 0; i < len; i++, t++, s++) {
			यदि (*t != *s)
				भङ्ग;
		}
		len = s - anchor;
	}
	प्रत्यागम len;
}

पूर्णाङ्क cmd_auto_complete(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, पूर्णाङ्क *np, पूर्णाङ्क *colp)
{
	वर्ण tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	पूर्णाङ्क n = *np, col = *colp;
	वर्ण *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	वर्ण *cmdv[20];
	वर्ण *s, *t;
	ध्रुव वर्ण *sep;
	पूर्णाङ्क i, j, k, len, seplen, argc;
	पूर्णाङ्क cnt;
	वर्ण last_char;
	ध्रुव वर्ण *ps_prompt;

	यदि (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		यदि (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} अन्यथा
		ps_prompt = CONFIG_SYS_PROMPT;

	यदि (म_भेद(prompt, ps_prompt) != 0)
		प्रत्यागम 0;	/* not in normal console */

	cnt = म_माप(buf);
	यदि (cnt >= 1)
		last_char = buf[cnt - 1];
	अन्यथा
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	म_नकल(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, परिमाण(argv)/परिमाण(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	यदि (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	यदि (i == 0) {
		यदि (argc > 1)	/* allow tab for non command */
			प्रत्यागम 0;
		अ_दो('\a');
		प्रत्यागम 1;
	}

	s = रिक्त;
	len = 0;
	sep = रिक्त;
	seplen = 0;
	यदि (i == 1) { /* one match; perfect */
		यदि (last_char != '\0' && !isblank(last_char))
			k = म_माप(argv[argc - 1]);
		अन्यथा
			k = 0;

		s = cmdv[0] + k;
		len = म_माप(s);
		sep = " ";
		seplen = 1;
	} अन्यथा यदि (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		यदि (last_char != '\0' && !isblank(last_char))
			k = म_माप(argv[argc - 1]);
		अन्यथा
			k = 0;

		j -= k;
		यदि (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	यदि (s != रिक्त) {
		k = len + seplen;
		/* make sure it fits */
		यदि (n + k >= CONFIG_SYS_CBSIZE - 2) {
			अ_दो('\a');
			प्रत्यागम 1;
		}

		t = buf + cnt;
		प्रति (i = 0; i < len; i++)
			*t++ = *s++;
		यदि (sep != रिक्त)
			प्रति (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		माला_दो(t - k);
		यदि (sep == रिक्त)
			अ_दो('\a');
		*np = n;
		*colp = col;
	} अन्यथा {
		print_argv(रिक्त, "  ", " ", 78, cmdv);

		माला_दो(prompt);
		माला_दो(buf);
	}
	प्रत्यागम 1;
}

#endif

#ifdef CMD_DATA_SIZE
पूर्णाङ्क cmd_get_data_size(ध्रुव वर्ण *arg, पूर्णाङ्क default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	पूर्णाङ्क len = म_माप(arg);
	यदि (len >= 2 && arg[len-2] == '.') {
		विकल्प (arg[len-1]) {
		पक्ष 'b':
			प्रत्यागम 1;
		पक्ष 'w':
			प्रत्यागम 2;
		पक्ष 'l':
			प्रत्यागम 4;
		पक्ष 's':
			प्रत्यागम CMD_DATA_SIZE_STR;
		पक्ष 'q':
			यदि (MEM_SUPPORT_64BIT_DATA)
				प्रत्यागम 8;
			fallthrough;
		स्वभाव:
			प्रत्यागम CMD_DATA_SIZE_ERR;
		}
	}
	प्रत्यागम default_size;
}
#endif

शून्य fixup_cmdtable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क size)
{
	पूर्णाङ्क	i;

	यदि (gd->reloc_off == 0)
		प्रत्यागम;

	प्रति (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(पूर्णाङ्क (*)(संरचना cmd_tbl *, पूर्णाङ्क, पूर्णाङ्क,
				 वर्ण * ध्रुव [], पूर्णाङ्क *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		म_लिखो("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (पूर्णाङ्क (*)(संरचना cmd_tbl *, पूर्णाङ्क, पूर्णाङ्क,
				      वर्ण *ध्रुव []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (वर्ण *)addr;
		यदि (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (वर्ण *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		यदि (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (वर्ण *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		यदि (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(पूर्णाङ्क (*)(पूर्णाङ्क, वर्ण * ध्रुव [], वर्ण, पूर्णाङ्क, वर्ण * []))addr;
		}
#endif
		cmdtp++;
	}
}

पूर्णाङ्क cmd_always_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			  वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	*repeatable = 1;

	प्रत्यागम cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णाङ्क cmd_never_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			 वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	*repeatable = 0;

	प्रत्यागम cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णाङ्क cmd_discard_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			   वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क repeatable;

	प्रत्यागम cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
}

/**
 * Call a command function. This should be the only route in U-Boot to call
 * a command, so that we can track whether we are waiting for input or
 * executing a command.
 *
 * @param cmdtp		Pointer to the command to execute
 * @param flag		Some flags normally 0 (see CMD_FLAG_.. above)
 * @param argc		Number of arguments (arg 0 must be the command text)
 * @param argv		Arguments
 * @param repeatable	Can the command be repeated
 * Return: 0 if command succeeded, else non-zero (CMD_RET_...)
 */
स्थिर पूर्णाङ्क cmd_call(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
		    वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	पूर्णाङ्क result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	यदि (result)
		debug("Command failed, result=%d\n", result);
	प्रत्यागम result;
}

गणना command_ret_t cmd_process(पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[],
			       पूर्णाङ्क *repeatable, ulong *ticks)
{
	गणना command_ret_t rc = CMD_RET_SUCCESS;
	संरचना cmd_tbl *cmdtp;

#यदि defined(CONFIG_SYS_XTRACE)
	वर्ण *xtrace;

	xtrace = env_get("xtrace");
	यदि (xtrace) {
		माला_दो("+");
		प्रति (पूर्णाङ्क i = 0; i < argc; i++) {
			माला_दो(" ");
			माला_दो(argv[i]);
		}
		माला_दो("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	यदि (cmdtp == रिक्त) {
		म_लिखो("Unknown command '%s' - try 'help'\n", argv[0]);
		प्रत्यागम 1;
	}

	/* found - check max args */
	यदि (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#यदि defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	अन्यथा यदि (cmdtp->cmd == do_bootd) {
		यदि (flag & CMD_FLAG_BOOTD) {
			माला_दो("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} अन्यथा {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	यदि (!rc) {
		पूर्णाङ्क newrep;

		यदि (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		यदि (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	यदि (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	प्रत्यागम rc;
}

पूर्णाङ्क cmd_process_error(संरचना cmd_tbl *cmdtp, पूर्णाङ्क err)
{
	यदि (err == CMD_RET_USAGE)
		प्रत्यागम CMD_RET_USAGE;

	यदि (err) {
		म_लिखो("Command '%s' failed: Error %d\n", cmdtp->name, err);
		प्रत्यागम CMD_RET_FAILURE;
	}

	प्रत्यागम CMD_RET_SUCCESS;
}

पूर्णाङ्क cmd_source_script(ulong addr, ध्रुव वर्ण *fit_uname, ध्रुव वर्ण *confname)
{
	वर्ण *data;
	शून्य *buf;
	uint len;
	पूर्णाङ्क ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	यदि (ret)
		प्रत्यागम CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	प्रत्यागम run_command_list(data, len, 0);
}
