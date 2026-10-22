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

ကိန်းပြည့် _do_help(sa_nracanaa cmd_tbl *cmd_start, ကိန်းပြည့် cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     ကိန်းပြည့် flag, ကိန်းပြည့် argc, var_na *const argv[])
{
	ကိန်းပြည့် i;
	ကိန်းပြည့် rcode = 0;

	အကယ်၍ (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		ကိန်းပြည့် i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		အတွက် (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		အတွက် (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			အတွက် (j = 0; j < i; ++j) {
				အကယ်၍ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			အကယ်၍ (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		အတွက် (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			အကယ်၍ (ctrlc())
				ပြန်ပေး 1;
			အကယ်၍ (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		ပြန်ပေး 0;
	}
	/*
	 * command help (long version)
	 */
	အတွက် (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		အကယ်၍ (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} မဟုတ်ပါက {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ပြန်ပေး rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     ကိန်းပြည့် table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	ကိန်းပြည့် len;
	ကိန်းပြည့် n_found = 0;

	အကယ်၍ (!cmd)
		ပြန်ပေး NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	အတွက် (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		အကယ်၍ (strncmp(cmd, cmdtp->name, len) == 0) {
			အကယ်၍ (len == strlen(cmdtp->name))
				ပြန်ပေး cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	အကယ်၍ (n_found == 1) {			/* exactly one match */
		ပြန်ပေး cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	ပြန်ပေး NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const ကိန်းပြည့် len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	ပြန်ပေး find_cmd_tbl(cmd, start, len);
}

ကိန်းပြည့် cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	အကယ်၍ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ပြန်ပေး 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	ပြန်ပေး 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

ကိန်းပြည့် var_complete(ကိန်းပြည့် argc, var_na *const argv[], var_na last_char, ကိန်းပြည့် maxv,
		 var_na *cmdv[])
{
	ကိန်းပြည့် space;

	space = last_char == '\0' || isblank(last_char);

	အကယ်၍ (space && argc == 1)
		ပြန်ပေး env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	အကယ်၍ (!space && argc == 2)
		ပြန်ပေး env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ပြန်ပေး 0;
}

static ကိန်းပြည့် dollar_complete(ကိန်းပြည့် argc, var_na *const argv[], var_na last_char,
			   ကိန်းပြည့် maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	အကယ်၍ (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		ပြန်ပေး 0;

	ပြန်ပေး env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ကိန်းပြည့် complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် count, ကိန်းပြည့် argc,
		     var_na *const argv[], var_na last_char,
		     ကိန်းပြည့် maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	ကိန်းပြည့် len, clen;
	ကိန်းပြည့် n_found = 0;
	const var_na *cmd;

	/* sanity? */
	အကယ်၍ (maxv < 2)
		ပြန်ပေး -2;

	cmdv[0] = NULL;

	အကယ်၍ (argc == 0) {
		/* output full list of commands */
		အတွက် (; cmdtp != cmdend; cmdtp++) {
			အကယ်၍ (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		ပြန်ပေး n_found;
	}

	/* more than one arg or one but the start of the next */
	အကယ်၍ (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		အကယ်၍ (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			ပြန်ပေး 0;
		}
		ပြန်ပေး (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	အကယ်၍ (p == NULL)
		len = strlen(cmd);
	မဟုတ်ပါက
		len = p - cmd;

	/* return the partial matches */
	အတွက် (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		အကယ်၍ (clen < len)
			continue;

		အကယ်၍ (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		အကယ်၍ (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	ပြန်ပေး n_found;
#မဟုတ်ပါက
	ပြန်ပေး 0;
#endif
}

static ကိန်းပြည့် complete_cmdv(ကိန်းပြည့် argc, var_na *const argv[], var_na last_char,
			 ကိန်းပြည့် maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	ပြန်ပေး complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#မဟုတ်ပါက
	ပြန်ပေး 0;
#endif
}

static ကိန်းပြည့် make_argv(var_na *s, ကိန်းပြည့် argvsz, var_na *argv[])
{
	ကိန်းပြည့် argc = 0;

	/* split into argv */
	နေစဉ် (argc < argvsz - 1) {

		/* skip any white space */
		နေစဉ် (isblank(*s))
			++s;

		အကယ်၍ (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		နေစဉ် (*s && !isblank(*s))
			++s;

		အကယ်၍ (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	ပြန်ပေး argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       ကိန်းပြည့် linemax, var_na *const argv[])
{
	ကိန်းပြည့် ll = leader != NULL ? strlen(leader) : 0;
	ကိန်းပြည့် sl = sep != NULL ? strlen(sep) : 0;
	ကိန်းပြည့် len, i;

	အကယ်၍ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	နေစဉ် (*argv != NULL) {
		len = strlen(*argv) + sl;
		အကယ်၍ (i + len >= linemax) {
			puts("\n");
			အကယ်၍ (leader)
				puts(leader);
			i = ll - sl;
		} မဟုတ်ပါက အကယ်၍ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static ကိန်းပြည့် find_common_prefix(var_na *const argv[])
{
	ကိန်းပြည့် i, len;
	var_na *anchor, *s, *t;

	အကယ်၍ (*argv == NULL)
		ပြန်ပေး 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	နေစဉ် ((t = *argv++) != NULL) {
		s = anchor;
		အတွက် (i = 0; i < len; i++, t++, s++) {
			အကယ်၍ (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	ပြန်ပေး len;
}

ကိန်းပြည့် cmd_auto_complete(const var_na *const prompt, var_na *buf, ကိန်းပြည့် *np, ကိန်းပြည့် *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ကိန်းပြည့် n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	ကိန်းပြည့် i, j, k, len, seplen, argc;
	ကိန်းပြည့် cnt;
	var_na last_char;
	const var_na *ps_prompt;

	အကယ်၍ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		အကယ်၍ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} မဟုတ်ပါက
		ps_prompt = CONFIG_SYS_PROMPT;

	အကယ်၍ (strcmp(prompt, ps_prompt) != 0)
		ပြန်ပေး 0;	/* not in normal console */

	cnt = strlen(buf);
	အကယ်၍ (cnt >= 1)
		last_char = buf[cnt - 1];
	မဟုတ်ပါက
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	အကယ်၍ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	အကယ်၍ (i == 0) {
		အကယ်၍ (argc > 1)	/* allow tab for non command */
			ပြန်ပေး 0;
		putc('\a');
		ပြန်ပေး 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	အကယ်၍ (i == 1) { /* one match; perfect */
		အကယ်၍ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		မဟုတ်ပါက
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} မဟုတ်ပါက အကယ်၍ (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		အကယ်၍ (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		မဟုတ်ပါက
			k = 0;

		j -= k;
		အကယ်၍ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	အကယ်၍ (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		အကယ်၍ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ပြန်ပေး 1;
		}

		t = buf + cnt;
		အတွက် (i = 0; i < len; i++)
			*t++ = *s++;
		အကယ်၍ (sep != NULL)
			အတွက် (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		အကယ်၍ (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} မဟုတ်ပါက {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ပြန်ပေး 1;
}

#endif

#ifdef CMD_DATA_SIZE
ကိန်းပြည့် cmd_get_data_size(const var_na *arg, ကိန်းပြည့် default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ကိန်းပြည့် len = strlen(arg);
	အကယ်၍ (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			ပြန်ပေး 1;
		pak_sa 'w':
			ပြန်ပေး 2;
		pak_sa 'l':
			ပြန်ပေး 4;
		pak_sa 's':
			ပြန်ပေး CMD_DATA_SIZE_STR;
		pak_sa 'q':
			အကယ်၍ (MEM_SUPPORT_64BIT_DATA)
				ပြန်ပေး 8;
			fallthrough;
		default:
			ပြန်ပေး CMD_DATA_SIZE_ERR;
		}
	}
	ပြန်ပေး default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် size)
{
	ကိန်းပြည့်	i;

	အကယ်၍ (gd->reloc_off == 0)
		ပြန်ပေး;

	အတွက် (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ကိန်းပြည့် (*)(sa_nracanaa cmd_tbl *, ကိန်းပြည့်, ကိန်းပြည့်,
				 var_na * const [], ကိန်းပြည့် *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ကိန်းပြည့် (*)(sa_nracanaa cmd_tbl *, ကိန်းပြည့်, ကိန်းပြည့်,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		အကယ်၍ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		အကယ်၍ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		အကယ်၍ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ကိန်းပြည့် (*)(ကိန်းပြည့်, var_na * const [], var_na, ကိန်းပြည့်, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

ကိန်းပြည့် cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် flag, ကိန်းပြည့် argc,
			  var_na *const argv[], ကိန်းပြည့် *repeatable)
{
	*repeatable = 1;

	ပြန်ပေး cmdtp->cmd(cmdtp, flag, argc, argv);
}

ကိန်းပြည့် cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် flag, ကိန်းပြည့် argc,
			 var_na *const argv[], ကိန်းပြည့် *repeatable)
{
	*repeatable = 0;

	ပြန်ပေး cmdtp->cmd(cmdtp, flag, argc, argv);
}

ကိန်းပြည့် cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် flag, ကိန်းပြည့် argc,
			   var_na *const argv[])
{
	ကိန်းပြည့် repeatable;

	ပြန်ပေး cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static ကိန်းပြည့် cmd_call(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် flag, ကိန်းပြည့် argc,
		    var_na *const argv[], ကိန်းပြည့် *repeatable)
{
	ကိန်းပြည့် result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	အကယ်၍ (result)
		debug("Command failed, result=%d\n", result);
	ပြန်ပေး result;
}

ga_nanaa command_ret_t cmd_process(ကိန်းပြည့် flag, ကိန်းပြည့် argc, var_na *const argv[],
			       ကိန်းပြည့် *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#အကယ်၍ defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	အကယ်၍ (xtrace) {
		puts("+");
		အတွက် (ကိန်းပြည့် i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	အကယ်၍ (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		ပြန်ပေး 1;
	}

	/* found - check max args */
	အကယ်၍ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#အကယ်၍ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	မဟုတ်ပါက အကယ်၍ (cmdtp->cmd == do_bootd) {
		အကယ်၍ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} မဟုတ်ပါက {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	အကယ်၍ (!rc) {
		ကိန်းပြည့် newrep;

		အကယ်၍ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		အကယ်၍ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	အကယ်၍ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ပြန်ပေး rc;
}

ကိန်းပြည့် cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, ကိန်းပြည့် err)
{
	အကယ်၍ (err == CMD_RET_USAGE)
		ပြန်ပေး CMD_RET_USAGE;

	အကယ်၍ (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ပြန်ပေး CMD_RET_FAILURE;
	}

	ပြန်ပေး CMD_RET_SUCCESS;
}

ကိန်းပြည့် cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	ကိန်းပြည့် ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	အကယ်၍ (ret)
		ပြန်ပေး CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ပြန်ပေး run_command_list(data, len, 0);
}
