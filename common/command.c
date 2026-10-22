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

ຈຳນວນເຕັມ _do_help(sa_nracanaa cmd_tbl *cmd_start, ຈຳນວນເຕັມ cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     ຈຳນວນເຕັມ flag, ຈຳນວນເຕັມ argc, var_na *const argv[])
{
	ຈຳນວນເຕັມ i;
	ຈຳນວນເຕັມ rcode = 0;

	ຖ້າ (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		ຈຳນວນເຕັມ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		ສຳລັບ (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ສຳລັບ (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			ສຳລັບ (j = 0; j < i; ++j) {
				ຖ້າ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ຖ້າ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		ສຳລັບ (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ຖ້າ (ctrlc())
				ສົ່ງຄືນ 1;
			ຖ້າ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ສົ່ງຄືນ 0;
	}
	/*
	 * command help (long version)
	 */
	ສຳລັບ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ຖ້າ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} ບໍ່ດັ່ງນັ້ນ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ສົ່ງຄືນ rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     ຈຳນວນເຕັມ table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	ຈຳນວນເຕັມ len;
	ຈຳນວນເຕັມ n_found = 0;

	ຖ້າ (!cmd)
		ສົ່ງຄືນ NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	ສຳລັບ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ຖ້າ (strncmp(cmd, cmdtp->name, len) == 0) {
			ຖ້າ (len == strlen(cmdtp->name))
				ສົ່ງຄືນ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ຖ້າ (n_found == 1) {			/* exactly one match */
		ສົ່ງຄືນ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ສົ່ງຄືນ NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const ຈຳນວນເຕັມ len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ສົ່ງຄືນ find_cmd_tbl(cmd, start, len);
}

ຈຳນວນເຕັມ cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ຖ້າ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ສົ່ງຄືນ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ສົ່ງຄືນ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

ຈຳນວນເຕັມ var_complete(ຈຳນວນເຕັມ argc, var_na *const argv[], var_na last_char, ຈຳນວນເຕັມ maxv,
		 var_na *cmdv[])
{
	ຈຳນວນເຕັມ space;

	space = last_char == '\0' || isblank(last_char);

	ຖ້າ (space && argc == 1)
		ສົ່ງຄືນ env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ຖ້າ (!space && argc == 2)
		ສົ່ງຄືນ env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ dollar_complete(ຈຳນວນເຕັມ argc, var_na *const argv[], var_na last_char,
			   ຈຳນວນເຕັມ maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ຖ້າ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ສົ່ງຄືນ 0;

	ສົ່ງຄືນ env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ຈຳນວນເຕັມ complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ count, ຈຳນວນເຕັມ argc,
		     var_na *const argv[], var_na last_char,
		     ຈຳນວນເຕັມ maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	ຈຳນວນເຕັມ len, clen;
	ຈຳນວນເຕັມ n_found = 0;
	const var_na *cmd;

	/* sanity? */
	ຖ້າ (maxv < 2)
		ສົ່ງຄືນ -2;

	cmdv[0] = NULL;

	ຖ້າ (argc == 0) {
		/* output full list of commands */
		ສຳລັບ (; cmdtp != cmdend; cmdtp++) {
			ຖ້າ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ສົ່ງຄືນ n_found;
	}

	/* more than one arg or one but the start of the next */
	ຖ້າ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ຖ້າ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ສົ່ງຄືນ 0;
		}
		ສົ່ງຄືນ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ຖ້າ (p == NULL)
		len = strlen(cmd);
	ບໍ່ດັ່ງນັ້ນ
		len = p - cmd;

	/* return the partial matches */
	ສຳລັບ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ຖ້າ (clen < len)
			continue;

		ຖ້າ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ຖ້າ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ສົ່ງຄືນ n_found;
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ 0;
#endif
}

static ຈຳນວນເຕັມ complete_cmdv(ຈຳນວນເຕັມ argc, var_na *const argv[], var_na last_char,
			 ຈຳນວນເຕັມ maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ສົ່ງຄືນ complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ບໍ່ດັ່ງນັ້ນ
	ສົ່ງຄືນ 0;
#endif
}

static ຈຳນວນເຕັມ make_argv(var_na *s, ຈຳນວນເຕັມ argvsz, var_na *argv[])
{
	ຈຳນວນເຕັມ argc = 0;

	/* split into argv */
	ໃນຂະນະທີ່ (argc < argvsz - 1) {

		/* skip any white space */
		ໃນຂະນະທີ່ (isblank(*s))
			++s;

		ຖ້າ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ໃນຂະນະທີ່ (*s && !isblank(*s))
			++s;

		ຖ້າ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ສົ່ງຄືນ argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       ຈຳນວນເຕັມ linemax, var_na *const argv[])
{
	ຈຳນວນເຕັມ ll = leader != NULL ? strlen(leader) : 0;
	ຈຳນວນເຕັມ sl = sep != NULL ? strlen(sep) : 0;
	ຈຳນວນເຕັມ len, i;

	ຖ້າ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ໃນຂະນະທີ່ (*argv != NULL) {
		len = strlen(*argv) + sl;
		ຖ້າ (i + len >= linemax) {
			puts("\n");
			ຖ້າ (leader)
				puts(leader);
			i = ll - sl;
		} ບໍ່ດັ່ງນັ້ນ ຖ້າ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static ຈຳນວນເຕັມ find_common_prefix(var_na *const argv[])
{
	ຈຳນວນເຕັມ i, len;
	var_na *anchor, *s, *t;

	ຖ້າ (*argv == NULL)
		ສົ່ງຄືນ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ໃນຂະນະທີ່ ((t = *argv++) != NULL) {
		s = anchor;
		ສຳລັບ (i = 0; i < len; i++, t++, s++) {
			ຖ້າ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ສົ່ງຄືນ len;
}

ຈຳນວນເຕັມ cmd_auto_complete(const var_na *const prompt, var_na *buf, ຈຳນວນເຕັມ *np, ຈຳນວນເຕັມ *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ຈຳນວນເຕັມ n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	ຈຳນວນເຕັມ i, j, k, len, seplen, argc;
	ຈຳນວນເຕັມ cnt;
	var_na last_char;
	const var_na *ps_prompt;

	ຖ້າ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ຖ້າ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ບໍ່ດັ່ງນັ້ນ
		ps_prompt = CONFIG_SYS_PROMPT;

	ຖ້າ (strcmp(prompt, ps_prompt) != 0)
		ສົ່ງຄືນ 0;	/* not in normal console */

	cnt = strlen(buf);
	ຖ້າ (cnt >= 1)
		last_char = buf[cnt - 1];
	ບໍ່ດັ່ງນັ້ນ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ຖ້າ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ຖ້າ (i == 0) {
		ຖ້າ (argc > 1)	/* allow tab for non command */
			ສົ່ງຄືນ 0;
		putc('\a');
		ສົ່ງຄືນ 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ຖ້າ (i == 1) { /* one match; perfect */
		ຖ້າ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ບໍ່ດັ່ງນັ້ນ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ບໍ່ດັ່ງນັ້ນ ຖ້າ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ຖ້າ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ບໍ່ດັ່ງນັ້ນ
			k = 0;

		j -= k;
		ຖ້າ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ຖ້າ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ຖ້າ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ສົ່ງຄືນ 1;
		}

		t = buf + cnt;
		ສຳລັບ (i = 0; i < len; i++)
			*t++ = *s++;
		ຖ້າ (sep != NULL)
			ສຳລັບ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ຖ້າ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} ບໍ່ດັ່ງນັ້ນ {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ສົ່ງຄືນ 1;
}

#endif

#ifdef CMD_DATA_SIZE
ຈຳນວນເຕັມ cmd_get_data_size(const var_na *arg, ຈຳນວນເຕັມ default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ຈຳນວນເຕັມ len = strlen(arg);
	ຖ້າ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ສົ່ງຄືນ 1;
		pak_sa 'w':
			ສົ່ງຄືນ 2;
		pak_sa 'l':
			ສົ່ງຄືນ 4;
		pak_sa 's':
			ສົ່ງຄືນ CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ຖ້າ (MEM_SUPPORT_64BIT_DATA)
				ສົ່ງຄືນ 8;
			fallthrough;
		default:
			ສົ່ງຄືນ CMD_DATA_SIZE_ERR;
		}
	}
	ສົ່ງຄືນ default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ size)
{
	ຈຳນວນເຕັມ	i;

	ຖ້າ (gd->reloc_off == 0)
		ສົ່ງຄືນ;

	ສຳລັບ (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ຈຳນວນເຕັມ (*)(sa_nracanaa cmd_tbl *, ຈຳນວນເຕັມ, ຈຳນວນເຕັມ,
				 var_na * const [], ຈຳນວນເຕັມ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ຈຳນວນເຕັມ (*)(sa_nracanaa cmd_tbl *, ຈຳນວນເຕັມ, ຈຳນວນເຕັມ,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		ຖ້າ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ຖ້າ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ຖ້າ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ຈຳນວນເຕັມ (*)(ຈຳນວນເຕັມ, var_na * const [], var_na, ຈຳນວນເຕັມ, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

ຈຳນວນເຕັມ cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ flag, ຈຳນວນເຕັມ argc,
			  var_na *const argv[], ຈຳນວນເຕັມ *repeatable)
{
	*repeatable = 1;

	ສົ່ງຄືນ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ຈຳນວນເຕັມ cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ flag, ຈຳນວນເຕັມ argc,
			 var_na *const argv[], ຈຳນວນເຕັມ *repeatable)
{
	*repeatable = 0;

	ສົ່ງຄືນ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ຈຳນວນເຕັມ cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ flag, ຈຳນວນເຕັມ argc,
			   var_na *const argv[])
{
	ຈຳນວນເຕັມ repeatable;

	ສົ່ງຄືນ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static ຈຳນວນເຕັມ cmd_call(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ flag, ຈຳນວນເຕັມ argc,
		    var_na *const argv[], ຈຳນວນເຕັມ *repeatable)
{
	ຈຳນວນເຕັມ result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ຖ້າ (result)
		debug("Command failed, result=%d\n", result);
	ສົ່ງຄືນ result;
}

ga_nanaa command_ret_t cmd_process(ຈຳນວນເຕັມ flag, ຈຳນວນເຕັມ argc, var_na *const argv[],
			       ຈຳນວນເຕັມ *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ຖ້າ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ຖ້າ (xtrace) {
		puts("+");
		ສຳລັບ (ຈຳນວນເຕັມ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ຖ້າ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ສົ່ງຄືນ 1;
	}

	/* found - check max args */
	ຖ້າ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ຖ້າ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ບໍ່ດັ່ງນັ້ນ ຖ້າ (cmdtp->cmd == do_bootd) {
		ຖ້າ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ບໍ່ດັ່ງນັ້ນ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ຖ້າ (!rc) {
		ຈຳນວນເຕັມ newrep;

		ຖ້າ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ຖ້າ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ຖ້າ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ສົ່ງຄືນ rc;
}

ຈຳນວນເຕັມ cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, ຈຳນວນເຕັມ err)
{
	ຖ້າ (err == CMD_RET_USAGE)
		ສົ່ງຄືນ CMD_RET_USAGE;

	ຖ້າ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ສົ່ງຄືນ CMD_RET_FAILURE;
	}

	ສົ່ງຄືນ CMD_RET_SUCCESS;
}

ຈຳນວນເຕັມ cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	ຈຳນວນເຕັມ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ຖ້າ (ret)
		ສົ່ງຄືນ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ສົ່ງຄືນ run_command_list(data, len, 0);
}
