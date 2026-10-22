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

DECLARE_GLOBAL_ڈیٹا_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

صحیح _do_help(ساخت cmd_tbl *cmd_آغاز, صحیح cmd_اشیاء, ساخت cmd_tbl *cmdtp,
	     صحیح flag, صحیح argc, حرف *ثابت argv[])
{
	صحیح i;
	صحیح rcode = 0;

	اگر (argc == 1) {	/* show list of commands */
		ساخت cmd_tbl *cmd_array[cmd_اشیاء];
		صحیح i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_آغاز;
		برائے (i = 0; i < cmd_اشیاء; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		برائے (i = cmd_اشیاء - 1; i > 0; --i) {
			swaps = 0;
			برائے (j = 0; j < i; ++j) {
				اگر (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ساخت cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			اگر (!swaps)
				توڑو;
		}

		/* print short help (usage) */
		برائے (i = 0; i < cmd_اشیاء; i++) {
			ثابت حرف *usage = cmd_array[i]->usage;

			/* allow user abort */
			اگر (ctrlc())
				واپس 1;
			اگر (usage == تہی)
				جاری;
			لکھو("%-*s- %s\n", CFG_SYS_HELP_CMD_چوڑائی,
			       cmd_array[i]->name, usage);
		}
		واپس 0;
	}
	/*
	 * command help (long version)
	 */
	برائے (i = 1; i < argc; ++i) {
		cmdtp = ڈھونڈو_cmd_tbl(argv[i], cmd_آغاز, cmd_اشیاء);
		اگر (cmdtp != تہی) {
			rcode |= cmd_usage(cmdtp);
		} ورنہ {
			لکھو("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	واپس rcode;
}

/* find command table entry for a command */
ساخت cmd_tbl *ڈھونڈو_cmd_tbl(ثابت حرف *cmd, ساخت cmd_tbl *جدول,
			     صحیح جدول_len)
{
#اگر_تعریف_شدہ CONFIG_CMDLINE
	ساخت cmd_tbl *cmdtp;
	ساخت cmd_tbl *cmdtp_عارضی = جدول;	/* Init value */
	ثابت حرف *p;
	صحیح len;
	صحیح n_ملا = 0;

	اگر (!cmd)
		واپس تہی;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == تہی) ? لڑی_لمبائی (cmd) : (p - cmd);

	برائے (cmdtp = جدول; cmdtp != جدول + جدول_len; cmdtp++) {
		اگر (لڑی_موازنہ(cmd, cmdtp->name, len) == 0) {
			اگر (len == لڑی_لمبائی(cmdtp->name))
				واپس cmdtp;	/* full match */

			cmdtp_عارضی = cmdtp;	/* abbreviated command ? */
			n_ملا++;
		}
	}
	اگر (n_ملا == 1) {			/* exactly one match */
		واپس cmdtp_عارضی;
	}
#اگر_ختم /* CONFIG_CMDLINE */

	واپس تہی;	/* not found or ambiguous command */
}

ساخت cmd_tbl *ڈھونڈو_cmd(ثابت حرف *cmd)
{
	ساخت cmd_tbl *آغاز = ll_اندراج_آغاز(ساخت cmd_tbl, cmd);
	ثابت صحیح len = ll_اندراج_گنتی(ساخت cmd_tbl, cmd);
	واپس ڈھونڈو_cmd_tbl(cmd, آغاز, len);
}

صحیح cmd_usage(ثابت ساخت cmd_tbl *cmdtp)
{
	لکھو("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#اگر_تعریف_شدہ	CONFIG_SYS_LONGHELP
	لکھو("Usage:\n%s ", cmdtp->name);

	اگر (!cmdtp->help) {
		puts ("- No additional help available.\n");
		واپس 1;
	}

	puts(cmdtp->help);
	putc('\n');
#اگر_ختم	/* CONFIG_SYS_LONGHELP */
	واپس 1;
}

#اگر_تعریف_شدہ CONFIG_AUTO_COMPLETE
ساکن حرف env_complete_buf[512];

صحیح var_complete(صحیح argc, حرف *ثابت argv[], حرف آخری_char, صحیح maxv,
		 حرف *cmdv[])
{
	صحیح space;

	space = آخری_char == '\0' || isblank(آخری_char);

	اگر (space && argc == 1)
		واپس env_complete("", maxv, cmdv, حجم(env_complete_buf),
				    env_complete_buf, false);

	اگر (!space && argc == 2)
		واپس env_complete(argv[1], maxv, cmdv,
				    حجم(env_complete_buf),
				    env_complete_buf, false);

	واپس 0;
}

ساکن صحیح dollar_complete(صحیح argc, حرف *ثابت argv[], حرف آخری_char,
			   صحیح maxv, حرف *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	اگر (argc < 1 || argv[argc - 1][0] != '$' ||
	    آخری_char == '\0' || isblank(آخری_char))
		واپس 0;

	واپس env_complete(argv[argc - 1], maxv, cmdv, حجم(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

صحیح complete_subcmdv(ساخت cmd_tbl *cmdtp, صحیح گنتی, صحیح argc,
		     حرف *ثابت argv[], حرف آخری_char,
		     صحیح maxv, حرف *cmdv[])
{
#اگر_تعریف_شدہ CONFIG_CMDLINE
	ثابت ساخت cmd_tbl *cmdend = cmdtp + گنتی;
	ثابت حرف *p;
	صحیح len, clen;
	صحیح n_ملا = 0;
	ثابت حرف *cmd;

	/* sanity? */
	اگر (maxv < 2)
		واپس -2;

	cmdv[0] = تہی;

	اگر (argc == 0) {
		/* output full list of commands */
		برائے (; cmdtp != cmdend; cmdtp++) {
			اگر (n_ملا >= maxv - 2) {
				cmdv[n_ملا++] = "...";
				توڑو;
			}
			cmdv[n_ملا++] = cmdtp->name;
		}
		cmdv[n_ملا] = تہی;
		واپس n_ملا;
	}

	/* more than one arg or one but the start of the next */
	اگر (argc > 1 || آخری_char == '\0' || isblank(آخری_char)) {
		cmdtp = ڈھونڈو_cmd_tbl(argv[0], cmdtp, گنتی);
		اگر (cmdtp == تہی || cmdtp->complete == تہی) {
			cmdv[0] = تہی;
			واپس 0;
		}
		واپس (*cmdtp->complete)(argc, argv, آخری_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	اگر (p == تہی)
		len = لڑی_لمبائی(cmd);
	ورنہ
		len = p - cmd;

	/* return the partial matches */
	برائے (; cmdtp != cmdend; cmdtp++) {

		clen = لڑی_لمبائی(cmdtp->name);
		اگر (clen < len)
			جاری;

		اگر (memcmp(cmd, cmdtp->name, len) != 0)
			جاری;

		/* too many! */
		اگر (n_ملا >= maxv - 2) {
			cmdv[n_ملا++] = "...";
			توڑو;
		}

		cmdv[n_ملا++] = cmdtp->name;
	}

	cmdv[n_ملا] = تہی;
	واپس n_ملا;
#ورنہ
	واپس 0;
#اگر_ختم
}

ساکن صحیح complete_cmdv(صحیح argc, حرف *ثابت argv[], حرف آخری_char,
			 صحیح maxv, حرف *cmdv[])
{
#اگر_تعریف_شدہ CONFIG_CMDLINE
	واپس complete_subcmdv(ll_اندراج_آغاز(ساخت cmd_tbl, cmd),
				ll_اندراج_گنتی(ساخت cmd_tbl, cmd), argc, argv,
				آخری_char, maxv, cmdv);
#ورنہ
	واپس 0;
#اگر_ختم
}

ساکن صحیح make_argv(حرف *s, صحیح argvsz, حرف *argv[])
{
	صحیح argc = 0;

	/* split into argv */
	جب (argc < argvsz - 1) {

		/* skip any white space */
		جب (isblank(*s))
			++s;

		اگر (*s == '\0')	/* end of s, no more args	*/
			توڑو;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		جب (*s && !isblank(*s))
			++s;

		اگر (*s == '\0')		/* end of s, no more args	*/
			توڑو;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = تہی;

	واپس argc;
}

ساکن خالی print_argv(ثابت حرف *banner, ثابت حرف *leader, ثابت حرف *sep,
		       صحیح linemax, حرف *ثابت argv[])
{
	صحیح ll = leader != تہی ? لڑی_لمبائی(leader) : 0;
	صحیح sl = sep != تہی ? لڑی_لمبائی(sep) : 0;
	صحیح len, i;

	اگر (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	جب (*argv != تہی) {
		len = لڑی_لمبائی(*argv) + sl;
		اگر (i + len >= linemax) {
			puts("\n");
			اگر (leader)
				puts(leader);
			i = ll - sl;
		} ورنہ اگر (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	لکھو("\n");
}

ساکن صحیح ڈھونڈو_common_سابقہ(حرف *ثابت argv[])
{
	صحیح i, len;
	حرف *anchor, *s, *t;

	اگر (*argv == تہی)
		واپس 0;

	/* begin with max */
	anchor = *argv++;
	len = لڑی_لمبائی(anchor);
	جب ((t = *argv++) != تہی) {
		s = anchor;
		برائے (i = 0; i < len; i++, t++, s++) {
			اگر (*t != *s)
				توڑو;
		}
		len = s - anchor;
	}
	واپس len;
}

صحیح cmd_auto_complete(ثابت حرف *ثابت prompt, حرف *buf, صحیح *np, صحیح *colp)
{
	حرف tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	صحیح n = *np, ستون = *colp;
	حرف *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	حرف *cmdv[20];
	حرف *s, *t;
	ثابت حرف *sep;
	صحیح i, j, k, len, seplen, argc;
	صحیح cnt;
	حرف آخری_char;
	ثابت حرف *ps_prompt;

	اگر (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		اگر (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ورنہ
		ps_prompt = CONFIG_SYS_PROMPT;

	اگر (strcmp(prompt, ps_prompt) != 0)
		واپس 0;	/* not in normal console */

	cnt = لڑی_لمبائی(buf);
	اگر (cnt >= 1)
		آخری_char = buf[cnt - 1];
	ورنہ
		آخری_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, حجم(argv)/حجم(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, آخری_char,
			    حجم(cmdv) / حجم(cmdv[0]), cmdv);
	اگر (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, آخری_char,
				  حجم(cmdv) / حجم(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	اگر (i == 0) {
		اگر (argc > 1)	/* allow tab for non command */
			واپس 0;
		putc('\a');
		واپس 1;
	}

	s = تہی;
	len = 0;
	sep = تہی;
	seplen = 0;
	اگر (i == 1) { /* one match; perfect */
		اگر (آخری_char != '\0' && !isblank(آخری_char))
			k = لڑی_لمبائی(argv[argc - 1]);
		ورنہ
			k = 0;

		s = cmdv[0] + k;
		len = لڑی_لمبائی(s);
		sep = " ";
		seplen = 1;
	} ورنہ اگر (i > 1 && (j = ڈھونڈو_common_سابقہ(cmdv)) != 0) { /*مزید*/
		اگر (آخری_char != '\0' && !isblank(آخری_char))
			k = لڑی_لمبائی(argv[argc - 1]);
		ورنہ
			k = 0;

		j -= k;
		اگر (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	اگر (s != تہی) {
		k = len + seplen;
		/* make sure it fits */
		اگر (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			واپس 1;
		}

		t = buf + cnt;
		برائے (i = 0; i < len; i++)
			*t++ = *s++;
		اگر (sep != تہی)
			برائے (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		ستون += k;
		puts(t - k);
		اگر (sep == تہی)
			putc('\a');
		*np = n;
		*colp = ستون;
	} ورنہ {
		print_argv(تہی, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	واپس 1;
}

#اگر_ختم

#اگر_تعریف_شدہ CMD_ڈیٹا_سائز
صحیح cmd_get_ڈیٹا_سائز(ثابت حرف *arg, صحیح default_سائز)
{
	/* Check for a size specification .b, .w or .l.
	 */
	صحیح len = لڑی_لمبائی(arg);
	اگر (len >= 2 && arg[len-2] == '.') {
		انتخاب (arg[len-1]) {
		حالت 'b':
			واپس 1;
		حالت 'w':
			واپس 2;
		حالت 'l':
			واپس 4;
		حالت 's':
			واپس CMD_ڈیٹا_سائز_STR;
		حالت 'q':
			اگر (MEM_SUPPORT_64_ثنائی_ہندسہ_ڈیٹا)
				واپس 8;
			fallthrough;
		طےشدہ:
			واپس CMD_ڈیٹا_سائز_ERR;
		}
	}
	واپس default_سائز;
}
#اگر_ختم

خالی fixup_cmdtable(ساخت cmd_tbl *cmdtp, صحیح سائز)
{
	صحیح	i;

	اگر (gd->reloc_off == 0)
		واپس;

	برائے (i = 0; i < سائز; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(صحیح (*)(ساخت cmd_tbl *, صحیح, صحیح,
				 حرف * ثابت [], صحیح *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#اگر_تعریف_شدہ DEBUG_COMMANDS
		لکھو("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#اگر_ختم
		cmdtp->cmd = (صحیح (*)(ساخت cmd_tbl *, صحیح, صحیح,
				      حرف *ثابت []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (حرف *)addr;
		اگر (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (حرف *)addr;
		}
#اگر_تعریف_شدہ	CONFIG_SYS_LONGHELP
		اگر (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (حرف *)addr;
		}
#اگر_ختم
#اگر_تعریف_شدہ CONFIG_AUTO_COMPLETE
		اگر (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(صحیح (*)(صحیح, حرف * ثابت [], حرف, صحیح, حرف * []))addr;
		}
#اگر_ختم
		cmdtp++;
	}
}

صحیح cmd_always_repeatable(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc,
			  حرف *ثابت argv[], صحیح *repeatable)
{
	*repeatable = 1;

	واپس cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحیح cmd_never_repeatable(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc,
			 حرف *ثابت argv[], صحیح *repeatable)
{
	*repeatable = 0;

	واپس cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحیح cmd_discard_repeatable(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc,
			   حرف *ثابت argv[])
{
	صحیح repeatable;

	واپس cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ساکن صحیح cmd_call(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc,
		    حرف *ثابت argv[], صحیح *repeatable)
{
	صحیح نتیجہ;

	نتیجہ = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	اگر (نتیجہ)
		debug("Command failed, result=%d\n", نتیجہ);
	واپس نتیجہ;
}

شمار command_ret_t cmd_process(صحیح flag, صحیح argc, حرف *ثابت argv[],
			       صحیح *repeatable, ulong *ticks)
{
	شمار command_ret_t rc = CMD_RET_SUCCESS;
	ساخت cmd_tbl *cmdtp;

#اگر defined(CONFIG_SYS_XTRACE)
	حرف *xtrace;

	xtrace = env_get("xtrace");
	اگر (xtrace) {
		puts("+");
		برائے (صحیح i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#اگر_ختم

	/* Look up command in command table */
	cmdtp = ڈھونڈو_cmd(argv[0]);
	اگر (cmdtp == تہی) {
		لکھو("Unknown command '%s' - try 'help'\n", argv[0]);
		واپس 1;
	}

	/* found - check max args */
	اگر (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#اگر defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ورنہ اگر (cmdtp->cmd == do_bootd) {
		اگر (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ورنہ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#اگر_ختم

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
	واپس rc;
}

صحیح cmd_process_error(ساخت cmd_tbl *cmdtp, صحیح err)
{
	اگر (err == CMD_RET_USAGE)
		واپس CMD_RET_USAGE;

	اگر (err) {
		لکھو("Command '%s' failed: Error %d\n", cmdtp->name, err);
		واپس CMD_RET_FAILURE;
	}

	واپس CMD_RET_SUCCESS;
}

صحیح cmd_ماخذ_script(ulong addr, ثابت حرف *fit_uname, ثابت حرف *confname)
{
	حرف *ڈیٹا;
	خالی *buf;
	uint len;
	صحیح ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &ڈیٹا, &len);
	unmap_sysmem(buf);
	اگر (ret)
		واپس CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	واپس تسلسل_command_list(ڈیٹا, len, 0);
}
