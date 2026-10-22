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

	ꦪꦼꦤ꧀ (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		puur_naa_nka i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		ꦏꦁꦒꦺꦴ (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ꦏꦁꦒꦺꦴ (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			ꦏꦁꦒꦺꦴ (j = 0; j < i; ++j) {
				ꦪꦼꦤ꧀ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ꦪꦼꦤ꧀ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		ꦏꦁꦒꦺꦴ (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ꦪꦼꦤ꧀ (ctrlc())
				ꦮꦭꦶ 1;
			ꦪꦼꦤ꧀ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ꦮꦭꦶ 0;
	}
	/*
	 * command help (long version)
	 */
	ꦏꦁꦒꦺꦴ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ꦪꦼꦤ꧀ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} ꦭꦶꦪ {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ꦮꦭꦶ rcode;
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

	ꦪꦼꦤ꧀ (!cmd)
		ꦮꦭꦶ NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	ꦏꦁꦒꦺꦴ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ꦪꦼꦤ꧀ (strncmp(cmd, cmdtp->name, len) == 0) {
			ꦪꦼꦤ꧀ (len == strlen(cmdtp->name))
				ꦮꦭꦶ cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ꦪꦼꦤ꧀ (n_found == 1) {			/* exactly one match */
		ꦮꦭꦶ cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ꦮꦭꦶ NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const puur_naa_nka len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ꦮꦭꦶ find_cmd_tbl(cmd, start, len);
}

puur_naa_nka cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ꦪꦼꦤ꧀ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ꦮꦭꦶ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ꦮꦭꦶ 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

puur_naa_nka var_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char, puur_naa_nka maxv,
		 var_na *cmdv[])
{
	puur_naa_nka space;

	space = last_char == '\0' || isblank(last_char);

	ꦪꦼꦤ꧀ (space && argc == 1)
		ꦮꦭꦶ env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ꦪꦼꦤ꧀ (!space && argc == 2)
		ꦮꦭꦶ env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ꦮꦭꦶ 0;
}

static puur_naa_nka dollar_complete(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			   puur_naa_nka maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ꦪꦼꦤ꧀ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ꦮꦭꦶ 0;

	ꦮꦭꦶ env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
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
	ꦪꦼꦤ꧀ (maxv < 2)
		ꦮꦭꦶ -2;

	cmdv[0] = NULL;

	ꦪꦼꦤ꧀ (argc == 0) {
		/* output full list of commands */
		ꦏꦁꦒꦺꦴ (; cmdtp != cmdend; cmdtp++) {
			ꦪꦼꦤ꧀ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ꦮꦭꦶ n_found;
	}

	/* more than one arg or one but the start of the next */
	ꦪꦼꦤ꧀ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ꦪꦼꦤ꧀ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ꦮꦭꦶ 0;
		}
		ꦮꦭꦶ (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ꦪꦼꦤ꧀ (p == NULL)
		len = strlen(cmd);
	ꦭꦶꦪ
		len = p - cmd;

	/* return the partial matches */
	ꦏꦁꦒꦺꦴ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ꦪꦼꦤ꧀ (clen < len)
			continue;

		ꦪꦼꦤ꧀ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ꦪꦼꦤ꧀ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ꦮꦭꦶ n_found;
#ꦭꦶꦪ
	ꦮꦭꦶ 0;
#endif
}

static puur_naa_nka complete_cmdv(puur_naa_nka argc, var_na *const argv[], var_na last_char,
			 puur_naa_nka maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ꦮꦭꦶ complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#ꦭꦶꦪ
	ꦮꦭꦶ 0;
#endif
}

static puur_naa_nka make_argv(var_na *s, puur_naa_nka argvsz, var_na *argv[])
{
	puur_naa_nka argc = 0;

	/* split into argv */
	ꦱꦼꦢꦼꦭ (argc < argvsz - 1) {

		/* skip any white space */
		ꦱꦼꦢꦼꦭ (isblank(*s))
			++s;

		ꦪꦼꦤ꧀ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ꦱꦼꦢꦼꦭ (*s && !isblank(*s))
			++s;

		ꦪꦼꦤ꧀ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ꦮꦭꦶ argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       puur_naa_nka linemax, var_na *const argv[])
{
	puur_naa_nka ll = leader != NULL ? strlen(leader) : 0;
	puur_naa_nka sl = sep != NULL ? strlen(sep) : 0;
	puur_naa_nka len, i;

	ꦪꦼꦤ꧀ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ꦱꦼꦢꦼꦭ (*argv != NULL) {
		len = strlen(*argv) + sl;
		ꦪꦼꦤ꧀ (i + len >= linemax) {
			puts("\n");
			ꦪꦼꦤ꧀ (leader)
				puts(leader);
			i = ll - sl;
		} ꦭꦶꦪ ꦪꦼꦤ꧀ (sep)
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

	ꦪꦼꦤ꧀ (*argv == NULL)
		ꦮꦭꦶ 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ꦱꦼꦢꦼꦭ ((t = *argv++) != NULL) {
		s = anchor;
		ꦏꦁꦒꦺꦴ (i = 0; i < len; i++, t++, s++) {
			ꦪꦼꦤ꧀ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ꦮꦭꦶ len;
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

	ꦪꦼꦤ꧀ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ꦪꦼꦤ꧀ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ꦭꦶꦪ
		ps_prompt = CONFIG_SYS_PROMPT;

	ꦪꦼꦤ꧀ (strcmp(prompt, ps_prompt) != 0)
		ꦮꦭꦶ 0;	/* not in normal console */

	cnt = strlen(buf);
	ꦪꦼꦤ꧀ (cnt >= 1)
		last_char = buf[cnt - 1];
	ꦭꦶꦪ
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ꦪꦼꦤ꧀ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ꦪꦼꦤ꧀ (i == 0) {
		ꦪꦼꦤ꧀ (argc > 1)	/* allow tab for non command */
			ꦮꦭꦶ 0;
		putc('\a');
		ꦮꦭꦶ 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ꦪꦼꦤ꧀ (i == 1) { /* one match; perfect */
		ꦪꦼꦤ꧀ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ꦭꦶꦪ
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} ꦭꦶꦪ ꦪꦼꦤ꧀ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ꦪꦼꦤ꧀ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		ꦭꦶꦪ
			k = 0;

		j -= k;
		ꦪꦼꦤ꧀ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ꦪꦼꦤ꧀ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ꦪꦼꦤ꧀ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ꦮꦭꦶ 1;
		}

		t = buf + cnt;
		ꦏꦁꦒꦺꦴ (i = 0; i < len; i++)
			*t++ = *s++;
		ꦪꦼꦤ꧀ (sep != NULL)
			ꦏꦁꦒꦺꦴ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ꦪꦼꦤ꧀ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} ꦭꦶꦪ {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ꦮꦭꦶ 1;
}

#endif

#ifdef CMD_DATA_SIZE
puur_naa_nka cmd_get_data_size(const var_na *arg, puur_naa_nka default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	puur_naa_nka len = strlen(arg);
	ꦪꦼꦤ꧀ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ꦮꦭꦶ 1;
		pak_sa 'w':
			ꦮꦭꦶ 2;
		pak_sa 'l':
			ꦮꦭꦶ 4;
		pak_sa 's':
			ꦮꦭꦶ CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ꦪꦼꦤ꧀ (MEM_SUPPORT_64BIT_DATA)
				ꦮꦭꦶ 8;
			fallthrough;
		default:
			ꦮꦭꦶ CMD_DATA_SIZE_ERR;
		}
	}
	ꦮꦭꦶ default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka size)
{
	puur_naa_nka	i;

	ꦪꦼꦤ꧀ (gd->reloc_off == 0)
		ꦮꦭꦶ;

	ꦏꦁꦒꦺꦴ (i = 0; i < size; i++) {
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
		ꦪꦼꦤ꧀ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ꦪꦼꦤ꧀ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ꦪꦼꦤ꧀ (cmdtp->complete) {
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

	ꦮꦭꦶ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			 var_na *const argv[], puur_naa_nka *repeatable)
{
	*repeatable = 0;

	ꦮꦭꦶ cmdtp->cmd(cmdtp, flag, argc, argv);
}

puur_naa_nka cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
			   var_na *const argv[])
{
	puur_naa_nka repeatable;

	ꦮꦭꦶ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
	ꦪꦼꦤ꧀ (result)
		debug("Command failed, result=%d\n", result);
	ꦮꦭꦶ result;
}

ga_nanaa command_ret_t cmd_process(puur_naa_nka flag, puur_naa_nka argc, var_na *const argv[],
			       puur_naa_nka *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ꦪꦼꦤ꧀ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ꦪꦼꦤ꧀ (xtrace) {
		puts("+");
		ꦏꦁꦒꦺꦴ (puur_naa_nka i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ꦪꦼꦤ꧀ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ꦮꦭꦶ 1;
	}

	/* found - check max args */
	ꦪꦼꦤ꧀ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ꦪꦼꦤ꧀ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ꦭꦶꦪ ꦪꦼꦤ꧀ (cmdtp->cmd == do_bootd) {
		ꦪꦼꦤ꧀ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ꦭꦶꦪ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ꦪꦼꦤ꧀ (!rc) {
		puur_naa_nka newrep;

		ꦪꦼꦤ꧀ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ꦪꦼꦤ꧀ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ꦪꦼꦤ꧀ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ꦮꦭꦶ rc;
}

puur_naa_nka cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka err)
{
	ꦪꦼꦤ꧀ (err == CMD_RET_USAGE)
		ꦮꦭꦶ CMD_RET_USAGE;

	ꦪꦼꦤ꧀ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ꦮꦭꦶ CMD_RET_FAILURE;
	}

	ꦮꦭꦶ CMD_RET_SUCCESS;
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
	ꦪꦼꦤ꧀ (ret)
		ꦮꦭꦶ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ꦮꦭꦶ run_command_list(data, len, 0);
}
