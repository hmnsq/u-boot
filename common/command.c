// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#समावेश <config.h>
#समावेश <compiler.h>
#समावेश <command.h>
#समावेश <console.h>
#समावेश <env.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <mapmem.h>
#समावेश <time.h>
#समावेश <asm/global_data.h>
#समावेश <linux/ctype.h>

DECLARE_GLOBAL_तथ्य_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

पूर्णांक _do_help(काष्ठा cmd_tbl *cmd_start, पूर्णांक cmd_वस्तुएँ, काष्ठा cmd_tbl *cmdtp,
	     पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिर argv[])
{
	पूर्णांक i;
	पूर्णांक rcode = 0;

	अगर (argc == 1) {	/* show list of commands */
		काष्ठा cmd_tbl *cmd_array[cmd_वस्तुएँ];
		पूर्णांक i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		क्रम (i = 0; i < cmd_वस्तुएँ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		क्रम (i = cmd_वस्तुएँ - 1; i > 0; --i) {
			swaps = 0;
			क्रम (j = 0; j < i; ++j) {
				अगर (म_भेद(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					काष्ठा cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			अगर (!swaps)
				अवरोध;
		}

		/* print short help (usage) */
		क्रम (i = 0; i < cmd_वस्तुएँ; i++) {
			स्थिर अक्षर *usage = cmd_array[i]->usage;

			/* allow user abort */
			अगर (ctrlc())
				वापस 1;
			अगर (usage == शून्य)
				जारी;
			म_लिखो("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		वापस 0;
	}
	/*
	 * command help (long version)
	 */
	क्रम (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_वस्तुएँ);
		अगर (cmdtp != शून्य) {
			rcode |= cmd_usage(cmdtp);
		} अन्यथा {
			म_लिखो("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	वापस rcode;
}

/* find command table entry for a command */
काष्ठा cmd_tbl *find_cmd_tbl(स्थिर अक्षर *cmd, काष्ठा cmd_tbl *table,
			     पूर्णांक table_len)
{
#अगर_घोषित CONFIG_CMDLINE
	काष्ठा cmd_tbl *cmdtp;
	काष्ठा cmd_tbl *cmdtp_अस्थायी = table;	/* Init value */
	स्थिर अक्षर *p;
	पूर्णांक len;
	पूर्णांक n_found = 0;

	अगर (!cmd)
		वापस शून्य;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = म_अक्षर(cmd, '.')) == शून्य) ? म_माप (cmd) : (p - cmd);

	क्रम (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		अगर (म_भेदन(cmd, cmdtp->name, len) == 0) {
			अगर (len == म_माप(cmdtp->name))
				वापस cmdtp;	/* full match */

			cmdtp_अस्थायी = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	अगर (n_found == 1) {			/* exactly one match */
		वापस cmdtp_अस्थायी;
	}
#पूर्ण_अगर /* CONFIG_CMDLINE */

	वापस शून्य;	/* not found or ambiguous command */
}

काष्ठा cmd_tbl *find_cmd(स्थिर अक्षर *cmd)
{
	काष्ठा cmd_tbl *start = ll_entry_start(काष्ठा cmd_tbl, cmd);
	स्थिर पूर्णांक len = ll_entry_गिनती(काष्ठा cmd_tbl, cmd);
	वापस find_cmd_tbl(cmd, start, len);
}

पूर्णांक cmd_usage(स्थिर काष्ठा cmd_tbl *cmdtp)
{
	म_लिखो("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#अगर_घोषित	CONFIG_SYS_LONGHELP
	म_लिखो("Usage:\n%s ", cmdtp->name);

	अगर (!cmdtp->help) {
		माला_दो ("- No additional help available.\n");
		वापस 1;
	}

	माला_दो(cmdtp->help);
	अ_दो('\n');
#पूर्ण_अगर	/* CONFIG_SYS_LONGHELP */
	वापस 1;
}

#अगर_घोषित CONFIG_AUTO_COMPLETE
अटल अक्षर env_complete_buf[512];

पूर्णांक var_complete(पूर्णांक argc, अक्षर *स्थिर argv[], अक्षर last_char, पूर्णांक maxv,
		 अक्षर *cmdv[])
{
	पूर्णांक space;

	space = last_char == '\0' || isblank(last_char);

	अगर (space && argc == 1)
		वापस env_complete("", maxv, cmdv, माप(env_complete_buf),
				    env_complete_buf, false);

	अगर (!space && argc == 2)
		वापस env_complete(argv[1], maxv, cmdv,
				    माप(env_complete_buf),
				    env_complete_buf, false);

	वापस 0;
}

अटल पूर्णांक dollar_complete(पूर्णांक argc, अक्षर *स्थिर argv[], अक्षर last_char,
			   पूर्णांक maxv, अक्षर *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	अगर (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		वापस 0;

	वापस env_complete(argv[argc - 1], maxv, cmdv, माप(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

पूर्णांक complete_subcmdv(काष्ठा cmd_tbl *cmdtp, पूर्णांक गिनती, पूर्णांक argc,
		     अक्षर *स्थिर argv[], अक्षर last_char,
		     पूर्णांक maxv, अक्षर *cmdv[])
{
#अगर_घोषित CONFIG_CMDLINE
	स्थिर काष्ठा cmd_tbl *cmdend = cmdtp + गिनती;
	स्थिर अक्षर *p;
	पूर्णांक len, clen;
	पूर्णांक n_found = 0;
	स्थिर अक्षर *cmd;

	/* sanity? */
	अगर (maxv < 2)
		वापस -2;

	cmdv[0] = शून्य;

	अगर (argc == 0) {
		/* output full list of commands */
		क्रम (; cmdtp != cmdend; cmdtp++) {
			अगर (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				अवरोध;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = शून्य;
		वापस n_found;
	}

	/* more than one arg or one but the start of the next */
	अगर (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, गिनती);
		अगर (cmdtp == शून्य || cmdtp->complete == शून्य) {
			cmdv[0] = शून्य;
			वापस 0;
		}
		वापस (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = म_अक्षर(cmd, '.');
	अगर (p == शून्य)
		len = म_माप(cmd);
	अन्यथा
		len = p - cmd;

	/* return the partial matches */
	क्रम (; cmdtp != cmdend; cmdtp++) {

		clen = म_माप(cmdtp->name);
		अगर (clen < len)
			जारी;

		अगर (स_भेद(cmd, cmdtp->name, len) != 0)
			जारी;

		/* too many! */
		अगर (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			अवरोध;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = शून्य;
	वापस n_found;
#अन्यथा
	वापस 0;
#पूर्ण_अगर
}

अटल पूर्णांक complete_cmdv(पूर्णांक argc, अक्षर *स्थिर argv[], अक्षर last_char,
			 पूर्णांक maxv, अक्षर *cmdv[])
{
#अगर_घोषित CONFIG_CMDLINE
	वापस complete_subcmdv(ll_entry_start(काष्ठा cmd_tbl, cmd),
				ll_entry_गिनती(काष्ठा cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#अन्यथा
	वापस 0;
#पूर्ण_अगर
}

अटल पूर्णांक make_argv(अक्षर *s, पूर्णांक argvsz, अक्षर *argv[])
{
	पूर्णांक argc = 0;

	/* split into argv */
	जबतक (argc < argvsz - 1) {

		/* skip any white space */
		जबतक (isblank(*s))
			++s;

		अगर (*s == '\0')	/* end of s, no more args	*/
			अवरोध;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		जबतक (*s && !isblank(*s))
			++s;

		अगर (*s == '\0')		/* end of s, no more args	*/
			अवरोध;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = शून्य;

	वापस argc;
}

अटल व्योम print_argv(स्थिर अक्षर *banner, स्थिर अक्षर *leader, स्थिर अक्षर *sep,
		       पूर्णांक linemax, अक्षर *स्थिर argv[])
{
	पूर्णांक ll = leader != शून्य ? म_माप(leader) : 0;
	पूर्णांक sl = sep != शून्य ? म_माप(sep) : 0;
	पूर्णांक len, i;

	अगर (banner) {
		माला_दो("\n");
		माला_दो(banner);
	}

	i = linemax;	/* force leader and newline */
	जबतक (*argv != शून्य) {
		len = म_माप(*argv) + sl;
		अगर (i + len >= linemax) {
			माला_दो("\n");
			अगर (leader)
				माला_दो(leader);
			i = ll - sl;
		} अन्यथा अगर (sep)
			माला_दो(sep);
		माला_दो(*argv++);
		i += len;
	}
	म_लिखो("\n");
}

अटल पूर्णांक find_common_prefix(अक्षर *स्थिर argv[])
{
	पूर्णांक i, len;
	अक्षर *anchor, *s, *t;

	अगर (*argv == शून्य)
		वापस 0;

	/* begin with max */
	anchor = *argv++;
	len = म_माप(anchor);
	जबतक ((t = *argv++) != शून्य) {
		s = anchor;
		क्रम (i = 0; i < len; i++, t++, s++) {
			अगर (*t != *s)
				अवरोध;
		}
		len = s - anchor;
	}
	वापस len;
}

पूर्णांक cmd_auto_complete(स्थिर अक्षर *स्थिर prompt, अक्षर *buf, पूर्णांक *np, पूर्णांक *colp)
{
	अक्षर tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	पूर्णांक n = *np, col = *colp;
	अक्षर *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	अक्षर *cmdv[20];
	अक्षर *s, *t;
	स्थिर अक्षर *sep;
	पूर्णांक i, j, k, len, seplen, argc;
	पूर्णांक cnt;
	अक्षर last_char;
	स्थिर अक्षर *ps_prompt;

	अगर (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		अगर (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} अन्यथा
		ps_prompt = CONFIG_SYS_PROMPT;

	अगर (म_भेद(prompt, ps_prompt) != 0)
		वापस 0;	/* not in normal console */

	cnt = म_माप(buf);
	अगर (cnt >= 1)
		last_char = buf[cnt - 1];
	अन्यथा
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	म_नकल(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, माप(argv)/माप(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    माप(cmdv) / माप(cmdv[0]), cmdv);
	अगर (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  माप(cmdv) / माप(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	अगर (i == 0) {
		अगर (argc > 1)	/* allow tab for non command */
			वापस 0;
		अ_दो('\a');
		वापस 1;
	}

	s = शून्य;
	len = 0;
	sep = शून्य;
	seplen = 0;
	अगर (i == 1) { /* one match; perfect */
		अगर (last_char != '\0' && !isblank(last_char))
			k = म_माप(argv[argc - 1]);
		अन्यथा
			k = 0;

		s = cmdv[0] + k;
		len = म_माप(s);
		sep = " ";
		seplen = 1;
	} अन्यथा अगर (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		अगर (last_char != '\0' && !isblank(last_char))
			k = म_माप(argv[argc - 1]);
		अन्यथा
			k = 0;

		j -= k;
		अगर (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	अगर (s != शून्य) {
		k = len + seplen;
		/* make sure it fits */
		अगर (n + k >= CONFIG_SYS_CBSIZE - 2) {
			अ_दो('\a');
			वापस 1;
		}

		t = buf + cnt;
		क्रम (i = 0; i < len; i++)
			*t++ = *s++;
		अगर (sep != शून्य)
			क्रम (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		माला_दो(t - k);
		अगर (sep == शून्य)
			अ_दो('\a');
		*np = n;
		*colp = col;
	} अन्यथा {
		print_argv(शून्य, "  ", " ", 78, cmdv);

		माला_दो(prompt);
		माला_दो(buf);
	}
	वापस 1;
}

#पूर्ण_अगर

#अगर_घोषित CMD_तथ्य_आकार
पूर्णांक cmd_get_तथ्य_आकार(स्थिर अक्षर *arg, पूर्णांक default_आकार)
{
	/* Check for a size specification .b, .w or .l.
	 */
	पूर्णांक len = म_माप(arg);
	अगर (len >= 2 && arg[len-2] == '.') {
		चयन (arg[len-1]) {
		हाल 'b':
			वापस 1;
		हाल 'w':
			वापस 2;
		हाल 'l':
			वापस 4;
		हाल 's':
			वापस CMD_तथ्य_आकार_STR;
		हाल 'q':
			अगर (MEM_SUPPORT_64_BIT_तथ्य)
				वापस 8;
			fallthrough;
		शेष:
			वापस CMD_तथ्य_आकार_ERR;
		}
	}
	वापस default_आकार;
}
#पूर्ण_अगर

व्योम fixup_cmdtable(काष्ठा cmd_tbl *cmdtp, पूर्णांक आकार)
{
	पूर्णांक	i;

	अगर (gd->reloc_off == 0)
		वापस;

	क्रम (i = 0; i < आकार; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(पूर्णांक (*)(काष्ठा cmd_tbl *, पूर्णांक, पूर्णांक,
				 अक्षर * स्थिर [], पूर्णांक *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#अगर_घोषित DEBUG_COMMANDS
		म_लिखो("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#पूर्ण_अगर
		cmdtp->cmd = (पूर्णांक (*)(काष्ठा cmd_tbl *, पूर्णांक, पूर्णांक,
				      अक्षर *स्थिर []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (अक्षर *)addr;
		अगर (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (अक्षर *)addr;
		}
#अगर_घोषित	CONFIG_SYS_LONGHELP
		अगर (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (अक्षर *)addr;
		}
#पूर्ण_अगर
#अगर_घोषित CONFIG_AUTO_COMPLETE
		अगर (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(पूर्णांक (*)(पूर्णांक, अक्षर * स्थिर [], अक्षर, पूर्णांक, अक्षर * []))addr;
		}
#पूर्ण_अगर
		cmdtp++;
	}
}

पूर्णांक cmd_always_repeatable(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
			  अक्षर *स्थिर argv[], पूर्णांक *repeatable)
{
	*repeatable = 1;

	वापस cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णांक cmd_never_repeatable(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
			 अक्षर *स्थिर argv[], पूर्णांक *repeatable)
{
	*repeatable = 0;

	वापस cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णांक cmd_discard_repeatable(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
			   अक्षर *स्थिर argv[])
{
	पूर्णांक repeatable;

	वापस cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
अटल पूर्णांक cmd_call(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
		    अक्षर *स्थिर argv[], पूर्णांक *repeatable)
{
	पूर्णांक परिणाम;

	परिणाम = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	अगर (परिणाम)
		debug("Command failed, result=%d\n", परिणाम);
	वापस परिणाम;
}

क्रमागत command_ret_t cmd_process(पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिर argv[],
			       पूर्णांक *repeatable, ulong *ticks)
{
	क्रमागत command_ret_t rc = CMD_RET_SUCCESS;
	काष्ठा cmd_tbl *cmdtp;

#अगर defined(CONFIG_SYS_XTRACE)
	अक्षर *xtrace;

	xtrace = env_get("xtrace");
	अगर (xtrace) {
		माला_दो("+");
		क्रम (पूर्णांक i = 0; i < argc; i++) {
			माला_दो(" ");
			माला_दो(argv[i]);
		}
		माला_दो("\n");
	}
#पूर्ण_अगर

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	अगर (cmdtp == शून्य) {
		म_लिखो("Unknown command '%s' - try 'help'\n", argv[0]);
		वापस 1;
	}

	/* found - check max args */
	अगर (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#अगर defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	अन्यथा अगर (cmdtp->cmd == do_bootd) {
		अगर (flag & CMD_FLAG_BOOTD) {
			माला_दो("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} अन्यथा {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#पूर्ण_अगर

	/* If OK so far, then do the command */
	अगर (!rc) {
		पूर्णांक newrep;

		अगर (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		अगर (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	अगर (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	वापस rc;
}

पूर्णांक cmd_process_error(काष्ठा cmd_tbl *cmdtp, पूर्णांक err)
{
	अगर (err == CMD_RET_USAGE)
		वापस CMD_RET_USAGE;

	अगर (err) {
		म_लिखो("Command '%s' failed: Error %d\n", cmdtp->name, err);
		वापस CMD_RET_FAILURE;
	}

	वापस CMD_RET_SUCCESS;
}

पूर्णांक cmd_source_script(ulong addr, स्थिर अक्षर *fit_uname, स्थिर अक्षर *confname)
{
	अक्षर *तथ्य;
	व्योम *buf;
	uint len;
	पूर्णांक ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &तथ्य, &len);
	unmap_sysmem(buf);
	अगर (ret)
		वापस CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	वापस run_command_list(तथ्य, len, 0);
}
