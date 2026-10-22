// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <compiler.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

പൂർണ്ണസംഖ്യ _do_help(ഘടന cmd_tbl *cmd_start, പൂർണ്ണസംഖ്യ cmd_ഇനങ്ങൾ, ഘടന cmd_tbl *cmdtp,
	     പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[])
{
	പൂർണ്ണസംഖ്യ i;
	പൂർണ്ണസംഖ്യ rcode = 0;

	എങ്കിൽ (argc == 1) {	/* show list of commands */
		ഘടന cmd_tbl *cmd_array[cmd_ഇനങ്ങൾ];
		പൂർണ്ണസംഖ്യ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		വേണ്ടി (i = 0; i < cmd_ഇനങ്ങൾ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		വേണ്ടി (i = cmd_ഇനങ്ങൾ - 1; i > 0; --i) {
			swaps = 0;
			വേണ്ടി (j = 0; j < i; ++j) {
				എങ്കിൽ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ഘടന cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			എങ്കിൽ (!swaps)
				നിർത്തുക;
		}

		/* print short help (usage) */
		വേണ്ടി (i = 0; i < cmd_ഇനങ്ങൾ; i++) {
			സ്ഥിരം അക്ഷരം *usage = cmd_array[i]->usage;

			/* allow user abort */
			എങ്കിൽ (ctrlc())
				തിരികെ 1;
			എങ്കിൽ (usage == ശൂന്യ)
				തുടരുക;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		തിരികെ 0;
	}
	/*
	 * command help (long version)
	 */
	വേണ്ടി (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_ഇനങ്ങൾ);
		എങ്കിൽ (cmdtp != ശൂന്യ) {
			rcode |= cmd_usage(cmdtp);
		} അല്ലെങ്കിൽ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	തിരികെ rcode;
}

/* find command table entry for a command */
ഘടന cmd_tbl *find_cmd_tbl(സ്ഥിരം അക്ഷരം *cmd, ഘടന cmd_tbl *table,
			     പൂർണ്ണസംഖ്യ table_len)
{
#ifdef CONFIG_CMDLINE
	ഘടന cmd_tbl *cmdtp;
	ഘടന cmd_tbl *cmdtp_temp = table;	/* Init value */
	സ്ഥിരം അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ len;
	പൂർണ്ണസംഖ്യ n_found = 0;

	എങ്കിൽ (!cmd)
		തിരികെ ശൂന്യ;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ശൂന്യ) ? strlen (cmd) : (p - cmd);

	വേണ്ടി (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		എങ്കിൽ (strncmp(cmd, cmdtp->name, len) == 0) {
			എങ്കിൽ (len == strlen(cmdtp->name))
				തിരികെ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	എങ്കിൽ (n_found == 1) {			/* exactly one match */
		തിരികെ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	തിരികെ ശൂന്യ;	/* not found or ambiguous command */
}

ഘടന cmd_tbl *find_cmd(സ്ഥിരം അക്ഷരം *cmd)
{
	ഘടന cmd_tbl *start = ll_entry_start(ഘടന cmd_tbl, cmd);
	സ്ഥിരം പൂർണ്ണസംഖ്യ len = ll_entry_എണ്ണം(ഘടന cmd_tbl, cmd);
	തിരികെ find_cmd_tbl(cmd, start, len);
}

പൂർണ്ണസംഖ്യ cmd_usage(സ്ഥിരം ഘടന cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	എങ്കിൽ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		തിരികെ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	തിരികെ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
സ്ഥിതം അക്ഷരം env_complete_buf[512];

പൂർണ്ണസംഖ്യ var_complete(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[], അക്ഷരം last_char, പൂർണ്ണസംഖ്യ maxv,
		 അക്ഷരം *cmdv[])
{
	പൂർണ്ണസംഖ്യ space;

	space = last_char == '\0' || isblank(last_char);

	എങ്കിൽ (space && argc == 1)
		തിരികെ env_complete("", maxv, cmdv, വലുപ്പം(env_complete_buf),
				    env_complete_buf, false);

	എങ്കിൽ (!space && argc == 2)
		തിരികെ env_complete(argv[1], maxv, cmdv,
				    വലുപ്പം(env_complete_buf),
				    env_complete_buf, false);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ dollar_complete(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[], അക്ഷരം last_char,
			   പൂർണ്ണസംഖ്യ maxv, അക്ഷരം *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	എങ്കിൽ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		തിരികെ 0;

	തിരികെ env_complete(argv[argc - 1], maxv, cmdv, വലുപ്പം(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

പൂർണ്ണസംഖ്യ complete_subcmdv(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ എണ്ണം, പൂർണ്ണസംഖ്യ argc,
		     അക്ഷരം *സ്ഥിരം argv[], അക്ഷരം last_char,
		     പൂർണ്ണസംഖ്യ maxv, അക്ഷരം *cmdv[])
{
#ifdef CONFIG_CMDLINE
	സ്ഥിരം ഘടന cmd_tbl *cmdend = cmdtp + എണ്ണം;
	സ്ഥിരം അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ len, clen;
	പൂർണ്ണസംഖ്യ n_found = 0;
	സ്ഥിരം അക്ഷരം *cmd;

	/* sanity? */
	എങ്കിൽ (maxv < 2)
		തിരികെ -2;

	cmdv[0] = ശൂന്യ;

	എങ്കിൽ (argc == 0) {
		/* output full list of commands */
		വേണ്ടി (; cmdtp != cmdend; cmdtp++) {
			എങ്കിൽ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				നിർത്തുക;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = ശൂന്യ;
		തിരികെ n_found;
	}

	/* more than one arg or one but the start of the next */
	എങ്കിൽ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, എണ്ണം);
		എങ്കിൽ (cmdtp == ശൂന്യ || cmdtp->complete == ശൂന്യ) {
			cmdv[0] = ശൂന്യ;
			തിരികെ 0;
		}
		തിരികെ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	എങ്കിൽ (p == ശൂന്യ)
		len = strlen(cmd);
	അല്ലെങ്കിൽ
		len = p - cmd;

	/* return the partial matches */
	വേണ്ടി (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		എങ്കിൽ (clen < len)
			തുടരുക;

		എങ്കിൽ (memcmp(cmd, cmdtp->name, len) != 0)
			തുടരുക;

		/* too many! */
		എങ്കിൽ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			നിർത്തുക;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = ശൂന്യ;
	തിരികെ n_found;
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ complete_cmdv(പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[], അക്ഷരം last_char,
			 പൂർണ്ണസംഖ്യ maxv, അക്ഷരം *cmdv[])
{
#ifdef CONFIG_CMDLINE
	തിരികെ complete_subcmdv(ll_entry_start(ഘടന cmd_tbl, cmd),
				ll_entry_എണ്ണം(ഘടന cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ make_argv(അക്ഷരം *s, പൂർണ്ണസംഖ്യ argvsz, അക്ഷരം *argv[])
{
	പൂർണ്ണസംഖ്യ argc = 0;

	/* split into argv */
	ആകുമ്പോൾ (argc < argvsz - 1) {

		/* skip any white space */
		ആകുമ്പോൾ (isblank(*s))
			++s;

		എങ്കിൽ (*s == '\0')	/* end of s, no more args	*/
			നിർത്തുക;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ആകുമ്പോൾ (*s && !isblank(*s))
			++s;

		എങ്കിൽ (*s == '\0')		/* end of s, no more args	*/
			നിർത്തുക;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ശൂന്യ;

	തിരികെ argc;
}

സ്ഥിതം ശൂന്യം print_argv(സ്ഥിരം അക്ഷരം *banner, സ്ഥിരം അക്ഷരം *leader, സ്ഥിരം അക്ഷരം *sep,
		       പൂർണ്ണസംഖ്യ linemax, അക്ഷരം *സ്ഥിരം argv[])
{
	പൂർണ്ണസംഖ്യ ll = leader != ശൂന്യ ? strlen(leader) : 0;
	പൂർണ്ണസംഖ്യ sl = sep != ശൂന്യ ? strlen(sep) : 0;
	പൂർണ്ണസംഖ്യ len, i;

	എങ്കിൽ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ആകുമ്പോൾ (*argv != ശൂന്യ) {
		len = strlen(*argv) + sl;
		എങ്കിൽ (i + len >= linemax) {
			puts("\n");
			എങ്കിൽ (leader)
				puts(leader);
			i = ll - sl;
		} അല്ലെങ്കിൽ എങ്കിൽ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ find_common_prefix(അക്ഷരം *സ്ഥിരം argv[])
{
	പൂർണ്ണസംഖ്യ i, len;
	അക്ഷരം *anchor, *s, *t;

	എങ്കിൽ (*argv == ശൂന്യ)
		തിരികെ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ആകുമ്പോൾ ((t = *argv++) != ശൂന്യ) {
		s = anchor;
		വേണ്ടി (i = 0; i < len; i++, t++, s++) {
			എങ്കിൽ (*t != *s)
				നിർത്തുക;
		}
		len = s - anchor;
	}
	തിരികെ len;
}

പൂർണ്ണസംഖ്യ cmd_auto_complete(സ്ഥിരം അക്ഷരം *സ്ഥിരം prompt, അക്ഷരം *buf, പൂർണ്ണസംഖ്യ *np, പൂർണ്ണസംഖ്യ *colp)
{
	അക്ഷരം tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	പൂർണ്ണസംഖ്യ n = *np, col = *colp;
	അക്ഷരം *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	അക്ഷരം *cmdv[20];
	അക്ഷരം *s, *t;
	സ്ഥിരം അക്ഷരം *sep;
	പൂർണ്ണസംഖ്യ i, j, k, len, seplen, argc;
	പൂർണ്ണസംഖ്യ cnt;
	അക്ഷരം last_char;
	സ്ഥിരം അക്ഷരം *ps_prompt;

	എങ്കിൽ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		എങ്കിൽ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} അല്ലെങ്കിൽ
		ps_prompt = CONFIG_SYS_PROMPT;

	എങ്കിൽ (strcmp(prompt, ps_prompt) != 0)
		തിരികെ 0;	/* not in normal console */

	cnt = strlen(buf);
	എങ്കിൽ (cnt >= 1)
		last_char = buf[cnt - 1];
	അല്ലെങ്കിൽ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, വലുപ്പം(argv)/വലുപ്പം(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    വലുപ്പം(cmdv) / വലുപ്പം(cmdv[0]), cmdv);
	എങ്കിൽ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  വലുപ്പം(cmdv) / വലുപ്പം(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	എങ്കിൽ (i == 0) {
		എങ്കിൽ (argc > 1)	/* allow tab for non command */
			തിരികെ 0;
		putc('\a');
		തിരികെ 1;
	}

	s = ശൂന്യ;
	len = 0;
	sep = ശൂന്യ;
	seplen = 0;
	എങ്കിൽ (i == 1) { /* one match; perfect */
		എങ്കിൽ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		അല്ലെങ്കിൽ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} അല്ലെങ്കിൽ എങ്കിൽ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		എങ്കിൽ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		അല്ലെങ്കിൽ
			k = 0;

		j -= k;
		എങ്കിൽ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	എങ്കിൽ (s != ശൂന്യ) {
		k = len + seplen;
		/* make sure it fits */
		എങ്കിൽ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			തിരികെ 1;
		}

		t = buf + cnt;
		വേണ്ടി (i = 0; i < len; i++)
			*t++ = *s++;
		എങ്കിൽ (sep != ശൂന്യ)
			വേണ്ടി (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		എങ്കിൽ (sep == ശൂന്യ)
			putc('\a');
		*np = n;
		*colp = col;
	} അല്ലെങ്കിൽ {
		print_argv(ശൂന്യ, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	തിരികെ 1;
}

#endif

#ifdef CMD_DATA_SIZE
പൂർണ്ണസംഖ്യ cmd_get_data_size(സ്ഥിരം അക്ഷരം *arg, പൂർണ്ണസംഖ്യ default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	പൂർണ്ണസംഖ്യ len = strlen(arg);
	എങ്കിൽ (len >= 2 && arg[len-2] == '.') {
		തിരഞ്ഞെടുപ്പ് (arg[len-1]) {
		അവസ്ഥ 'b':
			തിരികെ 1;
		അവസ്ഥ 'w':
			തിരികെ 2;
		അവസ്ഥ 'l':
			തിരികെ 4;
		അവസ്ഥ 's':
			തിരികെ CMD_DATA_SIZE_STR;
		അവസ്ഥ 'q':
			എങ്കിൽ (MEM_SUPPORT_64BIT_DATA)
				തിരികെ 8;
			fallthrough;
		സ്ഥിരസ്ഥിതി:
			തിരികെ CMD_DATA_SIZE_ERR;
		}
	}
	തിരികെ default_size;
}
#endif

ശൂന്യം fixup_cmdtable(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ size)
{
	പൂർണ്ണസംഖ്യ	i;

	എങ്കിൽ (gd->reloc_off == 0)
		തിരികെ;

	വേണ്ടി (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(പൂർണ്ണസംഖ്യ (*)(ഘടന cmd_tbl *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ,
				 അക്ഷരം * സ്ഥിരം [], പൂർണ്ണസംഖ്യ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (പൂർണ്ണസംഖ്യ (*)(ഘടന cmd_tbl *, പൂർണ്ണസംഖ്യ, പൂർണ്ണസംഖ്യ,
				      അക്ഷരം *സ്ഥിരം []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (അക്ഷരം *)addr;
		എങ്കിൽ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (അക്ഷരം *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		എങ്കിൽ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (അക്ഷരം *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		എങ്കിൽ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(പൂർണ്ണസംഖ്യ (*)(പൂർണ്ണസംഖ്യ, അക്ഷരം * സ്ഥിരം [], അക്ഷരം, പൂർണ്ണസംഖ്യ, അക്ഷരം * []))addr;
		}
#endif
		cmdtp++;
	}
}

പൂർണ്ണസംഖ്യ cmd_always_repeatable(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc,
			  അക്ഷരം *സ്ഥിരം argv[], പൂർണ്ണസംഖ്യ *repeatable)
{
	*repeatable = 1;

	തിരികെ cmdtp->cmd(cmdtp, flag, argc, argv);
}

പൂർണ്ണസംഖ്യ cmd_never_repeatable(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc,
			 അക്ഷരം *സ്ഥിരം argv[], പൂർണ്ണസംഖ്യ *repeatable)
{
	*repeatable = 0;

	തിരികെ cmdtp->cmd(cmdtp, flag, argc, argv);
}

പൂർണ്ണസംഖ്യ cmd_discard_repeatable(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc,
			   അക്ഷരം *സ്ഥിരം argv[])
{
	പൂർണ്ണസംഖ്യ repeatable;

	തിരികെ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ cmd_call(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc,
		    അക്ഷരം *സ്ഥിരം argv[], പൂർണ്ണസംഖ്യ *repeatable)
{
	പൂർണ്ണസംഖ്യ result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	എങ്കിൽ (result)
		debug("Command failed, result=%d\n", result);
	തിരികെ result;
}

എണ്ണൽ command_ret_t cmd_process(പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[],
			       പൂർണ്ണസംഖ്യ *repeatable, ulong *ticks)
{
	എണ്ണൽ command_ret_t rc = CMD_RET_SUCCESS;
	ഘടന cmd_tbl *cmdtp;

#എങ്കിൽ defined(CONFIG_SYS_XTRACE)
	അക്ഷരം *xtrace;

	xtrace = env_get("xtrace");
	എങ്കിൽ (xtrace) {
		puts("+");
		വേണ്ടി (പൂർണ്ണസംഖ്യ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	എങ്കിൽ (cmdtp == ശൂന്യ) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		തിരികെ 1;
	}

	/* found - check max args */
	എങ്കിൽ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#എങ്കിൽ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	അല്ലെങ്കിൽ എങ്കിൽ (cmdtp->cmd == do_bootd) {
		എങ്കിൽ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} അല്ലെങ്കിൽ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	എങ്കിൽ (!rc) {
		പൂർണ്ണസംഖ്യ newrep;

		എങ്കിൽ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		എങ്കിൽ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	എങ്കിൽ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	തിരികെ rc;
}

പൂർണ്ണസംഖ്യ cmd_process_error(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ err)
{
	എങ്കിൽ (err == CMD_RET_USAGE)
		തിരികെ CMD_RET_USAGE;

	എങ്കിൽ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		തിരികെ CMD_RET_FAILURE;
	}

	തിരികെ CMD_RET_SUCCESS;
}

പൂർണ്ണസംഖ്യ cmd_source_script(ulong addr, സ്ഥിരം അക്ഷരം *fit_uname, സ്ഥിരം അക്ഷരം *confname)
{
	അക്ഷരം *data;
	ശൂന്യം *buf;
	uint len;
	പൂർണ്ണസംഖ്യ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	എങ്കിൽ (ret)
		തിരികെ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	തിരികെ run_command_list(data, len, 0);
}
