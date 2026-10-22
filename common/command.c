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

ཧྲིལ _do_help(sa_nracanaa cmd_tbl *cmd_start, ཧྲིལ cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     ཧྲིལ flag, ཧྲིལ argc, var_na *const argv[])
{
	ཧྲིལ i;
	ཧྲིལ rcode = 0;

	གལ་ཏེ (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		ཧྲིལ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		ཆེད་དུ (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ཆེད་དུ (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			ཆེད་དུ (j = 0; j < i; ++j) {
				གལ་ཏེ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			གལ་ཏེ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		ཆེད་དུ (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			གལ་ཏེ (ctrlc())
				ཕྱིར 1;
			གལ་ཏེ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ཕྱིར 0;
	}
	/*
	 * command help (long version)
	 */
	ཆེད་དུ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		གལ་ཏེ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} དེ་མིན {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ཕྱིར rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     ཧྲིལ table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	ཧྲིལ len;
	ཧྲིལ n_found = 0;

	གལ་ཏེ (!cmd)
		ཕྱིར NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	ཆེད་དུ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		གལ་ཏེ (strncmp(cmd, cmdtp->name, len) == 0) {
			གལ་ཏེ (len == strlen(cmdtp->name))
				ཕྱིར cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	གལ་ཏེ (n_found == 1) {			/* exactly one match */
		ཕྱིར cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ཕྱིར NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const ཧྲིལ len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ཕྱིར find_cmd_tbl(cmd, start, len);
}

ཧྲིལ cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	གལ་ཏེ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ཕྱིར 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ཕྱིར 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

ཧྲིལ var_complete(ཧྲིལ argc, var_na *const argv[], var_na last_char, ཧྲིལ maxv,
		 var_na *cmdv[])
{
	ཧྲིལ space;

	space = last_char == '\0' || isblank(last_char);

	གལ་ཏེ (space && argc == 1)
		ཕྱིར env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	གལ་ཏེ (!space && argc == 2)
		ཕྱིར env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ཕྱིར 0;
}

static ཧྲིལ dollar_complete(ཧྲིལ argc, var_na *const argv[], var_na last_char,
			   ཧྲིལ maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	གལ་ཏེ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ཕྱིར 0;

	ཕྱིར env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ཧྲིལ complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ count, ཧྲིལ argc,
		     var_na *const argv[], var_na last_char,
		     ཧྲིལ maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	ཧྲིལ len, clen;
	ཧྲིལ n_found = 0;
	const var_na *cmd;

	/* sanity? */
	གལ་ཏེ (maxv < 2)
		ཕྱིར -2;

	cmdv[0] = NULL;

	གལ་ཏེ (argc == 0) {
		/* output full list of commands */
		ཆེད་དུ (; cmdtp != cmdend; cmdtp++) {
			གལ་ཏེ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ཕྱིར n_found;
	}

	/* more than one arg or one but the start of the next */
	གལ་ཏེ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		གལ་ཏེ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ཕྱིར 0;
		}
		ཕྱིར (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	གལ་ཏེ (p == NULL)
		len = strlen(cmd);
	དེ་མིན
		len = p - cmd;

	/* return the partial matches */
	ཆེད་དུ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		གལ་ཏེ (clen < len)
			continue;

		གལ་ཏེ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		གལ་ཏེ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ཕྱིར n_found;
#དེ་མིན
	ཕྱིར 0;
#endif
}

static ཧྲིལ complete_cmdv(ཧྲིལ argc, var_na *const argv[], var_na last_char,
			 ཧྲིལ maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ཕྱིར complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#དེ་མིན
	ཕྱིར 0;
#endif
}

static ཧྲིལ make_argv(var_na *s, ཧྲིལ argvsz, var_na *argv[])
{
	ཧྲིལ argc = 0;

	/* split into argv */
	སྐབས (argc < argvsz - 1) {

		/* skip any white space */
		སྐབས (isblank(*s))
			++s;

		གལ་ཏེ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		སྐབས (*s && !isblank(*s))
			++s;

		གལ་ཏེ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ཕྱིར argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       ཧྲིལ linemax, var_na *const argv[])
{
	ཧྲིལ ll = leader != NULL ? strlen(leader) : 0;
	ཧྲིལ sl = sep != NULL ? strlen(sep) : 0;
	ཧྲིལ len, i;

	གལ་ཏེ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	སྐབས (*argv != NULL) {
		len = strlen(*argv) + sl;
		གལ་ཏེ (i + len >= linemax) {
			puts("\n");
			གལ་ཏེ (leader)
				puts(leader);
			i = ll - sl;
		} དེ་མིན གལ་ཏེ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static ཧྲིལ find_common_prefix(var_na *const argv[])
{
	ཧྲིལ i, len;
	var_na *anchor, *s, *t;

	གལ་ཏེ (*argv == NULL)
		ཕྱིར 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	སྐབས ((t = *argv++) != NULL) {
		s = anchor;
		ཆེད་དུ (i = 0; i < len; i++, t++, s++) {
			གལ་ཏེ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ཕྱིར len;
}

ཧྲིལ cmd_auto_complete(const var_na *const prompt, var_na *buf, ཧྲིལ *np, ཧྲིལ *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ཧྲིལ n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	ཧྲིལ i, j, k, len, seplen, argc;
	ཧྲིལ cnt;
	var_na last_char;
	const var_na *ps_prompt;

	གལ་ཏེ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		གལ་ཏེ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} དེ་མིན
		ps_prompt = CONFIG_SYS_PROMPT;

	གལ་ཏེ (strcmp(prompt, ps_prompt) != 0)
		ཕྱིར 0;	/* not in normal console */

	cnt = strlen(buf);
	གལ་ཏེ (cnt >= 1)
		last_char = buf[cnt - 1];
	དེ་མིན
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	གལ་ཏེ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	གལ་ཏེ (i == 0) {
		གལ་ཏེ (argc > 1)	/* allow tab for non command */
			ཕྱིར 0;
		putc('\a');
		ཕྱིར 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	གལ་ཏེ (i == 1) { /* one match; perfect */
		གལ་ཏེ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		དེ་མིན
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} དེ་མིན གལ་ཏེ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		གལ་ཏེ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		དེ་མིན
			k = 0;

		j -= k;
		གལ་ཏེ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	གལ་ཏེ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		གལ་ཏེ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ཕྱིར 1;
		}

		t = buf + cnt;
		ཆེད་དུ (i = 0; i < len; i++)
			*t++ = *s++;
		གལ་ཏེ (sep != NULL)
			ཆེད་དུ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		གལ་ཏེ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} དེ་མིན {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ཕྱིར 1;
}

#endif

#ifdef CMD_DATA_SIZE
ཧྲིལ cmd_get_data_size(const var_na *arg, ཧྲིལ default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ཧྲིལ len = strlen(arg);
	གལ་ཏེ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ཕྱིར 1;
		pak_sa 'w':
			ཕྱིར 2;
		pak_sa 'l':
			ཕྱིར 4;
		pak_sa 's':
			ཕྱིར CMD_DATA_SIZE_STR;
		pak_sa 'q':
			གལ་ཏེ (MEM_SUPPORT_64BIT_DATA)
				ཕྱིར 8;
			fallthrough;
		default:
			ཕྱིར CMD_DATA_SIZE_ERR;
		}
	}
	ཕྱིར default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ size)
{
	ཧྲིལ	i;

	གལ་ཏེ (gd->reloc_off == 0)
		ཕྱིར;

	ཆེད་དུ (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ཧྲིལ (*)(sa_nracanaa cmd_tbl *, ཧྲིལ, ཧྲིལ,
				 var_na * const [], ཧྲིལ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ཧྲིལ (*)(sa_nracanaa cmd_tbl *, ཧྲིལ, ཧྲིལ,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		གལ་ཏེ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		གལ་ཏེ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		གལ་ཏེ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ཧྲིལ (*)(ཧྲིལ, var_na * const [], var_na, ཧྲིལ, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

ཧྲིལ cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ flag, ཧྲིལ argc,
			  var_na *const argv[], ཧྲིལ *repeatable)
{
	*repeatable = 1;

	ཕྱིར cmdtp->cmd(cmdtp, flag, argc, argv);
}

ཧྲིལ cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ flag, ཧྲིལ argc,
			 var_na *const argv[], ཧྲིལ *repeatable)
{
	*repeatable = 0;

	ཕྱིར cmdtp->cmd(cmdtp, flag, argc, argv);
}

ཧྲིལ cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ flag, ཧྲིལ argc,
			   var_na *const argv[])
{
	ཧྲིལ repeatable;

	ཕྱིར cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static ཧྲིལ cmd_call(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ flag, ཧྲིལ argc,
		    var_na *const argv[], ཧྲིལ *repeatable)
{
	ཧྲིལ result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	གལ་ཏེ (result)
		debug("Command failed, result=%d\n", result);
	ཕྱིར result;
}

ga_nanaa command_ret_t cmd_process(ཧྲིལ flag, ཧྲིལ argc, var_na *const argv[],
			       ཧྲིལ *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#གལ་ཏེ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	གལ་ཏེ (xtrace) {
		puts("+");
		ཆེད་དུ (ཧྲིལ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	གལ་ཏེ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ཕྱིར 1;
	}

	/* found - check max args */
	གལ་ཏེ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#གལ་ཏེ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	དེ་མིན གལ་ཏེ (cmdtp->cmd == do_bootd) {
		གལ་ཏེ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} དེ་མིན {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	གལ་ཏེ (!rc) {
		ཧྲིལ newrep;

		གལ་ཏེ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		གལ་ཏེ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	གལ་ཏེ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ཕྱིར rc;
}

ཧྲིལ cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, ཧྲིལ err)
{
	གལ་ཏེ (err == CMD_RET_USAGE)
		ཕྱིར CMD_RET_USAGE;

	གལ་ཏེ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ཕྱིར CMD_RET_FAILURE;
	}

	ཕྱིར CMD_RET_SUCCESS;
}

ཧྲིལ cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	ཧྲིལ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	གལ་ཏེ (ret)
		ཕྱིར CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ཕྱིར run_command_list(data, len, 0);
}
