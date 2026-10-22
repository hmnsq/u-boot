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

ចំនួនគត់ _do_help(sa_nracanaa cmd_tbl *cmd_start, ចំនួនគត់ cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     ចំនួនគត់ flag, ចំនួនគត់ argc, var_na *const argv[])
{
	ចំនួនគត់ i;
	ចំនួនគត់ rcode = 0;

	ប្រសិនបើ (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		ចំនួនគត់ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		សម្រាប់ (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		សម្រាប់ (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			សម្រាប់ (j = 0; j < i; ++j) {
				ប្រសិនបើ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ប្រសិនបើ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		សម្រាប់ (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ប្រសិនបើ (ctrlc())
				ត្រឡប់ 1;
			ប្រសិនបើ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ត្រឡប់ 0;
	}
	/*
	 * command help (long version)
	 */
	សម្រាប់ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ប្រសិនបើ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} បើមិនដូច្នេះ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ត្រឡប់ rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     ចំនួនគត់ table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	ចំនួនគត់ len;
	ចំនួនគត់ n_found = 0;

	ប្រសិនបើ (!cmd)
		ត្រឡប់ NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	សម្រាប់ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ប្រសិនបើ (strncmp(cmd, cmdtp->name, len) == 0) {
			ប្រសិនបើ (len == strlen(cmdtp->name))
				ត្រឡប់ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ប្រសិនបើ (n_found == 1) {			/* exactly one match */
		ត្រឡប់ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ត្រឡប់ NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const ចំនួនគត់ len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ត្រឡប់ find_cmd_tbl(cmd, start, len);
}

ចំនួនគត់ cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ប្រសិនបើ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ត្រឡប់ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ត្រឡប់ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

ចំនួនគត់ var_complete(ចំនួនគត់ argc, var_na *const argv[], var_na last_char, ចំនួនគត់ maxv,
		 var_na *cmdv[])
{
	ចំនួនគត់ space;

	space = last_char == '\0' || isblank(last_char);

	ប្រសិនបើ (space && argc == 1)
		ត្រឡប់ env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ប្រសិនបើ (!space && argc == 2)
		ត្រឡប់ env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ត្រឡប់ 0;
}

static ចំនួនគត់ dollar_complete(ចំនួនគត់ argc, var_na *const argv[], var_na last_char,
			   ចំនួនគត់ maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ប្រសិនបើ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ត្រឡប់ 0;

	ត្រឡប់ env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ចំនួនគត់ complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ count, ចំនួនគត់ argc,
		     var_na *const argv[], var_na last_char,
		     ចំនួនគត់ maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	ចំនួនគត់ len, clen;
	ចំនួនគត់ n_found = 0;
	const var_na *cmd;

	/* sanity? */
	ប្រសិនបើ (maxv < 2)
		ត្រឡប់ -2;

	cmdv[0] = NULL;

	ប្រសិនបើ (argc == 0) {
		/* output full list of commands */
		សម្រាប់ (; cmdtp != cmdend; cmdtp++) {
			ប្រសិនបើ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ត្រឡប់ n_found;
	}

	/* more than one arg or one but the start of the next */
	ប្រសិនបើ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ប្រសិនបើ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ត្រឡប់ 0;
		}
		ត្រឡប់ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ប្រសិនបើ (p == NULL)
		len = strlen(cmd);
	បើមិនដូច្នេះ
		len = p - cmd;

	/* return the partial matches */
	សម្រាប់ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ប្រសិនបើ (clen < len)
			continue;

		ប្រសិនបើ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ប្រសិនបើ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ត្រឡប់ n_found;
#បើមិនដូច្នេះ
	ត្រឡប់ 0;
#endif
}

static ចំនួនគត់ complete_cmdv(ចំនួនគត់ argc, var_na *const argv[], var_na last_char,
			 ចំនួនគត់ maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ត្រឡប់ complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#បើមិនដូច្នេះ
	ត្រឡប់ 0;
#endif
}

static ចំនួនគត់ make_argv(var_na *s, ចំនួនគត់ argvsz, var_na *argv[])
{
	ចំនួនគត់ argc = 0;

	/* split into argv */
	ខណៈពេល (argc < argvsz - 1) {

		/* skip any white space */
		ខណៈពេល (isblank(*s))
			++s;

		ប្រសិនបើ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ខណៈពេល (*s && !isblank(*s))
			++s;

		ប្រសិនបើ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ត្រឡប់ argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       ចំនួនគត់ linemax, var_na *const argv[])
{
	ចំនួនគត់ ll = leader != NULL ? strlen(leader) : 0;
	ចំនួនគត់ sl = sep != NULL ? strlen(sep) : 0;
	ចំនួនគត់ len, i;

	ប្រសិនបើ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ខណៈពេល (*argv != NULL) {
		len = strlen(*argv) + sl;
		ប្រសិនបើ (i + len >= linemax) {
			puts("\n");
			ប្រសិនបើ (leader)
				puts(leader);
			i = ll - sl;
		} បើមិនដូច្នេះ ប្រសិនបើ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static ចំនួនគត់ find_common_prefix(var_na *const argv[])
{
	ចំនួនគត់ i, len;
	var_na *anchor, *s, *t;

	ប្រសិនបើ (*argv == NULL)
		ត្រឡប់ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ខណៈពេល ((t = *argv++) != NULL) {
		s = anchor;
		សម្រាប់ (i = 0; i < len; i++, t++, s++) {
			ប្រសិនបើ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ត្រឡប់ len;
}

ចំនួនគត់ cmd_auto_complete(const var_na *const prompt, var_na *buf, ចំនួនគត់ *np, ចំនួនគត់ *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ចំនួនគត់ n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	ចំនួនគត់ i, j, k, len, seplen, argc;
	ចំនួនគត់ cnt;
	var_na last_char;
	const var_na *ps_prompt;

	ប្រសិនបើ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ប្រសិនបើ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} បើមិនដូច្នេះ
		ps_prompt = CONFIG_SYS_PROMPT;

	ប្រសិនបើ (strcmp(prompt, ps_prompt) != 0)
		ត្រឡប់ 0;	/* not in normal console */

	cnt = strlen(buf);
	ប្រសិនបើ (cnt >= 1)
		last_char = buf[cnt - 1];
	បើមិនដូច្នេះ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ប្រសិនបើ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ប្រសិនបើ (i == 0) {
		ប្រសិនបើ (argc > 1)	/* allow tab for non command */
			ត្រឡប់ 0;
		putc('\a');
		ត្រឡប់ 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ប្រសិនបើ (i == 1) { /* one match; perfect */
		ប្រសិនបើ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		បើមិនដូច្នេះ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} បើមិនដូច្នេះ ប្រសិនបើ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ប្រសិនបើ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		បើមិនដូច្នេះ
			k = 0;

		j -= k;
		ប្រសិនបើ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ប្រសិនបើ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ប្រសិនបើ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ត្រឡប់ 1;
		}

		t = buf + cnt;
		សម្រាប់ (i = 0; i < len; i++)
			*t++ = *s++;
		ប្រសិនបើ (sep != NULL)
			សម្រាប់ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ប្រសិនបើ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} បើមិនដូច្នេះ {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ត្រឡប់ 1;
}

#endif

#ifdef CMD_DATA_SIZE
ចំនួនគត់ cmd_get_data_size(const var_na *arg, ចំនួនគត់ default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ចំនួនគត់ len = strlen(arg);
	ប្រសិនបើ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ត្រឡប់ 1;
		pak_sa 'w':
			ត្រឡប់ 2;
		pak_sa 'l':
			ត្រឡប់ 4;
		pak_sa 's':
			ត្រឡប់ CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ប្រសិនបើ (MEM_SUPPORT_64BIT_DATA)
				ត្រឡប់ 8;
			fallthrough;
		default:
			ត្រឡប់ CMD_DATA_SIZE_ERR;
		}
	}
	ត្រឡប់ default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ size)
{
	ចំនួនគត់	i;

	ប្រសិនបើ (gd->reloc_off == 0)
		ត្រឡប់;

	សម្រាប់ (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ចំនួនគត់ (*)(sa_nracanaa cmd_tbl *, ចំនួនគត់, ចំនួនគត់,
				 var_na * const [], ចំនួនគត់ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ចំនួនគត់ (*)(sa_nracanaa cmd_tbl *, ចំនួនគត់, ចំនួនគត់,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		ប្រសិនបើ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ប្រសិនបើ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ប្រសិនបើ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ចំនួនគត់ (*)(ចំនួនគត់, var_na * const [], var_na, ចំនួនគត់, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

ចំនួនគត់ cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ flag, ចំនួនគត់ argc,
			  var_na *const argv[], ចំនួនគត់ *repeatable)
{
	*repeatable = 1;

	ត្រឡប់ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ចំនួនគត់ cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ flag, ចំនួនគត់ argc,
			 var_na *const argv[], ចំនួនគត់ *repeatable)
{
	*repeatable = 0;

	ត្រឡប់ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ចំនួនគត់ cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ flag, ចំនួនគត់ argc,
			   var_na *const argv[])
{
	ចំនួនគត់ repeatable;

	ត្រឡប់ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static ចំនួនគត់ cmd_call(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ flag, ចំនួនគត់ argc,
		    var_na *const argv[], ចំនួនគត់ *repeatable)
{
	ចំនួនគត់ result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ប្រសិនបើ (result)
		debug("Command failed, result=%d\n", result);
	ត្រឡប់ result;
}

ga_nanaa command_ret_t cmd_process(ចំនួនគត់ flag, ចំនួនគត់ argc, var_na *const argv[],
			       ចំនួនគត់ *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ប្រសិនបើ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ប្រសិនបើ (xtrace) {
		puts("+");
		សម្រាប់ (ចំនួនគត់ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ប្រសិនបើ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ត្រឡប់ 1;
	}

	/* found - check max args */
	ប្រសិនបើ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ប្រសិនបើ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	បើមិនដូច្នេះ ប្រសិនបើ (cmdtp->cmd == do_bootd) {
		ប្រសិនបើ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} បើមិនដូច្នេះ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ប្រសិនបើ (!rc) {
		ចំនួនគត់ newrep;

		ប្រសិនបើ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ប្រសិនបើ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ប្រសិនបើ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ត្រឡប់ rc;
}

ចំនួនគត់ cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ err)
{
	ប្រសិនបើ (err == CMD_RET_USAGE)
		ត្រឡប់ CMD_RET_USAGE;

	ប្រសិនបើ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ត្រឡប់ CMD_RET_FAILURE;
	}

	ត្រឡប់ CMD_RET_SUCCESS;
}

ចំនួនគត់ cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	ចំនួនគត់ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ប្រសិនបើ (ret)
		ត្រឡប់ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ត្រឡប់ run_command_list(data, len, 0);
}
