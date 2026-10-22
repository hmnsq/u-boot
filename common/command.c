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

	ᤀᤡ᤺ (argc == 1) {	/* show list of commands */
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
				ᤀᤡ᤺ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ᤀᤡ᤺ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ᤀᤡ᤺ (ctrlc())
				ᤀᤡᤴ 1;
			ᤀᤡ᤺ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ᤀᤡᤴ 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ᤀᤡ᤺ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} ᤀᤡ᤺ᤆᤡ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ᤀᤡᤴ rcode;
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

	ᤀᤡ᤺ (!cmd)
		ᤀᤡᤴ NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ᤀᤡ᤺ (strncmp(cmd, cmdtp->name, len) == 0) {
			ᤀᤡ᤺ (len == strlen(cmdtp->name))
				ᤀᤡᤴ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ᤀᤡ᤺ (n_found == 1) {			/* exactly one match */
		ᤀᤡᤴ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ᤀᤡᤴ NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const puur_naa_nka len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ᤀᤡᤴ find_cmd_tbl(cmd, start, len);
}

puur_naa_nka cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ᤀᤡ᤺ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ᤀᤡᤴ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ᤀᤡᤴ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

puur_naa_nka var_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char, puur_naa_nka maxv,
		 var_na *cmdv[])
{
	puur_naa_nka space;

	space = last_char == '\0' || isblank(last_char);

	ᤀᤡ᤺ (space && argc == 1)
		ᤀᤡᤴ env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ᤀᤡ᤺ (!space && argc == 2)
		ᤀᤡᤴ env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ᤀᤡᤴ 0;
}

static puur_naa_nka dollar_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			   puur_naa_nka maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ᤀᤡ᤺ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ᤀᤡᤴ 0;

	ᤀᤡᤴ env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
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
	ᤀᤡ᤺ (maxv < 2)
		ᤀᤡᤴ -2;

	cmdv[0] = NULL;

	ᤀᤡ᤺ (argc == 0) {
		/* output full list of commands */
		for (; cmdtp != cmdend; cmdtp++) {
			ᤀᤡ᤺ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ᤀᤡᤴ n_found;
	}

	/* more than one arg or one but the start of the next */
	ᤀᤡ᤺ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ᤀᤡ᤺ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ᤀᤡᤴ 0;
		}
		ᤀᤡᤴ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ᤀᤡ᤺ (p == NULL)
		len = strlen(cmd);
	ᤀᤡ᤺ᤆᤡ
		len = p - cmd;

	/* return the partial matches */
	for (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ᤀᤡ᤺ (clen < len)
			continue;

		ᤀᤡ᤺ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ᤀᤡ᤺ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ᤀᤡᤴ n_found;
#ᤀᤡ᤺ᤆᤡ
	ᤀᤡᤴ 0;
#endif
}

static puur_naa_nka complete_cmdv(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			 puur_naa_nka maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ᤀᤡᤴ complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ᤀᤡ᤺ᤆᤡ
	ᤀᤡᤴ 0;
#endif
}

static puur_naa_nka make_argv(var_na *s, puur_naa_nka argvsz, var_na *argv[])
{
	puur_naa_nka argc = 0;

	/* split into argv */
	ᤀᤠ᤺ (argc < argvsz - 1) {

		/* skip any white space */
		ᤀᤠ᤺ (isblank(*s))
			++s;

		ᤀᤡ᤺ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ᤀᤠ᤺ (*s && !isblank(*s))
			++s;

		ᤀᤡ᤺ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ᤀᤡᤴ argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       puur_naa_nka linemax, var_na *const argv[])
{
	puur_naa_nka ll = leader != NULL ? strlen(leader) : 0;
	puur_naa_nka sl = sep != NULL ? strlen(sep) : 0;
	puur_naa_nka len, i;

	ᤀᤡ᤺ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ᤀᤠ᤺ (*argv != NULL) {
		len = strlen(*argv) + sl;
		ᤀᤡ᤺ (i + len >= linemax) {
			puts("\n");
			ᤀᤡ᤺ (leader)
				puts(leader);
			i = ll - sl;
		} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (sep)
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

	ᤀᤡ᤺ (*argv == NULL)
		ᤀᤡᤴ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ᤀᤠ᤺ ((t = *argv++) != NULL) {
		s = anchor;
		for (i = 0; i < len; i++, t++, s++) {
			ᤀᤡ᤺ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ᤀᤡᤴ len;
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

	ᤀᤡ᤺ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ᤀᤡ᤺ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ᤀᤡ᤺ᤆᤡ
		ps_prompt = CONFIG_SYS_PROMPT;

	ᤀᤡ᤺ (strcmp(prompt, ps_prompt) != 0)
		ᤀᤡᤴ 0;	/* not in normal console */

	cnt = strlen(buf);
	ᤀᤡ᤺ (cnt >= 1)
		last_char = buf[cnt - 1];
	ᤀᤡ᤺ᤆᤡ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ᤀᤡ᤺ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ᤀᤡ᤺ (i == 0) {
		ᤀᤡ᤺ (argc > 1)	/* allow tab for non command */
			ᤀᤡᤴ 0;
		putc('\a');
		ᤀᤡᤴ 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ᤀᤡ᤺ (i == 1) { /* one match; perfect */
		ᤀᤡ᤺ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ᤀᤡ᤺ᤆᤡ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ᤀᤡ᤺ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ᤀᤡ᤺ᤆᤡ
			k = 0;

		j -= k;
		ᤀᤡ᤺ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ᤀᤡ᤺ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ᤀᤡ᤺ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ᤀᤡᤴ 1;
		}

		t = buf + cnt;
		for (i = 0; i < len; i++)
			*t++ = *s++;
		ᤀᤡ᤺ (sep != NULL)
			for (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ᤀᤡ᤺ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} ᤀᤡ᤺ᤆᤡ {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ᤀᤡᤴ 1;
}

#endif

#ifdef CMD_DATA_SIZE
puur_naa_nka cmd_get_data_size(const var_na *arg, puur_naa_nka default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	puur_naa_nka len = strlen(arg);
	ᤀᤡ᤺ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ᤀᤡᤴ 1;
		pak_sa 'w':
			ᤀᤡᤴ 2;
		pak_sa 'l':
			ᤀᤡᤴ 4;
		pak_sa 's':
			ᤀᤡᤴ CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ᤀᤡ᤺ (MEM_SUPPORT_64BIT_DATA)
				ᤀᤡᤴ 8;
			fallthrough;
		default:
			ᤀᤡᤴ CMD_DATA_SIZE_ERR;
		}
	}
	ᤀᤡᤴ default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka size)
{
	puur_naa_nka	i;

	ᤀᤡ᤺ (gd->reloc_off == 0)
		ᤀᤡᤴ;

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
		ᤀᤡ᤺ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ᤀᤡ᤺ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ᤀᤡ᤺ (cmdtp->complete) {
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

	ᤀᤡᤴ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			 var_na *const argv[], puur_naa_nka *repeatable)
{
	*repeatable = 0;

	ᤀᤡᤴ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			   var_na *const argv[])
{
	puur_naa_nka repeatable;

	ᤀᤡᤴ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	ᤀᤡ᤺ (result)
		debug("Command failed, result=%d\n", result);
	ᤀᤡᤴ result;
}

ga_nanaa command_ret_t cmd_process(puur_naa_nka flag, puur_naa_nka argc, var_na *const argv[],
			       puur_naa_nka *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ᤀᤡ᤺ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ᤀᤡ᤺ (xtrace) {
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
	ᤀᤡ᤺ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ᤀᤡᤴ 1;
	}

	/* found - check max args */
	ᤀᤡ᤺ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ᤀᤡ᤺ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ᤀᤡ᤺ᤆᤡ ᤀᤡ᤺ (cmdtp->cmd == do_bootd) {
		ᤀᤡ᤺ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ᤀᤡ᤺ᤆᤡ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ᤀᤡ᤺ (!rc) {
		puur_naa_nka newrep;

		ᤀᤡ᤺ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ᤀᤡ᤺ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ᤀᤡ᤺ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ᤀᤡᤴ rc;
}

puur_naa_nka cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka err)
{
	ᤀᤡ᤺ (err == CMD_RET_USAGE)
		ᤀᤡᤴ CMD_RET_USAGE;

	ᤀᤡ᤺ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ᤀᤡᤴ CMD_RET_FAILURE;
	}

	ᤀᤡᤴ CMD_RET_SUCCESS;
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
	ᤀᤡ᤺ (ret)
		ᤀᤡᤴ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ᤀᤡᤴ run_command_list(data, len, 0);
}
