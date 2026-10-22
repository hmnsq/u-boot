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

	जइ (argc == 1) {	/* show list of commands */
		संरचना cmd_tbl *cmd_array[cmd_वस्तूनि];
		पूर्णाङ्क i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		प्रति (i = 0; i < cmd_वस्तूनि; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		प्रति (i = cmd_वस्तूनि - 1; i > 0; --i) {
			swaps = 0;
			प्रति (j = 0; j < i; ++j) {
				जइ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					संरचना cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			जइ (!swaps)
				भङ्ग;
		}

		/* print short help (usage) */
		प्रति (i = 0; i < cmd_वस्तूनि; i++) {
			ध्रुव वर्ण *usage = cmd_array[i]->usage;

			/* allow user abort */
			जइ (ctrlc())
				प्रत्यागम 1;
			जइ (usage == रिक्त)
				अनुवर्त;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		प्रत्यागम 0;
	}
	/*
	 * command help (long version)
	 */
	प्रति (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_वस्तूनि);
		जइ (cmdtp != रिक्त) {
			rcode |= cmd_usage(cmdtp);
		} अण्णहा {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
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

	जइ (!cmd)
		प्रत्यागम रिक्त;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == रिक्त) ? strlen (cmd) : (p - cmd);

	प्रति (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		जइ (strncmp(cmd, cmdtp->name, len) == 0) {
			जइ (len == strlen(cmdtp->name))
				प्रत्यागम cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	जइ (n_found == 1) {			/* exactly one match */
		प्रत्यागम cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	प्रत्यागम रिक्त;	/* not found or ambiguous command */
}

संरचना cmd_tbl *find_cmd(ध्रुव वर्ण *cmd)
{
	संरचना cmd_tbl *start = ll_entry_start(संरचना cmd_tbl, cmd);
	ध्रुव पूर्णाङ्क len = ll_entry_गणनम्(संरचना cmd_tbl, cmd);
	प्रत्यागम find_cmd_tbl(cmd, start, len);
}

पूर्णाङ्क cmd_usage(ध्रुव संरचना cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	जइ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		प्रत्यागम 1;
	}

	puts(cmdtp->help);
	putc('\n');
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

	जइ (space && argc == 1)
		प्रत्यागम env_complete("", maxv, cmdv, परिमाण(env_complete_buf),
				    env_complete_buf, false);

	जइ (!space && argc == 2)
		प्रत्यागम env_complete(argv[1], maxv, cmdv,
				    परिमाण(env_complete_buf),
				    env_complete_buf, false);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क dollar_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char,
			   पूर्णाङ्क maxv, वर्ण *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	जइ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		प्रत्यागम 0;

	प्रत्यागम env_complete(argv[argc - 1], maxv, cmdv, परिमाण(env_complete_buf),
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
	जइ (maxv < 2)
		प्रत्यागम -2;

	cmdv[0] = रिक्त;

	जइ (argc == 0) {
		/* output full list of commands */
		प्रति (; cmdtp != cmdend; cmdtp++) {
			जइ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				भङ्ग;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = रिक्त;
		प्रत्यागम n_found;
	}

	/* more than one arg or one but the start of the next */
	जइ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, गणनम्);
		जइ (cmdtp == रिक्त || cmdtp->complete == रिक्त) {
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
	p = strchr(cmd, '.');
	जइ (p == रिक्त)
		len = strlen(cmd);
	अण्णहा
		len = p - cmd;

	/* return the partial matches */
	प्रति (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		जइ (clen < len)
			अनुवर्त;

		जइ (memcmp(cmd, cmdtp->name, len) != 0)
			अनुवर्त;

		/* too many! */
		जइ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			भङ्ग;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = रिक्त;
	प्रत्यागम n_found;
#अण्णहा
	प्रत्यागम 0;
#endif
}

स्थिर पूर्णाङ्क complete_cmdv(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण last_char,
			 पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#ifdef CONFIG_CMDLINE
	प्रत्यागम complete_subcmdv(ll_entry_start(संरचना cmd_tbl, cmd),
				ll_entry_गणनम्(संरचना cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#अण्णहा
	प्रत्यागम 0;
#endif
}

स्थिर पूर्णाङ्क make_argv(वर्ण *s, पूर्णाङ्क argvsz, वर्ण *argv[])
{
	पूर्णाङ्क argc = 0;

	/* split into argv */
	जाव (argc < argvsz - 1) {

		/* skip any white space */
		जाव (isblank(*s))
			++s;

		जइ (*s == '\0')	/* end of s, no more args	*/
			भङ्ग;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		जाव (*s && !isblank(*s))
			++s;

		जइ (*s == '\0')		/* end of s, no more args	*/
			भङ्ग;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = रिक्त;

	प्रत्यागम argc;
}

स्थिर शून्य print_argv(ध्रुव वर्ण *banner, ध्रुव वर्ण *leader, ध्रुव वर्ण *sep,
		       पूर्णाङ्क linemax, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क ll = leader != रिक्त ? strlen(leader) : 0;
	पूर्णाङ्क sl = sep != रिक्त ? strlen(sep) : 0;
	पूर्णाङ्क len, i;

	जइ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	जाव (*argv != रिक्त) {
		len = strlen(*argv) + sl;
		जइ (i + len >= linemax) {
			puts("\n");
			जइ (leader)
				puts(leader);
			i = ll - sl;
		} अण्णहा जइ (sep)
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

	जइ (*argv == रिक्त)
		प्रत्यागम 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	जाव ((t = *argv++) != रिक्त) {
		s = anchor;
		प्रति (i = 0; i < len; i++, t++, s++) {
			जइ (*t != *s)
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

	जइ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		जइ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} अण्णहा
		ps_prompt = CONFIG_SYS_PROMPT;

	जइ (strcmp(prompt, ps_prompt) != 0)
		प्रत्यागम 0;	/* not in normal console */

	cnt = strlen(buf);
	जइ (cnt >= 1)
		last_char = buf[cnt - 1];
	अण्णहा
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, परिमाण(argv)/परिमाण(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	जइ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	जइ (i == 0) {
		जइ (argc > 1)	/* allow tab for non command */
			प्रत्यागम 0;
		putc('\a');
		प्रत्यागम 1;
	}

	s = रिक्त;
	len = 0;
	sep = रिक्त;
	seplen = 0;
	जइ (i == 1) { /* one match; perfect */
		जइ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		अण्णहा
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} अण्णहा जइ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		जइ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		अण्णहा
			k = 0;

		j -= k;
		जइ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	जइ (s != रिक्त) {
		k = len + seplen;
		/* make sure it fits */
		जइ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			प्रत्यागम 1;
		}

		t = buf + cnt;
		प्रति (i = 0; i < len; i++)
			*t++ = *s++;
		जइ (sep != रिक्त)
			प्रति (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		जइ (sep == रिक्त)
			putc('\a');
		*np = n;
		*colp = col;
	} अण्णहा {
		print_argv(रिक्त, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	प्रत्यागम 1;
}

#endif

#ifdef CMD_DATA_SIZE
पूर्णाङ्क cmd_get_data_size(ध्रुव वर्ण *arg, पूर्णाङ्क default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	पूर्णाङ्क len = strlen(arg);
	जइ (len >= 2 && arg[len-2] == '.') {
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
			जइ (MEM_SUPPORT_64BIT_DATA)
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

	जइ (gd->reloc_off == 0)
		प्रत्यागम;

	प्रति (i = 0; i < size; i++) {
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
		जइ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (वर्ण *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		जइ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (वर्ण *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		जइ (cmdtp->complete) {
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
	जइ (result)
		debug("Command failed, result=%d\n", result);
	प्रत्यागम result;
}

गणना command_ret_t cmd_process(पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[],
			       पूर्णाङ्क *repeatable, ulong *ticks)
{
	गणना command_ret_t rc = CMD_RET_SUCCESS;
	संरचना cmd_tbl *cmdtp;

#जइ defined(CONFIG_SYS_XTRACE)
	वर्ण *xtrace;

	xtrace = env_get("xtrace");
	जइ (xtrace) {
		puts("+");
		प्रति (पूर्णाङ्क i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	जइ (cmdtp == रिक्त) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		प्रत्यागम 1;
	}

	/* found - check max args */
	जइ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#जइ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	अण्णहा जइ (cmdtp->cmd == do_bootd) {
		जइ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} अण्णहा {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	जइ (!rc) {
		पूर्णाङ्क newrep;

		जइ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		जइ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	जइ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	प्रत्यागम rc;
}

पूर्णाङ्क cmd_process_error(संरचना cmd_tbl *cmdtp, पूर्णाङ्क err)
{
	जइ (err == CMD_RET_USAGE)
		प्रत्यागम CMD_RET_USAGE;

	जइ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
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
	जइ (ret)
		प्रत्यागम CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	प्रत्यागम run_command_list(data, len, 0);
}
