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

पूर्णांक _do_help(संरचना cmd_tbl *cmd_start, पूर्णांक cmd_वस्तुसंच, संरचना cmd_tbl *cmdtp,
	     पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिरांक argv[])
{
	पूर्णांक i;
	पूर्णांक rcode = 0;

	जर (argc == 1) {	/* show list of commands */
		संरचना cmd_tbl *cmd_array[cmd_वस्तुसंच];
		पूर्णांक i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		साठी (i = 0; i < cmd_वस्तुसंच; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		साठी (i = cmd_वस्तुसंच - 1; i > 0; --i) {
			swaps = 0;
			साठी (j = 0; j < i; ++j) {
				जर (म_भेद(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					संरचना cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			जर (!swaps)
				थांबा;
		}

		/* print short help (usage) */
		साठी (i = 0; i < cmd_वस्तुसंच; i++) {
			स्थिरांक अक्षर *usage = cmd_array[i]->usage;

			/* allow user abort */
			जर (ctrlc())
				परत 1;
			जर (usage == रिक्त)
				सुरूठेवा;
			म_लिखो("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		परत 0;
	}
	/*
	 * command help (long version)
	 */
	साठी (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_वस्तुसंच);
		जर (cmdtp != रिक्त) {
			rcode |= cmd_usage(cmdtp);
		} अन्यथा {
			म_लिखो("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	परत rcode;
}

/* find command table entry for a command */
संरचना cmd_tbl *find_cmd_tbl(स्थिरांक अक्षर *cmd, संरचना cmd_tbl *table,
			     पूर्णांक table_len)
{
#ifdef CONFIG_CMDLINE
	संरचना cmd_tbl *cmdtp;
	संरचना cmd_tbl *cmdtp_temp = table;	/* Init value */
	स्थिरांक अक्षर *p;
	पूर्णांक len;
	पूर्णांक n_found = 0;

	जर (!cmd)
		परत रिक्त;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = म_अक्षर(cmd, '.')) == रिक्त) ? म_माप (cmd) : (p - cmd);

	साठी (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		जर (म_भेदन(cmd, cmdtp->name, len) == 0) {
			जर (len == म_माप(cmdtp->name))
				परत cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	जर (n_found == 1) {			/* exactly one match */
		परत cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	परत रिक्त;	/* not found or ambiguous command */
}

संरचना cmd_tbl *find_cmd(स्थिरांक अक्षर *cmd)
{
	संरचना cmd_tbl *start = ll_entry_start(संरचना cmd_tbl, cmd);
	स्थिरांक पूर्णांक len = ll_entry_मोजणी(संरचना cmd_tbl, cmd);
	परत find_cmd_tbl(cmd, start, len);
}

पूर्णांक cmd_usage(स्थिरांक संरचना cmd_tbl *cmdtp)
{
	म_लिखो("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	म_लिखो("Usage:\n%s ", cmdtp->name);

	जर (!cmdtp->help) {
		माला_दो ("- No additional help available.\n");
		परत 1;
	}

	माला_दो(cmdtp->help);
	अ_दो('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	परत 1;
}

#ifdef CONFIG_AUTO_COMPLETE
स्थिर अक्षर env_complete_buf[512];

पूर्णांक var_complete(पूर्णांक argc, अक्षर *स्थिरांक argv[], अक्षर last_char, पूर्णांक maxv,
		 अक्षर *cmdv[])
{
	पूर्णांक space;

	space = last_char == '\0' || isblank(last_char);

	जर (space && argc == 1)
		परत env_complete("", maxv, cmdv, आकार(env_complete_buf),
				    env_complete_buf, false);

	जर (!space && argc == 2)
		परत env_complete(argv[1], maxv, cmdv,
				    आकार(env_complete_buf),
				    env_complete_buf, false);

	परत 0;
}

स्थिर पूर्णांक dollar_complete(पूर्णांक argc, अक्षर *स्थिरांक argv[], अक्षर last_char,
			   पूर्णांक maxv, अक्षर *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	जर (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		परत 0;

	परत env_complete(argv[argc - 1], maxv, cmdv, आकार(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

पूर्णांक complete_subcmdv(संरचना cmd_tbl *cmdtp, पूर्णांक मोजणी, पूर्णांक argc,
		     अक्षर *स्थिरांक argv[], अक्षर last_char,
		     पूर्णांक maxv, अक्षर *cmdv[])
{
#ifdef CONFIG_CMDLINE
	स्थिरांक संरचना cmd_tbl *cmdend = cmdtp + मोजणी;
	स्थिरांक अक्षर *p;
	पूर्णांक len, clen;
	पूर्णांक n_found = 0;
	स्थिरांक अक्षर *cmd;

	/* sanity? */
	जर (maxv < 2)
		परत -2;

	cmdv[0] = रिक्त;

	जर (argc == 0) {
		/* output full list of commands */
		साठी (; cmdtp != cmdend; cmdtp++) {
			जर (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				थांबा;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = रिक्त;
		परत n_found;
	}

	/* more than one arg or one but the start of the next */
	जर (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, मोजणी);
		जर (cmdtp == रिक्त || cmdtp->complete == रिक्त) {
			cmdv[0] = रिक्त;
			परत 0;
		}
		परत (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = म_अक्षर(cmd, '.');
	जर (p == रिक्त)
		len = म_माप(cmd);
	अन्यथा
		len = p - cmd;

	/* return the partial matches */
	साठी (; cmdtp != cmdend; cmdtp++) {

		clen = म_माप(cmdtp->name);
		जर (clen < len)
			सुरूठेवा;

		जर (स_भेद(cmd, cmdtp->name, len) != 0)
			सुरूठेवा;

		/* too many! */
		जर (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			थांबा;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = रिक्त;
	परत n_found;
#अन्यथा
	परत 0;
#endif
}

स्थिर पूर्णांक complete_cmdv(पूर्णांक argc, अक्षर *स्थिरांक argv[], अक्षर last_char,
			 पूर्णांक maxv, अक्षर *cmdv[])
{
#ifdef CONFIG_CMDLINE
	परत complete_subcmdv(ll_entry_start(संरचना cmd_tbl, cmd),
				ll_entry_मोजणी(संरचना cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#अन्यथा
	परत 0;
#endif
}

स्थिर पूर्णांक make_argv(अक्षर *s, पूर्णांक argvsz, अक्षर *argv[])
{
	पूर्णांक argc = 0;

	/* split into argv */
	पर्यंत (argc < argvsz - 1) {

		/* skip any white space */
		पर्यंत (isblank(*s))
			++s;

		जर (*s == '\0')	/* end of s, no more args	*/
			थांबा;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		पर्यंत (*s && !isblank(*s))
			++s;

		जर (*s == '\0')		/* end of s, no more args	*/
			थांबा;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = रिक्त;

	परत argc;
}

स्थिर शून्य print_argv(स्थिरांक अक्षर *banner, स्थिरांक अक्षर *leader, स्थिरांक अक्षर *sep,
		       पूर्णांक linemax, अक्षर *स्थिरांक argv[])
{
	पूर्णांक ll = leader != रिक्त ? म_माप(leader) : 0;
	पूर्णांक sl = sep != रिक्त ? म_माप(sep) : 0;
	पूर्णांक len, i;

	जर (banner) {
		माला_दो("\n");
		माला_दो(banner);
	}

	i = linemax;	/* force leader and newline */
	पर्यंत (*argv != रिक्त) {
		len = म_माप(*argv) + sl;
		जर (i + len >= linemax) {
			माला_दो("\n");
			जर (leader)
				माला_दो(leader);
			i = ll - sl;
		} अन्यथा जर (sep)
			माला_दो(sep);
		माला_दो(*argv++);
		i += len;
	}
	म_लिखो("\n");
}

स्थिर पूर्णांक find_common_prefix(अक्षर *स्थिरांक argv[])
{
	पूर्णांक i, len;
	अक्षर *anchor, *s, *t;

	जर (*argv == रिक्त)
		परत 0;

	/* begin with max */
	anchor = *argv++;
	len = म_माप(anchor);
	पर्यंत ((t = *argv++) != रिक्त) {
		s = anchor;
		साठी (i = 0; i < len; i++, t++, s++) {
			जर (*t != *s)
				थांबा;
		}
		len = s - anchor;
	}
	परत len;
}

पूर्णांक cmd_auto_complete(स्थिरांक अक्षर *स्थिरांक prompt, अक्षर *buf, पूर्णांक *np, पूर्णांक *colp)
{
	अक्षर tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	पूर्णांक n = *np, col = *colp;
	अक्षर *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	अक्षर *cmdv[20];
	अक्षर *s, *t;
	स्थिरांक अक्षर *sep;
	पूर्णांक i, j, k, len, seplen, argc;
	पूर्णांक cnt;
	अक्षर last_char;
	स्थिरांक अक्षर *ps_prompt;

	जर (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		जर (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} अन्यथा
		ps_prompt = CONFIG_SYS_PROMPT;

	जर (म_भेद(prompt, ps_prompt) != 0)
		परत 0;	/* not in normal console */

	cnt = म_माप(buf);
	जर (cnt >= 1)
		last_char = buf[cnt - 1];
	अन्यथा
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	म_नकल(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, आकार(argv)/आकार(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    आकार(cmdv) / आकार(cmdv[0]), cmdv);
	जर (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  आकार(cmdv) / आकार(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	जर (i == 0) {
		जर (argc > 1)	/* allow tab for non command */
			परत 0;
		अ_दो('\a');
		परत 1;
	}

	s = रिक्त;
	len = 0;
	sep = रिक्त;
	seplen = 0;
	जर (i == 1) { /* one match; perfect */
		जर (last_char != '\0' && !isblank(last_char))
			k = म_माप(argv[argc - 1]);
		अन्यथा
			k = 0;

		s = cmdv[0] + k;
		len = म_माप(s);
		sep = " ";
		seplen = 1;
	} अन्यथा जर (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		जर (last_char != '\0' && !isblank(last_char))
			k = म_माप(argv[argc - 1]);
		अन्यथा
			k = 0;

		j -= k;
		जर (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	जर (s != रिक्त) {
		k = len + seplen;
		/* make sure it fits */
		जर (n + k >= CONFIG_SYS_CBSIZE - 2) {
			अ_दो('\a');
			परत 1;
		}

		t = buf + cnt;
		साठी (i = 0; i < len; i++)
			*t++ = *s++;
		जर (sep != रिक्त)
			साठी (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		माला_दो(t - k);
		जर (sep == रिक्त)
			अ_दो('\a');
		*np = n;
		*colp = col;
	} अन्यथा {
		print_argv(रिक्त, "  ", " ", 78, cmdv);

		माला_दो(prompt);
		माला_दो(buf);
	}
	परत 1;
}

#endif

#ifdef CMD_DATA_SIZE
पूर्णांक cmd_get_data_size(स्थिरांक अक्षर *arg, पूर्णांक default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	पूर्णांक len = म_माप(arg);
	जर (len >= 2 && arg[len-2] == '.') {
		निवड (arg[len-1]) {
		स्थिती 'b':
			परत 1;
		स्थिती 'w':
			परत 2;
		स्थिती 'l':
			परत 4;
		स्थिती 's':
			परत CMD_DATA_SIZE_STR;
		स्थिती 'q':
			जर (MEM_SUPPORT_64BIT_DATA)
				परत 8;
			fallthrough;
		पूर्वनिर्धारित:
			परत CMD_DATA_SIZE_ERR;
		}
	}
	परत default_size;
}
#endif

शून्य fixup_cmdtable(संरचना cmd_tbl *cmdtp, पूर्णांक size)
{
	पूर्णांक	i;

	जर (gd->reloc_off == 0)
		परत;

	साठी (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(पूर्णांक (*)(संरचना cmd_tbl *, पूर्णांक, पूर्णांक,
				 अक्षर * स्थिरांक [], पूर्णांक *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		म_लिखो("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (पूर्णांक (*)(संरचना cmd_tbl *, पूर्णांक, पूर्णांक,
				      अक्षर *स्थिरांक []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (अक्षर *)addr;
		जर (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (अक्षर *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		जर (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (अक्षर *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		जर (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(पूर्णांक (*)(पूर्णांक, अक्षर * स्थिरांक [], अक्षर, पूर्णांक, अक्षर * []))addr;
		}
#endif
		cmdtp++;
	}
}

पूर्णांक cmd_always_repeatable(संरचना cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
			  अक्षर *स्थिरांक argv[], पूर्णांक *repeatable)
{
	*repeatable = 1;

	परत cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णांक cmd_never_repeatable(संरचना cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
			 अक्षर *स्थिरांक argv[], पूर्णांक *repeatable)
{
	*repeatable = 0;

	परत cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णांक cmd_discard_repeatable(संरचना cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
			   अक्षर *स्थिरांक argv[])
{
	पूर्णांक repeatable;

	परत cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
स्थिर पूर्णांक cmd_call(संरचना cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
		    अक्षर *स्थिरांक argv[], पूर्णांक *repeatable)
{
	पूर्णांक result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	जर (result)
		debug("Command failed, result=%d\n", result);
	परत result;
}

गणना command_ret_t cmd_process(पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिरांक argv[],
			       पूर्णांक *repeatable, ulong *ticks)
{
	गणना command_ret_t rc = CMD_RET_SUCCESS;
	संरचना cmd_tbl *cmdtp;

#जर defined(CONFIG_SYS_XTRACE)
	अक्षर *xtrace;

	xtrace = env_get("xtrace");
	जर (xtrace) {
		माला_दो("+");
		साठी (पूर्णांक i = 0; i < argc; i++) {
			माला_दो(" ");
			माला_दो(argv[i]);
		}
		माला_दो("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	जर (cmdtp == रिक्त) {
		म_लिखो("Unknown command '%s' - try 'help'\n", argv[0]);
		परत 1;
	}

	/* found - check max args */
	जर (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#जर defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	अन्यथा जर (cmdtp->cmd == do_bootd) {
		जर (flag & CMD_FLAG_BOOTD) {
			माला_दो("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} अन्यथा {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	जर (!rc) {
		पूर्णांक newrep;

		जर (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		जर (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	जर (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	परत rc;
}

पूर्णांक cmd_process_error(संरचना cmd_tbl *cmdtp, पूर्णांक err)
{
	जर (err == CMD_RET_USAGE)
		परत CMD_RET_USAGE;

	जर (err) {
		म_लिखो("Command '%s' failed: Error %d\n", cmdtp->name, err);
		परत CMD_RET_FAILURE;
	}

	परत CMD_RET_SUCCESS;
}

पूर्णांक cmd_source_script(ulong addr, स्थिरांक अक्षर *fit_uname, स्थिरांक अक्षर *confname)
{
	अक्षर *data;
	शून्य *buf;
	uint len;
	पूर्णांक ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	जर (ret)
		परत CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	परत run_command_list(data, len, 0);
}
