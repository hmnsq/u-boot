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

පූර්ණ_සංඛ්‍යා _do_help(sa_nracanaa cmd_tbl *cmd_start, පූර්ණ_සංඛ්‍යා cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc, var_na *const argv[])
{
	පූර්ණ_සංඛ්‍යා i;
	පූර්ණ_සංඛ්‍යා rcode = 0;

	නම් (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		පූර්ණ_සංඛ්‍යා i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		සඳහා (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		සඳහා (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			සඳහා (j = 0; j < i; ++j) {
				නම් (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			නම් (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		සඳහා (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			නම් (ctrlc())
				ආපසු 1;
			නම් (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ආපසු 0;
	}
	/*
	 * command help (long version)
	 */
	සඳහා (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		නම් (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} නැතහොත් {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ආපසු rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     පූර්ණ_සංඛ්‍යා table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	පූර්ණ_සංඛ්‍යා len;
	පූර්ණ_සංඛ්‍යා n_found = 0;

	නම් (!cmd)
		ආපසු NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	සඳහා (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		නම් (strncmp(cmd, cmdtp->name, len) == 0) {
			නම් (len == strlen(cmdtp->name))
				ආපසු cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	නම් (n_found == 1) {			/* exactly one match */
		ආපසු cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ආපසු NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const පූර්ණ_සංඛ්‍යා len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ආපසු find_cmd_tbl(cmd, start, len);
}

පූර්ණ_සංඛ්‍යා cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	නම් (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ආපසු 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ආපසු 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

පූර්ණ_සංඛ්‍යා var_complete(පූර්ණ_සංඛ්‍යා argc, var_na *const argv[], var_na last_char, පූර්ණ_සංඛ්‍යා maxv,
		 var_na *cmdv[])
{
	පූර්ණ_සංඛ්‍යා space;

	space = last_char == '\0' || isblank(last_char);

	නම් (space && argc == 1)
		ආපසු env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	නම් (!space && argc == 2)
		ආපසු env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා dollar_complete(පූර්ණ_සංඛ්‍යා argc, var_na *const argv[], var_na last_char,
			   පූර්ණ_සංඛ්‍යා maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	නම් (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ආපසු 0;

	ආපසු env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

පූර්ණ_සංඛ්‍යා complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා count, පූර්ණ_සංඛ්‍යා argc,
		     var_na *const argv[], var_na last_char,
		     පූර්ණ_සංඛ්‍යා maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	පූර්ණ_සංඛ්‍යා len, clen;
	පූර්ණ_සංඛ්‍යා n_found = 0;
	const var_na *cmd;

	/* sanity? */
	නම් (maxv < 2)
		ආපසු -2;

	cmdv[0] = NULL;

	නම් (argc == 0) {
		/* output full list of commands */
		සඳහා (; cmdtp != cmdend; cmdtp++) {
			නම් (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ආපසු n_found;
	}

	/* more than one arg or one but the start of the next */
	නම් (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		නම් (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ආපසු 0;
		}
		ආපසු (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	නම් (p == NULL)
		len = strlen(cmd);
	නැතහොත්
		len = p - cmd;

	/* return the partial matches */
	සඳහා (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		නම් (clen < len)
			continue;

		නම් (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		නම් (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ආපසු n_found;
#නැතහොත්
	ආපසු 0;
#endif
}

static පූර්ණ_සංඛ්‍යා complete_cmdv(පූර්ණ_සංඛ්‍යා argc, var_na *const argv[], var_na last_char,
			 පූර්ණ_සංඛ්‍යා maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ආපසු complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#නැතහොත්
	ආපසු 0;
#endif
}

static පූර්ණ_සංඛ්‍යා make_argv(var_na *s, පූර්ණ_සංඛ්‍යා argvsz, var_na *argv[])
{
	පූර්ණ_සංඛ්‍යා argc = 0;

	/* split into argv */
	අතරතුර (argc < argvsz - 1) {

		/* skip any white space */
		අතරතුර (isblank(*s))
			++s;

		නම් (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		අතරතුර (*s && !isblank(*s))
			++s;

		නම් (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ආපසු argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       පූර්ණ_සංඛ්‍යා linemax, var_na *const argv[])
{
	පූර්ණ_සංඛ්‍යා ll = leader != NULL ? strlen(leader) : 0;
	පූර්ණ_සංඛ්‍යා sl = sep != NULL ? strlen(sep) : 0;
	පූර්ණ_සංඛ්‍යා len, i;

	නම් (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	අතරතුර (*argv != NULL) {
		len = strlen(*argv) + sl;
		නම් (i + len >= linemax) {
			puts("\n");
			නම් (leader)
				puts(leader);
			i = ll - sl;
		} නැතහොත් නම් (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static පූර්ණ_සංඛ්‍යා find_common_prefix(var_na *const argv[])
{
	පූර්ණ_සංඛ්‍යා i, len;
	var_na *anchor, *s, *t;

	නම් (*argv == NULL)
		ආපසු 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	අතරතුර ((t = *argv++) != NULL) {
		s = anchor;
		සඳහා (i = 0; i < len; i++, t++, s++) {
			නම් (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ආපසු len;
}

පූර්ණ_සංඛ්‍යා cmd_auto_complete(const var_na *const prompt, var_na *buf, පූර්ණ_සංඛ්‍යා *np, පූර්ණ_සංඛ්‍යා *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	පූර්ණ_සංඛ්‍යා n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	පූර්ණ_සංඛ්‍යා i, j, k, len, seplen, argc;
	පූර්ණ_සංඛ්‍යා cnt;
	var_na last_char;
	const var_na *ps_prompt;

	නම් (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		නම් (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} නැතහොත්
		ps_prompt = CONFIG_SYS_PROMPT;

	නම් (strcmp(prompt, ps_prompt) != 0)
		ආපසු 0;	/* not in normal console */

	cnt = strlen(buf);
	නම් (cnt >= 1)
		last_char = buf[cnt - 1];
	නැතහොත්
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	නම් (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	නම් (i == 0) {
		නම් (argc > 1)	/* allow tab for non command */
			ආපසු 0;
		putc('\a');
		ආපසු 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	නම් (i == 1) { /* one match; perfect */
		නම් (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		නැතහොත්
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} නැතහොත් නම් (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		නම් (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		නැතහොත්
			k = 0;

		j -= k;
		නම් (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	නම් (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		නම් (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ආපසු 1;
		}

		t = buf + cnt;
		සඳහා (i = 0; i < len; i++)
			*t++ = *s++;
		නම් (sep != NULL)
			සඳහා (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		නම් (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} නැතහොත් {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ආපසු 1;
}

#endif

#ifdef CMD_DATA_SIZE
පූර්ණ_සංඛ්‍යා cmd_get_data_size(const var_na *arg, පූර්ණ_සංඛ්‍යා default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	පූර්ණ_සංඛ්‍යා len = strlen(arg);
	නම් (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ආපසු 1;
		pak_sa 'w':
			ආපසු 2;
		pak_sa 'l':
			ආපසු 4;
		pak_sa 's':
			ආපසු CMD_DATA_SIZE_STR;
		pak_sa 'q':
			නම් (MEM_SUPPORT_64BIT_DATA)
				ආපසු 8;
			fallthrough;
		default:
			ආපසු CMD_DATA_SIZE_ERR;
		}
	}
	ආපසු default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා size)
{
	පූර්ණ_සංඛ්‍යා	i;

	නම් (gd->reloc_off == 0)
		ආපසු;

	සඳහා (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(පූර්ණ_සංඛ්‍යා (*)(sa_nracanaa cmd_tbl *, පූර්ණ_සංඛ්‍යා, පූර්ණ_සංඛ්‍යා,
				 var_na * const [], පූර්ණ_සංඛ්‍යා *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (පූර්ණ_සංඛ්‍යා (*)(sa_nracanaa cmd_tbl *, පූර්ණ_සංඛ්‍යා, පූර්ණ_සංඛ්‍යා,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		නම් (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		නම් (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		නම් (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(පූර්ණ_සංඛ්‍යා (*)(පූර්ණ_සංඛ්‍යා, var_na * const [], var_na, පූර්ණ_සංඛ්‍යා, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

පූර්ණ_සංඛ්‍යා cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc,
			  var_na *const argv[], පූර්ණ_සංඛ්‍යා *repeatable)
{
	*repeatable = 1;

	ආපසු cmdtp->cmd(cmdtp, flag, argc, argv);
}

පූර්ණ_සංඛ්‍යා cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc,
			 var_na *const argv[], පූර්ණ_සංඛ්‍යා *repeatable)
{
	*repeatable = 0;

	ආපසු cmdtp->cmd(cmdtp, flag, argc, argv);
}

පූර්ණ_සංඛ්‍යා cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc,
			   var_na *const argv[])
{
	පූර්ණ_සංඛ්‍යා repeatable;

	ආපසු cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static පූර්ණ_සංඛ්‍යා cmd_call(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc,
		    var_na *const argv[], පූර්ණ_සංඛ්‍යා *repeatable)
{
	පූර්ණ_සංඛ්‍යා result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	නම් (result)
		debug("Command failed, result=%d\n", result);
	ආපසු result;
}

ga_nanaa command_ret_t cmd_process(පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc, var_na *const argv[],
			       පූර්ණ_සංඛ්‍යා *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#නම් defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	නම් (xtrace) {
		puts("+");
		සඳහා (පූර්ණ_සංඛ්‍යා i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	නම් (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ආපසු 1;
	}

	/* found - check max args */
	නම් (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#නම් defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	නැතහොත් නම් (cmdtp->cmd == do_bootd) {
		නම් (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} නැතහොත් {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	නම් (!rc) {
		පූර්ණ_සංඛ්‍යා newrep;

		නම් (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		නම් (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	නම් (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ආපසු rc;
}

පූර්ණ_සංඛ්‍යා cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා err)
{
	නම් (err == CMD_RET_USAGE)
		ආපසු CMD_RET_USAGE;

	නම් (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ආපසු CMD_RET_FAILURE;
	}

	ආපසු CMD_RET_SUCCESS;
}

පූර්ණ_සංඛ්‍යා cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	පූර්ණ_සංඛ්‍යා ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	නම් (ret)
		ආපසු CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ආපසු run_command_list(data, len, 0);
}
