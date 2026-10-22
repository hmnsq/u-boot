// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <compiler.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

முழு _do_help(கட்டமைப்பு cmd_tbl *cmd_start, முழு cmd_பொருட்கள், கட்டமைப்பு cmd_tbl *cmdtp,
	     முழு flag, முழு argc, எழுத்து *மாறிலி argv[])
{
	முழு i;
	முழு rcode = 0;

	எனில் (argc == 1) {	/* show list of commands */
		கட்டமைப்பு cmd_tbl *cmd_array[cmd_பொருட்கள்];
		முழு i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		க்கு (i = 0; i < cmd_பொருட்கள்; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		க்கு (i = cmd_பொருட்கள் - 1; i > 0; --i) {
			swaps = 0;
			க்கு (j = 0; j < i; ++j) {
				எனில் (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					கட்டமைப்பு cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			எனில் (!swaps)
				நிறுத்து;
		}

		/* print short help (usage) */
		க்கு (i = 0; i < cmd_பொருட்கள்; i++) {
			மாறிலி எழுத்து *usage = cmd_array[i]->usage;

			/* allow user abort */
			எனில் (ctrlc())
				திரும்பு 1;
			எனில் (usage == வெற்று)
				தொடர்;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		திரும்பு 0;
	}
	/*
	 * command help (long version)
	 */
	க்கு (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_பொருட்கள்);
		எனில் (cmdtp != வெற்று) {
			rcode |= cmd_usage(cmdtp);
		} இல்லையெனில் {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	திரும்பு rcode;
}

/* find command table entry for a command */
கட்டமைப்பு cmd_tbl *find_cmd_tbl(மாறிலி எழுத்து *cmd, கட்டமைப்பு cmd_tbl *table,
			     முழு table_len)
{
#ifdef CONFIG_CMDLINE
	கட்டமைப்பு cmd_tbl *cmdtp;
	கட்டமைப்பு cmd_tbl *cmdtp_temp = table;	/* Init value */
	மாறிலி எழுத்து *p;
	முழு len;
	முழு n_found = 0;

	எனில் (!cmd)
		திரும்பு வெற்று;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == வெற்று) ? strlen (cmd) : (p - cmd);

	க்கு (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		எனில் (strncmp(cmd, cmdtp->name, len) == 0) {
			எனில் (len == strlen(cmdtp->name))
				திரும்பு cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	எனில் (n_found == 1) {			/* exactly one match */
		திரும்பு cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	திரும்பு வெற்று;	/* not found or ambiguous command */
}

கட்டமைப்பு cmd_tbl *find_cmd(மாறிலி எழுத்து *cmd)
{
	கட்டமைப்பு cmd_tbl *start = ll_entry_start(கட்டமைப்பு cmd_tbl, cmd);
	மாறிலி முழு len = ll_entry_மொத்தம்(கட்டமைப்பு cmd_tbl, cmd);
	திரும்பு find_cmd_tbl(cmd, start, len);
}

முழு cmd_usage(மாறிலி கட்டமைப்பு cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	எனில் (!cmdtp->help) {
		puts ("- No additional help available.\n");
		திரும்பு 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	திரும்பு 1;
}

#ifdef CONFIG_AUTO_COMPLETE
நிலையான எழுத்து env_complete_buf[512];

முழு var_complete(முழு argc, எழுத்து *மாறிலி argv[], எழுத்து last_char, முழு maxv,
		 எழுத்து *cmdv[])
{
	முழு space;

	space = last_char == '\0' || isblank(last_char);

	எனில் (space && argc == 1)
		திரும்பு env_complete("", maxv, cmdv, அளவு(env_complete_buf),
				    env_complete_buf, false);

	எனில் (!space && argc == 2)
		திரும்பு env_complete(argv[1], maxv, cmdv,
				    அளவு(env_complete_buf),
				    env_complete_buf, false);

	திரும்பு 0;
}

நிலையான முழு dollar_complete(முழு argc, எழுத்து *மாறிலி argv[], எழுத்து last_char,
			   முழு maxv, எழுத்து *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	எனில் (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		திரும்பு 0;

	திரும்பு env_complete(argv[argc - 1], maxv, cmdv, அளவு(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

முழு complete_subcmdv(கட்டமைப்பு cmd_tbl *cmdtp, முழு மொத்தம், முழு argc,
		     எழுத்து *மாறிலி argv[], எழுத்து last_char,
		     முழு maxv, எழுத்து *cmdv[])
{
#ifdef CONFIG_CMDLINE
	மாறிலி கட்டமைப்பு cmd_tbl *cmdend = cmdtp + மொத்தம்;
	மாறிலி எழுத்து *p;
	முழு len, clen;
	முழு n_found = 0;
	மாறிலி எழுத்து *cmd;

	/* sanity? */
	எனில் (maxv < 2)
		திரும்பு -2;

	cmdv[0] = வெற்று;

	எனில் (argc == 0) {
		/* output full list of commands */
		க்கு (; cmdtp != cmdend; cmdtp++) {
			எனில் (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				நிறுத்து;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = வெற்று;
		திரும்பு n_found;
	}

	/* more than one arg or one but the start of the next */
	எனில் (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, மொத்தம்);
		எனில் (cmdtp == வெற்று || cmdtp->complete == வெற்று) {
			cmdv[0] = வெற்று;
			திரும்பு 0;
		}
		திரும்பு (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	எனில் (p == வெற்று)
		len = strlen(cmd);
	இல்லையெனில்
		len = p - cmd;

	/* return the partial matches */
	க்கு (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		எனில் (clen < len)
			தொடர்;

		எனில் (memcmp(cmd, cmdtp->name, len) != 0)
			தொடர்;

		/* too many! */
		எனில் (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			நிறுத்து;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = வெற்று;
	திரும்பு n_found;
#இல்லையெனில்
	திரும்பு 0;
#endif
}

நிலையான முழு complete_cmdv(முழு argc, எழுத்து *மாறிலி argv[], எழுத்து last_char,
			 முழு maxv, எழுத்து *cmdv[])
{
#ifdef CONFIG_CMDLINE
	திரும்பு complete_subcmdv(ll_entry_start(கட்டமைப்பு cmd_tbl, cmd),
				ll_entry_மொத்தம்(கட்டமைப்பு cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#இல்லையெனில்
	திரும்பு 0;
#endif
}

நிலையான முழு make_argv(எழுத்து *s, முழு argvsz, எழுத்து *argv[])
{
	முழு argc = 0;

	/* split into argv */
	போது (argc < argvsz - 1) {

		/* skip any white space */
		போது (isblank(*s))
			++s;

		எனில் (*s == '\0')	/* end of s, no more args	*/
			நிறுத்து;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		போது (*s && !isblank(*s))
			++s;

		எனில் (*s == '\0')		/* end of s, no more args	*/
			நிறுத்து;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = வெற்று;

	திரும்பு argc;
}

நிலையான வெறுமை print_argv(மாறிலி எழுத்து *banner, மாறிலி எழுத்து *leader, மாறிலி எழுத்து *sep,
		       முழு linemax, எழுத்து *மாறிலி argv[])
{
	முழு ll = leader != வெற்று ? strlen(leader) : 0;
	முழு sl = sep != வெற்று ? strlen(sep) : 0;
	முழு len, i;

	எனில் (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	போது (*argv != வெற்று) {
		len = strlen(*argv) + sl;
		எனில் (i + len >= linemax) {
			puts("\n");
			எனில் (leader)
				puts(leader);
			i = ll - sl;
		} இல்லையெனில் எனில் (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

நிலையான முழு find_common_prefix(எழுத்து *மாறிலி argv[])
{
	முழு i, len;
	எழுத்து *anchor, *s, *t;

	எனில் (*argv == வெற்று)
		திரும்பு 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	போது ((t = *argv++) != வெற்று) {
		s = anchor;
		க்கு (i = 0; i < len; i++, t++, s++) {
			எனில் (*t != *s)
				நிறுத்து;
		}
		len = s - anchor;
	}
	திரும்பு len;
}

முழு cmd_auto_complete(மாறிலி எழுத்து *மாறிலி prompt, எழுத்து *buf, முழு *np, முழு *colp)
{
	எழுத்து tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	முழு n = *np, col = *colp;
	எழுத்து *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	எழுத்து *cmdv[20];
	எழுத்து *s, *t;
	மாறிலி எழுத்து *sep;
	முழு i, j, k, len, seplen, argc;
	முழு cnt;
	எழுத்து last_char;
	மாறிலி எழுத்து *ps_prompt;

	எனில் (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		எனில் (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} இல்லையெனில்
		ps_prompt = CONFIG_SYS_PROMPT;

	எனில் (strcmp(prompt, ps_prompt) != 0)
		திரும்பு 0;	/* not in normal console */

	cnt = strlen(buf);
	எனில் (cnt >= 1)
		last_char = buf[cnt - 1];
	இல்லையெனில்
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, அளவு(argv)/அளவு(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    அளவு(cmdv) / அளவு(cmdv[0]), cmdv);
	எனில் (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  அளவு(cmdv) / அளவு(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	எனில் (i == 0) {
		எனில் (argc > 1)	/* allow tab for non command */
			திரும்பு 0;
		putc('\a');
		திரும்பு 1;
	}

	s = வெற்று;
	len = 0;
	sep = வெற்று;
	seplen = 0;
	எனில் (i == 1) { /* one match; perfect */
		எனில் (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		இல்லையெனில்
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} இல்லையெனில் எனில் (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		எனில் (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		இல்லையெனில்
			k = 0;

		j -= k;
		எனில் (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	எனில் (s != வெற்று) {
		k = len + seplen;
		/* make sure it fits */
		எனில் (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			திரும்பு 1;
		}

		t = buf + cnt;
		க்கு (i = 0; i < len; i++)
			*t++ = *s++;
		எனில் (sep != வெற்று)
			க்கு (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		எனில் (sep == வெற்று)
			putc('\a');
		*np = n;
		*colp = col;
	} இல்லையெனில் {
		print_argv(வெற்று, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	திரும்பு 1;
}

#endif

#ifdef CMD_DATA_SIZE
முழு cmd_get_data_size(மாறிலி எழுத்து *arg, முழு default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	முழு len = strlen(arg);
	எனில் (len >= 2 && arg[len-2] == '.') {
		தேர்வு (arg[len-1]) {
		நிலை 'b':
			திரும்பு 1;
		நிலை 'w':
			திரும்பு 2;
		நிலை 'l':
			திரும்பு 4;
		நிலை 's':
			திரும்பு CMD_DATA_SIZE_STR;
		நிலை 'q':
			எனில் (MEM_SUPPORT_64BIT_DATA)
				திரும்பு 8;
			fallthrough;
		இயல்பு:
			திரும்பு CMD_DATA_SIZE_ERR;
		}
	}
	திரும்பு default_size;
}
#endif

வெறுமை fixup_cmdtable(கட்டமைப்பு cmd_tbl *cmdtp, முழு size)
{
	முழு	i;

	எனில் (gd->reloc_off == 0)
		திரும்பு;

	க்கு (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(முழு (*)(கட்டமைப்பு cmd_tbl *, முழு, முழு,
				 எழுத்து * மாறிலி [], முழு *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (முழு (*)(கட்டமைப்பு cmd_tbl *, முழு, முழு,
				      எழுத்து *மாறிலி []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (எழுத்து *)addr;
		எனில் (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (எழுத்து *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		எனில் (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (எழுத்து *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		எனில் (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(முழு (*)(முழு, எழுத்து * மாறிலி [], எழுத்து, முழு, எழுத்து * []))addr;
		}
#endif
		cmdtp++;
	}
}

முழு cmd_always_repeatable(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc,
			  எழுத்து *மாறிலி argv[], முழு *repeatable)
{
	*repeatable = 1;

	திரும்பு cmdtp->cmd(cmdtp, flag, argc, argv);
}

முழு cmd_never_repeatable(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc,
			 எழுத்து *மாறிலி argv[], முழு *repeatable)
{
	*repeatable = 0;

	திரும்பு cmdtp->cmd(cmdtp, flag, argc, argv);
}

முழு cmd_discard_repeatable(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc,
			   எழுத்து *மாறிலி argv[])
{
	முழு repeatable;

	திரும்பு cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
நிலையான முழு cmd_call(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc,
		    எழுத்து *மாறிலி argv[], முழு *repeatable)
{
	முழு result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	எனில் (result)
		debug("Command failed, result=%d\n", result);
	திரும்பு result;
}

எண்ணிக்கை command_ret_t cmd_process(முழு flag, முழு argc, எழுத்து *மாறிலி argv[],
			       முழு *repeatable, ulong *ticks)
{
	எண்ணிக்கை command_ret_t rc = CMD_RET_SUCCESS;
	கட்டமைப்பு cmd_tbl *cmdtp;

#எனில் defined(CONFIG_SYS_XTRACE)
	எழுத்து *xtrace;

	xtrace = env_get("xtrace");
	எனில் (xtrace) {
		puts("+");
		க்கு (முழு i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	எனில் (cmdtp == வெற்று) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		திரும்பு 1;
	}

	/* found - check max args */
	எனில் (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#எனில் defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	இல்லையெனில் எனில் (cmdtp->cmd == do_bootd) {
		எனில் (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} இல்லையெனில் {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	எனில் (!rc) {
		முழு newrep;

		எனில் (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		எனில் (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	எனில் (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	திரும்பு rc;
}

முழு cmd_process_error(கட்டமைப்பு cmd_tbl *cmdtp, முழு err)
{
	எனில் (err == CMD_RET_USAGE)
		திரும்பு CMD_RET_USAGE;

	எனில் (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		திரும்பு CMD_RET_FAILURE;
	}

	திரும்பு CMD_RET_SUCCESS;
}

முழு cmd_source_script(ulong addr, மாறிலி எழுத்து *fit_uname, மாறிலி எழுத்து *confname)
{
	எழுத்து *data;
	வெறுமை *buf;
	uint len;
	முழு ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	எனில் (ret)
		திரும்பு CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	திரும்பு run_command_list(data, len, 0);
}
