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

DECLARE_GLOBAL_معلومات_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

صحيح _do_help(بنية cmd_tbl *cmd_شروعات, صحيح cmd_شيون, بنية cmd_tbl *cmdtp,
	     صحيح flag, صحيح argc, حرف *ثابت argv[])
{
	صحيح i;
	صحيح rcode = 0;

	إذا (argc == 1) {	/* show list of commands */
		بنية cmd_tbl *cmd_array[cmd_شيون];
		صحيح i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_شروعات;
		لكل (i = 0; i < cmd_شيون; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		لكل (i = cmd_شيون - 1; i > 0; --i) {
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
		لكل (i = 0; i < cmd_شيون; i++) {
			ثابت حرف *usage = cmd_array[i]->usage;

			/* allow user abort */
			إذا (ctrlc())
				أرجع 1;
			إذا (usage == عدم)
				استمر;
			لکو("%-*s- %s\n", CFG_SYS_HELP_CMD_ويڪر,
			       cmd_array[i]->name, usage);
		}
		أرجع 0;
	}
	/*
	 * command help (long version)
	 */
	لكل (i = 1; i < argc; ++i) {
		cmdtp = ڳوليو_cmd_tbl(argv[i], cmd_شروعات, cmd_شيون);
		إذا (cmdtp != عدم) {
			rcode |= cmd_usage(cmdtp);
		} وإلا {
			لکو("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	أرجع rcode;
}

/* find command table entry for a command */
بنية cmd_tbl *ڳوليو_cmd_tbl(ثابت حرف *cmd, بنية cmd_tbl *جدول,
			     صحيح جدول_len)
{
#جيڪڏهن_وصف_ٿيل CONFIG_CMDLINE
	بنية cmd_tbl *cmdtp;
	بنية cmd_tbl *cmdtp_عارضي = جدول;	/* Init value */
	ثابت حرف *p;
	صحيح len;
	صحيح n_مليو = 0;

	إذا (!cmd)
		أرجع عدم;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == عدم) ? لڙي_ڊيگهه (cmd) : (p - cmd);

	لكل (cmdtp = جدول; cmdtp != جدول + جدول_len; cmdtp++) {
		إذا (لڙي_ڀيٽ(cmd, cmdtp->name, len) == 0) {
			إذا (len == لڙي_ڊيگهه(cmdtp->name))
				أرجع cmdtp;	/* full match */

			cmdtp_عارضي = cmdtp;	/* abbreviated command ? */
			n_مليو++;
		}
	}
	إذا (n_مليو == 1) {			/* exactly one match */
		أرجع cmdtp_عارضي;
	}
#جيڪڏهن_پورو /* CONFIG_CMDLINE */

	أرجع عدم;	/* not found or ambiguous command */
}

بنية cmd_tbl *ڳوليو_cmd(ثابت حرف *cmd)
{
	بنية cmd_tbl *شروعات = ll_داخلا_شروعات(بنية cmd_tbl, cmd);
	ثابت صحيح len = ll_داخلا_ڳڻپ(بنية cmd_tbl, cmd);
	أرجع ڳوليو_cmd_tbl(cmd, شروعات, len);
}

صحيح cmd_usage(ثابت بنية cmd_tbl *cmdtp)
{
	لکو("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#جيڪڏهن_وصف_ٿيل	CONFIG_SYS_LONGHELP
	لکو("Usage:\n%s ", cmdtp->name);

	إذا (!cmdtp->help) {
		puts ("- No additional help available.\n");
		أرجع 1;
	}

	puts(cmdtp->help);
	putc('\n');
#جيڪڏهن_پورو	/* CONFIG_SYS_LONGHELP */
	أرجع 1;
}

#جيڪڏهن_وصف_ٿيل CONFIG_AUTO_COMPLETE
ساكن حرف env_complete_buf[512];

صحيح var_complete(صحيح argc, حرف *ثابت argv[], حرف آخري_char, صحيح maxv,
		 حرف *cmdv[])
{
	صحيح space;

	space = آخري_char == '\0' || isblank(آخري_char);

	إذا (space && argc == 1)
		أرجع env_complete("", maxv, cmdv, حجم(env_complete_buf),
				    env_complete_buf, false);

	إذا (!space && argc == 2)
		أرجع env_complete(argv[1], maxv, cmdv,
				    حجم(env_complete_buf),
				    env_complete_buf, false);

	أرجع 0;
}

ساكن صحيح dollar_complete(صحيح argc, حرف *ثابت argv[], حرف آخري_char,
			   صحيح maxv, حرف *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	إذا (argc < 1 || argv[argc - 1][0] != '$' ||
	    آخري_char == '\0' || isblank(آخري_char))
		أرجع 0;

	أرجع env_complete(argv[argc - 1], maxv, cmdv, حجم(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

صحيح complete_subcmdv(بنية cmd_tbl *cmdtp, صحيح ڳڻپ, صحيح argc,
		     حرف *ثابت argv[], حرف آخري_char,
		     صحيح maxv, حرف *cmdv[])
{
#جيڪڏهن_وصف_ٿيل CONFIG_CMDLINE
	ثابت بنية cmd_tbl *cmdend = cmdtp + ڳڻپ;
	ثابت حرف *p;
	صحيح len, clen;
	صحيح n_مليو = 0;
	ثابت حرف *cmd;

	/* sanity? */
	إذا (maxv < 2)
		أرجع -2;

	cmdv[0] = عدم;

	إذا (argc == 0) {
		/* output full list of commands */
		لكل (; cmdtp != cmdend; cmdtp++) {
			إذا (n_مليو >= maxv - 2) {
				cmdv[n_مليو++] = "...";
				اكسر;
			}
			cmdv[n_مليو++] = cmdtp->name;
		}
		cmdv[n_مليو] = عدم;
		أرجع n_مليو;
	}

	/* more than one arg or one but the start of the next */
	إذا (argc > 1 || آخري_char == '\0' || isblank(آخري_char)) {
		cmdtp = ڳوليو_cmd_tbl(argv[0], cmdtp, ڳڻپ);
		إذا (cmdtp == عدم || cmdtp->complete == عدم) {
			cmdv[0] = عدم;
			أرجع 0;
		}
		أرجع (*cmdtp->complete)(argc, argv, آخري_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	إذا (p == عدم)
		len = لڙي_ڊيگهه(cmd);
	وإلا
		len = p - cmd;

	/* return the partial matches */
	لكل (; cmdtp != cmdend; cmdtp++) {

		clen = لڙي_ڊيگهه(cmdtp->name);
		إذا (clen < len)
			استمر;

		إذا (memcmp(cmd, cmdtp->name, len) != 0)
			استمر;

		/* too many! */
		إذا (n_مليو >= maxv - 2) {
			cmdv[n_مليو++] = "...";
			اكسر;
		}

		cmdv[n_مليو++] = cmdtp->name;
	}

	cmdv[n_مليو] = عدم;
	أرجع n_مليو;
#وإلا
	أرجع 0;
#جيڪڏهن_پورو
}

ساكن صحيح complete_cmdv(صحيح argc, حرف *ثابت argv[], حرف آخري_char,
			 صحيح maxv, حرف *cmdv[])
{
#جيڪڏهن_وصف_ٿيل CONFIG_CMDLINE
	أرجع complete_subcmdv(ll_داخلا_شروعات(بنية cmd_tbl, cmd),
				ll_داخلا_ڳڻپ(بنية cmd_tbl, cmd), argc, argv,
				آخري_char, maxv, cmdv);
#وإلا
	أرجع 0;
#جيڪڏهن_پورو
}

ساكن صحيح make_argv(حرف *s, صحيح argvsz, حرف *argv[])
{
	صحيح argc = 0;

	/* split into argv */
	طالما (argc < argvsz - 1) {

		/* skip any white space */
		طالما (isblank(*s))
			++s;

		إذا (*s == '\0')	/* end of s, no more args	*/
			اكسر;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		طالما (*s && !isblank(*s))
			++s;

		إذا (*s == '\0')		/* end of s, no more args	*/
			اكسر;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = عدم;

	أرجع argc;
}

ساكن فراغ print_argv(ثابت حرف *banner, ثابت حرف *leader, ثابت حرف *sep,
		       صحيح linemax, حرف *ثابت argv[])
{
	صحيح ll = leader != عدم ? لڙي_ڊيگهه(leader) : 0;
	صحيح sl = sep != عدم ? لڙي_ڊيگهه(sep) : 0;
	صحيح len, i;

	إذا (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	طالما (*argv != عدم) {
		len = لڙي_ڊيگهه(*argv) + sl;
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
	لکو("\n");
}

ساكن صحيح ڳوليو_common_اڳياڙي(حرف *ثابت argv[])
{
	صحيح i, len;
	حرف *anchor, *s, *t;

	إذا (*argv == عدم)
		أرجع 0;

	/* begin with max */
	anchor = *argv++;
	len = لڙي_ڊيگهه(anchor);
	طالما ((t = *argv++) != عدم) {
		s = anchor;
		لكل (i = 0; i < len; i++, t++, s++) {
			إذا (*t != *s)
				اكسر;
		}
		len = s - anchor;
	}
	أرجع len;
}

صحيح cmd_auto_complete(ثابت حرف *ثابت prompt, حرف *buf, صحيح *np, صحيح *colp)
{
	حرف tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	صحيح n = *np, کانو = *colp;
	حرف *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	حرف *cmdv[20];
	حرف *s, *t;
	ثابت حرف *sep;
	صحيح i, j, k, len, seplen, argc;
	صحيح cnt;
	حرف آخري_char;
	ثابت حرف *ps_prompt;

	إذا (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		إذا (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} وإلا
		ps_prompt = CONFIG_SYS_PROMPT;

	إذا (strcmp(prompt, ps_prompt) != 0)
		أرجع 0;	/* not in normal console */

	cnt = لڙي_ڊيگهه(buf);
	إذا (cnt >= 1)
		آخري_char = buf[cnt - 1];
	وإلا
		آخري_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, حجم(argv)/حجم(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, آخري_char,
			    حجم(cmdv) / حجم(cmdv[0]), cmdv);
	إذا (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, آخري_char,
				  حجم(cmdv) / حجم(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	إذا (i == 0) {
		إذا (argc > 1)	/* allow tab for non command */
			أرجع 0;
		putc('\a');
		أرجع 1;
	}

	s = عدم;
	len = 0;
	sep = عدم;
	seplen = 0;
	إذا (i == 1) { /* one match; perfect */
		إذا (آخري_char != '\0' && !isblank(آخري_char))
			k = لڙي_ڊيگهه(argv[argc - 1]);
		وإلا
			k = 0;

		s = cmdv[0] + k;
		len = لڙي_ڊيگهه(s);
		sep = " ";
		seplen = 1;
	} وإلا إذا (i > 1 && (j = ڳوليو_common_اڳياڙي(cmdv)) != 0) { /*وڌيڪ*/
		إذا (آخري_char != '\0' && !isblank(آخري_char))
			k = لڙي_ڊيگهه(argv[argc - 1]);
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
			أرجع 1;
		}

		t = buf + cnt;
		لكل (i = 0; i < len; i++)
			*t++ = *s++;
		إذا (sep != عدم)
			لكل (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		کانو += k;
		puts(t - k);
		إذا (sep == عدم)
			putc('\a');
		*np = n;
		*colp = کانو;
	} وإلا {
		print_argv(عدم, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	أرجع 1;
}

#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CMD_معلومات_سائيز
صحيح cmd_get_معلومات_سائيز(ثابت حرف *arg, صحيح default_سائيز)
{
	/* Check for a size specification .b, .w or .l.
	 */
	صحيح len = لڙي_ڊيگهه(arg);
	إذا (len >= 2 && arg[len-2] == '.') {
		بدل (arg[len-1]) {
		حالة 'b':
			أرجع 1;
		حالة 'w':
			أرجع 2;
		حالة 'l':
			أرجع 4;
		حالة 's':
			أرجع CMD_معلومات_سائيز_STR;
		حالة 'q':
			إذا (MEM_SUPPORT_64_ٻٽو_انگ_معلومات)
				أرجع 8;
			fallthrough;
		افتراضي:
			أرجع CMD_معلومات_سائيز_ERR;
		}
	}
	أرجع default_سائيز;
}
#جيڪڏهن_پورو

فراغ fixup_cmdtable(بنية cmd_tbl *cmdtp, صحيح سائيز)
{
	صحيح	i;

	إذا (gd->reloc_off == 0)
		أرجع;

	لكل (i = 0; i < سائيز; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(صحيح (*)(بنية cmd_tbl *, صحيح, صحيح,
				 حرف * ثابت [], صحيح *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#جيڪڏهن_وصف_ٿيل DEBUG_COMMANDS
		لکو("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#جيڪڏهن_پورو
		cmdtp->cmd = (صحيح (*)(بنية cmd_tbl *, صحيح, صحيح,
				      حرف *ثابت []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (حرف *)addr;
		إذا (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (حرف *)addr;
		}
#جيڪڏهن_وصف_ٿيل	CONFIG_SYS_LONGHELP
		إذا (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (حرف *)addr;
		}
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل CONFIG_AUTO_COMPLETE
		إذا (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(صحيح (*)(صحيح, حرف * ثابت [], حرف, صحيح, حرف * []))addr;
		}
#جيڪڏهن_پورو
		cmdtp++;
	}
}

صحيح cmd_always_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			  حرف *ثابت argv[], صحيح *repeatable)
{
	*repeatable = 1;

	أرجع cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_never_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			 حرف *ثابت argv[], صحيح *repeatable)
{
	*repeatable = 0;

	أرجع cmdtp->cmd(cmdtp, flag, argc, argv);
}

صحيح cmd_discard_repeatable(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
			   حرف *ثابت argv[])
{
	صحيح repeatable;

	أرجع cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	صحيح نتيجو;

	نتيجو = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	إذا (نتيجو)
		debug("Command failed, result=%d\n", نتيجو);
	أرجع نتيجو;
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
#جيڪڏهن_پورو

	/* Look up command in command table */
	cmdtp = ڳوليو_cmd(argv[0]);
	إذا (cmdtp == عدم) {
		لکو("Unknown command '%s' - try 'help'\n", argv[0]);
		أرجع 1;
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
#جيڪڏهن_پورو

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
	أرجع rc;
}

صحيح cmd_process_error(بنية cmd_tbl *cmdtp, صحيح err)
{
	إذا (err == CMD_RET_USAGE)
		أرجع CMD_RET_USAGE;

	إذا (err) {
		لکو("Command '%s' failed: Error %d\n", cmdtp->name, err);
		أرجع CMD_RET_FAILURE;
	}

	أرجع CMD_RET_SUCCESS;
}

صحيح cmd_ذريعو_script(ulong addr, ثابت حرف *fit_uname, ثابت حرف *confname)
{
	حرف *معلومات;
	فراغ *buf;
	uint len;
	صحيح ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &معلومات, &len);
	unmap_sysmem(buf);
	إذا (ret)
		أرجع CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	أرجع لاڳيتو_command_list(معلومات, len, 0);
}
