// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <compiler.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା _do_help(ସଂରଚନା cmd_tbl *cmd_start, ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_ବସ୍ତୁଗୁଡ଼ିକ, ସଂରଚନା cmd_tbl *cmdtp,
	     ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = 0;

	ଯଦି (argc == 1) {	/* show list of commands */
		ସଂରଚନା cmd_tbl *cmd_array[cmd_ବସ୍ତୁଗୁଡ଼ିକ];
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		ପାଇଁ (i = 0; i < cmd_ବସ୍ତୁଗୁଡ଼ିକ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ପାଇଁ (i = cmd_ବସ୍ତୁଗୁଡ଼ିକ - 1; i > 0; --i) {
			swaps = 0;
			ପାଇଁ (j = 0; j < i; ++j) {
				ଯଦି (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ସଂରଚନା cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ଯଦି (!swaps)
				ଭାଙ୍ଗ;
		}

		/* print short help (usage) */
		ପାଇଁ (i = 0; i < cmd_ବସ୍ତୁଗୁଡ଼ିକ; i++) {
			ଅଚଳ ଅକ୍ଷର *usage = cmd_array[i]->usage;

			/* allow user abort */
			ଯଦି (ctrlc())
				ଫେରନ୍ତୁ 1;
			ଯଦି (usage == ଖାଲି)
				ଜାରି;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ଫେରନ୍ତୁ 0;
	}
	/*
	 * command help (long version)
	 */
	ପାଇଁ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_ବସ୍ତୁଗୁଡ଼ିକ);
		ଯଦି (cmdtp != ଖାଲି) {
			rcode |= cmd_usage(cmdtp);
		} ଅନ୍ୟଥା {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ଫେରନ୍ତୁ rcode;
}

/* find command table entry for a command */
ସଂରଚନା cmd_tbl *find_cmd_tbl(ଅଚଳ ଅକ୍ଷର *cmd, ସଂରଚନା cmd_tbl *table,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା table_len)
{
#ifdef CONFIG_CMDLINE
	ସଂରଚନା cmd_tbl *cmdtp;
	ସଂରଚନା cmd_tbl *cmdtp_temp = table;	/* Init value */
	ଅଚଳ ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_found = 0;

	ଯଦି (!cmd)
		ଫେରନ୍ତୁ ଖାଲି;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ଖାଲି) ? strlen (cmd) : (p - cmd);

	ପାଇଁ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ଯଦି (strncmp(cmd, cmdtp->name, len) == 0) {
			ଯଦି (len == strlen(cmdtp->name))
				ଫେରନ୍ତୁ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ଯଦି (n_found == 1) {			/* exactly one match */
		ଫେରନ୍ତୁ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ଫେରନ୍ତୁ ଖାଲି;	/* not found or ambiguous command */
}

ସଂରଚନା cmd_tbl *find_cmd(ଅଚଳ ଅକ୍ଷର *cmd)
{
	ସଂରଚନା cmd_tbl *start = ll_entry_start(ସଂରଚନା cmd_tbl, cmd);
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = ll_entry_ଗଣନା(ସଂରଚନା cmd_tbl, cmd);
	ଫେରନ୍ତୁ find_cmd_tbl(cmd, start, len);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_usage(ଅଚଳ ସଂରଚନା cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ଯଦି (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ଫେରନ୍ତୁ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ଫେରନ୍ତୁ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
ସ୍ଥିର ଅକ୍ଷର env_complete_buf[512];

ପୂର୍ଣ୍ଣସଂଖ୍ୟା var_complete(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[], ଅକ୍ଷର last_char, ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxv,
		 ଅକ୍ଷର *cmdv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା space;

	space = last_char == '\0' || isblank(last_char);

	ଯଦି (space && argc == 1)
		ଫେରନ୍ତୁ env_complete("", maxv, cmdv, ଆକାର(env_complete_buf),
				    env_complete_buf, false);

	ଯଦି (!space && argc == 2)
		ଫେରନ୍ତୁ env_complete(argv[1], maxv, cmdv,
				    ଆକାର(env_complete_buf),
				    env_complete_buf, false);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dollar_complete(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[], ଅକ୍ଷର last_char,
			   ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxv, ଅକ୍ଷର *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ଯଦି (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ env_complete(argv[argc - 1], maxv, cmdv, ଆକାର(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା complete_subcmdv(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
		     ଅକ୍ଷର *ଅଚଳ argv[], ଅକ୍ଷର last_char,
		     ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxv, ଅକ୍ଷର *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ଅଚଳ ସଂରଚନା cmd_tbl *cmdend = cmdtp + ଗଣନା_2;
	ଅଚଳ ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, clen;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_found = 0;
	ଅଚଳ ଅକ୍ଷର *cmd;

	/* sanity? */
	ଯଦି (maxv < 2)
		ଫେରନ୍ତୁ -2;

	cmdv[0] = ଖାଲି;

	ଯଦି (argc == 0) {
		/* output full list of commands */
		ପାଇଁ (; cmdtp != cmdend; cmdtp++) {
			ଯଦି (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				ଭାଙ୍ଗ;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = ଖାଲି;
		ଫେରନ୍ତୁ n_found;
	}

	/* more than one arg or one but the start of the next */
	ଯଦି (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, ଗଣନା_2);
		ଯଦି (cmdtp == ଖାଲି || cmdtp->complete == ଖାଲି) {
			cmdv[0] = ଖାଲି;
			ଫେରନ୍ତୁ 0;
		}
		ଫେରନ୍ତୁ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ଯଦି (p == ଖାଲି)
		len = strlen(cmd);
	ଅନ୍ୟଥା
		len = p - cmd;

	/* return the partial matches */
	ପାଇଁ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ଯଦି (clen < len)
			ଜାରି;

		ଯଦି (memcmp(cmd, cmdtp->name, len) != 0)
			ଜାରି;

		/* too many! */
		ଯଦି (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			ଭାଙ୍ଗ;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = ଖାଲି;
	ଫେରନ୍ତୁ n_found;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା complete_cmdv(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[], ଅକ୍ଷର last_char,
			 ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxv, ଅକ୍ଷର *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ଫେରନ୍ତୁ complete_subcmdv(ll_entry_start(ସଂରଚନା cmd_tbl, cmd),
				ll_entry_ଗଣନା(ସଂରଚନା cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା make_argv(ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argvsz, ଅକ୍ଷର *argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc = 0;

	/* split into argv */
	ଯେତେବେଳେ (argc < argvsz - 1) {

		/* skip any white space */
		ଯେତେବେଳେ (isblank(*s))
			++s;

		ଯଦି (*s == '\0')	/* end of s, no more args	*/
			ଭାଙ୍ଗ;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ଯେତେବେଳେ (*s && !isblank(*s))
			++s;

		ଯଦି (*s == '\0')		/* end of s, no more args	*/
			ଭାଙ୍ଗ;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ଖାଲି;

	ଫେରନ୍ତୁ argc;
}

ସ୍ଥିର ଶୂନ୍ୟ print_argv(ଅଚଳ ଅକ୍ଷର *banner, ଅଚଳ ଅକ୍ଷର *leader, ଅଚଳ ଅକ୍ଷର *sep,
		       ପୂର୍ଣ୍ଣସଂଖ୍ୟା linemax, ଅକ୍ଷର *ଅଚଳ argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ll = leader != ଖାଲି ? strlen(leader) : 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sl = sep != ଖାଲି ? strlen(sep) : 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, i;

	ଯଦି (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ଯେତେବେଳେ (*argv != ଖାଲି) {
		len = strlen(*argv) + sl;
		ଯଦି (i + len >= linemax) {
			puts("\n");
			ଯଦି (leader)
				puts(leader);
			i = ll - sl;
		} ଅନ୍ୟଥା ଯଦି (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା find_common_prefix(ଅକ୍ଷର *ଅଚଳ argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, len;
	ଅକ୍ଷର *anchor, *s, *t;

	ଯଦି (*argv == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ଯେତେବେଳେ ((t = *argv++) != ଖାଲି) {
		s = anchor;
		ପାଇଁ (i = 0; i < len; i++, t++, s++) {
			ଯଦି (*t != *s)
				ଭାଙ୍ଗ;
		}
		len = s - anchor;
	}
	ଫେରନ୍ତୁ len;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_auto_complete(ଅଚଳ ଅକ୍ଷର *ଅଚଳ prompt, ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *np, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *colp)
{
	ଅକ୍ଷର tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = *np, col = *colp;
	ଅକ୍ଷର *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	ଅକ୍ଷର *cmdv[20];
	ଅକ୍ଷର *s, *t;
	ଅଚଳ ଅକ୍ଷର *sep;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j, k, len, seplen, argc;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt;
	ଅକ୍ଷର last_char;
	ଅଚଳ ଅକ୍ଷର *ps_prompt;

	ଯଦି (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ଯଦି (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ଅନ୍ୟଥା
		ps_prompt = CONFIG_SYS_PROMPT;

	ଯଦି (strcmp(prompt, ps_prompt) != 0)
		ଫେରନ୍ତୁ 0;	/* not in normal console */

	cnt = strlen(buf);
	ଯଦି (cnt >= 1)
		last_char = buf[cnt - 1];
	ଅନ୍ୟଥା
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, ଆକାର(argv)/ଆକାର(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    ଆକାର(cmdv) / ଆକାର(cmdv[0]), cmdv);
	ଯଦି (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  ଆକାର(cmdv) / ଆକାର(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ଯଦି (i == 0) {
		ଯଦି (argc > 1)	/* allow tab for non command */
			ଫେରନ୍ତୁ 0;
		putc('\a');
		ଫେରନ୍ତୁ 1;
	}

	s = ଖାଲି;
	len = 0;
	sep = ଖାଲି;
	seplen = 0;
	ଯଦି (i == 1) { /* one match; perfect */
		ଯଦି (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ଅନ୍ୟଥା
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ଅନ୍ୟଥା ଯଦି (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ଯଦି (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ଅନ୍ୟଥା
			k = 0;

		j -= k;
		ଯଦି (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ଯଦି (s != ଖାଲି) {
		k = len + seplen;
		/* make sure it fits */
		ଯଦି (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ଫେରନ୍ତୁ 1;
		}

		t = buf + cnt;
		ପାଇଁ (i = 0; i < len; i++)
			*t++ = *s++;
		ଯଦି (sep != ଖାଲି)
			ପାଇଁ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ଯଦି (sep == ଖାଲି)
			putc('\a');
		*np = n;
		*colp = col;
	} ଅନ୍ୟଥା {
		print_argv(ଖାଲି, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ଫେରନ୍ତୁ 1;
}

#endif

#ifdef CMD_DATA_SIZE
ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_get_data_size(ଅଚଳ ଅକ୍ଷର *arg, ପୂର୍ଣ୍ଣସଂଖ୍ୟା default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = strlen(arg);
	ଯଦି (len >= 2 && arg[len-2] == '.') {
		ବାଛ (arg[len-1]) {
		ସ୍ଥିତି 'b':
			ଫେରନ୍ତୁ 1;
		ସ୍ଥିତି 'w':
			ଫେରନ୍ତୁ 2;
		ସ୍ଥିତି 'l':
			ଫେରନ୍ତୁ 4;
		ସ୍ଥିତି 's':
			ଫେରନ୍ତୁ CMD_DATA_SIZE_STR;
		ସ୍ଥିତି 'q':
			ଯଦି (MEM_SUPPORT_64BIT_DATA)
				ଫେରନ୍ତୁ 8;
			fallthrough;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଫେରନ୍ତୁ CMD_DATA_SIZE_ERR;
		}
	}
	ଫେରନ୍ତୁ default_size;
}
#endif

ଶୂନ୍ୟ fixup_cmdtable(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା	i;

	ଯଦି (gd->reloc_off == 0)
		ଫେରନ୍ତୁ;

	ପାଇଁ (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*)(ସଂରଚନା cmd_tbl *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା,
				 ଅକ୍ଷର * ଅଚଳ [], ପୂର୍ଣ୍ଣସଂଖ୍ୟା *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*)(ସଂରଚନା cmd_tbl *, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା,
				      ଅକ୍ଷର *ଅଚଳ []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (ଅକ୍ଷର *)addr;
		ଯଦି (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (ଅକ୍ଷର *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ଯଦି (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (ଅକ୍ଷର *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ଯଦି (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର * ଅଚଳ [], ଅକ୍ଷର, ପୂର୍ଣ୍ଣସଂଖ୍ୟା, ଅକ୍ଷର * []))addr;
		}
#endif
		cmdtp++;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_always_repeatable(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
			  ଅକ୍ଷର *ଅଚଳ argv[], ପୂର୍ଣ୍ଣସଂଖ୍ୟା *repeatable)
{
	*repeatable = 1;

	ଫେରନ୍ତୁ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_never_repeatable(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
			 ଅକ୍ଷର *ଅଚଳ argv[], ପୂର୍ଣ୍ଣସଂଖ୍ୟା *repeatable)
{
	*repeatable = 0;

	ଫେରନ୍ତୁ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_discard_repeatable(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
			   ଅକ୍ଷର *ଅଚଳ argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା repeatable;

	ଫେରନ୍ତୁ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_call(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
		    ଅକ୍ଷର *ଅଚଳ argv[], ପୂର୍ଣ୍ଣସଂଖ୍ୟା *repeatable)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ଯଦି (result)
		debug("Command failed, result=%d\n", result);
	ଫେରନ୍ତୁ result;
}

ଗଣନା command_ret_t cmd_process(ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[],
			       ପୂର୍ଣ୍ଣସଂଖ୍ୟା *repeatable, ulong *ticks)
{
	ଗଣନା command_ret_t rc = CMD_RET_SUCCESS;
	ସଂରଚନା cmd_tbl *cmdtp;

#ଯଦି defined(CONFIG_SYS_XTRACE)
	ଅକ୍ଷର *xtrace;

	xtrace = env_get("xtrace");
	ଯଦି (xtrace) {
		puts("+");
		ପାଇଁ (ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ଯଦି (cmdtp == ଖାଲି) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ଫେରନ୍ତୁ 1;
	}

	/* found - check max args */
	ଯଦି (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ଯଦି defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ଅନ୍ୟଥା ଯଦି (cmdtp->cmd == do_bootd) {
		ଯଦି (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ଅନ୍ୟଥା {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ଯଦି (!rc) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା newrep;

		ଯଦି (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ଯଦି (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ଯଦି (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ଫେରନ୍ତୁ rc;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_process_error(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା err)
{
	ଯଦି (err == CMD_RET_USAGE)
		ଫେରନ୍ତୁ CMD_RET_USAGE;

	ଯଦି (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ଫେରନ୍ତୁ CMD_RET_FAILURE;
	}

	ଫେରନ୍ତୁ CMD_RET_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_source_script(ulong addr, ଅଚଳ ଅକ୍ଷର *fit_uname, ଅଚଳ ଅକ୍ଷର *confname)
{
	ଅକ୍ଷର *data;
	ଶୂନ୍ୟ *buf;
	uint len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ଯଦି (ret)
		ଫେରନ୍ତୁ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ଫେରନ୍ତୁ run_command_list(data, len, 0);
}
