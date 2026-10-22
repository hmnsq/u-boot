// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#অন্তর্ভাৱ <config.h>
#অন্তর্ভাৱ <compiler.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

পূর্ণাঙ্ক _do_help(সংরচনা cmd_tbl *cmd_start, পূর্ণাঙ্ক cmd_বস্তুসমূহ, সংরচনা cmd_tbl *cmdtp,
	     পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc, ৱর্ণ *ধ্রুৱ argv[])
{
	পূর্ণাঙ্ক i;
	পূর্ণাঙ্ক rcode = 0;

	যদি (argc == 1) {	/* show list of commands */
		সংরচনা cmd_tbl *cmd_array[cmd_বস্তুসমূহ];
		পূর্ণাঙ্ক i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		বাবে (i = 0; i < cmd_বস্তুসমূহ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		বাবে (i = cmd_বস্তুসমূহ - 1; i > 0; --i) {
			swaps = 0;
			বাবে (j = 0; j < i; ++j) {
				যদি (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					সংরচনা cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			যদি (!swaps)
				ভঙ্গ;
		}

		/* print short help (usage) */
		বাবে (i = 0; i < cmd_বস্তুসমূহ; i++) {
			ধ্রুৱ ৱর্ণ *usage = cmd_array[i]->usage;

			/* allow user abort */
			যদি (ctrlc())
				ঘূৰাই 1;
			যদি (usage == রিক্ত)
				অনুৱর্ত;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ঘূৰাই 0;
	}
	/*
	 * command help (long version)
	 */
	বাবে (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_বস্তুসমূহ);
		যদি (cmdtp != রিক্ত) {
			rcode |= cmd_usage(cmdtp);
		} অন্যথা {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ঘূৰাই rcode;
}

/* find command table entry for a command */
সংরচনা cmd_tbl *find_cmd_tbl(ধ্রুৱ ৱর্ণ *cmd, সংরচনা cmd_tbl *table,
			     পূর্ণাঙ্ক table_len)
{
#ifdef CONFIG_CMDLINE
	সংরচনা cmd_tbl *cmdtp;
	সংরচনা cmd_tbl *cmdtp_temp = table;	/* Init value */
	ধ্রুৱ ৱর্ণ *p;
	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক n_found = 0;

	যদি (!cmd)
		ঘূৰাই রিক্ত;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == রিক্ত) ? strlen (cmd) : (p - cmd);

	বাবে (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		যদি (strncmp(cmd, cmdtp->name, len) == 0) {
			যদি (len == strlen(cmdtp->name))
				ঘূৰাই cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	যদি (n_found == 1) {			/* exactly one match */
		ঘূৰাই cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ঘূৰাই রিক্ত;	/* not found or ambiguous command */
}

সংরচনা cmd_tbl *find_cmd(ধ্রুৱ ৱর্ণ *cmd)
{
	সংরচনা cmd_tbl *start = ll_entry_start(সংরচনা cmd_tbl, cmd);
	ধ্রুৱ পূর্ণাঙ্ক len = ll_entry_গুনতি(সংরচনা cmd_tbl, cmd);
	ঘূৰাই find_cmd_tbl(cmd, start, len);
}

পূর্ণাঙ্ক cmd_usage(ধ্রুৱ সংরচনা cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	যদি (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ঘূৰাই 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ঘূৰাই 1;
}

#ifdef CONFIG_AUTO_COMPLETE
স্থির ৱর্ণ env_complete_buf[512];

পূর্ণাঙ্ক var_complete(পূর্ণাঙ্ক argc, ৱর্ণ *ধ্রুৱ argv[], ৱর্ণ last_char, পূর্ণাঙ্ক maxv,
		 ৱর্ণ *cmdv[])
{
	পূর্ণাঙ্ক space;

	space = last_char == '\0' || isblank(last_char);

	যদি (space && argc == 1)
		ঘূৰাই env_complete("", maxv, cmdv, পরিমাণ(env_complete_buf),
				    env_complete_buf, false);

	যদি (!space && argc == 2)
		ঘূৰাই env_complete(argv[1], maxv, cmdv,
				    পরিমাণ(env_complete_buf),
				    env_complete_buf, false);

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক dollar_complete(পূর্ণাঙ্ক argc, ৱর্ণ *ধ্রুৱ argv[], ৱর্ণ last_char,
			   পূর্ণাঙ্ক maxv, ৱর্ণ *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	যদি (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ঘূৰাই 0;

	ঘূৰাই env_complete(argv[argc - 1], maxv, cmdv, পরিমাণ(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

পূর্ণাঙ্ক complete_subcmdv(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক গুনতি, পূর্ণাঙ্ক argc,
		     ৱর্ণ *ধ্রুৱ argv[], ৱর্ণ last_char,
		     পূর্ণাঙ্ক maxv, ৱর্ণ *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ধ্রুৱ সংরচনা cmd_tbl *cmdend = cmdtp + গুনতি;
	ধ্রুৱ ৱর্ণ *p;
	পূর্ণাঙ্ক len, clen;
	পূর্ণাঙ্ক n_found = 0;
	ধ্রুৱ ৱর্ণ *cmd;

	/* sanity? */
	যদি (maxv < 2)
		ঘূৰাই -2;

	cmdv[0] = রিক্ত;

	যদি (argc == 0) {
		/* output full list of commands */
		বাবে (; cmdtp != cmdend; cmdtp++) {
			যদি (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				ভঙ্গ;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = রিক্ত;
		ঘূৰাই n_found;
	}

	/* more than one arg or one but the start of the next */
	যদি (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, গুনতি);
		যদি (cmdtp == রিক্ত || cmdtp->complete == রিক্ত) {
			cmdv[0] = রিক্ত;
			ঘূৰাই 0;
		}
		ঘূৰাই (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	যদি (p == রিক্ত)
		len = strlen(cmd);
	অন্যথা
		len = p - cmd;

	/* return the partial matches */
	বাবে (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		যদি (clen < len)
			অনুৱর্ত;

		যদি (memcmp(cmd, cmdtp->name, len) != 0)
			অনুৱর্ত;

		/* too many! */
		যদি (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			ভঙ্গ;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = রিক্ত;
	ঘূৰাই n_found;
#অন্যথা
	ঘূৰাই 0;
#endif
}

স্থির পূর্ণাঙ্ক complete_cmdv(পূর্ণাঙ্ক argc, ৱর্ণ *ধ্রুৱ argv[], ৱর্ণ last_char,
			 পূর্ণাঙ্ক maxv, ৱর্ণ *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ঘূৰাই complete_subcmdv(ll_entry_start(সংরচনা cmd_tbl, cmd),
				ll_entry_গুনতি(সংরচনা cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#অন্যথা
	ঘূৰাই 0;
#endif
}

স্থির পূর্ণাঙ্ক make_argv(ৱর্ণ *s, পূর্ণাঙ্ক argvsz, ৱর্ণ *argv[])
{
	পূর্ণাঙ্ক argc = 0;

	/* split into argv */
	যেতিয়ালৈ (argc < argvsz - 1) {

		/* skip any white space */
		যেতিয়ালৈ (isblank(*s))
			++s;

		যদি (*s == '\0')	/* end of s, no more args	*/
			ভঙ্গ;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		যেতিয়ালৈ (*s && !isblank(*s))
			++s;

		যদি (*s == '\0')		/* end of s, no more args	*/
			ভঙ্গ;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = রিক্ত;

	ঘূৰাই argc;
}

স্থির শূন্য print_argv(ধ্রুৱ ৱর্ণ *banner, ধ্রুৱ ৱর্ণ *leader, ধ্রুৱ ৱর্ণ *sep,
		       পূর্ণাঙ্ক linemax, ৱর্ণ *ধ্রুৱ argv[])
{
	পূর্ণাঙ্ক ll = leader != রিক্ত ? strlen(leader) : 0;
	পূর্ণাঙ্ক sl = sep != রিক্ত ? strlen(sep) : 0;
	পূর্ণাঙ্ক len, i;

	যদি (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	যেতিয়ালৈ (*argv != রিক্ত) {
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

স্থির পূর্ণাঙ্ক find_common_prefix(ৱর্ণ *ধ্রুৱ argv[])
{
	পূর্ণাঙ্ক i, len;
	ৱর্ণ *anchor, *s, *t;

	যদি (*argv == রিক্ত)
		ঘূৰাই 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	যেতিয়ালৈ ((t = *argv++) != রিক্ত) {
		s = anchor;
		বাবে (i = 0; i < len; i++, t++, s++) {
			যদি (*t != *s)
				ভঙ্গ;
		}
		len = s - anchor;
	}
	ঘূৰাই len;
}

পূর্ণাঙ্ক cmd_auto_complete(ধ্রুৱ ৱর্ণ *ধ্রুৱ prompt, ৱর্ণ *buf, পূর্ণাঙ্ক *np, পূর্ণাঙ্ক *colp)
{
	ৱর্ণ tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	পূর্ণাঙ্ক n = *np, col = *colp;
	ৱর্ণ *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	ৱর্ণ *cmdv[20];
	ৱর্ণ *s, *t;
	ধ্রুৱ ৱর্ণ *sep;
	পূর্ণাঙ্ক i, j, k, len, seplen, argc;
	পূর্ণাঙ্ক cnt;
	ৱর্ণ last_char;
	ধ্রুৱ ৱর্ণ *ps_prompt;

	যদি (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		যদি (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} অন্যথা
		ps_prompt = CONFIG_SYS_PROMPT;

	যদি (strcmp(prompt, ps_prompt) != 0)
		ঘূৰাই 0;	/* not in normal console */

	cnt = strlen(buf);
	যদি (cnt >= 1)
		last_char = buf[cnt - 1];
	অন্যথা
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, পরিমাণ(argv)/পরিমাণ(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    পরিমাণ(cmdv) / পরিমাণ(cmdv[0]), cmdv);
	যদি (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  পরিমাণ(cmdv) / পরিমাণ(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	যদি (i == 0) {
		যদি (argc > 1)	/* allow tab for non command */
			ঘূৰাই 0;
		putc('\a');
		ঘূৰাই 1;
	}

	s = রিক্ত;
	len = 0;
	sep = রিক্ত;
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

	যদি (s != রিক্ত) {
		k = len + seplen;
		/* make sure it fits */
		যদি (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ঘূৰাই 1;
		}

		t = buf + cnt;
		বাবে (i = 0; i < len; i++)
			*t++ = *s++;
		যদি (sep != রিক্ত)
			বাবে (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		যদি (sep == রিক্ত)
			putc('\a');
		*np = n;
		*colp = col;
	} অন্যথা {
		print_argv(রিক্ত, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ঘূৰাই 1;
}

#endif

#ifdef CMD_DATA_SIZE
পূর্ণাঙ্ক cmd_get_data_size(ধ্রুৱ ৱর্ণ *arg, পূর্ণাঙ্ক default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	পূর্ণাঙ্ক len = strlen(arg);
	যদি (len >= 2 && arg[len-2] == '.') {
		ৱিকল্প (arg[len-1]) {
		পক্ষ 'b':
			ঘূৰাই 1;
		পক্ষ 'w':
			ঘূৰাই 2;
		পক্ষ 'l':
			ঘূৰাই 4;
		পক্ষ 's':
			ঘূৰাই CMD_DATA_SIZE_STR;
		পক্ষ 'q':
			যদি (MEM_SUPPORT_64BIT_DATA)
				ঘূৰাই 8;
			fallthrough;
		স্ৱভাৱ:
			ঘূৰাই CMD_DATA_SIZE_ERR;
		}
	}
	ঘূৰাই default_size;
}
#endif

শূন্য fixup_cmdtable(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক size)
{
	পূর্ণাঙ্ক	i;

	যদি (gd->reloc_off == 0)
		ঘূৰাই;

	বাবে (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(পূর্ণাঙ্ক (*)(সংরচনা cmd_tbl *, পূর্ণাঙ্ক, পূর্ণাঙ্ক,
				 ৱর্ণ * ধ্রুৱ [], পূর্ণাঙ্ক *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (পূর্ণাঙ্ক (*)(সংরচনা cmd_tbl *, পূর্ণাঙ্ক, পূর্ণাঙ্ক,
				      ৱর্ণ *ধ্রুৱ []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (ৱর্ণ *)addr;
		যদি (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (ৱর্ণ *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		যদি (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (ৱর্ণ *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		যদি (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(পূর্ণাঙ্ক (*)(পূর্ণাঙ্ক, ৱর্ণ * ধ্রুৱ [], ৱর্ণ, পূর্ণাঙ্ক, ৱর্ণ * []))addr;
		}
#endif
		cmdtp++;
	}
}

পূর্ণাঙ্ক cmd_always_repeatable(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc,
			  ৱর্ণ *ধ্রুৱ argv[], পূর্ণাঙ্ক *repeatable)
{
	*repeatable = 1;

	ঘূৰাই cmdtp->cmd(cmdtp, flag, argc, argv);
}

পূর্ণাঙ্ক cmd_never_repeatable(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc,
			 ৱর্ণ *ধ্রুৱ argv[], পূর্ণাঙ্ক *repeatable)
{
	*repeatable = 0;

	ঘূৰাই cmdtp->cmd(cmdtp, flag, argc, argv);
}

পূর্ণাঙ্ক cmd_discard_repeatable(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc,
			   ৱর্ণ *ধ্রুৱ argv[])
{
	পূর্ণাঙ্ক repeatable;

	ঘূৰাই cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
স্থির পূর্ণাঙ্ক cmd_call(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc,
		    ৱর্ণ *ধ্রুৱ argv[], পূর্ণাঙ্ক *repeatable)
{
	পূর্ণাঙ্ক result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	যদি (result)
		debug("Command failed, result=%d\n", result);
	ঘূৰাই result;
}

গণনা command_ret_t cmd_process(পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc, ৱর্ণ *ধ্রুৱ argv[],
			       পূর্ণাঙ্ক *repeatable, ulong *ticks)
{
	গণনা command_ret_t rc = CMD_RET_SUCCESS;
	সংরচনা cmd_tbl *cmdtp;

#যদি defined(CONFIG_SYS_XTRACE)
	ৱর্ণ *xtrace;

	xtrace = env_get("xtrace");
	যদি (xtrace) {
		puts("+");
		বাবে (পূর্ণাঙ্ক i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	যদি (cmdtp == রিক্ত) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ঘূৰাই 1;
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
		পূর্ণাঙ্ক newrep;

		যদি (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		যদি (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	যদি (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ঘূৰাই rc;
}

পূর্ণাঙ্ক cmd_process_error(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক err)
{
	যদি (err == CMD_RET_USAGE)
		ঘূৰাই CMD_RET_USAGE;

	যদি (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ঘূৰাই CMD_RET_FAILURE;
	}

	ঘূৰাই CMD_RET_SUCCESS;
}

পূর্ণাঙ্ক cmd_source_script(ulong addr, ধ্রুৱ ৱর্ণ *fit_uname, ধ্রুৱ ৱর্ণ *confname)
{
	ৱর্ণ *data;
	শূন্য *buf;
	uint len;
	পূর্ণাঙ্ক ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	যদি (ret)
		ঘূৰাই CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ঘূৰাই run_command_list(data, len, 0);
}
