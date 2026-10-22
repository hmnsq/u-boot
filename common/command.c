// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#include <config.h>
#include <compiler.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <mapmem.h>
#include <time.h>
#include <asm/global_data.h>
#include <linux/ctype.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

puur_naa_nka _do_help(sa_nracanaa cmd_tbl *cmd_start, puur_naa_nka cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     puur_naa_nka flag, puur_naa_nka argc, var_na *const argv[])
{
	puur_naa_nka i;
	puur_naa_nka rcode = 0;

	ပေႃး (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		puur_naa_nka i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		for (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		for (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			for (j = 0; j < i; ++j) {
				ပေႃး (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ပေႃး (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ပေႃး (ctrlc())
				ပွၵ်ႈ 1;
			ပေႃး (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ပွၵ်ႈ 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ပေႃး (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} ပေႃးဢမ်ႇ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ပွၵ်ႈ rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     puur_naa_nka table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	puur_naa_nka len;
	puur_naa_nka n_found = 0;

	ပေႃး (!cmd)
		ပွၵ်ႈ NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ပေႃး (strncmp(cmd, cmdtp->name, len) == 0) {
			ပေႃး (len == strlen(cmdtp->name))
				ပွၵ်ႈ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ပေႃး (n_found == 1) {			/* exactly one match */
		ပွၵ်ႈ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ပွၵ်ႈ NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const puur_naa_nka len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ပွၵ်ႈ find_cmd_tbl(cmd, start, len);
}

puur_naa_nka cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ပေႃး (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ပွၵ်ႈ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ပွၵ်ႈ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

puur_naa_nka var_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char, puur_naa_nka maxv,
		 var_na *cmdv[])
{
	puur_naa_nka space;

	space = last_char == '\0' || isblank(last_char);

	ပေႃး (space && argc == 1)
		ပွၵ်ႈ env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ပေႃး (!space && argc == 2)
		ပွၵ်ႈ env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ပွၵ်ႈ 0;
}

static puur_naa_nka dollar_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			   puur_naa_nka maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ပေႃး (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ပွၵ်ႈ 0;

	ပွၵ်ႈ env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

puur_naa_nka complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka count, puur_naa_nka argc,
		     var_na *const argv[], var_na last_char,
		     puur_naa_nka maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	puur_naa_nka len, clen;
	puur_naa_nka n_found = 0;
	const var_na *cmd;

	/* sanity? */
	ပေႃး (maxv < 2)
		ပွၵ်ႈ -2;

	cmdv[0] = NULL;

	ပေႃး (argc == 0) {
		/* output full list of commands */
		for (; cmdtp != cmdend; cmdtp++) {
			ပေႃး (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ပွၵ်ႈ n_found;
	}

	/* more than one arg or one but the start of the next */
	ပေႃး (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ပေႃး (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ပွၵ်ႈ 0;
		}
		ပွၵ်ႈ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ပေႃး (p == NULL)
		len = strlen(cmd);
	ပေႃးဢမ်ႇ
		len = p - cmd;

	/* return the partial matches */
	for (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ပေႃး (clen < len)
			continue;

		ပေႃး (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ပေႃး (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ပွၵ်ႈ n_found;
#ပေႃးဢမ်ႇ
	ပွၵ်ႈ 0;
#endif
}

static puur_naa_nka complete_cmdv(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			 puur_naa_nka maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ပွၵ်ႈ complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ပေႃးဢမ်ႇ
	ပွၵ်ႈ 0;
#endif
}

static puur_naa_nka make_argv(var_na *s, puur_naa_nka argvsz, var_na *argv[])
{
	puur_naa_nka argc = 0;

	/* split into argv */
	ၶၢဝ်း (argc < argvsz - 1) {

		/* skip any white space */
		ၶၢဝ်း (isblank(*s))
			++s;

		ပေႃး (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ၶၢဝ်း (*s && !isblank(*s))
			++s;

		ပေႃး (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ပွၵ်ႈ argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       puur_naa_nka linemax, var_na *const argv[])
{
	puur_naa_nka ll = leader != NULL ? strlen(leader) : 0;
	puur_naa_nka sl = sep != NULL ? strlen(sep) : 0;
	puur_naa_nka len, i;

	ပေႃး (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ၶၢဝ်း (*argv != NULL) {
		len = strlen(*argv) + sl;
		ပေႃး (i + len >= linemax) {
			puts("\n");
			ပေႃး (leader)
				puts(leader);
			i = ll - sl;
		} ပေႃးဢမ်ႇ ပေႃး (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static puur_naa_nka find_common_prefix(var_na *const argv[])
{
	puur_naa_nka i, len;
	var_na *anchor, *s, *t;

	ပေႃး (*argv == NULL)
		ပွၵ်ႈ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ၶၢဝ်း ((t = *argv++) != NULL) {
		s = anchor;
		for (i = 0; i < len; i++, t++, s++) {
			ပေႃး (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ပွၵ်ႈ len;
}

puur_naa_nka cmd_auto_complete(const var_na *const prompt, var_na *buf, puur_naa_nka *np, puur_naa_nka *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	puur_naa_nka n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	puur_naa_nka i, j, k, len, seplen, argc;
	puur_naa_nka cnt;
	var_na last_char;
	const var_na *ps_prompt;

	ပေႃး (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ပေႃး (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ပေႃးဢမ်ႇ
		ps_prompt = CONFIG_SYS_PROMPT;

	ပေႃး (strcmp(prompt, ps_prompt) != 0)
		ပွၵ်ႈ 0;	/* not in normal console */

	cnt = strlen(buf);
	ပေႃး (cnt >= 1)
		last_char = buf[cnt - 1];
	ပေႃးဢမ်ႇ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ပေႃး (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ပေႃး (i == 0) {
		ပေႃး (argc > 1)	/* allow tab for non command */
			ပွၵ်ႈ 0;
		putc('\a');
		ပွၵ်ႈ 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ပေႃး (i == 1) { /* one match; perfect */
		ပေႃး (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ပေႃးဢမ်ႇ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ပေႃးဢမ်ႇ ပေႃး (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ပေႃး (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ပေႃးဢမ်ႇ
			k = 0;

		j -= k;
		ပေႃး (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ပေႃး (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ပေႃး (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ပွၵ်ႈ 1;
		}

		t = buf + cnt;
		for (i = 0; i < len; i++)
			*t++ = *s++;
		ပေႃး (sep != NULL)
			for (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ပေႃး (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} ပေႃးဢမ်ႇ {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ပွၵ်ႈ 1;
}

#endif

#ifdef CMD_DATA_SIZE
puur_naa_nka cmd_get_data_size(const var_na *arg, puur_naa_nka default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	puur_naa_nka len = strlen(arg);
	ပေႃး (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ပွၵ်ႈ 1;
		pak_sa 'w':
			ပွၵ်ႈ 2;
		pak_sa 'l':
			ပွၵ်ႈ 4;
		pak_sa 's':
			ပွၵ်ႈ CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ပေႃး (MEM_SUPPORT_64BIT_DATA)
				ပွၵ်ႈ 8;
			fallthrough;
		default:
			ပွၵ်ႈ CMD_DATA_SIZE_ERR;
		}
	}
	ပွၵ်ႈ default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka size)
{
	puur_naa_nka	i;

	ပေႃး (gd->reloc_off == 0)
		ပွၵ်ႈ;

	for (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(puur_naa_nka (*)(sa_nracanaa cmd_tbl *, puur_naa_nka, puur_naa_nka,
				 var_na * const [], puur_naa_nka *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (puur_naa_nka (*)(sa_nracanaa cmd_tbl *, puur_naa_nka, puur_naa_nka,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		ပေႃး (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ပေႃး (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ပေႃး (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(puur_naa_nka (*)(puur_naa_nka, var_na * const [], var_na, puur_naa_nka, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

puur_naa_nka cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			  var_na *const argv[], puur_naa_nka *repeatable)
{
	*repeatable = 1;

	ပွၵ်ႈ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			 var_na *const argv[], puur_naa_nka *repeatable)
{
	*repeatable = 0;

	ပွၵ်ႈ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			   var_na *const argv[])
{
	puur_naa_nka repeatable;

	ပွၵ်ႈ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static puur_naa_nka cmd_call(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
		    var_na *const argv[], puur_naa_nka *repeatable)
{
	puur_naa_nka result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ပေႃး (result)
		debug("Command failed, result=%d\n", result);
	ပွၵ်ႈ result;
}

ga_nanaa command_ret_t cmd_process(puur_naa_nka flag, puur_naa_nka argc, var_na *const argv[],
			       puur_naa_nka *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ပေႃး defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ပေႃး (xtrace) {
		puts("+");
		for (puur_naa_nka i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ပေႃး (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ပွၵ်ႈ 1;
	}

	/* found - check max args */
	ပေႃး (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ပေႃး defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ပေႃးဢမ်ႇ ပေႃး (cmdtp->cmd == do_bootd) {
		ပေႃး (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ပေႃးဢမ်ႇ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ပေႃး (!rc) {
		puur_naa_nka newrep;

		ပေႃး (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ပေႃး (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ပေႃး (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ပွၵ်ႈ rc;
}

puur_naa_nka cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka err)
{
	ပေႃး (err == CMD_RET_USAGE)
		ပွၵ်ႈ CMD_RET_USAGE;

	ပေႃး (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ပွၵ်ႈ CMD_RET_FAILURE;
	}

	ပွၵ်ႈ CMD_RET_SUCCESS;
}

puur_naa_nka cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	puur_naa_nka ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ပေႃး (ret)
		ပွၵ်ႈ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ပွၵ်ႈ run_command_list(data, len, 0);
}
