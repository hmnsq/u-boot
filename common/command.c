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

	ᯀᯮᯝ (argc == 1) {	/* show list of commands */
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
				ᯀᯮᯝ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ᯀᯮᯝ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		for (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ᯀᯮᯝ (ctrlc())
				ᯀᯮᯞ 1;
			ᯀᯮᯝ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ᯀᯮᯞ 0;
	}
	/*
	 * command help (long version)
	 */
	for (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ᯀᯮᯝ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} ᯀᯮᯝᯂᯪ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ᯀᯮᯞ rcode;
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

	ᯀᯮᯝ (!cmd)
		ᯀᯮᯞ NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ᯀᯮᯝ (strncmp(cmd, cmdtp->name, len) == 0) {
			ᯀᯮᯝ (len == strlen(cmdtp->name))
				ᯀᯮᯞ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ᯀᯮᯝ (n_found == 1) {			/* exactly one match */
		ᯀᯮᯞ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ᯀᯮᯞ NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const puur_naa_nka len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ᯀᯮᯞ find_cmd_tbl(cmd, start, len);
}

puur_naa_nka cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ᯀᯮᯝ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ᯀᯮᯞ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ᯀᯮᯞ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

puur_naa_nka var_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char, puur_naa_nka maxv,
		 var_na *cmdv[])
{
	puur_naa_nka space;

	space = last_char == '\0' || isblank(last_char);

	ᯀᯮᯝ (space && argc == 1)
		ᯀᯮᯞ env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ᯀᯮᯝ (!space && argc == 2)
		ᯀᯮᯞ env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ᯀᯮᯞ 0;
}

static puur_naa_nka dollar_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			   puur_naa_nka maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ᯀᯮᯝ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ᯀᯮᯞ 0;

	ᯀᯮᯞ env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
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
	ᯀᯮᯝ (maxv < 2)
		ᯀᯮᯞ -2;

	cmdv[0] = NULL;

	ᯀᯮᯝ (argc == 0) {
		/* output full list of commands */
		for (; cmdtp != cmdend; cmdtp++) {
			ᯀᯮᯝ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ᯀᯮᯞ n_found;
	}

	/* more than one arg or one but the start of the next */
	ᯀᯮᯝ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ᯀᯮᯝ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ᯀᯮᯞ 0;
		}
		ᯀᯮᯞ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ᯀᯮᯝ (p == NULL)
		len = strlen(cmd);
	ᯀᯮᯝᯂᯪ
		len = p - cmd;

	/* return the partial matches */
	for (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ᯀᯮᯝ (clen < len)
			continue;

		ᯀᯮᯝ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ᯀᯮᯝ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ᯀᯮᯞ n_found;
#ᯀᯮᯝᯂᯪ
	ᯀᯮᯞ 0;
#endif
}

static puur_naa_nka complete_cmdv(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			 puur_naa_nka maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ᯀᯮᯞ complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ᯀᯮᯝᯂᯪ
	ᯀᯮᯞ 0;
#endif
}

static puur_naa_nka make_argv(var_na *s, puur_naa_nka argvsz, var_na *argv[])
{
	puur_naa_nka argc = 0;

	/* split into argv */
	ᯀᯬᯝ (argc < argvsz - 1) {

		/* skip any white space */
		ᯀᯬᯝ (isblank(*s))
			++s;

		ᯀᯮᯝ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ᯀᯬᯝ (*s && !isblank(*s))
			++s;

		ᯀᯮᯝ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ᯀᯮᯞ argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       puur_naa_nka linemax, var_na *const argv[])
{
	puur_naa_nka ll = leader != NULL ? strlen(leader) : 0;
	puur_naa_nka sl = sep != NULL ? strlen(sep) : 0;
	puur_naa_nka len, i;

	ᯀᯮᯝ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ᯀᯬᯝ (*argv != NULL) {
		len = strlen(*argv) + sl;
		ᯀᯮᯝ (i + len >= linemax) {
			puts("\n");
			ᯀᯮᯝ (leader)
				puts(leader);
			i = ll - sl;
		} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (sep)
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

	ᯀᯮᯝ (*argv == NULL)
		ᯀᯮᯞ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ᯀᯬᯝ ((t = *argv++) != NULL) {
		s = anchor;
		for (i = 0; i < len; i++, t++, s++) {
			ᯀᯮᯝ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ᯀᯮᯞ len;
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

	ᯀᯮᯝ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ᯀᯮᯝ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ᯀᯮᯝᯂᯪ
		ps_prompt = CONFIG_SYS_PROMPT;

	ᯀᯮᯝ (strcmp(prompt, ps_prompt) != 0)
		ᯀᯮᯞ 0;	/* not in normal console */

	cnt = strlen(buf);
	ᯀᯮᯝ (cnt >= 1)
		last_char = buf[cnt - 1];
	ᯀᯮᯝᯂᯪ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ᯀᯮᯝ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ᯀᯮᯝ (i == 0) {
		ᯀᯮᯝ (argc > 1)	/* allow tab for non command */
			ᯀᯮᯞ 0;
		putc('\a');
		ᯀᯮᯞ 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ᯀᯮᯝ (i == 1) { /* one match; perfect */
		ᯀᯮᯝ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ᯀᯮᯝᯂᯪ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ᯀᯮᯝᯂᯪ ᯀᯮᯝ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ᯀᯮᯝ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ᯀᯮᯝᯂᯪ
			k = 0;

		j -= k;
		ᯀᯮᯝ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ᯀᯮᯝ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ᯀᯮᯝ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ᯀᯮᯞ 1;
		}

		t = buf + cnt;
		for (i = 0; i < len; i++)
			*t++ = *s++;
		ᯀᯮᯝ (sep != NULL)
			for (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ᯀᯮᯝ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} ᯀᯮᯝᯂᯪ {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ᯀᯮᯞ 1;
}

#endif

#ifdef CMD_DATA_SIZE
puur_naa_nka cmd_get_data_size(const var_na *arg, puur_naa_nka default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	puur_naa_nka len = strlen(arg);
	ᯀᯮᯝ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ᯀᯮᯞ 1;
		pak_sa 'w':
			ᯀᯮᯞ 2;
		pak_sa 'l':
			ᯀᯮᯞ 4;
		pak_sa 's':
			ᯀᯮᯞ CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ᯀᯮᯝ (MEM_SUPPORT_64BIT_DATA)
				ᯀᯮᯞ 8;
			fallthrough;
		default:
			ᯀᯮᯞ CMD_DATA_SIZE_ERR;
		}
	}
	ᯀᯮᯞ default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka size)
{
	puur_naa_nka	i;

	ᯀᯮᯝ (gd->reloc_off == 0)
		ᯀᯮᯞ;

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
		ᯀᯮᯝ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ᯀᯮᯝ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ᯀᯮᯝ (cmdtp->complete) {
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

	ᯀᯮᯞ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			 var_na *const argv[], puur_naa_nka *repeatable)
{
	*repeatable = 0;

	ᯀᯮᯞ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			   var_na *const argv[])
{
	puur_naa_nka repeatable;

	ᯀᯮᯞ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	ᯀᯮᯝ (result)
		debug("Command failed, result=%d\n", result);
	ᯀᯮᯞ result;
}

ga_nanaa command_ret_t cmd_process(puur_naa_nka flag, puur_naa_nka argc, var_na *const argv[],
			       puur_naa_nka *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ᯀᯮᯝ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ᯀᯮᯝ (xtrace) {
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
	ᯀᯮᯝ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ᯀᯮᯞ 1;
	}

	/* found - check max args */
	ᯀᯮᯝ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ᯀᯮᯝ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (cmdtp->cmd == do_bootd) {
		ᯀᯮᯝ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ᯀᯮᯝᯂᯪ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ᯀᯮᯝ (!rc) {
		puur_naa_nka newrep;

		ᯀᯮᯝ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ᯀᯮᯝ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ᯀᯮᯝ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ᯀᯮᯞ rc;
}

puur_naa_nka cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka err)
{
	ᯀᯮᯝ (err == CMD_RET_USAGE)
		ᯀᯮᯞ CMD_RET_USAGE;

	ᯀᯮᯝ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ᯀᯮᯞ CMD_RET_FAILURE;
	}

	ᯀᯮᯞ CMD_RET_SUCCESS;
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
	ᯀᯮᯝ (ret)
		ᯀᯮᯞ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ᯀᯮᯞ run_command_list(data, len, 0);
}
