// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

ਪੂਰਨ _do_help(ਬਣਤਰ cmd_tbl *cmd_ਸ਼ੁਰੂ, ਪੂਰਨ cmd_ਚੀਜ਼ਾਂ, ਬਣਤਰ cmd_tbl *cmdtp,
	     ਪੂਰਨ flag, ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[])
{
	ਪੂਰਨ i;
	ਪੂਰਨ rcode = 0;

	ਜੇ (argc == 1) {	/* show list of commands */
		ਬਣਤਰ cmd_tbl *cmd_array[cmd_ਚੀਜ਼ਾਂ];
		ਪੂਰਨ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_ਸ਼ੁਰੂ;
		ਲਈ (i = 0; i < cmd_ਚੀਜ਼ਾਂ; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ਲਈ (i = cmd_ਚੀਜ਼ਾਂ - 1; i > 0; --i) {
			swaps = 0;
			ਲਈ (j = 0; j < i; ++j) {
				ਜੇ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ਬਣਤਰ cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ਜੇ (!swaps)
				ਰੋਕੋ;
		}

		/* print short help (usage) */
		ਲਈ (i = 0; i < cmd_ਚੀਜ਼ਾਂ; i++) {
			ਅਚਲ ਅੱਖਰ *usage = cmd_array[i]->usage;

			/* allow user abort */
			ਜੇ (ctrlc())
				ਵਾਪਸ 1;
			ਜੇ (usage == ਨਿਰਾ)
				ਜਾਰੀ;
			ਲਿਖੋ("%-*s- %s\n", CFG_SYS_HELP_CMD_ਚੌੜਾਈ,
			       cmd_array[i]->name, usage);
		}
		ਵਾਪਸ 0;
	}
	/*
	 * command help (long version)
	 */
	ਲਈ (i = 1; i < argc; ++i) {
		cmdtp = ਲੱਭੋ_cmd_tbl(argv[i], cmd_ਸ਼ੁਰੂ, cmd_ਚੀਜ਼ਾਂ);
		ਜੇ (cmdtp != ਨਿਰਾ) {
			rcode |= cmd_usage(cmdtp);
		} ਨਹੀਂਤਾਂ {
			ਲਿਖੋ("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ਵਾਪਸ rcode;
}

/* find command table entry for a command */
ਬਣਤਰ cmd_tbl *ਲੱਭੋ_cmd_tbl(ਅਚਲ ਅੱਖਰ *cmd, ਬਣਤਰ cmd_tbl *ਸਾਰਣੀ,
			     ਪੂਰਨ ਸਾਰਣੀ_len)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
	ਬਣਤਰ cmd_tbl *cmdtp;
	ਬਣਤਰ cmd_tbl *cmdtp_ਆਰਜ਼ੀ = ਸਾਰਣੀ;	/* Init value */
	ਅਚਲ ਅੱਖਰ *p;
	ਪੂਰਨ len;
	ਪੂਰਨ n_ਮਿਲਿਆ = 0;

	ਜੇ (!cmd)
		ਵਾਪਸ ਨਿਰਾ;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ਨਿਰਾ) ? ਲੜੀ_ਲੰਬਾਈ (cmd) : (p - cmd);

	ਲਈ (cmdtp = ਸਾਰਣੀ; cmdtp != ਸਾਰਣੀ + ਸਾਰਣੀ_len; cmdtp++) {
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(cmd, cmdtp->name, len) == 0) {
			ਜੇ (len == ਲੜੀ_ਲੰਬਾਈ(cmdtp->name))
				ਵਾਪਸ cmdtp;	/* full match */

			cmdtp_ਆਰਜ਼ੀ = cmdtp;	/* abbreviated command ? */
			n_ਮਿਲਿਆ++;
		}
	}
	ਜੇ (n_ਮਿਲਿਆ == 1) {			/* exactly one match */
		ਵਾਪਸ cmdtp_ਆਰਜ਼ੀ;
	}
#ਜੇ_ਅੰਤ /* CONFIG_CMDLINE */

	ਵਾਪਸ ਨਿਰਾ;	/* not found or ambiguous command */
}

ਬਣਤਰ cmd_tbl *ਲੱਭੋ_cmd(ਅਚਲ ਅੱਖਰ *cmd)
{
	ਬਣਤਰ cmd_tbl *ਸ਼ੁਰੂ = ll_ਇੰਦਰਾਜ_ਸ਼ੁਰੂ(ਬਣਤਰ cmd_tbl, cmd);
	ਅਚਲ ਪੂਰਨ len = ll_ਇੰਦਰਾਜ_ਗਿਣਤੀ(ਬਣਤਰ cmd_tbl, cmd);
	ਵਾਪਸ ਲੱਭੋ_cmd_tbl(cmd, ਸ਼ੁਰੂ, len);
}

ਪੂਰਨ cmd_usage(ਅਚਲ ਬਣਤਰ cmd_tbl *cmdtp)
{
	ਲਿਖੋ("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ	CONFIG_SYS_LONGHELP
	ਲਿਖੋ("Usage:\n%s ", cmdtp->name);

	ਜੇ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ਵਾਪਸ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#ਜੇ_ਅੰਤ	/* CONFIG_SYS_LONGHELP */
	ਵਾਪਸ 1;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AUTO_COMPLETE
ਸਥਿਰ ਅੱਖਰ env_complete_buf[512];

ਪੂਰਨ var_complete(ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[], ਅੱਖਰ ਆਖਰੀ_char, ਪੂਰਨ maxv,
		 ਅੱਖਰ *cmdv[])
{
	ਪੂਰਨ space;

	space = ਆਖਰੀ_char == '\0' || isblank(ਆਖਰੀ_char);

	ਜੇ (space && argc == 1)
		ਵਾਪਸ env_complete("", maxv, cmdv, ਆਕਾਰ(env_complete_buf),
				    env_complete_buf, false);

	ਜੇ (!space && argc == 2)
		ਵਾਪਸ env_complete(argv[1], maxv, cmdv,
				    ਆਕਾਰ(env_complete_buf),
				    env_complete_buf, false);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ dollar_complete(ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[], ਅੱਖਰ ਆਖਰੀ_char,
			   ਪੂਰਨ maxv, ਅੱਖਰ *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ਜੇ (argc < 1 || argv[argc - 1][0] != '$' ||
	    ਆਖਰੀ_char == '\0' || isblank(ਆਖਰੀ_char))
		ਵਾਪਸ 0;

	ਵਾਪਸ env_complete(argv[argc - 1], maxv, cmdv, ਆਕਾਰ(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ਪੂਰਨ complete_subcmdv(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ ਗਿਣਤੀ_2, ਪੂਰਨ argc,
		     ਅੱਖਰ *ਅਚਲ argv[], ਅੱਖਰ ਆਖਰੀ_char,
		     ਪੂਰਨ maxv, ਅੱਖਰ *cmdv[])
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
	ਅਚਲ ਬਣਤਰ cmd_tbl *cmdend = cmdtp + ਗਿਣਤੀ_2;
	ਅਚਲ ਅੱਖਰ *p;
	ਪੂਰਨ len, clen;
	ਪੂਰਨ n_ਮਿਲਿਆ = 0;
	ਅਚਲ ਅੱਖਰ *cmd;

	/* sanity? */
	ਜੇ (maxv < 2)
		ਵਾਪਸ -2;

	cmdv[0] = ਨਿਰਾ;

	ਜੇ (argc == 0) {
		/* output full list of commands */
		ਲਈ (; cmdtp != cmdend; cmdtp++) {
			ਜੇ (n_ਮਿਲਿਆ >= maxv - 2) {
				cmdv[n_ਮਿਲਿਆ++] = "...";
				ਰੋਕੋ;
			}
			cmdv[n_ਮਿਲਿਆ++] = cmdtp->name;
		}
		cmdv[n_ਮਿਲਿਆ] = ਨਿਰਾ;
		ਵਾਪਸ n_ਮਿਲਿਆ;
	}

	/* more than one arg or one but the start of the next */
	ਜੇ (argc > 1 || ਆਖਰੀ_char == '\0' || isblank(ਆਖਰੀ_char)) {
		cmdtp = ਲੱਭੋ_cmd_tbl(argv[0], cmdtp, ਗਿਣਤੀ_2);
		ਜੇ (cmdtp == ਨਿਰਾ || cmdtp->complete == ਨਿਰਾ) {
			cmdv[0] = ਨਿਰਾ;
			ਵਾਪਸ 0;
		}
		ਵਾਪਸ (*cmdtp->complete)(argc, argv, ਆਖਰੀ_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ਜੇ (p == ਨਿਰਾ)
		len = ਲੜੀ_ਲੰਬਾਈ(cmd);
	ਨਹੀਂਤਾਂ
		len = p - cmd;

	/* return the partial matches */
	ਲਈ (; cmdtp != cmdend; cmdtp++) {

		clen = ਲੜੀ_ਲੰਬਾਈ(cmdtp->name);
		ਜੇ (clen < len)
			ਜਾਰੀ;

		ਜੇ (memcmp(cmd, cmdtp->name, len) != 0)
			ਜਾਰੀ;

		/* too many! */
		ਜੇ (n_ਮਿਲਿਆ >= maxv - 2) {
			cmdv[n_ਮਿਲਿਆ++] = "...";
			ਰੋਕੋ;
		}

		cmdv[n_ਮਿਲਿਆ++] = cmdtp->name;
	}

	cmdv[n_ਮਿਲਿਆ] = ਨਿਰਾ;
	ਵਾਪਸ n_ਮਿਲਿਆ;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ complete_cmdv(ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[], ਅੱਖਰ ਆਖਰੀ_char,
			 ਪੂਰਨ maxv, ਅੱਖਰ *cmdv[])
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
	ਵਾਪਸ complete_subcmdv(ll_ਇੰਦਰਾਜ_ਸ਼ੁਰੂ(ਬਣਤਰ cmd_tbl, cmd),
				ll_ਇੰਦਰਾਜ_ਗਿਣਤੀ(ਬਣਤਰ cmd_tbl, cmd), argc, argv,
				ਆਖਰੀ_char, maxv, cmdv);
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ make_argv(ਅੱਖਰ *s, ਪੂਰਨ argvsz, ਅੱਖਰ *argv[])
{
	ਪੂਰਨ argc = 0;

	/* split into argv */
	ਜਦੋਂ (argc < argvsz - 1) {

		/* skip any white space */
		ਜਦੋਂ (isblank(*s))
			++s;

		ਜੇ (*s == '\0')	/* end of s, no more args	*/
			ਰੋਕੋ;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ਜਦੋਂ (*s && !isblank(*s))
			++s;

		ਜੇ (*s == '\0')		/* end of s, no more args	*/
			ਰੋਕੋ;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ਨਿਰਾ;

	ਵਾਪਸ argc;
}

ਸਥਿਰ ਖਾਲੀ print_argv(ਅਚਲ ਅੱਖਰ *banner, ਅਚਲ ਅੱਖਰ *leader, ਅਚਲ ਅੱਖਰ *sep,
		       ਪੂਰਨ linemax, ਅੱਖਰ *ਅਚਲ argv[])
{
	ਪੂਰਨ ll = leader != ਨਿਰਾ ? ਲੜੀ_ਲੰਬਾਈ(leader) : 0;
	ਪੂਰਨ sl = sep != ਨਿਰਾ ? ਲੜੀ_ਲੰਬਾਈ(sep) : 0;
	ਪੂਰਨ len, i;

	ਜੇ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ਜਦੋਂ (*argv != ਨਿਰਾ) {
		len = ਲੜੀ_ਲੰਬਾਈ(*argv) + sl;
		ਜੇ (i + len >= linemax) {
			puts("\n");
			ਜੇ (leader)
				puts(leader);
			i = ll - sl;
		} ਨਹੀਂਤਾਂ ਜੇ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	ਲਿਖੋ("\n");
}

ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_common_ਅਗੇਤਰ(ਅੱਖਰ *ਅਚਲ argv[])
{
	ਪੂਰਨ i, len;
	ਅੱਖਰ *anchor, *s, *t;

	ਜੇ (*argv == ਨਿਰਾ)
		ਵਾਪਸ 0;

	/* begin with max */
	anchor = *argv++;
	len = ਲੜੀ_ਲੰਬਾਈ(anchor);
	ਜਦੋਂ ((t = *argv++) != ਨਿਰਾ) {
		s = anchor;
		ਲਈ (i = 0; i < len; i++, t++, s++) {
			ਜੇ (*t != *s)
				ਰੋਕੋ;
		}
		len = s - anchor;
	}
	ਵਾਪਸ len;
}

ਪੂਰਨ cmd_auto_complete(ਅਚਲ ਅੱਖਰ *ਅਚਲ prompt, ਅੱਖਰ *buf, ਪੂਰਨ *np, ਪੂਰਨ *colp)
{
	ਅੱਖਰ tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ਪੂਰਨ n = *np, ਖਾਨਾ = *colp;
	ਅੱਖਰ *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	ਅੱਖਰ *cmdv[20];
	ਅੱਖਰ *s, *t;
	ਅਚਲ ਅੱਖਰ *sep;
	ਪੂਰਨ i, j, k, len, seplen, argc;
	ਪੂਰਨ cnt;
	ਅੱਖਰ ਆਖਰੀ_char;
	ਅਚਲ ਅੱਖਰ *ps_prompt;

	ਜੇ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ਜੇ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ਨਹੀਂਤਾਂ
		ps_prompt = CONFIG_SYS_PROMPT;

	ਜੇ (strcmp(prompt, ps_prompt) != 0)
		ਵਾਪਸ 0;	/* not in normal console */

	cnt = ਲੜੀ_ਲੰਬਾਈ(buf);
	ਜੇ (cnt >= 1)
		ਆਖਰੀ_char = buf[cnt - 1];
	ਨਹੀਂਤਾਂ
		ਆਖਰੀ_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, ਆਕਾਰ(argv)/ਆਕਾਰ(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, ਆਖਰੀ_char,
			    ਆਕਾਰ(cmdv) / ਆਕਾਰ(cmdv[0]), cmdv);
	ਜੇ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, ਆਖਰੀ_char,
				  ਆਕਾਰ(cmdv) / ਆਕਾਰ(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ਜੇ (i == 0) {
		ਜੇ (argc > 1)	/* allow tab for non command */
			ਵਾਪਸ 0;
		putc('\a');
		ਵਾਪਸ 1;
	}

	s = ਨਿਰਾ;
	len = 0;
	sep = ਨਿਰਾ;
	seplen = 0;
	ਜੇ (i == 1) { /* one match; perfect */
		ਜੇ (ਆਖਰੀ_char != '\0' && !isblank(ਆਖਰੀ_char))
			k = ਲੜੀ_ਲੰਬਾਈ(argv[argc - 1]);
		ਨਹੀਂਤਾਂ
			k = 0;

		s = cmdv[0] + k;
		len = ਲੜੀ_ਲੰਬਾਈ(s);
		sep = " ";
		seplen = 1;
	} ਨਹੀਂਤਾਂ ਜੇ (i > 1 && (j = ਲੱਭੋ_common_ਅਗੇਤਰ(cmdv)) != 0) { /*ਹੋਰ*/
		ਜੇ (ਆਖਰੀ_char != '\0' && !isblank(ਆਖਰੀ_char))
			k = ਲੜੀ_ਲੰਬਾਈ(argv[argc - 1]);
		ਨਹੀਂਤਾਂ
			k = 0;

		j -= k;
		ਜੇ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ਜੇ (s != ਨਿਰਾ) {
		k = len + seplen;
		/* make sure it fits */
		ਜੇ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ਵਾਪਸ 1;
		}

		t = buf + cnt;
		ਲਈ (i = 0; i < len; i++)
			*t++ = *s++;
		ਜੇ (sep != ਨਿਰਾ)
			ਲਈ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		ਖਾਨਾ += k;
		puts(t - k);
		ਜੇ (sep == ਨਿਰਾ)
			putc('\a');
		*np = n;
		*colp = ਖਾਨਾ;
	} ਨਹੀਂਤਾਂ {
		print_argv(ਨਿਰਾ, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ਵਾਪਸ 1;
}

#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CMD_ਅੰਕੜੇ_ਮਾਪ
ਪੂਰਨ cmd_get_ਅੰਕੜੇ_ਮਾਪ(ਅਚਲ ਅੱਖਰ *arg, ਪੂਰਨ default_ਮਾਪ)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ਪੂਰਨ len = ਲੜੀ_ਲੰਬਾਈ(arg);
	ਜੇ (len >= 2 && arg[len-2] == '.') {
		ਚੋਣ (arg[len-1]) {
		ਹਾਲਤ 'b':
			ਵਾਪਸ 1;
		ਹਾਲਤ 'w':
			ਵਾਪਸ 2;
		ਹਾਲਤ 'l':
			ਵਾਪਸ 4;
		ਹਾਲਤ 's':
			ਵਾਪਸ CMD_ਅੰਕੜੇ_ਮਾਪ_STR;
		ਹਾਲਤ 'q':
			ਜੇ (MEM_SUPPORT_64_ਦੋਆਧਾਰੀ_ਅੰਕ_ਅੰਕੜੇ)
				ਵਾਪਸ 8;
			fallthrough;
		ਮੂਲ:
			ਵਾਪਸ CMD_ਅੰਕੜੇ_ਮਾਪ_ERR;
		}
	}
	ਵਾਪਸ default_ਮਾਪ;
}
#ਜੇ_ਅੰਤ

ਖਾਲੀ fixup_cmdtable(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ ਮਾਪ)
{
	ਪੂਰਨ	i;

	ਜੇ (gd->reloc_off == 0)
		ਵਾਪਸ;

	ਲਈ (i = 0; i < ਮਾਪ; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ਪੂਰਨ (*)(ਬਣਤਰ cmd_tbl *, ਪੂਰਨ, ਪੂਰਨ,
				 ਅੱਖਰ * ਅਚਲ [], ਪੂਰਨ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG_COMMANDS
		ਲਿਖੋ("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#ਜੇ_ਅੰਤ
		cmdtp->cmd = (ਪੂਰਨ (*)(ਬਣਤਰ cmd_tbl *, ਪੂਰਨ, ਪੂਰਨ,
				      ਅੱਖਰ *ਅਚਲ []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (ਅੱਖਰ *)addr;
		ਜੇ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (ਅੱਖਰ *)addr;
		}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ	CONFIG_SYS_LONGHELP
		ਜੇ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (ਅੱਖਰ *)addr;
		}
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_AUTO_COMPLETE
		ਜੇ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ਪੂਰਨ (*)(ਪੂਰਨ, ਅੱਖਰ * ਅਚਲ [], ਅੱਖਰ, ਪੂਰਨ, ਅੱਖਰ * []))addr;
		}
#ਜੇ_ਅੰਤ
		cmdtp++;
	}
}

ਪੂਰਨ cmd_always_repeatable(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc,
			  ਅੱਖਰ *ਅਚਲ argv[], ਪੂਰਨ *repeatable)
{
	*repeatable = 1;

	ਵਾਪਸ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ਪੂਰਨ cmd_never_repeatable(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc,
			 ਅੱਖਰ *ਅਚਲ argv[], ਪੂਰਨ *repeatable)
{
	*repeatable = 0;

	ਵਾਪਸ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ਪੂਰਨ cmd_discard_repeatable(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc,
			   ਅੱਖਰ *ਅਚਲ argv[])
{
	ਪੂਰਨ repeatable;

	ਵਾਪਸ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ਸਥਿਰ ਪੂਰਨ cmd_call(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc,
		    ਅੱਖਰ *ਅਚਲ argv[], ਪੂਰਨ *repeatable)
{
	ਪੂਰਨ ਨਤੀਜਾ;

	ਨਤੀਜਾ = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ਜੇ (ਨਤੀਜਾ)
		debug("Command failed, result=%d\n", ਨਤੀਜਾ);
	ਵਾਪਸ ਨਤੀਜਾ;
}

ਗਿਣਤੀ command_ret_t cmd_process(ਪੂਰਨ flag, ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[],
			       ਪੂਰਨ *repeatable, ulong *ticks)
{
	ਗਿਣਤੀ command_ret_t rc = CMD_RET_SUCCESS;
	ਬਣਤਰ cmd_tbl *cmdtp;

#ਜੇ defined(CONFIG_SYS_XTRACE)
	ਅੱਖਰ *xtrace;

	xtrace = env_get("xtrace");
	ਜੇ (xtrace) {
		puts("+");
		ਲਈ (ਪੂਰਨ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#ਜੇ_ਅੰਤ

	/* Look up command in command table */
	cmdtp = ਲੱਭੋ_cmd(argv[0]);
	ਜੇ (cmdtp == ਨਿਰਾ) {
		ਲਿਖੋ("Unknown command '%s' - try 'help'\n", argv[0]);
		ਵਾਪਸ 1;
	}

	/* found - check max args */
	ਜੇ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ਜੇ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ਨਹੀਂਤਾਂ ਜੇ (cmdtp->cmd == do_bootd) {
		ਜੇ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ਨਹੀਂਤਾਂ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#ਜੇ_ਅੰਤ

	/* If OK so far, then do the command */
	ਜੇ (!rc) {
		ਪੂਰਨ newrep;

		ਜੇ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ਜੇ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ਜੇ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ਵਾਪਸ rc;
}

ਪੂਰਨ cmd_process_error(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ err)
{
	ਜੇ (err == CMD_RET_USAGE)
		ਵਾਪਸ CMD_RET_USAGE;

	ਜੇ (err) {
		ਲਿਖੋ("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ਵਾਪਸ CMD_RET_FAILURE;
	}

	ਵਾਪਸ CMD_RET_SUCCESS;
}

ਪੂਰਨ cmd_ਸਰੋਤ_script(ulong addr, ਅਚਲ ਅੱਖਰ *fit_uname, ਅਚਲ ਅੱਖਰ *confname)
{
	ਅੱਖਰ *ਅੰਕੜੇ;
	ਖਾਲੀ *buf;
	uint len;
	ਪੂਰਨ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &ਅੰਕੜੇ, &len);
	unmap_sysmem(buf);
	ਜੇ (ret)
		ਵਾਪਸ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ਵਾਪਸ ਲੜੀਵਾਰ_command_list(ਅੰਕੜੇ, len, 0);
}
