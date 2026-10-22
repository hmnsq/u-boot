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

	ܐܢ (argc == 1) {	/* show list of commands */
		بنية cmd_tbl *cmd_array[cmd_items];
		صحيح i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		لكل (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		لكل (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			لكل (j = 0; j < i; ++j) {
				ܐܢ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					بنية cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ܐܢ (!swaps)
				اكسر;
		}

		/* print short help (usage) */
		لكل (i = 0; i < cmd_items; i++) {
			ثابت حرف *usage = cmd_array[i]->usage;

			/* allow user abort */
			ܐܢ (ctrlc())
				ܦܢܐ 1;
			ܐܢ (usage == عدم)
				استمر;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ܦܢܐ 0;
	}
	/*
	 * command help (long version)
	 */
	لكل (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ܐܢ (cmdtp != عدم) {
			rcode |= cmd_usage(cmdtp);
		} ܐܚܪܢܐ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ܦܢܐ rcode;
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

	ܐܢ (!cmd)
		ܦܢܐ عدم;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == عدم) ? strlen (cmd) : (p - cmd);

	لكل (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ܐܢ (strncmp(cmd, cmdtp->name, len) == 0) {
			ܐܢ (len == strlen(cmdtp->name))
				ܦܢܐ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ܐܢ (n_found == 1) {			/* exactly one match */
		ܦܢܐ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ܦܢܐ عدم;	/* not found or ambiguous command */
}

بنية cmd_tbl *find_cmd(ثابت حرف *cmd)
{
	بنية cmd_tbl *start = ll_entry_start(بنية cmd_tbl, cmd);
	ثابت صحيح len = ll_entry_count(بنية cmd_tbl, cmd);
	ܦܢܐ find_cmd_tbl(cmd, start, len);
}

صحيح cmd_usage(ثابت بنية cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ܐܢ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ܦܢܐ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ܦܢܐ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
ساكن حرف env_complete_buf[512];

صحيح var_complete(صحيح argc, حرف *ثابت argv[], حرف last_char, صحيح maxv,
		 حرف *cmdv[])
{
	صحيح space;

	space = last_char == '\0' || isblank(last_char);

	ܐܢ (space && argc == 1)
		ܦܢܐ env_complete("", maxv, cmdv, حجم(env_complete_buf),
				    env_complete_buf, false);

	ܐܢ (!space && argc == 2)
		ܦܢܐ env_complete(argv[1], maxv, cmdv,
				    حجم(env_complete_buf),
				    env_complete_buf, false);

	ܦܢܐ 0;
}

ساكن صحيح dollar_complete(صحيح argc, حرف *ثابت argv[], حرف last_char,
			   صحيح maxv, حرف *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ܐܢ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ܦܢܐ 0;

	ܦܢܐ env_complete(argv[argc - 1], maxv, cmdv, حجم(env_complete_buf),
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
	ܐܢ (maxv < 2)
		ܦܢܐ -2;

	cmdv[0] = عدم;

	ܐܢ (argc == 0) {
		/* output full list of commands */
		لكل (; cmdtp != cmdend; cmdtp++) {
			ܐܢ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				اكسر;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = عدم;
		ܦܢܐ n_found;
	}

	/* more than one arg or one but the start of the next */
	ܐܢ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ܐܢ (cmdtp == عدم || cmdtp->complete == عدم) {
			cmdv[0] = عدم;
			ܦܢܐ 0;
		}
		ܦܢܐ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ܐܢ (p == عدم)
		len = strlen(cmd);
	ܐܚܪܢܐ
		len = p - cmd;

	/* return the partial matches */
	لكل (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ܐܢ (clen < len)
			استمر;

		ܐܢ (memcmp(cmd, cmdtp->name, len) != 0)
			استمر;

		/* too many! */
		ܐܢ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			اكسر;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = عدم;
	ܦܢܐ n_found;
#ܐܚܪܢܐ
	ܦܢܐ 0;
#endif
}

ساكن صحيح complete_cmdv(صحيح argc, حرف *ثابت argv[], حرف last_char,
			 صحيح maxv, حرف *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ܦܢܐ complete_subcmdv(ll_entry_start(بنية cmd_tbl, cmd),
				ll_entry_count(بنية cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ܐܚܪܢܐ
	ܦܢܐ 0;
#endif
}

ساكن صحيح make_argv(حرف *s, صحيح argvsz, حرف *argv[])
{
	صحيح argc = 0;

	/* split into argv */
	ܟܡܐ (argc < argvsz - 1) {

		/* skip any white space */
		ܟܡܐ (isblank(*s))
			++s;

		ܐܢ (*s == '\0')	/* end of s, no more args	*/
			اكسر;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ܟܡܐ (*s && !isblank(*s))
			++s;

		ܐܢ (*s == '\0')		/* end of s, no more args	*/
			اكسر;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = عدم;

	ܦܢܐ argc;
}

ساكن فراغ print_argv(ثابت حرف *banner, ثابت حرف *leader, ثابت حرف *sep,
		       صحيح linemax, حرف *ثابت argv[])
{
	صحيح ll = leader != عدم ? strlen(leader) : 0;
	صحيح sl = sep != عدم ? strlen(sep) : 0;
	صحيح len, i;

	ܐܢ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ܟܡܐ (*argv != عدم) {
		len = strlen(*argv) + sl;
		ܐܢ (i + len >= linemax) {
			puts("\n");
			ܐܢ (leader)
				puts(leader);
			i = ll - sl;
		} ܐܚܪܢܐ ܐܢ (sep)
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

	ܐܢ (*argv == عدم)
		ܦܢܐ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ܟܡܐ ((t = *argv++) != عدم) {
		s = anchor;
		لكل (i = 0; i < len; i++, t++, s++) {
			ܐܢ (*t != *s)
				اكسر;
		}
		len = s - anchor;
	}
	ܦܢܐ len;
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

	ܐܢ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ܐܢ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ܐܚܪܢܐ
		ps_prompt = CONFIG_SYS_PROMPT;

	ܐܢ (strcmp(prompt, ps_prompt) != 0)
		ܦܢܐ 0;	/* not in normal console */

	cnt = strlen(buf);
	ܐܢ (cnt >= 1)
		last_char = buf[cnt - 1];
	ܐܚܪܢܐ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, حجم(argv)/حجم(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    حجم(cmdv) / حجم(cmdv[0]), cmdv);
	ܐܢ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  حجم(cmdv) / حجم(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ܐܢ (i == 0) {
		ܐܢ (argc > 1)	/* allow tab for non command */
			ܦܢܐ 0;
		putc('\a');
		ܦܢܐ 1;
	}

	s = عدم;
	len = 0;
	sep = عدم;
	seplen = 0;
	ܐܢ (i == 1) { /* one match; perfect */
		ܐܢ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ܐܚܪܢܐ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ܐܚܪܢܐ ܐܢ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ܐܢ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ܐܚܪܢܐ
			k = 0;

		j -= k;
		ܐܢ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ܐܢ (s != عدم) {
		k = len + seplen;
		/* make sure it fits */
		ܐܢ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ܦܢܐ 1;
		}

		t = buf + cnt;
		لكل (i = 0; i < len; i++)
			*t++ = *s++;
		ܐܢ (sep != عدم)
			لكل (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ܐܢ (sep == عدم)
			putc('\a');
		*np = n;
		*colp = col;
	} ܐܚܪܢܐ {
		print_argv(عدم, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ܦܢܐ 1;
}

#endif

#ifdef CMD_DATA_SIZE
صحيح cmd_get_data_size(ثابت حرف *arg, صحيح default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	صحيح len = strlen(arg);
	ܐܢ (len >= 2 && arg[len-2] == '.') {
		بدل (arg[len-1]) {
		حالة 'b':
			ܦܢܐ 1;
		حالة 'w':
			ܦܢܐ 2;
		حالة 'l':
			ܦܢܐ 4;
		حالة 's':
			ܦܢܐ CMD_DATA_SIZE_STR;
		حالة 'q':
			ܐܢ (MEM_SUPPORT_64BIT_DATA)
				ܦܢܐ 8;
			fallthrough;
		افتراضي:
			ܦܢܐ CMD_DATA_SIZE_ERR;
		}
	}
	ܦܢܐ default_size;
}
#endif

فراغ fixup_cmdtable(بنية cmd_tbl *cmdtp, صحيح size)
{
	صحيح	i;

	ܐܢ (gd->reloc_off == 0)
		ܦܢܐ;

	لكل (i = 0; i < size; i++) {
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
		ܐܢ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (حرف *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ܐܢ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (حرف *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ܐܢ (cmdtp->complete) {
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

	ܦܢܐ cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_never_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			 حرف *ثابت argv[], صحيح *repeatable)
{
	*repeatable = 0;

	ܦܢܐ cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_discard_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			   حرف *ثابت argv[])
{
	صحيح repeatable;

	ܦܢܐ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	ܐܢ (result)
		debug("Command failed, result=%d\n", result);
	ܦܢܐ result;
}

تعداد command_ret_t cmd_process(صحيح flag, صحيح argc, حرف *ثابت argv[],
			       صحيح *repeatable, ulong *ticks)
{
	تعداد command_ret_t rc = CMD_RET_SUCCESS;
	بنية cmd_tbl *cmdtp;

#ܐܢ defined(CONFIG_SYS_XTRACE)
	حرف *xtrace;

	xtrace = env_get("xtrace");
	ܐܢ (xtrace) {
		puts("+");
		لكل (صحيح i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ܐܢ (cmdtp == عدم) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ܦܢܐ 1;
	}

	/* found - check max args */
	ܐܢ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ܐܢ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ܐܚܪܢܐ ܐܢ (cmdtp->cmd == do_bootd) {
		ܐܢ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ܐܚܪܢܐ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ܐܢ (!rc) {
		صحيح newrep;

		ܐܢ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ܐܢ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ܐܢ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ܦܢܐ rc;
}

صحيح cmd_process_error(بنية cmd_tbl *cmdtp, صحيح err)
{
	ܐܢ (err == CMD_RET_USAGE)
		ܦܢܐ CMD_RET_USAGE;

	ܐܢ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ܦܢܐ CMD_RET_FAILURE;
	}

	ܦܢܐ CMD_RET_SUCCESS;
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
	ܐܢ (ret)
		ܦܢܐ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ܦܢܐ run_command_list(data, len, 0);
}
