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

पूर्णाङ्क _do_help(संरचना cmd_tbl *cmd_start, पूर्णाङ्क cmd_वस्तूनि, संरचना cmd_tbl *cmdtp,
	     पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i;
	पूर्णाङ्क rcode = 0;

	सचे (argc == 1) {	/* show list of commands */
		संरचना cmd_tbl *cmd_array[cmd_वस्तूनि];
		पूर्णाङ्क i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		अत्थाय (i = 0; i < cmd_वस्तूनि; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		अत्थाय (i = cmd_वस्तूनि - 1; i > 0; --i) {
			swaps = 0;
			अत्थाय (j = 0; j < i; ++j) {
				सचे (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					संरचना cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			सचे (!swaps)
				भङ्ग;
		}

		/* print short help (usage) */
		अत्थाय (i = 0; i < cmd_वस्तूनि; i++) {
			ध्रुव वर्ण *usage = cmd_array[i]->usage;

			/* allow user abort */
			सचे (ctrlc())
				पटिनिवत्त 1;
			सचे (usage == रिक्त)
				अनुवर्त;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		पटिनिवत्त 0;
	}
	/*
	 * command help (long version)
	 */
	अत्थाय (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_वस्तूनि);
		सचे (cmdtp != रिक्त) {
			rcode |= cmd_usage(cmdtp);
		} अञ्ञथा {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	पटिनिवत्त rcode;
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

	सचे (!cmd)
		पटिनिवत्त रिक्त;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == रिक्त) ? strlen (cmd) : (p - cmd);

	अत्थाय (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		सचे (strncmp(cmd, cmdtp->name, len) == 0) {
			सचे (len == strlen(cmdtp->name))
				पटिनिवत्त cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	सचे (n_found == 1) {			/* exactly one match */
		पटिनिवत्त cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	पटिनिवत्त रिक्त;	/* not found or ambiguous command */
}

संरचना cmd_tbl *find_cmd(ध्रुव वर्ण *cmd)
{
	संरचना cmd_tbl *start = ll_entry_start(संरचना cmd_tbl, cmd);
	ध्रुव पूर्णाङ्क len = ll_entry_गणनम्(संरचना cmd_tbl, cmd);
	पटिनिवत्त find_cmd_tbl(cmd, start, len);
}

पूर्णाङ्क cmd_usage(ध्रुव संरचना cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	सचे (!cmdtp->help) {
		puts ("- No additional help available.\n");
		पटिनिवत्त 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	पटिनिवत्त 1;
}

#ifdef CONFIG_AUTO_COMPLETE
स्थिर वर्ण env_complete_buf[512];

पूर्णाङ्क var_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char, पूर्णाङ्क maxv,
		 वर्ण *cmdv[])
{
	पूर्णाङ्क space;

	space = last_char == '\0' || isblank(last_char);

	सचे (space && argc == 1)
		पटिनिवत्त env_complete("", maxv, cmdv, परिमाण(env_complete_buf),
				    env_complete_buf, false);

	सचे (!space && argc == 2)
		पटिनिवत्त env_complete(argv[1], maxv, cmdv,
				    परिमाण(env_complete_buf),
				    env_complete_buf, false);

	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क dollar_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char,
			   पूर्णाङ्क maxv, वर्ण *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	सचे (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		पटिनिवत्त 0;

	पटिनिवत्त env_complete(argv[argc - 1], maxv, cmdv, परिमाण(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

पूर्णाङ्क complete_subcmdv(संरचना cmd_tbl *cmdtp, पूर्णाङ्क गणनम्, पूर्णाङ्क argc,
		     वर्ण *ध्रुव argv[], वर्ण last_char,
		     पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ध्रुव संरचना cmd_tbl *cmdend = cmdtp + गणनम्;
	ध्रुव वर्ण *p;
	पूर्णाङ्क len, clen;
	पूर्णाङ्क n_found = 0;
	ध्रुव वर्ण *cmd;

	/* sanity? */
	सचे (maxv < 2)
		पटिनिवत्त -2;

	cmdv[0] = रिक्त;

	सचे (argc == 0) {
		/* output full list of commands */
		अत्थाय (; cmdtp != cmdend; cmdtp++) {
			सचे (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				भङ्ग;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = रिक्त;
		पटिनिवत्त n_found;
	}

	/* more than one arg or one but the start of the next */
	सचे (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, गणनम्);
		सचे (cmdtp == रिक्त || cmdtp->complete == रिक्त) {
			cmdv[0] = रिक्त;
			पटिनिवत्त 0;
		}
		पटिनिवत्त (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	सचे (p == रिक्त)
		len = strlen(cmd);
	अञ्ञथा
		len = p - cmd;

	/* return the partial matches */
	अत्थाय (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		सचे (clen < len)
			अनुवर्त;

		सचे (memcmp(cmd, cmdtp->name, len) != 0)
			अनुवर्त;

		/* too many! */
		सचे (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			भङ्ग;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = रिक्त;
	पटिनिवत्त n_found;
#अञ्ञथा
	पटिनिवत्त 0;
#endif
}

स्थिर पूर्णाङ्क complete_cmdv(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char,
			 पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#ifdef CONFIG_CMDLINE
	पटिनिवत्त complete_subcmdv(ll_entry_start(संरचना cmd_tbl, cmd),
				ll_entry_गणनम्(संरचना cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#अञ्ञथा
	पटिनिवत्त 0;
#endif
}

स्थिर पूर्णाङ्क make_argv(वर्ण *s, पूर्णाङ्क argvsz, वर्ण *argv[])
{
	पूर्णाङ्क argc = 0;

	/* split into argv */
	याव (argc < argvsz - 1) {

		/* skip any white space */
		याव (isblank(*s))
			++s;

		सचे (*s == '\0')	/* end of s, no more args	*/
			भङ्ग;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		याव (*s && !isblank(*s))
			++s;

		सचे (*s == '\0')		/* end of s, no more args	*/
			भङ्ग;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = रिक्त;

	पटिनिवत्त argc;
}

स्थिर शून्य print_argv(ध्रुव वर्ण *banner, ध्रुव वर्ण *leader, ध्रुव वर्ण *sep,
		       पूर्णाङ्क linemax, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क ll = leader != रिक्त ? strlen(leader) : 0;
	पूर्णाङ्क sl = sep != रिक्त ? strlen(sep) : 0;
	पूर्णाङ्क len, i;

	सचे (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	याव (*argv != रिक्त) {
		len = strlen(*argv) + sl;
		सचे (i + len >= linemax) {
			puts("\n");
			सचे (leader)
				puts(leader);
			i = ll - sl;
		} अञ्ञथा सचे (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

स्थिर पूर्णाङ्क find_common_prefix(वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i, len;
	वर्ण *anchor, *s, *t;

	सचे (*argv == रिक्त)
		पटिनिवत्त 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	याव ((t = *argv++) != रिक्त) {
		s = anchor;
		अत्थाय (i = 0; i < len; i++, t++, s++) {
			सचे (*t != *s)
				भङ्ग;
		}
		len = s - anchor;
	}
	पटिनिवत्त len;
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

	सचे (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		सचे (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} अञ्ञथा
		ps_prompt = CONFIG_SYS_PROMPT;

	सचे (strcmp(prompt, ps_prompt) != 0)
		पटिनिवत्त 0;	/* not in normal console */

	cnt = strlen(buf);
	सचे (cnt >= 1)
		last_char = buf[cnt - 1];
	अञ्ञथा
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, परिमाण(argv)/परिमाण(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	सचे (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	सचे (i == 0) {
		सचे (argc > 1)	/* allow tab for non command */
			पटिनिवत्त 0;
		putc('\a');
		पटिनिवत्त 1;
	}

	s = रिक्त;
	len = 0;
	sep = रिक्त;
	seplen = 0;
	सचे (i == 1) { /* one match; perfect */
		सचे (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		अञ्ञथा
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} अञ्ञथा सचे (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		सचे (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		अञ्ञथा
			k = 0;

		j -= k;
		सचे (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	सचे (s != रिक्त) {
		k = len + seplen;
		/* make sure it fits */
		सचे (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			पटिनिवत्त 1;
		}

		t = buf + cnt;
		अत्थाय (i = 0; i < len; i++)
			*t++ = *s++;
		सचे (sep != रिक्त)
			अत्थाय (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		सचे (sep == रिक्त)
			putc('\a');
		*np = n;
		*colp = col;
	} अञ्ञथा {
		print_argv(रिक्त, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	पटिनिवत्त 1;
}

#endif

#ifdef CMD_DATA_SIZE
पूर्णाङ्क cmd_get_data_size(ध्रुव वर्ण *arg, पूर्णाङ्क default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	पूर्णाङ्क len = strlen(arg);
	सचे (len >= 2 && arg[len-2] == '.') {
		विकल्प (arg[len-1]) {
		पक्ष 'b':
			पटिनिवत्त 1;
		पक्ष 'w':
			पटिनिवत्त 2;
		पक्ष 'l':
			पटिनिवत्त 4;
		पक्ष 's':
			पटिनिवत्त CMD_DATA_SIZE_STR;
		पक्ष 'q':
			सचे (MEM_SUPPORT_64BIT_DATA)
				पटिनिवत्त 8;
			fallthrough;
		स्वभाव:
			पटिनिवत्त CMD_DATA_SIZE_ERR;
		}
	}
	पटिनिवत्त default_size;
}
#endif

शून्य fixup_cmdtable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क size)
{
	पूर्णाङ्क	i;

	सचे (gd->reloc_off == 0)
		पटिनिवत्त;

	अत्थाय (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(पूर्णाङ्क (*)(संरचना cmd_tbl *, पूर्णाङ्क, पूर्णाङ्क,
				 वर्ण * ध्रुव [], पूर्णाङ्क *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (पूर्णाङ्क (*)(संरचना cmd_tbl *, पूर्णाङ्क, पूर्णाङ्क,
				      वर्ण *ध्रुव []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (वर्ण *)addr;
		सचे (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (वर्ण *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		सचे (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (वर्ण *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		सचे (cmdtp->complete) {
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

	पटिनिवत्त cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णाङ्क cmd_never_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			 वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	*repeatable = 0;

	पटिनिवत्त cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णाङ्क cmd_discard_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			   वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क repeatable;

	पटिनिवत्त cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	सचे (result)
		debug("Command failed, result=%d\n", result);
	पटिनिवत्त result;
}

गणना command_ret_t cmd_process(पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[],
			       पूर्णाङ्क *repeatable, ulong *ticks)
{
	गणना command_ret_t rc = CMD_RET_SUCCESS;
	संरचना cmd_tbl *cmdtp;

#सचे defined(CONFIG_SYS_XTRACE)
	वर्ण *xtrace;

	xtrace = env_get("xtrace");
	सचे (xtrace) {
		puts("+");
		अत्थाय (पूर्णाङ्क i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	सचे (cmdtp == रिक्त) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		पटिनिवत्त 1;
	}

	/* found - check max args */
	सचे (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#सचे defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	अञ्ञथा सचे (cmdtp->cmd == do_bootd) {
		सचे (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} अञ्ञथा {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	सचे (!rc) {
		पूर्णाङ्क newrep;

		सचे (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		सचे (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	सचे (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	पटिनिवत्त rc;
}

पूर्णाङ्क cmd_process_error(संरचना cmd_tbl *cmdtp, पूर्णाङ्क err)
{
	सचे (err == CMD_RET_USAGE)
		पटिनिवत्त CMD_RET_USAGE;

	सचे (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		पटिनिवत्त CMD_RET_FAILURE;
	}

	पटिनिवत्त CMD_RET_SUCCESS;
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
	सचे (ret)
		पटिनिवत्त CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	पटिनिवत्त run_command_list(data, len, 0);
}
