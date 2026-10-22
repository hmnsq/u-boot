// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#ضمن <config.h>
#ضمن <compiler.h>
#ضمن <command.h>
#ضمن <console.h>
#ضمن <env.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <time.h>
#ضمن <asm/global_data.h>
#ضمن <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

صحيح _do_help(بنية cmd_tbl *cmd_start, صحيح cmd_items, بنية cmd_tbl *cmdtp,
	     صحيح flag, صحيح argc, حرف *ثابت argv[])
{
	صحيح i;
	صحيح rcode = 0;

	אם (argc == 1) {	/* show list of commands */
		بنية cmd_tbl *cmd_array[cmd_items];
		صحيح i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		עבור (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		עבור (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			עבור (j = 0; j < i; ++j) {
				אם (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					بنية cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			אם (!swaps)
				اكسر;
		}

		/* print short help (usage) */
		עבור (i = 0; i < cmd_items; i++) {
			ثابت حرف *usage = cmd_array[i]->usage;

			/* allow user abort */
			אם (ctrlc())
				החזר 1;
			אם (usage == عدم)
				استمر;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		החזר 0;
	}
	/*
	 * command help (long version)
	 */
	עבור (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		אם (cmdtp != عدم) {
			rcode |= cmd_usage(cmdtp);
		} אחרת {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	החזר rcode;
}

/* find command table entry for a command */
بنية cmd_tbl *find_cmd_tbl(ثابت حرف *cmd, بنية cmd_tbl *table,
			     صحيح table_len)
{
#ifdef CONFIG_CMDLINE
	بنية cmd_tbl *cmdtp;
	بنية cmd_tbl *cmdtp_temp = table;	/* Init value */
	ثابت حرف *p;
	صحيح len;
	صحيح n_found = 0;

	אם (!cmd)
		החזר عدم;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == عدم) ? strlen (cmd) : (p - cmd);

	עבור (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		אם (strncmp(cmd, cmdtp->name, len) == 0) {
			אם (len == strlen(cmdtp->name))
				החזר cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	אם (n_found == 1) {			/* exactly one match */
		החזר cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	החזר عدم;	/* not found or ambiguous command */
}

بنية cmd_tbl *find_cmd(ثابت حرف *cmd)
{
	بنية cmd_tbl *start = ll_entry_start(بنية cmd_tbl, cmd);
	ثابت صحيح len = ll_entry_count(بنية cmd_tbl, cmd);
	החזר find_cmd_tbl(cmd, start, len);
}

صحيح cmd_usage(ثابت بنية cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	אם (!cmdtp->help) {
		puts ("- No additional help available.\n");
		החזר 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	החזר 1;
}

#ifdef CONFIG_AUTO_COMPLETE
ساكن حرف env_complete_buf[512];

صحيح var_complete(صحيح argc, حرف *ثابت argv[], حرف last_char, صحيح maxv,
		 حرف *cmdv[])
{
	صحيح space;

	space = last_char == '\0' || isblank(last_char);

	אם (space && argc == 1)
		החזר env_complete("", maxv, cmdv, حجم(env_complete_buf),
				    env_complete_buf, false);

	אם (!space && argc == 2)
		החזר env_complete(argv[1], maxv, cmdv,
				    حجم(env_complete_buf),
				    env_complete_buf, false);

	החזר 0;
}

ساكن صحيح dollar_complete(صحيح argc, حرف *ثابت argv[], حرف last_char,
			   صحيح maxv, حرف *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	אם (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		החזר 0;

	החזר env_complete(argv[argc - 1], maxv, cmdv, حجم(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

صحيح complete_subcmdv(بنية cmd_tbl *cmdtp, صحيح count, صحيح argc,
		     حرف *ثابت argv[], حرف last_char,
		     صحيح maxv, حرف *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ثابت بنية cmd_tbl *cmdend = cmdtp + count;
	ثابت حرف *p;
	صحيح len, clen;
	صحيح n_found = 0;
	ثابت حرف *cmd;

	/* sanity? */
	אם (maxv < 2)
		החזר -2;

	cmdv[0] = عدم;

	אם (argc == 0) {
		/* output full list of commands */
		עבור (; cmdtp != cmdend; cmdtp++) {
			אם (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				اكسر;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = عدم;
		החזר n_found;
	}

	/* more than one arg or one but the start of the next */
	אם (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		אם (cmdtp == عدم || cmdtp->complete == عدم) {
			cmdv[0] = عدم;
			החזר 0;
		}
		החזר (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	אם (p == عدم)
		len = strlen(cmd);
	אחרת
		len = p - cmd;

	/* return the partial matches */
	עבור (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		אם (clen < len)
			استمر;

		אם (memcmp(cmd, cmdtp->name, len) != 0)
			استمر;

		/* too many! */
		אם (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			اكسر;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = عدم;
	החזר n_found;
#אחרת
	החזר 0;
#endif
}

ساكن صحيح complete_cmdv(صحيح argc, حرف *ثابت argv[], حرف last_char,
			 صحيح maxv, حرف *cmdv[])
{
#ifdef CONFIG_CMDLINE
	החזר complete_subcmdv(ll_entry_start(بنية cmd_tbl, cmd),
				ll_entry_count(بنية cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#אחרת
	החזר 0;
#endif
}

ساكن صحيح make_argv(حرف *s, صحيح argvsz, حرف *argv[])
{
	صحيح argc = 0;

	/* split into argv */
	כלעוד (argc < argvsz - 1) {

		/* skip any white space */
		כלעוד (isblank(*s))
			++s;

		אם (*s == '\0')	/* end of s, no more args	*/
			اكسر;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		כלעוד (*s && !isblank(*s))
			++s;

		אם (*s == '\0')		/* end of s, no more args	*/
			اكسر;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = عدم;

	החזר argc;
}

ساكن فراغ print_argv(ثابت حرف *banner, ثابت حرف *leader, ثابت حرف *sep,
		       صحيح linemax, حرف *ثابت argv[])
{
	صحيح ll = leader != عدم ? strlen(leader) : 0;
	صحيح sl = sep != عدم ? strlen(sep) : 0;
	صحيح len, i;

	אם (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	כלעוד (*argv != عدم) {
		len = strlen(*argv) + sl;
		אם (i + len >= linemax) {
			puts("\n");
			אם (leader)
				puts(leader);
			i = ll - sl;
		} אחרת אם (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

ساكن صحيح find_common_prefix(حرف *ثابت argv[])
{
	صحيح i, len;
	حرف *anchor, *s, *t;

	אם (*argv == عدم)
		החזר 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	כלעוד ((t = *argv++) != عدم) {
		s = anchor;
		עבור (i = 0; i < len; i++, t++, s++) {
			אם (*t != *s)
				اكسر;
		}
		len = s - anchor;
	}
	החזר len;
}

صحيح cmd_auto_complete(ثابت حرف *ثابت prompt, حرف *buf, صحيح *np, صحيح *colp)
{
	حرف tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	صحيح n = *np, col = *colp;
	حرف *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	حرف *cmdv[20];
	حرف *s, *t;
	ثابت حرف *sep;
	صحيح i, j, k, len, seplen, argc;
	صحيح cnt;
	حرف last_char;
	ثابت حرف *ps_prompt;

	אם (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		אם (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} אחרת
		ps_prompt = CONFIG_SYS_PROMPT;

	אם (strcmp(prompt, ps_prompt) != 0)
		החזר 0;	/* not in normal console */

	cnt = strlen(buf);
	אם (cnt >= 1)
		last_char = buf[cnt - 1];
	אחרת
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, حجم(argv)/حجم(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    حجم(cmdv) / حجم(cmdv[0]), cmdv);
	אם (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  حجم(cmdv) / حجم(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	אם (i == 0) {
		אם (argc > 1)	/* allow tab for non command */
			החזר 0;
		putc('\a');
		החזר 1;
	}

	s = عدم;
	len = 0;
	sep = عدم;
	seplen = 0;
	אם (i == 1) { /* one match; perfect */
		אם (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		אחרת
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} אחרת אם (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		אם (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		אחרת
			k = 0;

		j -= k;
		אם (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	אם (s != عدم) {
		k = len + seplen;
		/* make sure it fits */
		אם (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			החזר 1;
		}

		t = buf + cnt;
		עבור (i = 0; i < len; i++)
			*t++ = *s++;
		אם (sep != عدم)
			עבור (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		אם (sep == عدم)
			putc('\a');
		*np = n;
		*colp = col;
	} אחרת {
		print_argv(عدم, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	החזר 1;
}

#endif

#ifdef CMD_DATA_SIZE
صحيح cmd_get_data_size(ثابت حرف *arg, صحيح default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	صحيح len = strlen(arg);
	אם (len >= 2 && arg[len-2] == '.') {
		بدل (arg[len-1]) {
		حالة 'b':
			החזר 1;
		حالة 'w':
			החזר 2;
		حالة 'l':
			החזר 4;
		حالة 's':
			החזר CMD_DATA_SIZE_STR;
		حالة 'q':
			אם (MEM_SUPPORT_64BIT_DATA)
				החזר 8;
			fallthrough;
		افتراضي:
			החזר CMD_DATA_SIZE_ERR;
		}
	}
	החזר default_size;
}
#endif

فراغ fixup_cmdtable(بنية cmd_tbl *cmdtp, صحيح size)
{
	صحيح	i;

	אם (gd->reloc_off == 0)
		החזר;

	עבור (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(صحيح (*)(بنية cmd_tbl *, صحيح, صحيح,
				 حرف * ثابت [], صحيح *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (صحيح (*)(بنية cmd_tbl *, صحيح, صحيح,
				      حرف *ثابت []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (حرف *)addr;
		אם (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (حرف *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		אם (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (حرف *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		אם (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(صحيح (*)(صحيح, حرف * ثابت [], حرف, صحيح, حرف * []))addr;
		}
#endif
		cmdtp++;
	}
}

صحيح cmd_always_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			  حرف *ثابت argv[], صحيح *repeatable)
{
	*repeatable = 1;

	החזר cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_never_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			 حرف *ثابت argv[], صحيح *repeatable)
{
	*repeatable = 0;

	החזר cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_discard_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			   حرف *ثابت argv[])
{
	صحيح repeatable;

	החזר cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ساكن صحيح cmd_call(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
		    حرف *ثابت argv[], صحيح *repeatable)
{
	صحيح result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	אם (result)
		debug("Command failed, result=%d\n", result);
	החזר result;
}

تعداد command_ret_t cmd_process(صحيح flag, صحيح argc, حرف *ثابت argv[],
			       صحيح *repeatable, ulong *ticks)
{
	تعداد command_ret_t rc = CMD_RET_SUCCESS;
	بنية cmd_tbl *cmdtp;

#אם defined(CONFIG_SYS_XTRACE)
	حرف *xtrace;

	xtrace = env_get("xtrace");
	אם (xtrace) {
		puts("+");
		עבור (صحيح i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	אם (cmdtp == عدم) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		החזר 1;
	}

	/* found - check max args */
	אם (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#אם defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	אחרת אם (cmdtp->cmd == do_bootd) {
		אם (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} אחרת {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	אם (!rc) {
		صحيح newrep;

		אם (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		אם (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	אם (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	החזר rc;
}

صحيح cmd_process_error(بنية cmd_tbl *cmdtp, صحيح err)
{
	אם (err == CMD_RET_USAGE)
		החזר CMD_RET_USAGE;

	אם (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		החזר CMD_RET_FAILURE;
	}

	החזר CMD_RET_SUCCESS;
}

صحيح cmd_source_script(ulong addr, ثابت حرف *fit_uname, ثابت حرف *confname)
{
	حرف *data;
	فراغ *buf;
	uint len;
	صحيح ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	אם (ret)
		החזר CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	החזר run_command_list(data, len, 0);
}
