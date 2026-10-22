// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <compiler.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

પૂર્ણાંક _do_help(સંરચના cmd_tbl *cmd_start, પૂર્ણાંક cmd_વસ્તુઓ, સંરચના cmd_tbl *cmdtp,
	     પૂર્ણાંક flag, પૂર્ણાંક argc, અક્ષર *અચલ argv[])
{
	પૂર્ણાંક i;
	પૂર્ણાંક rcode = 0;

	જો (argc == 1) {	/* show list of commands */
		સંરચના cmd_tbl *cmd_array[cmd_વસ્તુઓ];
		પૂર્ણાંક i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		માટે (i = 0; i < cmd_વસ્તુઓ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		માટે (i = cmd_વસ્તુઓ - 1; i > 0; --i) {
			swaps = 0;
			માટે (j = 0; j < i; ++j) {
				જો (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					સંરચના cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			જો (!swaps)
				તોડો;
		}

		/* print short help (usage) */
		માટે (i = 0; i < cmd_વસ્તુઓ; i++) {
			અચલ અક્ષર *usage = cmd_array[i]->usage;

			/* allow user abort */
			જો (ctrlc())
				પાછા 1;
			જો (usage == ખાલી)
				ચાલુ;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		પાછા 0;
	}
	/*
	 * command help (long version)
	 */
	માટે (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_વસ્તુઓ);
		જો (cmdtp != ખાલી) {
			rcode |= cmd_usage(cmdtp);
		} નહીંતર {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	પાછા rcode;
}

/* find command table entry for a command */
સંરચના cmd_tbl *find_cmd_tbl(અચલ અક્ષર *cmd, સંરચના cmd_tbl *table,
			     પૂર્ણાંક table_len)
{
#ifdef CONFIG_CMDLINE
	સંરચના cmd_tbl *cmdtp;
	સંરચના cmd_tbl *cmdtp_temp = table;	/* Init value */
	અચલ અક્ષર *p;
	પૂર્ણાંક len;
	પૂર્ણાંક n_found = 0;

	જો (!cmd)
		પાછા ખાલી;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ખાલી) ? strlen (cmd) : (p - cmd);

	માટે (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		જો (strncmp(cmd, cmdtp->name, len) == 0) {
			જો (len == strlen(cmdtp->name))
				પાછા cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	જો (n_found == 1) {			/* exactly one match */
		પાછા cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	પાછા ખાલી;	/* not found or ambiguous command */
}

સંરચના cmd_tbl *find_cmd(અચલ અક્ષર *cmd)
{
	સંરચના cmd_tbl *start = ll_entry_start(સંરચના cmd_tbl, cmd);
	અચલ પૂર્ણાંક len = ll_entry_ગણતરી(સંરચના cmd_tbl, cmd);
	પાછા find_cmd_tbl(cmd, start, len);
}

પૂર્ણાંક cmd_usage(અચલ સંરચના cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	જો (!cmdtp->help) {
		puts ("- No additional help available.\n");
		પાછા 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	પાછા 1;
}

#ifdef CONFIG_AUTO_COMPLETE
સ્થિર અક્ષર env_complete_buf[512];

પૂર્ણાંક var_complete(પૂર્ણાંક argc, અક્ષર *અચલ argv[], અક્ષર last_char, પૂર્ણાંક maxv,
		 અક્ષર *cmdv[])
{
	પૂર્ણાંક space;

	space = last_char == '\0' || isblank(last_char);

	જો (space && argc == 1)
		પાછા env_complete("", maxv, cmdv, કદ(env_complete_buf),
				    env_complete_buf, false);

	જો (!space && argc == 2)
		પાછા env_complete(argv[1], maxv, cmdv,
				    કદ(env_complete_buf),
				    env_complete_buf, false);

	પાછા 0;
}

સ્થિર પૂર્ણાંક dollar_complete(પૂર્ણાંક argc, અક્ષર *અચલ argv[], અક્ષર last_char,
			   પૂર્ણાંક maxv, અક્ષર *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	જો (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		પાછા 0;

	પાછા env_complete(argv[argc - 1], maxv, cmdv, કદ(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

પૂર્ણાંક complete_subcmdv(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક ગણતરી, પૂર્ણાંક argc,
		     અક્ષર *અચલ argv[], અક્ષર last_char,
		     પૂર્ણાંક maxv, અક્ષર *cmdv[])
{
#ifdef CONFIG_CMDLINE
	અચલ સંરચના cmd_tbl *cmdend = cmdtp + ગણતરી;
	અચલ અક્ષર *p;
	પૂર્ણાંક len, clen;
	પૂર્ણાંક n_found = 0;
	અચલ અક્ષર *cmd;

	/* sanity? */
	જો (maxv < 2)
		પાછા -2;

	cmdv[0] = ખાલી;

	જો (argc == 0) {
		/* output full list of commands */
		માટે (; cmdtp != cmdend; cmdtp++) {
			જો (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				તોડો;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = ખાલી;
		પાછા n_found;
	}

	/* more than one arg or one but the start of the next */
	જો (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, ગણતરી);
		જો (cmdtp == ખાલી || cmdtp->complete == ખાલી) {
			cmdv[0] = ખાલી;
			પાછા 0;
		}
		પાછા (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	જો (p == ખાલી)
		len = strlen(cmd);
	નહીંતર
		len = p - cmd;

	/* return the partial matches */
	માટે (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		જો (clen < len)
			ચાલુ;

		જો (memcmp(cmd, cmdtp->name, len) != 0)
			ચાલુ;

		/* too many! */
		જો (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			તોડો;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = ખાલી;
	પાછા n_found;
#નહીંતર
	પાછા 0;
#endif
}

સ્થિર પૂર્ણાંક complete_cmdv(પૂર્ણાંક argc, અક્ષર *અચલ argv[], અક્ષર last_char,
			 પૂર્ણાંક maxv, અક્ષર *cmdv[])
{
#ifdef CONFIG_CMDLINE
	પાછા complete_subcmdv(ll_entry_start(સંરચના cmd_tbl, cmd),
				ll_entry_ગણતરી(સંરચના cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#નહીંતર
	પાછા 0;
#endif
}

સ્થિર પૂર્ણાંક make_argv(અક્ષર *s, પૂર્ણાંક argvsz, અક્ષર *argv[])
{
	પૂર્ણાંક argc = 0;

	/* split into argv */
	જ્યારે (argc < argvsz - 1) {

		/* skip any white space */
		જ્યારે (isblank(*s))
			++s;

		જો (*s == '\0')	/* end of s, no more args	*/
			તોડો;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		જ્યારે (*s && !isblank(*s))
			++s;

		જો (*s == '\0')		/* end of s, no more args	*/
			તોડો;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ખાલી;

	પાછા argc;
}

સ્થિર શૂન્ય print_argv(અચલ અક્ષર *banner, અચલ અક્ષર *leader, અચલ અક્ષર *sep,
		       પૂર્ણાંક linemax, અક્ષર *અચલ argv[])
{
	પૂર્ણાંક ll = leader != ખાલી ? strlen(leader) : 0;
	પૂર્ણાંક sl = sep != ખાલી ? strlen(sep) : 0;
	પૂર્ણાંક len, i;

	જો (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	જ્યારે (*argv != ખાલી) {
		len = strlen(*argv) + sl;
		જો (i + len >= linemax) {
			puts("\n");
			જો (leader)
				puts(leader);
			i = ll - sl;
		} નહીંતર જો (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

સ્થિર પૂર્ણાંક find_common_prefix(અક્ષર *અચલ argv[])
{
	પૂર્ણાંક i, len;
	અક્ષર *anchor, *s, *t;

	જો (*argv == ખાલી)
		પાછા 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	જ્યારે ((t = *argv++) != ખાલી) {
		s = anchor;
		માટે (i = 0; i < len; i++, t++, s++) {
			જો (*t != *s)
				તોડો;
		}
		len = s - anchor;
	}
	પાછા len;
}

પૂર્ણાંક cmd_auto_complete(અચલ અક્ષર *અચલ prompt, અક્ષર *buf, પૂર્ણાંક *np, પૂર્ણાંક *colp)
{
	અક્ષર tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	પૂર્ણાંક n = *np, col = *colp;
	અક્ષર *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	અક્ષર *cmdv[20];
	અક્ષર *s, *t;
	અચલ અક્ષર *sep;
	પૂર્ણાંક i, j, k, len, seplen, argc;
	પૂર્ણાંક cnt;
	અક્ષર last_char;
	અચલ અક્ષર *ps_prompt;

	જો (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		જો (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} નહીંતર
		ps_prompt = CONFIG_SYS_PROMPT;

	જો (strcmp(prompt, ps_prompt) != 0)
		પાછા 0;	/* not in normal console */

	cnt = strlen(buf);
	જો (cnt >= 1)
		last_char = buf[cnt - 1];
	નહીંતર
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, કદ(argv)/કદ(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    કદ(cmdv) / કદ(cmdv[0]), cmdv);
	જો (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  કદ(cmdv) / કદ(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	જો (i == 0) {
		જો (argc > 1)	/* allow tab for non command */
			પાછા 0;
		putc('\a');
		પાછા 1;
	}

	s = ખાલી;
	len = 0;
	sep = ખાલી;
	seplen = 0;
	જો (i == 1) { /* one match; perfect */
		જો (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		નહીંતર
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} નહીંતર જો (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		જો (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		નહીંતર
			k = 0;

		j -= k;
		જો (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	જો (s != ખાલી) {
		k = len + seplen;
		/* make sure it fits */
		જો (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			પાછા 1;
		}

		t = buf + cnt;
		માટે (i = 0; i < len; i++)
			*t++ = *s++;
		જો (sep != ખાલી)
			માટે (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		જો (sep == ખાલી)
			putc('\a');
		*np = n;
		*colp = col;
	} નહીંતર {
		print_argv(ખાલી, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	પાછા 1;
}

#endif

#ifdef CMD_DATA_SIZE
પૂર્ણાંક cmd_get_data_size(અચલ અક્ષર *arg, પૂર્ણાંક default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	પૂર્ણાંક len = strlen(arg);
	જો (len >= 2 && arg[len-2] == '.') {
		પસંદગી (arg[len-1]) {
		સ્થિતિ 'b':
			પાછા 1;
		સ્થિતિ 'w':
			પાછા 2;
		સ્થિતિ 'l':
			પાછા 4;
		સ્થિતિ 's':
			પાછા CMD_DATA_SIZE_STR;
		સ્થિતિ 'q':
			જો (MEM_SUPPORT_64BIT_DATA)
				પાછા 8;
			fallthrough;
		મૂળભૂત:
			પાછા CMD_DATA_SIZE_ERR;
		}
	}
	પાછા default_size;
}
#endif

શૂન્ય fixup_cmdtable(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક size)
{
	પૂર્ણાંક	i;

	જો (gd->reloc_off == 0)
		પાછા;

	માટે (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(પૂર્ણાંક (*)(સંરચના cmd_tbl *, પૂર્ણાંક, પૂર્ણાંક,
				 અક્ષર * અચલ [], પૂર્ણાંક *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (પૂર્ણાંક (*)(સંરચના cmd_tbl *, પૂર્ણાંક, પૂર્ણાંક,
				      અક્ષર *અચલ []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (અક્ષર *)addr;
		જો (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (અક્ષર *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		જો (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (અક્ષર *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		જો (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(પૂર્ણાંક (*)(પૂર્ણાંક, અક્ષર * અચલ [], અક્ષર, પૂર્ણાંક, અક્ષર * []))addr;
		}
#endif
		cmdtp++;
	}
}

પૂર્ણાંક cmd_always_repeatable(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc,
			  અક્ષર *અચલ argv[], પૂર્ણાંક *repeatable)
{
	*repeatable = 1;

	પાછા cmdtp->cmd(cmdtp, flag, argc, argv);
}

પૂર્ણાંક cmd_never_repeatable(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc,
			 અક્ષર *અચલ argv[], પૂર્ણાંક *repeatable)
{
	*repeatable = 0;

	પાછા cmdtp->cmd(cmdtp, flag, argc, argv);
}

પૂર્ણાંક cmd_discard_repeatable(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc,
			   અક્ષર *અચલ argv[])
{
	પૂર્ણાંક repeatable;

	પાછા cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
સ્થિર પૂર્ણાંક cmd_call(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc,
		    અક્ષર *અચલ argv[], પૂર્ણાંક *repeatable)
{
	પૂર્ણાંક result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	જો (result)
		debug("Command failed, result=%d\n", result);
	પાછા result;
}

ગણના command_ret_t cmd_process(પૂર્ણાંક flag, પૂર્ણાંક argc, અક્ષર *અચલ argv[],
			       પૂર્ણાંક *repeatable, ulong *ticks)
{
	ગણના command_ret_t rc = CMD_RET_SUCCESS;
	સંરચના cmd_tbl *cmdtp;

#જો defined(CONFIG_SYS_XTRACE)
	અક્ષર *xtrace;

	xtrace = env_get("xtrace");
	જો (xtrace) {
		puts("+");
		માટે (પૂર્ણાંક i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	જો (cmdtp == ખાલી) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		પાછા 1;
	}

	/* found - check max args */
	જો (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#જો defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	નહીંતર જો (cmdtp->cmd == do_bootd) {
		જો (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} નહીંતર {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	જો (!rc) {
		પૂર્ણાંક newrep;

		જો (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		જો (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	જો (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	પાછા rc;
}

પૂર્ણાંક cmd_process_error(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક err)
{
	જો (err == CMD_RET_USAGE)
		પાછા CMD_RET_USAGE;

	જો (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		પાછા CMD_RET_FAILURE;
	}

	પાછા CMD_RET_SUCCESS;
}

પૂર્ણાંક cmd_source_script(ulong addr, અચલ અક્ષર *fit_uname, અચલ અક્ષર *confname)
{
	અક્ષર *data;
	શૂન્ય *buf;
	uint len;
	પૂર્ણાંક ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	જો (ret)
		પાછા CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	પાછા run_command_list(data, len, 0);
}
