// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <compiler.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/ctype.h>

DECLARE_GLOBAL_তথ্য_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

পূর্ণসংখ্যা _do_help(কাঠামো cmd_tbl *cmd_start, পূর্ণসংখ্যা cmd_বস্তুসমূহ, কাঠামো cmd_tbl *cmdtp,
	     পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[])
{
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা rcode = 0;

	যদি (argc == 1) {	/* show list of commands */
		কাঠামো cmd_tbl *cmd_array[cmd_বস্তুসমূহ];
		পূর্ণসংখ্যা i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		জন্য (i = 0; i < cmd_বস্তুসমূহ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		জন্য (i = cmd_বস্তুসমূহ - 1; i > 0; --i) {
			swaps = 0;
			জন্য (j = 0; j < i; ++j) {
				যদি (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					কাঠামো cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			যদি (!swaps)
				বিরতি;
		}

		/* print short help (usage) */
		জন্য (i = 0; i < cmd_বস্তুসমূহ; i++) {
			ধ্রুবক অক্ষর *usage = cmd_array[i]->usage;

			/* allow user abort */
			যদি (ctrlc())
				ফেরত 1;
			যদি (usage == ফাঁকা)
				চালু;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ফেরত 0;
	}
	/*
	 * command help (long version)
	 */
	জন্য (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_বস্তুসমূহ);
		যদি (cmdtp != ফাঁকা) {
			rcode |= cmd_usage(cmdtp);
		} অন্যথা {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ফেরত rcode;
}

/* find command table entry for a command */
কাঠামো cmd_tbl *find_cmd_tbl(ধ্রুবক অক্ষর *cmd, কাঠামো cmd_tbl *table,
			     পূর্ণসংখ্যা table_len)
{
#ifdef CONFIG_CMDLINE
	কাঠামো cmd_tbl *cmdtp;
	কাঠামো cmd_tbl *cmdtp_অস্থায়ী = table;	/* Init value */
	ধ্রুবক অক্ষর *p;
	পূর্ণসংখ্যা len;
	পূর্ণসংখ্যা n_found = 0;

	যদি (!cmd)
		ফেরত ফাঁকা;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ফাঁকা) ? strlen (cmd) : (p - cmd);

	জন্য (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		যদি (strncmp(cmd, cmdtp->name, len) == 0) {
			যদি (len == strlen(cmdtp->name))
				ফেরত cmdtp;	/* full match */

			cmdtp_অস্থায়ী = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	যদি (n_found == 1) {			/* exactly one match */
		ফেরত cmdtp_অস্থায়ী;
	}
#endif /* CONFIG_CMDLINE */

	ফেরত ফাঁকা;	/* not found or ambiguous command */
}

কাঠামো cmd_tbl *find_cmd(ধ্রুবক অক্ষর *cmd)
{
	কাঠামো cmd_tbl *start = ll_entry_start(কাঠামো cmd_tbl, cmd);
	ধ্রুবক পূর্ণসংখ্যা len = ll_entry_গুনতি(কাঠামো cmd_tbl, cmd);
	ফেরত find_cmd_tbl(cmd, start, len);
}

পূর্ণসংখ্যা cmd_usage(ধ্রুবক কাঠামো cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	যদি (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ফেরত 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ফেরত 1;
}

#ifdef CONFIG_AUTO_COMPLETE
স্থির অক্ষর env_complete_buf[512];

পূর্ণসংখ্যা var_complete(পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[], অক্ষর last_char, পূর্ণসংখ্যা maxv,
		 অক্ষর *cmdv[])
{
	পূর্ণসংখ্যা space;

	space = last_char == '\0' || isblank(last_char);

	যদি (space && argc == 1)
		ফেরত env_complete("", maxv, cmdv, আকার(env_complete_buf),
				    env_complete_buf, false);

	যদি (!space && argc == 2)
		ফেরত env_complete(argv[1], maxv, cmdv,
				    আকার(env_complete_buf),
				    env_complete_buf, false);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা dollar_complete(পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[], অক্ষর last_char,
			   পূর্ণসংখ্যা maxv, অক্ষর *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	যদি (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ফেরত 0;

	ফেরত env_complete(argv[argc - 1], maxv, cmdv, আকার(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

পূর্ণসংখ্যা complete_subcmdv(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা গুনতি, পূর্ণসংখ্যা argc,
		     অক্ষর *ধ্রুবক argv[], অক্ষর last_char,
		     পূর্ণসংখ্যা maxv, অক্ষর *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ধ্রুবক কাঠামো cmd_tbl *cmdend = cmdtp + গুনতি;
	ধ্রুবক অক্ষর *p;
	পূর্ণসংখ্যা len, clen;
	পূর্ণসংখ্যা n_found = 0;
	ধ্রুবক অক্ষর *cmd;

	/* sanity? */
	যদি (maxv < 2)
		ফেরত -2;

	cmdv[0] = ফাঁকা;

	যদি (argc == 0) {
		/* output full list of commands */
		জন্য (; cmdtp != cmdend; cmdtp++) {
			যদি (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				বিরতি;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = ফাঁকা;
		ফেরত n_found;
	}

	/* more than one arg or one but the start of the next */
	যদি (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, গুনতি);
		যদি (cmdtp == ফাঁকা || cmdtp->complete == ফাঁকা) {
			cmdv[0] = ফাঁকা;
			ফেরত 0;
		}
		ফেরত (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	যদি (p == ফাঁকা)
		len = strlen(cmd);
	অন্যথা
		len = p - cmd;

	/* return the partial matches */
	জন্য (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		যদি (clen < len)
			চালু;

		যদি (memcmp(cmd, cmdtp->name, len) != 0)
			চালু;

		/* too many! */
		যদি (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			বিরতি;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = ফাঁকা;
	ফেরত n_found;
#অন্যথা
	ফেরত 0;
#endif
}

স্থির পূর্ণসংখ্যা complete_cmdv(পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[], অক্ষর last_char,
			 পূর্ণসংখ্যা maxv, অক্ষর *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ফেরত complete_subcmdv(ll_entry_start(কাঠামো cmd_tbl, cmd),
				ll_entry_গুনতি(কাঠামো cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#অন্যথা
	ফেরত 0;
#endif
}

স্থির পূর্ণসংখ্যা make_argv(অক্ষর *s, পূর্ণসংখ্যা argvsz, অক্ষর *argv[])
{
	পূর্ণসংখ্যা argc = 0;

	/* split into argv */
	যখন (argc < argvsz - 1) {

		/* skip any white space */
		যখন (isblank(*s))
			++s;

		যদি (*s == '\0')	/* end of s, no more args	*/
			বিরতি;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		যখন (*s && !isblank(*s))
			++s;

		যদি (*s == '\0')		/* end of s, no more args	*/
			বিরতি;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ফাঁকা;

	ফেরত argc;
}

স্থির শূন্য print_argv(ধ্রুবক অক্ষর *banner, ধ্রুবক অক্ষর *leader, ধ্রুবক অক্ষর *sep,
		       পূর্ণসংখ্যা linemax, অক্ষর *ধ্রুবক argv[])
{
	পূর্ণসংখ্যা ll = leader != ফাঁকা ? strlen(leader) : 0;
	পূর্ণসংখ্যা sl = sep != ফাঁকা ? strlen(sep) : 0;
	পূর্ণসংখ্যা len, i;

	যদি (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	যখন (*argv != ফাঁকা) {
		len = strlen(*argv) + sl;
		যদি (i + len >= linemax) {
			puts("\n");
			যদি (leader)
				puts(leader);
			i = ll - sl;
		} অন্যথা যদি (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

স্থির পূর্ণসংখ্যা find_common_prefix(অক্ষর *ধ্রুবক argv[])
{
	পূর্ণসংখ্যা i, len;
	অক্ষর *anchor, *s, *t;

	যদি (*argv == ফাঁকা)
		ফেরত 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	যখন ((t = *argv++) != ফাঁকা) {
		s = anchor;
		জন্য (i = 0; i < len; i++, t++, s++) {
			যদি (*t != *s)
				বিরতি;
		}
		len = s - anchor;
	}
	ফেরত len;
}

পূর্ণসংখ্যা cmd_auto_complete(ধ্রুবক অক্ষর *ধ্রুবক prompt, অক্ষর *buf, পূর্ণসংখ্যা *np, পূর্ণসংখ্যা *colp)
{
	অক্ষর tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	পূর্ণসংখ্যা n = *np, col = *colp;
	অক্ষর *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	অক্ষর *cmdv[20];
	অক্ষর *s, *t;
	ধ্রুবক অক্ষর *sep;
	পূর্ণসংখ্যা i, j, k, len, seplen, argc;
	পূর্ণসংখ্যা cnt;
	অক্ষর last_char;
	ধ্রুবক অক্ষর *ps_prompt;

	যদি (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		যদি (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} অন্যথা
		ps_prompt = CONFIG_SYS_PROMPT;

	যদি (strcmp(prompt, ps_prompt) != 0)
		ফেরত 0;	/* not in normal console */

	cnt = strlen(buf);
	যদি (cnt >= 1)
		last_char = buf[cnt - 1];
	অন্যথা
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, আকার(argv)/আকার(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    আকার(cmdv) / আকার(cmdv[0]), cmdv);
	যদি (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  আকার(cmdv) / আকার(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	যদি (i == 0) {
		যদি (argc > 1)	/* allow tab for non command */
			ফেরত 0;
		putc('\a');
		ফেরত 1;
	}

	s = ফাঁকা;
	len = 0;
	sep = ফাঁকা;
	seplen = 0;
	যদি (i == 1) { /* one match; perfect */
		যদি (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		অন্যথা
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} অন্যথা যদি (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		যদি (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		অন্যথা
			k = 0;

		j -= k;
		যদি (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	যদি (s != ফাঁকা) {
		k = len + seplen;
		/* make sure it fits */
		যদি (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ফেরত 1;
		}

		t = buf + cnt;
		জন্য (i = 0; i < len; i++)
			*t++ = *s++;
		যদি (sep != ফাঁকা)
			জন্য (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		যদি (sep == ফাঁকা)
			putc('\a');
		*np = n;
		*colp = col;
	} অন্যথা {
		print_argv(ফাঁকা, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ফেরত 1;
}

#endif

#ifdef CMD_তথ্য_আকার
পূর্ণসংখ্যা cmd_get_তথ্য_আকার(ধ্রুবক অক্ষর *arg, পূর্ণসংখ্যা default_আকার)
{
	/* Check for a size specification .b, .w or .l.
	 */
	পূর্ণসংখ্যা len = strlen(arg);
	যদি (len >= 2 && arg[len-2] == '.') {
		নির্বাচন (arg[len-1]) {
		ক্ষেত্র 'b':
			ফেরত 1;
		ক্ষেত্র 'w':
			ফেরত 2;
		ক্ষেত্র 'l':
			ফেরত 4;
		ক্ষেত্র 's':
			ফেরত CMD_তথ্য_আকার_STR;
		ক্ষেত্র 'q':
			যদি (MEM_SUPPORT_64_BIT_তথ্য)
				ফেরত 8;
			fallthrough;
		পূর্বনির্ধারিত:
			ফেরত CMD_তথ্য_আকার_ERR;
		}
	}
	ফেরত default_আকার;
}
#endif

শূন্য fixup_cmdtable(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা আকার_2)
{
	পূর্ণসংখ্যা	i;

	যদি (gd->reloc_off == 0)
		ফেরত;

	জন্য (i = 0; i < আকার_2; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(পূর্ণসংখ্যা (*)(কাঠামো cmd_tbl *, পূর্ণসংখ্যা, পূর্ণসংখ্যা,
				 অক্ষর * ধ্রুবক [], পূর্ণসংখ্যা *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (পূর্ণসংখ্যা (*)(কাঠামো cmd_tbl *, পূর্ণসংখ্যা, পূর্ণসংখ্যা,
				      অক্ষর *ধ্রুবক []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (অক্ষর *)addr;
		যদি (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (অক্ষর *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		যদি (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (অক্ষর *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		যদি (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(পূর্ণসংখ্যা (*)(পূর্ণসংখ্যা, অক্ষর * ধ্রুবক [], অক্ষর, পূর্ণসংখ্যা, অক্ষর * []))addr;
		}
#endif
		cmdtp++;
	}
}

পূর্ণসংখ্যা cmd_always_repeatable(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc,
			  অক্ষর *ধ্রুবক argv[], পূর্ণসংখ্যা *repeatable)
{
	*repeatable = 1;

	ফেরত cmdtp->cmd(cmdtp, flag, argc, argv);
}

পূর্ণসংখ্যা cmd_never_repeatable(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc,
			 অক্ষর *ধ্রুবক argv[], পূর্ণসংখ্যা *repeatable)
{
	*repeatable = 0;

	ফেরত cmdtp->cmd(cmdtp, flag, argc, argv);
}

পূর্ণসংখ্যা cmd_discard_repeatable(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc,
			   অক্ষর *ধ্রুবক argv[])
{
	পূর্ণসংখ্যা repeatable;

	ফেরত cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
স্থির পূর্ণসংখ্যা cmd_call(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc,
		    অক্ষর *ধ্রুবক argv[], পূর্ণসংখ্যা *repeatable)
{
	পূর্ণসংখ্যা ফলাফল;

	ফলাফল = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	যদি (ফলাফল)
		debug("Command failed, result=%d\n", ফলাফল);
	ফেরত ফলাফল;
}

গণনা command_ret_t cmd_process(পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[],
			       পূর্ণসংখ্যা *repeatable, ulong *ticks)
{
	গণনা command_ret_t rc = CMD_RET_SUCCESS;
	কাঠামো cmd_tbl *cmdtp;

#যদি defined(CONFIG_SYS_XTRACE)
	অক্ষর *xtrace;

	xtrace = env_get("xtrace");
	যদি (xtrace) {
		puts("+");
		জন্য (পূর্ণসংখ্যা i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	যদি (cmdtp == ফাঁকা) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ফেরত 1;
	}

	/* found - check max args */
	যদি (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#যদি defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	অন্যথা যদি (cmdtp->cmd == do_bootd) {
		যদি (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} অন্যথা {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	যদি (!rc) {
		পূর্ণসংখ্যা newrep;

		যদি (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		যদি (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	যদি (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ফেরত rc;
}

পূর্ণসংখ্যা cmd_process_error(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা err)
{
	যদি (err == CMD_RET_USAGE)
		ফেরত CMD_RET_USAGE;

	যদি (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ফেরত CMD_RET_FAILURE;
	}

	ফেরত CMD_RET_SUCCESS;
}

পূর্ণসংখ্যা cmd_source_script(ulong addr, ধ্রুবক অক্ষর *fit_uname, ধ্রুবক অক্ষর *confname)
{
	অক্ষর *তথ্য;
	শূন্য *buf;
	uint len;
	পূর্ণসংখ্যা ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &তথ্য, &len);
	unmap_sysmem(buf);
	যদি (ret)
		ফেরত CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ফেরত run_command_list(তথ্য, len, 0);
}
