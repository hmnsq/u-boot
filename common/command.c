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

صحيح _do_help(بنية cmd_tbl *cmd_start, صحيح cmd_عناصر, بنية cmd_tbl *cmdtp,
	     صحيح flag, صحيح argc, حرف *ثابت argv[])
{
	صحيح i;
	صحيح rcode = 0;

	إذا (argc == 1) {	/* show list of commands */
		بنية cmd_tbl *cmd_array[cmd_عناصر];
		صحيح i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		لكل (i = 0; i < cmd_عناصر; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		لكل (i = cmd_عناصر - 1; i > 0; --i) {
			swaps = 0;
			لكل (j = 0; j < i; ++j) {
				إذا (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					بنية cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			إذا (!swaps)
				اكسر;
		}

		/* print short help (usage) */
		لكل (i = 0; i < cmd_عناصر; i++) {
			ثابت حرف *usage = cmd_array[i]->usage;

			/* allow user abort */
			إذا (ctrlc())
				إرجاع 1;
			إذا (usage == عدم)
				استمر;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		إرجاع 0;
	}
	/*
	 * command help (long version)
	 */
	لكل (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_عناصر);
		إذا (cmdtp != عدم) {
			rcode |= cmd_usage(cmdtp);
		} وإلا {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	إرجاع rcode;
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

	إذا (!cmd)
		إرجاع عدم;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == عدم) ? strlen (cmd) : (p - cmd);

	لكل (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		إذا (strncmp(cmd, cmdtp->name, len) == 0) {
			إذا (len == strlen(cmdtp->name))
				إرجاع cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	إذا (n_found == 1) {			/* exactly one match */
		إرجاع cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	إرجاع عدم;	/* not found or ambiguous command */
}

بنية cmd_tbl *find_cmd(ثابت حرف *cmd)
{
	بنية cmd_tbl *start = ll_entry_start(بنية cmd_tbl, cmd);
	ثابت صحيح len = ll_entry_تعداد(بنية cmd_tbl, cmd);
	إرجاع find_cmd_tbl(cmd, start, len);
}

صحيح cmd_usage(ثابت بنية cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	إذا (!cmdtp->help) {
		puts ("- No additional help available.\n");
		إرجاع 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	إرجاع 1;
}

#ifdef CONFIG_AUTO_COMPLETE
ساكن حرف env_complete_buf[512];

صحيح var_complete(صحيح argc, حرف *ثابت argv[], حرف last_char, صحيح maxv,
		 حرف *cmdv[])
{
	صحيح space;

	space = last_char == '\0' || isblank(last_char);

	إذا (space && argc == 1)
		إرجاع env_complete("", maxv, cmdv, حجم(env_complete_buf),
				    env_complete_buf, false);

	إذا (!space && argc == 2)
		إرجاع env_complete(argv[1], maxv, cmdv,
				    حجم(env_complete_buf),
				    env_complete_buf, false);

	إرجاع 0;
}

ساكن صحيح dollar_complete(صحيح argc, حرف *ثابت argv[], حرف last_char,
			   صحيح maxv, حرف *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	إذا (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		إرجاع 0;

	إرجاع env_complete(argv[argc - 1], maxv, cmdv, حجم(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

صحيح complete_subcmdv(بنية cmd_tbl *cmdtp, صحيح تعداد_2, صحيح argc,
		     حرف *ثابت argv[], حرف last_char,
		     صحيح maxv, حرف *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ثابت بنية cmd_tbl *cmdend = cmdtp + تعداد_2;
	ثابت حرف *p;
	صحيح len, clen;
	صحيح n_found = 0;
	ثابت حرف *cmd;

	/* sanity? */
	إذا (maxv < 2)
		إرجاع -2;

	cmdv[0] = عدم;

	إذا (argc == 0) {
		/* output full list of commands */
		لكل (; cmdtp != cmdend; cmdtp++) {
			إذا (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				اكسر;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = عدم;
		إرجاع n_found;
	}

	/* more than one arg or one but the start of the next */
	إذا (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, تعداد_2);
		إذا (cmdtp == عدم || cmdtp->complete == عدم) {
			cmdv[0] = عدم;
			إرجاع 0;
		}
		إرجاع (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	إذا (p == عدم)
		len = strlen(cmd);
	وإلا
		len = p - cmd;

	/* return the partial matches */
	لكل (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		إذا (clen < len)
			استمر;

		إذا (memcmp(cmd, cmdtp->name, len) != 0)
			استمر;

		/* too many! */
		إذا (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			اكسر;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = عدم;
	إرجاع n_found;
#وإلا
	إرجاع 0;
#endif
}

ساكن صحيح complete_cmdv(صحيح argc, حرف *ثابت argv[], حرف last_char,
			 صحيح maxv, حرف *cmdv[])
{
#ifdef CONFIG_CMDLINE
	إرجاع complete_subcmdv(ll_entry_start(بنية cmd_tbl, cmd),
				ll_entry_تعداد(بنية cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#وإلا
	إرجاع 0;
#endif
}

ساكن صحيح make_argv(حرف *s, صحيح argvsz, حرف *argv[])
{
	صحيح argc = 0;

	/* split into argv */
	بينما (argc < argvsz - 1) {

		/* skip any white space */
		بينما (isblank(*s))
			++s;

		إذا (*s == '\0')	/* end of s, no more args	*/
			اكسر;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		بينما (*s && !isblank(*s))
			++s;

		إذا (*s == '\0')		/* end of s, no more args	*/
			اكسر;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = عدم;

	إرجاع argc;
}

ساكن فراغ print_argv(ثابت حرف *banner, ثابت حرف *leader, ثابت حرف *sep,
		       صحيح linemax, حرف *ثابت argv[])
{
	صحيح ll = leader != عدم ? strlen(leader) : 0;
	صحيح sl = sep != عدم ? strlen(sep) : 0;
	صحيح len, i;

	إذا (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	بينما (*argv != عدم) {
		len = strlen(*argv) + sl;
		إذا (i + len >= linemax) {
			puts("\n");
			إذا (leader)
				puts(leader);
			i = ll - sl;
		} وإلا إذا (sep)
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

	إذا (*argv == عدم)
		إرجاع 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	بينما ((t = *argv++) != عدم) {
		s = anchor;
		لكل (i = 0; i < len; i++, t++, s++) {
			إذا (*t != *s)
				اكسر;
		}
		len = s - anchor;
	}
	إرجاع len;
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

	إذا (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		إذا (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} وإلا
		ps_prompt = CONFIG_SYS_PROMPT;

	إذا (strcmp(prompt, ps_prompt) != 0)
		إرجاع 0;	/* not in normal console */

	cnt = strlen(buf);
	إذا (cnt >= 1)
		last_char = buf[cnt - 1];
	وإلا
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, حجم(argv)/حجم(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    حجم(cmdv) / حجم(cmdv[0]), cmdv);
	إذا (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  حجم(cmdv) / حجم(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	إذا (i == 0) {
		إذا (argc > 1)	/* allow tab for non command */
			إرجاع 0;
		putc('\a');
		إرجاع 1;
	}

	s = عدم;
	len = 0;
	sep = عدم;
	seplen = 0;
	إذا (i == 1) { /* one match; perfect */
		إذا (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		وإلا
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} وإلا إذا (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		إذا (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		وإلا
			k = 0;

		j -= k;
		إذا (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	إذا (s != عدم) {
		k = len + seplen;
		/* make sure it fits */
		إذا (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			إرجاع 1;
		}

		t = buf + cnt;
		لكل (i = 0; i < len; i++)
			*t++ = *s++;
		إذا (sep != عدم)
			لكل (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		إذا (sep == عدم)
			putc('\a');
		*np = n;
		*colp = col;
	} وإلا {
		print_argv(عدم, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	إرجاع 1;
}

#endif

#ifdef CMD_DATA_SIZE
صحيح cmd_get_data_size(ثابت حرف *arg, صحيح default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	صحيح len = strlen(arg);
	إذا (len >= 2 && arg[len-2] == '.') {
		بدل (arg[len-1]) {
		حالة 'b':
			إرجاع 1;
		حالة 'w':
			إرجاع 2;
		حالة 'l':
			إرجاع 4;
		حالة 's':
			إرجاع CMD_DATA_SIZE_STR;
		حالة 'q':
			إذا (MEM_SUPPORT_64BIT_DATA)
				إرجاع 8;
			fallthrough;
		افتراضي:
			إرجاع CMD_DATA_SIZE_ERR;
		}
	}
	إرجاع default_size;
}
#endif

فراغ fixup_cmdtable(بنية cmd_tbl *cmdtp, صحيح size)
{
	صحيح	i;

	إذا (gd->reloc_off == 0)
		إرجاع;

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
		إذا (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (حرف *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		إذا (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (حرف *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		إذا (cmdtp->complete) {
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

	إرجاع cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_never_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			 حرف *ثابت argv[], صحيح *repeatable)
{
	*repeatable = 0;

	إرجاع cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_discard_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			   حرف *ثابت argv[])
{
	صحيح repeatable;

	إرجاع cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	إذا (result)
		debug("Command failed, result=%d\n", result);
	إرجاع result;
}

تعداد command_ret_t cmd_process(صحيح flag, صحيح argc, حرف *ثابت argv[],
			       صحيح *repeatable, ulong *ticks)
{
	تعداد command_ret_t rc = CMD_RET_SUCCESS;
	بنية cmd_tbl *cmdtp;

#إذا defined(CONFIG_SYS_XTRACE)
	حرف *xtrace;

	xtrace = env_get("xtrace");
	إذا (xtrace) {
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
	إذا (cmdtp == عدم) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		إرجاع 1;
	}

	/* found - check max args */
	إذا (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#إذا defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	وإلا إذا (cmdtp->cmd == do_bootd) {
		إذا (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} وإلا {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	إذا (!rc) {
		صحيح newrep;

		إذا (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		إذا (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	إذا (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	إرجاع rc;
}

صحيح cmd_process_error(بنية cmd_tbl *cmdtp, صحيح err)
{
	إذا (err == CMD_RET_USAGE)
		إرجاع CMD_RET_USAGE;

	إذا (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		إرجاع CMD_RET_FAILURE;
	}

	إرجاع CMD_RET_SUCCESS;
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
	إذا (ret)
		إرجاع CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	إرجاع run_command_list(data, len, 0);
}
