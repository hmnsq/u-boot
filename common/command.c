// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <compiler.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

ಪೂರ್ಣಾಙ್ಕ _do_help(ಸಂರಚನಾ cmd_tbl *cmd_start, ಪೂರ್ಣಾಙ್ಕ cmd_ವಸ್ತುಗಳು, ಸಂರಚನಾ cmd_tbl *cmdtp,
	     ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[])
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ rcode = 0;

	ಯದಿ (argc == 1) {	/* show list of commands */
		ಸಂರಚನಾ cmd_tbl *cmd_array[cmd_ವಸ್ತುಗಳು];
		ಪೂರ್ಣಾಙ್ಕ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		ಪ್ರತಿ (i = 0; i < cmd_ವಸ್ತುಗಳು; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ಪ್ರತಿ (i = cmd_ವಸ್ತುಗಳು - 1; i > 0; --i) {
			swaps = 0;
			ಪ್ರತಿ (j = 0; j < i; ++j) {
				ಯದಿ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ಸಂರಚನಾ cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ಯದಿ (!swaps)
				ಭಙ್ಗ;
		}

		/* print short help (usage) */
		ಪ್ರತಿ (i = 0; i < cmd_ವಸ್ತುಗಳು; i++) {
			ಧ್ರುವ ವರ್ಣ *usage = cmd_array[i]->usage;

			/* allow user abort */
			ಯದಿ (ctrlc())
				ಪ್ರತ್ಯಾಗಮ 1;
			ಯದಿ (usage == ರಿಕ್ತ)
				ಅನುವರ್ತ;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	/*
	 * command help (long version)
	 */
	ಪ್ರತಿ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_ವಸ್ತುಗಳು);
		ಯದಿ (cmdtp != ರಿಕ್ತ) {
			rcode |= cmd_usage(cmdtp);
		} ಅನ್ಯಥಾ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ಪ್ರತ್ಯಾಗಮ rcode;
}

/* find command table entry for a command */
ಸಂರಚನಾ cmd_tbl *find_cmd_tbl(ಧ್ರುವ ವರ್ಣ *cmd, ಸಂರಚನಾ cmd_tbl *table,
			     ಪೂರ್ಣಾಙ್ಕ table_len)
{
#ifdef CONFIG_CMDLINE
	ಸಂರಚನಾ cmd_tbl *cmdtp;
	ಸಂರಚನಾ cmd_tbl *cmdtp_temp = table;	/* Init value */
	ಧ್ರುವ ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ len;
	ಪೂರ್ಣಾಙ್ಕ n_found = 0;

	ಯದಿ (!cmd)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ರಿಕ್ತ) ? strlen (cmd) : (p - cmd);

	ಪ್ರತಿ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ಯದಿ (strncmp(cmd, cmdtp->name, len) == 0) {
			ಯದಿ (len == strlen(cmdtp->name))
				ಪ್ರತ್ಯಾಗಮ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ಯದಿ (n_found == 1) {			/* exactly one match */
		ಪ್ರತ್ಯಾಗಮ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;	/* not found or ambiguous command */
}

ಸಂರಚನಾ cmd_tbl *find_cmd(ಧ್ರುವ ವರ್ಣ *cmd)
{
	ಸಂರಚನಾ cmd_tbl *start = ll_entry_start(ಸಂರಚನಾ cmd_tbl, cmd);
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ len = ll_entry_ಎಣಿಕೆ(ಸಂರಚನಾ cmd_tbl, cmd);
	ಪ್ರತ್ಯಾಗಮ find_cmd_tbl(cmd, start, len);
}

ಪೂರ್ಣಾಙ್ಕ cmd_usage(ಧ್ರುವ ಸಂರಚನಾ cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ಯದಿ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ಪ್ರತ್ಯಾಗಮ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
ಸ್ಥಿರ ವರ್ಣ env_complete_buf[512];

ಪೂರ್ಣಾಙ್ಕ var_complete(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[], ವರ್ಣ last_char, ಪೂರ್ಣಾಙ್ಕ maxv,
		 ವರ್ಣ *cmdv[])
{
	ಪೂರ್ಣಾಙ್ಕ space;

	space = last_char == '\0' || isblank(last_char);

	ಯದಿ (space && argc == 1)
		ಪ್ರತ್ಯಾಗಮ env_complete("", maxv, cmdv, ಪರಿಮಾಣ(env_complete_buf),
				    env_complete_buf, false);

	ಯದಿ (!space && argc == 2)
		ಪ್ರತ್ಯಾಗಮ env_complete(argv[1], maxv, cmdv,
				    ಪರಿಮಾಣ(env_complete_buf),
				    env_complete_buf, false);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ dollar_complete(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[], ವರ್ಣ last_char,
			   ಪೂರ್ಣಾಙ್ಕ maxv, ವರ್ಣ *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ಯದಿ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ env_complete(argv[argc - 1], maxv, cmdv, ಪರಿಮಾಣ(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ಪೂರ್ಣಾಙ್ಕ complete_subcmdv(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ, ಪೂರ್ಣಾಙ್ಕ argc,
		     ವರ್ಣ *ಧ್ರುವ argv[], ವರ್ಣ last_char,
		     ಪೂರ್ಣಾಙ್ಕ maxv, ವರ್ಣ *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ಧ್ರುವ ಸಂರಚನಾ cmd_tbl *cmdend = cmdtp + ಎಣಿಕೆ;
	ಧ್ರುವ ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ len, clen;
	ಪೂರ್ಣಾಙ್ಕ n_found = 0;
	ಧ್ರುವ ವರ್ಣ *cmd;

	/* sanity? */
	ಯದಿ (maxv < 2)
		ಪ್ರತ್ಯಾಗಮ -2;

	cmdv[0] = ರಿಕ್ತ;

	ಯದಿ (argc == 0) {
		/* output full list of commands */
		ಪ್ರತಿ (; cmdtp != cmdend; cmdtp++) {
			ಯದಿ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				ಭಙ್ಗ;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = ರಿಕ್ತ;
		ಪ್ರತ್ಯಾಗಮ n_found;
	}

	/* more than one arg or one but the start of the next */
	ಯದಿ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, ಎಣಿಕೆ);
		ಯದಿ (cmdtp == ರಿಕ್ತ || cmdtp->complete == ರಿಕ್ತ) {
			cmdv[0] = ರಿಕ್ತ;
			ಪ್ರತ್ಯಾಗಮ 0;
		}
		ಪ್ರತ್ಯಾಗಮ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ಯದಿ (p == ರಿಕ್ತ)
		len = strlen(cmd);
	ಅನ್ಯಥಾ
		len = p - cmd;

	/* return the partial matches */
	ಪ್ರತಿ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ಯದಿ (clen < len)
			ಅನುವರ್ತ;

		ಯದಿ (memcmp(cmd, cmdtp->name, len) != 0)
			ಅನುವರ್ತ;

		/* too many! */
		ಯದಿ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			ಭಙ್ಗ;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ n_found;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ complete_cmdv(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[], ವರ್ಣ last_char,
			 ಪೂರ್ಣಾಙ್ಕ maxv, ವರ್ಣ *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ಪ್ರತ್ಯಾಗಮ complete_subcmdv(ll_entry_start(ಸಂರಚನಾ cmd_tbl, cmd),
				ll_entry_ಎಣಿಕೆ(ಸಂರಚನಾ cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ make_argv(ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ argvsz, ವರ್ಣ *argv[])
{
	ಪೂರ್ಣಾಙ್ಕ argc = 0;

	/* split into argv */
	ಯಾವತ್ (argc < argvsz - 1) {

		/* skip any white space */
		ಯಾವತ್ (isblank(*s))
			++s;

		ಯದಿ (*s == '\0')	/* end of s, no more args	*/
			ಭಙ್ಗ;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ಯಾವತ್ (*s && !isblank(*s))
			++s;

		ಯದಿ (*s == '\0')		/* end of s, no more args	*/
			ಭಙ್ಗ;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ರಿಕ್ತ;

	ಪ್ರತ್ಯಾಗಮ argc;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_argv(ಧ್ರುವ ವರ್ಣ *banner, ಧ್ರುವ ವರ್ಣ *leader, ಧ್ರುವ ವರ್ಣ *sep,
		       ಪೂರ್ಣಾಙ್ಕ linemax, ವರ್ಣ *ಧ್ರುವ argv[])
{
	ಪೂರ್ಣಾಙ್ಕ ll = leader != ರಿಕ್ತ ? strlen(leader) : 0;
	ಪೂರ್ಣಾಙ್ಕ sl = sep != ರಿಕ್ತ ? strlen(sep) : 0;
	ಪೂರ್ಣಾಙ್ಕ len, i;

	ಯದಿ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ಯಾವತ್ (*argv != ರಿಕ್ತ) {
		len = strlen(*argv) + sl;
		ಯದಿ (i + len >= linemax) {
			puts("\n");
			ಯದಿ (leader)
				puts(leader);
			i = ll - sl;
		} ಅನ್ಯಥಾ ಯದಿ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ find_common_prefix(ವರ್ಣ *ಧ್ರುವ argv[])
{
	ಪೂರ್ಣಾಙ್ಕ i, len;
	ವರ್ಣ *anchor, *s, *t;

	ಯದಿ (*argv == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ಯಾವತ್ ((t = *argv++) != ರಿಕ್ತ) {
		s = anchor;
		ಪ್ರತಿ (i = 0; i < len; i++, t++, s++) {
			ಯದಿ (*t != *s)
				ಭಙ್ಗ;
		}
		len = s - anchor;
	}
	ಪ್ರತ್ಯಾಗಮ len;
}

ಪೂರ್ಣಾಙ್ಕ cmd_auto_complete(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ prompt, ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ *np, ಪೂರ್ಣಾಙ್ಕ *colp)
{
	ವರ್ಣ tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ಪೂರ್ಣಾಙ್ಕ n = *np, col = *colp;
	ವರ್ಣ *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	ವರ್ಣ *cmdv[20];
	ವರ್ಣ *s, *t;
	ಧ್ರುವ ವರ್ಣ *sep;
	ಪೂರ್ಣಾಙ್ಕ i, j, k, len, seplen, argc;
	ಪೂರ್ಣಾಙ್ಕ cnt;
	ವರ್ಣ last_char;
	ಧ್ರುವ ವರ್ಣ *ps_prompt;

	ಯದಿ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ಯದಿ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ಅನ್ಯಥಾ
		ps_prompt = CONFIG_SYS_PROMPT;

	ಯದಿ (strcmp(prompt, ps_prompt) != 0)
		ಪ್ರತ್ಯಾಗಮ 0;	/* not in normal console */

	cnt = strlen(buf);
	ಯದಿ (cnt >= 1)
		last_char = buf[cnt - 1];
	ಅನ್ಯಥಾ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, ಪರಿಮಾಣ(argv)/ಪರಿಮಾಣ(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    ಪರಿಮಾಣ(cmdv) / ಪರಿಮಾಣ(cmdv[0]), cmdv);
	ಯದಿ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  ಪರಿಮಾಣ(cmdv) / ಪರಿಮಾಣ(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ಯದಿ (i == 0) {
		ಯದಿ (argc > 1)	/* allow tab for non command */
			ಪ್ರತ್ಯಾಗಮ 0;
		putc('\a');
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	s = ರಿಕ್ತ;
	len = 0;
	sep = ರಿಕ್ತ;
	seplen = 0;
	ಯದಿ (i == 1) { /* one match; perfect */
		ಯದಿ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ಅನ್ಯಥಾ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ಅನ್ಯಥಾ ಯದಿ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ಯದಿ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ಅನ್ಯಥಾ
			k = 0;

		j -= k;
		ಯದಿ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ಯದಿ (s != ರಿಕ್ತ) {
		k = len + seplen;
		/* make sure it fits */
		ಯದಿ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ಪ್ರತ್ಯಾಗಮ 1;
		}

		t = buf + cnt;
		ಪ್ರತಿ (i = 0; i < len; i++)
			*t++ = *s++;
		ಯದಿ (sep != ರಿಕ್ತ)
			ಪ್ರತಿ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ಯದಿ (sep == ರಿಕ್ತ)
			putc('\a');
		*np = n;
		*colp = col;
	} ಅನ್ಯಥಾ {
		print_argv(ರಿಕ್ತ, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ಪ್ರತ್ಯಾಗಮ 1;
}

#endif

#ifdef CMD_DATA_SIZE
ಪೂರ್ಣಾಙ್ಕ cmd_get_data_size(ಧ್ರುವ ವರ್ಣ *arg, ಪೂರ್ಣಾಙ್ಕ default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ಪೂರ್ಣಾಙ್ಕ len = strlen(arg);
	ಯದಿ (len >= 2 && arg[len-2] == '.') {
		ವಿಕಲ್ಪ (arg[len-1]) {
		ಪಕ್ಷ 'b':
			ಪ್ರತ್ಯಾಗಮ 1;
		ಪಕ್ಷ 'w':
			ಪ್ರತ್ಯಾಗಮ 2;
		ಪಕ್ಷ 'l':
			ಪ್ರತ್ಯಾಗಮ 4;
		ಪಕ್ಷ 's':
			ಪ್ರತ್ಯಾಗಮ CMD_DATA_SIZE_STR;
		ಪಕ್ಷ 'q':
			ಯದಿ (MEM_SUPPORT_64BIT_DATA)
				ಪ್ರತ್ಯಾಗಮ 8;
			fallthrough;
		ಸ್ವಭಾವ:
			ಪ್ರತ್ಯಾಗಮ CMD_DATA_SIZE_ERR;
		}
	}
	ಪ್ರತ್ಯಾಗಮ default_size;
}
#endif

ಶೂನ್ಯ fixup_cmdtable(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ size)
{
	ಪೂರ್ಣಾಙ್ಕ	i;

	ಯದಿ (gd->reloc_off == 0)
		ಪ್ರತ್ಯಾಗಮ;

	ಪ್ರತಿ (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ಪೂರ್ಣಾಙ್ಕ (*)(ಸಂರಚನಾ cmd_tbl *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ,
				 ವರ್ಣ * ಧ್ರುವ [], ಪೂರ್ಣಾಙ್ಕ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ಪೂರ್ಣಾಙ್ಕ (*)(ಸಂರಚನಾ cmd_tbl *, ಪೂರ್ಣಾಙ್ಕ, ಪೂರ್ಣಾಙ್ಕ,
				      ವರ್ಣ *ಧ್ರುವ []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (ವರ್ಣ *)addr;
		ಯದಿ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (ವರ್ಣ *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ಯದಿ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (ವರ್ಣ *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ಯದಿ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ಪೂರ್ಣಾಙ್ಕ (*)(ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ * ಧ್ರುವ [], ವರ್ಣ, ಪೂರ್ಣಾಙ್ಕ, ವರ್ಣ * []))addr;
		}
#endif
		cmdtp++;
	}
}

ಪೂರ್ಣಾಙ್ಕ cmd_always_repeatable(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc,
			  ವರ್ಣ *ಧ್ರುವ argv[], ಪೂರ್ಣಾಙ್ಕ *repeatable)
{
	*repeatable = 1;

	ಪ್ರತ್ಯಾಗಮ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ಪೂರ್ಣಾಙ್ಕ cmd_never_repeatable(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc,
			 ವರ್ಣ *ಧ್ರುವ argv[], ಪೂರ್ಣಾಙ್ಕ *repeatable)
{
	*repeatable = 0;

	ಪ್ರತ್ಯಾಗಮ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ಪೂರ್ಣಾಙ್ಕ cmd_discard_repeatable(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc,
			   ವರ್ಣ *ಧ್ರುವ argv[])
{
	ಪೂರ್ಣಾಙ್ಕ repeatable;

	ಪ್ರತ್ಯಾಗಮ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cmd_call(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc,
		    ವರ್ಣ *ಧ್ರುವ argv[], ಪೂರ್ಣಾಙ್ಕ *repeatable)
{
	ಪೂರ್ಣಾಙ್ಕ result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ಯದಿ (result)
		debug("Command failed, result=%d\n", result);
	ಪ್ರತ್ಯಾಗಮ result;
}

ಗಣನಾ command_ret_t cmd_process(ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[],
			       ಪೂರ್ಣಾಙ್ಕ *repeatable, ulong *ticks)
{
	ಗಣನಾ command_ret_t rc = CMD_RET_SUCCESS;
	ಸಂರಚನಾ cmd_tbl *cmdtp;

#ಯದಿ defined(CONFIG_SYS_XTRACE)
	ವರ್ಣ *xtrace;

	xtrace = env_get("xtrace");
	ಯದಿ (xtrace) {
		puts("+");
		ಪ್ರತಿ (ಪೂರ್ಣಾಙ್ಕ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ಯದಿ (cmdtp == ರಿಕ್ತ) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ಪ್ರತ್ಯಾಗಮ 1;
	}

	/* found - check max args */
	ಯದಿ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ಯದಿ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ಅನ್ಯಥಾ ಯದಿ (cmdtp->cmd == do_bootd) {
		ಯದಿ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ಅನ್ಯಥಾ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ಯದಿ (!rc) {
		ಪೂರ್ಣಾಙ್ಕ newrep;

		ಯದಿ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ಯದಿ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ಯದಿ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ಪ್ರತ್ಯಾಗಮ rc;
}

ಪೂರ್ಣಾಙ್ಕ cmd_process_error(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ err)
{
	ಯದಿ (err == CMD_RET_USAGE)
		ಪ್ರತ್ಯಾಗಮ CMD_RET_USAGE;

	ಯದಿ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ಪ್ರತ್ಯಾಗಮ CMD_RET_FAILURE;
	}

	ಪ್ರತ್ಯಾಗಮ CMD_RET_SUCCESS;
}

ಪೂರ್ಣಾಙ್ಕ cmd_source_script(ulong addr, ಧ್ರುವ ವರ್ಣ *fit_uname, ಧ್ರುವ ವರ್ಣ *confname)
{
	ವರ್ಣ *data;
	ಶೂನ್ಯ *buf;
	uint len;
	ಪೂರ್ಣಾಙ್ಕ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ಪ್ರತ್ಯಾಗಮ run_command_list(data, len, 0);
}
