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

จำนวนเต็ม _do_help(sa_nracanaa cmd_tbl *cmd_start, จำนวนเต็ม cmd_items, sa_nracanaa cmd_tbl *cmdtp,
	     จำนวนเต็ม flag, จำนวนเต็ม argc, var_na *const argv[])
{
	จำนวนเต็ม i;
	จำนวนเต็ม rcode = 0;

	ถ้า (argc == 1) {	/* show list of commands */
		sa_nracanaa cmd_tbl *cmd_array[cmd_items];
		จำนวนเต็ม i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		สำหรับ (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		สำหรับ (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			สำหรับ (j = 0; j < i; ++j) {
				ถ้า (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					sa_nracanaa cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ถ้า (!swaps)
				bha_nga;
		}

		/* print short help (usage) */
		สำหรับ (i = 0; i < cmd_items; i++) {
			const var_na *usage = cmd_array[i]->usage;

			/* allow user abort */
			ถ้า (ctrlc())
				คืน 1;
			ถ้า (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		คืน 0;
	}
	/*
	 * command help (long version)
	 */
	สำหรับ (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ถ้า (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} มิฉะนั้น {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	คืน rcode;
}

/* find command table entry for a command */
sa_nracanaa cmd_tbl *find_cmd_tbl(const var_na *cmd, sa_nracanaa cmd_tbl *table,
			     จำนวนเต็ม table_len)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
	sa_nracanaa cmd_tbl *cmdtp_temp = table;	/* Init value */
	const var_na *p;
	จำนวนเต็ม len;
	จำนวนเต็ม n_found = 0;

	ถ้า (!cmd)
		คืน NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	สำหรับ (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ถ้า (strncmp(cmd, cmdtp->name, len) == 0) {
			ถ้า (len == strlen(cmdtp->name))
				คืน cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ถ้า (n_found == 1) {			/* exactly one match */
		คืน cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	คืน NULL;	/* not found or ambiguous command */
}

sa_nracanaa cmd_tbl *find_cmd(const var_na *cmd)
{
	sa_nracanaa cmd_tbl *start = ll_entry_start(sa_nracanaa cmd_tbl, cmd);
	const จำนวนเต็ม len = ll_entry_count(sa_nracanaa cmd_tbl, cmd);
	คืน find_cmd_tbl(cmd, start, len);
}

จำนวนเต็ม cmd_usage(const sa_nracanaa cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ถ้า (!cmdtp->help) {
		puts ("- No additional help available.\n");
		คืน 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	คืน 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static var_na env_complete_buf[512];

จำนวนเต็ม var_complete(จำนวนเต็ม argc, var_na *const argv[], var_na last_char, จำนวนเต็ม maxv,
		 var_na *cmdv[])
{
	จำนวนเต็ม space;

	space = last_char == '\0' || isblank(last_char);

	ถ้า (space && argc == 1)
		คืน env_complete("", maxv, cmdv, parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	ถ้า (!space && argc == 2)
		คืน env_complete(argv[1], maxv, cmdv,
				    parimaa_na(env_complete_buf),
				    env_complete_buf, false);

	คืน 0;
}

static จำนวนเต็ม dollar_complete(จำนวนเต็ม argc, var_na *const argv[], var_na last_char,
			   จำนวนเต็ม maxv, var_na *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ถ้า (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		คืน 0;

	คืน env_complete(argv[argc - 1], maxv, cmdv, parimaa_na(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

จำนวนเต็ม complete_subcmdv(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม count, จำนวนเต็ม argc,
		     var_na *const argv[], var_na last_char,
		     จำนวนเต็ม maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const sa_nracanaa cmd_tbl *cmdend = cmdtp + count;
	const var_na *p;
	จำนวนเต็ม len, clen;
	จำนวนเต็ม n_found = 0;
	const var_na *cmd;

	/* sanity? */
	ถ้า (maxv < 2)
		คืน -2;

	cmdv[0] = NULL;

	ถ้า (argc == 0) {
		/* output full list of commands */
		สำหรับ (; cmdtp != cmdend; cmdtp++) {
			ถ้า (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				bha_nga;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		คืน n_found;
	}

	/* more than one arg or one but the start of the next */
	ถ้า (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ถ้า (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			คืน 0;
		}
		คืน (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ถ้า (p == NULL)
		len = strlen(cmd);
	มิฉะนั้น
		len = p - cmd;

	/* return the partial matches */
	สำหรับ (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ถ้า (clen < len)
			continue;

		ถ้า (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ถ้า (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			bha_nga;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	คืน n_found;
#มิฉะนั้น
	คืน 0;
#endif
}

static จำนวนเต็ม complete_cmdv(จำนวนเต็ม argc, var_na *const argv[], var_na last_char,
			 จำนวนเต็ม maxv, var_na *cmdv[])
{
#ifdef CONFIG_CMDLINE
	คืน complete_subcmdv(ll_entry_start(sa_nracanaa cmd_tbl, cmd),
				ll_entry_count(sa_nracanaa cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#มิฉะนั้น
	คืน 0;
#endif
}

static จำนวนเต็ม make_argv(var_na *s, จำนวนเต็ม argvsz, var_na *argv[])
{
	จำนวนเต็ม argc = 0;

	/* split into argv */
	ขณะ (argc < argvsz - 1) {

		/* skip any white space */
		ขณะ (isblank(*s))
			++s;

		ถ้า (*s == '\0')	/* end of s, no more args	*/
			bha_nga;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ขณะ (*s && !isblank(*s))
			++s;

		ถ้า (*s == '\0')		/* end of s, no more args	*/
			bha_nga;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	คืน argc;
}

static void print_argv(const var_na *banner, const var_na *leader, const var_na *sep,
		       จำนวนเต็ม linemax, var_na *const argv[])
{
	จำนวนเต็ม ll = leader != NULL ? strlen(leader) : 0;
	จำนวนเต็ม sl = sep != NULL ? strlen(sep) : 0;
	จำนวนเต็ม len, i;

	ถ้า (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ขณะ (*argv != NULL) {
		len = strlen(*argv) + sl;
		ถ้า (i + len >= linemax) {
			puts("\n");
			ถ้า (leader)
				puts(leader);
			i = ll - sl;
		} มิฉะนั้น ถ้า (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static จำนวนเต็ม find_common_prefix(var_na *const argv[])
{
	จำนวนเต็ม i, len;
	var_na *anchor, *s, *t;

	ถ้า (*argv == NULL)
		คืน 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	ขณะ ((t = *argv++) != NULL) {
		s = anchor;
		สำหรับ (i = 0; i < len; i++, t++, s++) {
			ถ้า (*t != *s)
				bha_nga;
		}
		len = s - anchor;
	}
	คืน len;
}

จำนวนเต็ม cmd_auto_complete(const var_na *const prompt, var_na *buf, จำนวนเต็ม *np, จำนวนเต็ม *colp)
{
	var_na tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	จำนวนเต็ม n = *np, col = *colp;
	var_na *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	var_na *cmdv[20];
	var_na *s, *t;
	const var_na *sep;
	จำนวนเต็ม i, j, k, len, seplen, argc;
	จำนวนเต็ม cnt;
	var_na last_char;
	const var_na *ps_prompt;

	ถ้า (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ถ้า (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} มิฉะนั้น
		ps_prompt = CONFIG_SYS_PROMPT;

	ถ้า (strcmp(prompt, ps_prompt) != 0)
		คืน 0;	/* not in normal console */

	cnt = strlen(buf);
	ถ้า (cnt >= 1)
		last_char = buf[cnt - 1];
	มิฉะนั้น
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, parimaa_na(argv)/parimaa_na(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	ถ้า (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  parimaa_na(cmdv) / parimaa_na(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ถ้า (i == 0) {
		ถ้า (argc > 1)	/* allow tab for non command */
			คืน 0;
		putc('\a');
		คืน 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ถ้า (i == 1) { /* one match; perfect */
		ถ้า (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		มิฉะนั้น
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} มิฉะนั้น ถ้า (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ถ้า (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		มิฉะนั้น
			k = 0;

		j -= k;
		ถ้า (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ถ้า (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ถ้า (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			คืน 1;
		}

		t = buf + cnt;
		สำหรับ (i = 0; i < len; i++)
			*t++ = *s++;
		ถ้า (sep != NULL)
			สำหรับ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ถ้า (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} มิฉะนั้น {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	คืน 1;
}

#endif

#ifdef CMD_DATA_SIZE
จำนวนเต็ม cmd_get_data_size(const var_na *arg, จำนวนเต็ม default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	จำนวนเต็ม len = strlen(arg);
	ถ้า (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		pak_sa 'b':
			คืน 1;
		pak_sa 'w':
			คืน 2;
		pak_sa 'l':
			คืน 4;
		pak_sa 's':
			คืน CMD_DATA_SIZE_STR;
		pak_sa 'q':
			ถ้า (MEM_SUPPORT_64BIT_DATA)
				คืน 8;
			fallthrough;
		default:
			คืน CMD_DATA_SIZE_ERR;
		}
	}
	คืน default_size;
}
#endif

void fixup_cmdtable(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม size)
{
	จำนวนเต็ม	i;

	ถ้า (gd->reloc_off == 0)
		คืน;

	สำหรับ (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(จำนวนเต็ม (*)(sa_nracanaa cmd_tbl *, จำนวนเต็ม, จำนวนเต็ม,
				 var_na * const [], จำนวนเต็ม *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (จำนวนเต็ม (*)(sa_nracanaa cmd_tbl *, จำนวนเต็ม, จำนวนเต็ม,
				      var_na *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (var_na *)addr;
		ถ้า (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (var_na *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ถ้า (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (var_na *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ถ้า (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(จำนวนเต็ม (*)(จำนวนเต็ม, var_na * const [], var_na, จำนวนเต็ม, var_na * []))addr;
		}
#endif
		cmdtp++;
	}
}

จำนวนเต็ม cmd_always_repeatable(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม flag, จำนวนเต็ม argc,
			  var_na *const argv[], จำนวนเต็ม *repeatable)
{
	*repeatable = 1;

	คืน cmdtp->cmd(cmdtp, flag, argc, argv);
}

จำนวนเต็ม cmd_never_repeatable(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม flag, จำนวนเต็ม argc,
			 var_na *const argv[], จำนวนเต็ม *repeatable)
{
	*repeatable = 0;

	คืน cmdtp->cmd(cmdtp, flag, argc, argv);
}

จำนวนเต็ม cmd_discard_repeatable(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม flag, จำนวนเต็ม argc,
			   var_na *const argv[])
{
	จำนวนเต็ม repeatable;

	คืน cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static จำนวนเต็ม cmd_call(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม flag, จำนวนเต็ม argc,
		    var_na *const argv[], จำนวนเต็ม *repeatable)
{
	จำนวนเต็ม result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ถ้า (result)
		debug("Command failed, result=%d\n", result);
	คืน result;
}

ga_nanaa command_ret_t cmd_process(จำนวนเต็ม flag, จำนวนเต็ม argc, var_na *const argv[],
			       จำนวนเต็ม *repeatable, ulong *ticks)
{
	ga_nanaa command_ret_t rc = CMD_RET_SUCCESS;
	sa_nracanaa cmd_tbl *cmdtp;

#ถ้า defined(CONFIG_SYS_XTRACE)
	var_na *xtrace;

	xtrace = env_get("xtrace");
	ถ้า (xtrace) {
		puts("+");
		สำหรับ (จำนวนเต็ม i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ถ้า (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		คืน 1;
	}

	/* found - check max args */
	ถ้า (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ถ้า defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	มิฉะนั้น ถ้า (cmdtp->cmd == do_bootd) {
		ถ้า (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} มิฉะนั้น {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ถ้า (!rc) {
		จำนวนเต็ม newrep;

		ถ้า (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ถ้า (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ถ้า (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	คืน rc;
}

จำนวนเต็ม cmd_process_error(sa_nracanaa cmd_tbl *cmdtp, จำนวนเต็ม err)
{
	ถ้า (err == CMD_RET_USAGE)
		คืน CMD_RET_USAGE;

	ถ้า (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		คืน CMD_RET_FAILURE;
	}

	คืน CMD_RET_SUCCESS;
}

จำนวนเต็ม cmd_source_script(ulong addr, const var_na *fit_uname, const var_na *confname)
{
	var_na *data;
	void *buf;
	uint len;
	จำนวนเต็ม ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ถ้า (ret)
		คืน CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	คืน run_command_list(data, len, 0);
}
