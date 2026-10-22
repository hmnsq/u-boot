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

цяло _do_help(struct cmd_tbl *cmd_start, цяло cmd_items, struct cmd_tbl *cmdtp,
	     цяло flag, цяло argc, char *const argv[])
{
	цяло i;
	цяло rcode = 0;

	ако (argc == 1) {	/* show list of commands */
		struct cmd_tbl *cmd_array[cmd_items];
		цяло i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		за (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		за (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			за (j = 0; j < i; ++j) {
				ако (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ако (!swaps)
				break;
		}

		/* print short help (usage) */
		за (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			ако (ctrlc())
				върни 1;
			ако (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		върни 0;
	}
	/*
	 * command help (long version)
	 */
	за (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		ако (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} иначе {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	върни rcode;
}

/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     цяло table_len)
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *cmdtp;
	struct cmd_tbl *cmdtp_temp = table;	/* Init value */
	const char *p;
	цяло len;
	цяло n_found = 0;

	ако (!cmd)
		върни NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	за (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		ако (strncmp(cmd, cmdtp->name, len) == 0) {
			ако (len == strlen(cmdtp->name))
				върни cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	ако (n_found == 1) {			/* exactly one match */
		върни cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	върни NULL;	/* not found or ambiguous command */
}

struct cmd_tbl *find_cmd(const char *cmd)
{
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const цяло len = ll_entry_count(struct cmd_tbl, cmd);
	върни find_cmd_tbl(cmd, start, len);
}

цяло cmd_usage(const struct cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	ако (!cmdtp->help) {
		puts ("- No additional help available.\n");
		върни 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	върни 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static char env_complete_buf[512];

цяло var_complete(цяло argc, char *const argv[], char last_char, цяло maxv,
		 char *cmdv[])
{
	цяло space;

	space = last_char == '\0' || isblank(last_char);

	ако (space && argc == 1)
		върни env_complete("", maxv, cmdv, sizeof(env_complete_buf),
				    env_complete_buf, false);

	ако (!space && argc == 2)
		върни env_complete(argv[1], maxv, cmdv,
				    sizeof(env_complete_buf),
				    env_complete_buf, false);

	върни 0;
}

static цяло dollar_complete(цяло argc, char *const argv[], char last_char,
			   цяло maxv, char *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ако (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		върни 0;

	върни env_complete(argv[argc - 1], maxv, cmdv, sizeof(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

цяло complete_subcmdv(struct cmd_tbl *cmdtp, цяло count, цяло argc,
		     char *const argv[], char last_char,
		     цяло maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const struct cmd_tbl *cmdend = cmdtp + count;
	const char *p;
	цяло len, clen;
	цяло n_found = 0;
	const char *cmd;

	/* sanity? */
	ако (maxv < 2)
		върни -2;

	cmdv[0] = NULL;

	ако (argc == 0) {
		/* output full list of commands */
		за (; cmdtp != cmdend; cmdtp++) {
			ако (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		върни n_found;
	}

	/* more than one arg or one but the start of the next */
	ако (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		ако (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			върни 0;
		}
		върни (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ако (p == NULL)
		len = strlen(cmd);
	иначе
		len = p - cmd;

	/* return the partial matches */
	за (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		ако (clen < len)
			continue;

		ако (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		ако (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	върни n_found;
#иначе
	върни 0;
#endif
}

static цяло complete_cmdv(цяло argc, char *const argv[], char last_char,
			 цяло maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	върни complete_subcmdv(ll_entry_start(struct cmd_tbl, cmd),
				ll_entry_count(struct cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#иначе
	върни 0;
#endif
}

static цяло make_argv(char *s, цяло argvsz, char *argv[])
{
	цяло argc = 0;

	/* split into argv */
	докато (argc < argvsz - 1) {

		/* skip any white space */
		докато (isblank(*s))
			++s;

		ако (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		докато (*s && !isblank(*s))
			++s;

		ако (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	върни argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep,
		       цяло linemax, char *const argv[])
{
	цяло ll = leader != NULL ? strlen(leader) : 0;
	цяло sl = sep != NULL ? strlen(sep) : 0;
	цяло len, i;

	ако (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	докато (*argv != NULL) {
		len = strlen(*argv) + sl;
		ако (i + len >= linemax) {
			puts("\n");
			ако (leader)
				puts(leader);
			i = ll - sl;
		} иначе ако (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static цяло find_common_prefix(char *const argv[])
{
	цяло i, len;
	char *anchor, *s, *t;

	ако (*argv == NULL)
		върни 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	докато ((t = *argv++) != NULL) {
		s = anchor;
		за (i = 0; i < len; i++, t++, s++) {
			ако (*t != *s)
				break;
		}
		len = s - anchor;
	}
	върни len;
}

цяло cmd_auto_complete(const char *const prompt, char *buf, цяло *np, цяло *colp)
{
	char tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	цяло n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	цяло i, j, k, len, seplen, argc;
	цяло cnt;
	char last_char;
	const char *ps_prompt;

	ако (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ако (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} иначе
		ps_prompt = CONFIG_SYS_PROMPT;

	ако (strcmp(prompt, ps_prompt) != 0)
		върни 0;	/* not in normal console */

	cnt = strlen(buf);
	ако (cnt >= 1)
		last_char = buf[cnt - 1];
	иначе
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	ако (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ако (i == 0) {
		ако (argc > 1)	/* allow tab for non command */
			върни 0;
		putc('\a');
		върни 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	ако (i == 1) { /* one match; perfect */
		ако (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		иначе
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} иначе ако (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		ако (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		иначе
			k = 0;

		j -= k;
		ако (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ако (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		ако (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			върни 1;
		}

		t = buf + cnt;
		за (i = 0; i < len; i++)
			*t++ = *s++;
		ако (sep != NULL)
			за (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		ако (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} иначе {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	върни 1;
}

#endif

#ifdef CMD_DATA_SIZE
цяло cmd_get_data_size(const char *arg, цяло default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	цяло len = strlen(arg);
	ако (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		case 'b':
			върни 1;
		case 'w':
			върни 2;
		case 'l':
			върни 4;
		case 's':
			върни CMD_DATA_SIZE_STR;
		case 'q':
			ако (MEM_SUPPORT_64BIT_DATA)
				върни 8;
			fallthrough;
		default:
			върни CMD_DATA_SIZE_ERR;
		}
	}
	върни default_size;
}
#endif

void fixup_cmdtable(struct cmd_tbl *cmdtp, цяло size)
{
	цяло	i;

	ако (gd->reloc_off == 0)
		върни;

	за (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(цяло (*)(struct cmd_tbl *, цяло, цяло,
				 char * const [], цяло *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (цяло (*)(struct cmd_tbl *, цяло, цяло,
				      char *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;
		ако (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		ако (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		ако (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(цяло (*)(цяло, char * const [], char, цяло, char * []))addr;
		}
#endif
		cmdtp++;
	}
}

цяло cmd_always_repeatable(struct cmd_tbl *cmdtp, цяло flag, цяло argc,
			  char *const argv[], цяло *repeatable)
{
	*repeatable = 1;

	върни cmdtp->cmd(cmdtp, flag, argc, argv);
}

цяло cmd_never_repeatable(struct cmd_tbl *cmdtp, цяло flag, цяло argc,
			 char *const argv[], цяло *repeatable)
{
	*repeatable = 0;

	върни cmdtp->cmd(cmdtp, flag, argc, argv);
}

цяло cmd_discard_repeatable(struct cmd_tbl *cmdtp, цяло flag, цяло argc,
			   char *const argv[])
{
	цяло repeatable;

	върни cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static цяло cmd_call(struct cmd_tbl *cmdtp, цяло flag, цяло argc,
		    char *const argv[], цяло *repeatable)
{
	цяло result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ако (result)
		debug("Command failed, result=%d\n", result);
	върни result;
}

enum command_ret_t cmd_process(цяло flag, цяло argc, char *const argv[],
			       цяло *repeatable, ulong *ticks)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct cmd_tbl *cmdtp;

#ако defined(CONFIG_SYS_XTRACE)
	char *xtrace;

	xtrace = env_get("xtrace");
	ако (xtrace) {
		puts("+");
		за (цяло i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	ако (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		върни 1;
	}

	/* found - check max args */
	ако (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ако defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	иначе ако (cmdtp->cmd == do_bootd) {
		ако (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} иначе {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	ако (!rc) {
		цяло newrep;

		ако (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ако (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ако (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	върни rc;
}

цяло cmd_process_error(struct cmd_tbl *cmdtp, цяло err)
{
	ако (err == CMD_RET_USAGE)
		върни CMD_RET_USAGE;

	ако (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		върни CMD_RET_FAILURE;
	}

	върни CMD_RET_SUCCESS;
}

цяло cmd_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	цяло ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	ако (ret)
		върни CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	върни run_command_list(data, len, 0);
}
