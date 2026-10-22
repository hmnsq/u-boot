// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#شامل <config.h>
#شامل <compiler.h>
#شامل <command.h>
#شامل <console.h>
#شامل <env.h>
#شامل <image.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <time.h>
#شامل <asm/global_data.h>
#شامل <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

صحیح _do_help(ساختار cmd_tbl *cmd_start, صحیح cmd_اقلام, ساختار cmd_tbl *cmdtp,
	     صحیح flag, صحیح argc, نویسه *ثابت argv[])
{
	صحیح i;
	صحیح rcode = 0;

	اگر (argc == 1) {	/* show list of commands */
		ساختار cmd_tbl *cmd_array[cmd_اقلام];
		صحیح i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		برای (i = 0; i < cmd_اقلام; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		برای (i = cmd_اقلام - 1; i > 0; --i) {
			swaps = 0;
			برای (j = 0; j < i; ++j) {
				اگر (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ساختار cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			اگر (!swaps)
				بشکن;
		}

		/* print short help (usage) */
		برای (i = 0; i < cmd_اقلام; i++) {
			ثابت نویسه *usage = cmd_array[i]->usage;

			/* allow user abort */
			اگر (ctrlc())
				بازگشت 1;
			اگر (usage == پوچ)
				ادامه;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		بازگشت 0;
	}
	/*
	 * command help (long version)
	 */
	برای (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_اقلام);
		اگر (cmdtp != پوچ) {
			rcode |= cmd_usage(cmdtp);
		} وگرنه {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	بازگشت rcode;
}

/* find command table entry for a command */
ساختار cmd_tbl *find_cmd_tbl(ثابت نویسه *cmd, ساختار cmd_tbl *table,
			     صحیح table_len)
{
#ifdef CONFIG_CMDLINE
	ساختار cmd_tbl *cmdtp;
	ساختار cmd_tbl *cmdtp_temp = table;	/* Init value */
	ثابت نویسه *p;
	صحیح len;
	صحیح n_found = 0;

	اگر (!cmd)
		بازگشت پوچ;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == پوچ) ? strlen (cmd) : (p - cmd);

	برای (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		اگر (strncmp(cmd, cmdtp->name, len) == 0) {
			اگر (len == strlen(cmdtp->name))
				بازگشت cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	اگر (n_found == 1) {			/* exactly one match */
		بازگشت cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	بازگشت پوچ;	/* not found or ambiguous command */
}

ساختار cmd_tbl *find_cmd(ثابت نویسه *cmd)
{
	ساختار cmd_tbl *start = ll_entry_start(ساختار cmd_tbl, cmd);
	ثابت صحیح len = ll_entry_شمار(ساختار cmd_tbl, cmd);
	بازگشت find_cmd_tbl(cmd, start, len);
}

صحیح cmd_usage(ثابت ساختار cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	اگر (!cmdtp->help) {
		puts ("- No additional help available.\n");
		بازگشت 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	بازگشت 1;
}

#ifdef CONFIG_AUTO_COMPLETE
ایستا نویسه env_complete_buf[512];

صحیح var_complete(صحیح argc, نویسه *ثابت argv[], نویسه last_char, صحیح maxv,
		 نویسه *cmdv[])
{
	صحیح space;

	space = last_char == '\0' || isblank(last_char);

	اگر (space && argc == 1)
		بازگشت env_complete("", maxv, cmdv, اندازه(env_complete_buf),
				    env_complete_buf, false);

	اگر (!space && argc == 2)
		بازگشت env_complete(argv[1], maxv, cmdv,
				    اندازه(env_complete_buf),
				    env_complete_buf, false);

	بازگشت 0;
}

ایستا صحیح dollar_complete(صحیح argc, نویسه *ثابت argv[], نویسه last_char,
			   صحیح maxv, نویسه *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	اگر (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		بازگشت 0;

	بازگشت env_complete(argv[argc - 1], maxv, cmdv, اندازه(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

صحیح complete_subcmdv(ساختار cmd_tbl *cmdtp, صحیح شمار, صحیح argc,
		     نویسه *ثابت argv[], نویسه last_char,
		     صحیح maxv, نویسه *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ثابت ساختار cmd_tbl *cmdend = cmdtp + شمار;
	ثابت نویسه *p;
	صحیح len, clen;
	صحیح n_found = 0;
	ثابت نویسه *cmd;

	/* sanity? */
	اگر (maxv < 2)
		بازگشت -2;

	cmdv[0] = پوچ;

	اگر (argc == 0) {
		/* output full list of commands */
		برای (; cmdtp != cmdend; cmdtp++) {
			اگر (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				بشکن;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = پوچ;
		بازگشت n_found;
	}

	/* more than one arg or one but the start of the next */
	اگر (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, شمار);
		اگر (cmdtp == پوچ || cmdtp->complete == پوچ) {
			cmdv[0] = پوچ;
			بازگشت 0;
		}
		بازگشت (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	اگر (p == پوچ)
		len = strlen(cmd);
	وگرنه
		len = p - cmd;

	/* return the partial matches */
	برای (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		اگر (clen < len)
			ادامه;

		اگر (memcmp(cmd, cmdtp->name, len) != 0)
			ادامه;

		/* too many! */
		اگر (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			بشکن;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = پوچ;
	بازگشت n_found;
#وگرنه
	بازگشت 0;
#endif
}

ایستا صحیح complete_cmdv(صحیح argc, نویسه *ثابت argv[], نویسه last_char,
			 صحیح maxv, نویسه *cmdv[])
{
#ifdef CONFIG_CMDLINE
	بازگشت complete_subcmdv(ll_entry_start(ساختار cmd_tbl, cmd),
				ll_entry_شمار(ساختار cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#وگرنه
	بازگشت 0;
#endif
}

ایستا صحیح make_argv(نویسه *s, صحیح argvsz, نویسه *argv[])
{
	صحیح argc = 0;

	/* split into argv */
	وقتی (argc < argvsz - 1) {

		/* skip any white space */
		وقتی (isblank(*s))
			++s;

		اگر (*s == '\0')	/* end of s, no more args	*/
			بشکن;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		وقتی (*s && !isblank(*s))
			++s;

		اگر (*s == '\0')		/* end of s, no more args	*/
			بشکن;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = پوچ;

	بازگشت argc;
}

ایستا تهی print_argv(ثابت نویسه *banner, ثابت نویسه *leader, ثابت نویسه *sep,
		       صحیح linemax, نویسه *ثابت argv[])
{
	صحیح ll = leader != پوچ ? strlen(leader) : 0;
	صحیح sl = sep != پوچ ? strlen(sep) : 0;
	صحیح len, i;

	اگر (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	وقتی (*argv != پوچ) {
		len = strlen(*argv) + sl;
		اگر (i + len >= linemax) {
			puts("\n");
			اگر (leader)
				puts(leader);
			i = ll - sl;
		} وگرنه اگر (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

ایستا صحیح find_common_prefix(نویسه *ثابت argv[])
{
	صحیح i, len;
	نویسه *anchor, *s, *t;

	اگر (*argv == پوچ)
		بازگشت 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	وقتی ((t = *argv++) != پوچ) {
		s = anchor;
		برای (i = 0; i < len; i++, t++, s++) {
			اگر (*t != *s)
				بشکن;
		}
		len = s - anchor;
	}
	بازگشت len;
}

صحیح cmd_auto_complete(ثابت نویسه *ثابت prompt, نویسه *buf, صحیح *np, صحیح *colp)
{
	نویسه tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	صحیح n = *np, col = *colp;
	نویسه *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	نویسه *cmdv[20];
	نویسه *s, *t;
	ثابت نویسه *sep;
	صحیح i, j, k, len, seplen, argc;
	صحیح cnt;
	نویسه last_char;
	ثابت نویسه *ps_prompt;

	اگر (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		اگر (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} وگرنه
		ps_prompt = CONFIG_SYS_PROMPT;

	اگر (strcmp(prompt, ps_prompt) != 0)
		بازگشت 0;	/* not in normal console */

	cnt = strlen(buf);
	اگر (cnt >= 1)
		last_char = buf[cnt - 1];
	وگرنه
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, اندازه(argv)/اندازه(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    اندازه(cmdv) / اندازه(cmdv[0]), cmdv);
	اگر (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  اندازه(cmdv) / اندازه(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	اگر (i == 0) {
		اگر (argc > 1)	/* allow tab for non command */
			بازگشت 0;
		putc('\a');
		بازگشت 1;
	}

	s = پوچ;
	len = 0;
	sep = پوچ;
	seplen = 0;
	اگر (i == 1) { /* one match; perfect */
		اگر (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		وگرنه
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} وگرنه اگر (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		اگر (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		وگرنه
			k = 0;

		j -= k;
		اگر (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	اگر (s != پوچ) {
		k = len + seplen;
		/* make sure it fits */
		اگر (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			بازگشت 1;
		}

		t = buf + cnt;
		برای (i = 0; i < len; i++)
			*t++ = *s++;
		اگر (sep != پوچ)
			برای (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		اگر (sep == پوچ)
			putc('\a');
		*np = n;
		*colp = col;
	} وگرنه {
		print_argv(پوچ, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	بازگشت 1;
}

#endif

#ifdef CMD_DATA_SIZE
صحیح cmd_get_data_size(ثابت نویسه *arg, صحیح default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	صحیح len = strlen(arg);
	اگر (len >= 2 && arg[len-2] == '.') {
		انتخاب (arg[len-1]) {
		حالت 'b':
			بازگشت 1;
		حالت 'w':
			بازگشت 2;
		حالت 'l':
			بازگشت 4;
		حالت 's':
			بازگشت CMD_DATA_SIZE_STR;
		حالت 'q':
			اگر (MEM_SUPPORT_64BIT_DATA)
				بازگشت 8;
			fallthrough;
		پیشفرض:
			بازگشت CMD_DATA_SIZE_ERR;
		}
	}
	بازگشت default_size;
}
#endif

تهی fixup_cmdtable(ساختار cmd_tbl *cmdtp, صحیح size)
{
	صحیح	i;

	اگر (gd->reloc_off == 0)
		بازگشت;

	برای (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(صحیح (*)(ساختار cmd_tbl *, صحیح, صحیح,
				 نویسه * ثابت [], صحیح *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (صحیح (*)(ساختار cmd_tbl *, صحیح, صحیح,
				      نویسه *ثابت []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (نویسه *)addr;
		اگر (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (نویسه *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		اگر (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (نویسه *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		اگر (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(صحیح (*)(صحیح, نویسه * ثابت [], نویسه, صحیح, نویسه * []))addr;
		}
#endif
		cmdtp++;
	}
}

صحیح cmd_always_repeatable(ساختار cmd_tbl *cmdtp, صحیح flag, صحیح argc,
			  نویسه *ثابت argv[], صحیح *repeatable)
{
	*repeatable = 1;

	بازگشت cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحیح cmd_never_repeatable(ساختار cmd_tbl *cmdtp, صحیح flag, صحیح argc,
			 نویسه *ثابت argv[], صحیح *repeatable)
{
	*repeatable = 0;

	بازگشت cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحیح cmd_discard_repeatable(ساختار cmd_tbl *cmdtp, صحیح flag, صحیح argc,
			   نویسه *ثابت argv[])
{
	صحیح repeatable;

	بازگشت cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ایستا صحیح cmd_call(ساختار cmd_tbl *cmdtp, صحیح flag, صحیح argc,
		    نویسه *ثابت argv[], صحیح *repeatable)
{
	صحیح result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	اگر (result)
		debug("Command failed, result=%d\n", result);
	بازگشت result;
}

شمارش command_ret_t cmd_process(صحیح flag, صحیح argc, نویسه *ثابت argv[],
			       صحیح *repeatable, ulong *ticks)
{
	شمارش command_ret_t rc = CMD_RET_SUCCESS;
	ساختار cmd_tbl *cmdtp;

#اگر defined(CONFIG_SYS_XTRACE)
	نویسه *xtrace;

	xtrace = env_get("xtrace");
	اگر (xtrace) {
		puts("+");
		برای (صحیح i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	اگر (cmdtp == پوچ) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		بازگشت 1;
	}

	/* found - check max args */
	اگر (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#اگر defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	وگرنه اگر (cmdtp->cmd == do_bootd) {
		اگر (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} وگرنه {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	اگر (!rc) {
		صحیح newrep;

		اگر (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		اگر (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	اگر (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	بازگشت rc;
}

صحیح cmd_process_error(ساختار cmd_tbl *cmdtp, صحیح err)
{
	اگر (err == CMD_RET_USAGE)
		بازگشت CMD_RET_USAGE;

	اگر (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		بازگشت CMD_RET_FAILURE;
	}

	بازگشت CMD_RET_SUCCESS;
}

صحیح cmd_source_script(ulong addr, ثابت نویسه *fit_uname, ثابت نویسه *confname)
{
	نویسه *data;
	تهی *buf;
	uint len;
	صحیح ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	اگر (ret)
		بازگشت CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	بازگشت run_command_list(data, len, 0);
}
