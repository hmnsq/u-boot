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

số_nguyên _do_help(struct cmd_tbl *cmd_start, số_nguyên cmd_items, struct cmd_tbl *cmdtp,
	     số_nguyên flag, số_nguyên argc, char *const argv[])
{
	số_nguyên i;
	số_nguyên rcode = 0;

	nếu (argc == 1) {	/* show list of commands */
		struct cmd_tbl *cmd_array[cmd_items];
		số_nguyên i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		cho (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		cho (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			cho (j = 0; j < i; ++j) {
				nếu (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			nếu (!swaps)
				break;
		}

		/* print short help (usage) */
		cho (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			nếu (ctrlc())
				trả_về 1;
			nếu (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		trả_về 0;
	}
	/*
	 * command help (long version)
	 */
	cho (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		nếu (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} không_thì {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	trả_về rcode;
}

/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     số_nguyên table_len)
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *cmdtp;
	struct cmd_tbl *cmdtp_temp = table;	/* Init value */
	const char *p;
	số_nguyên len;
	số_nguyên n_found = 0;

	nếu (!cmd)
		trả_về NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	cho (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		nếu (strncmp(cmd, cmdtp->name, len) == 0) {
			nếu (len == strlen(cmdtp->name))
				trả_về cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	nếu (n_found == 1) {			/* exactly one match */
		trả_về cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	trả_về NULL;	/* not found or ambiguous command */
}

struct cmd_tbl *find_cmd(const char *cmd)
{
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const số_nguyên len = ll_entry_count(struct cmd_tbl, cmd);
	trả_về find_cmd_tbl(cmd, start, len);
}

số_nguyên cmd_usage(const struct cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	nếu (!cmdtp->help) {
		puts ("- No additional help available.\n");
		trả_về 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	trả_về 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static char env_complete_buf[512];

số_nguyên var_complete(số_nguyên argc, char *const argv[], char last_char, số_nguyên maxv,
		 char *cmdv[])
{
	số_nguyên space;

	space = last_char == '\0' || isblank(last_char);

	nếu (space && argc == 1)
		trả_về env_complete("", maxv, cmdv, sizeof(env_complete_buf),
				    env_complete_buf, false);

	nếu (!space && argc == 2)
		trả_về env_complete(argv[1], maxv, cmdv,
				    sizeof(env_complete_buf),
				    env_complete_buf, false);

	trả_về 0;
}

static số_nguyên dollar_complete(số_nguyên argc, char *const argv[], char last_char,
			   số_nguyên maxv, char *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	nếu (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		trả_về 0;

	trả_về env_complete(argv[argc - 1], maxv, cmdv, sizeof(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

số_nguyên complete_subcmdv(struct cmd_tbl *cmdtp, số_nguyên count, số_nguyên argc,
		     char *const argv[], char last_char,
		     số_nguyên maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const struct cmd_tbl *cmdend = cmdtp + count;
	const char *p;
	số_nguyên len, clen;
	số_nguyên n_found = 0;
	const char *cmd;

	/* sanity? */
	nếu (maxv < 2)
		trả_về -2;

	cmdv[0] = NULL;

	nếu (argc == 0) {
		/* output full list of commands */
		cho (; cmdtp != cmdend; cmdtp++) {
			nếu (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		trả_về n_found;
	}

	/* more than one arg or one but the start of the next */
	nếu (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		nếu (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			trả_về 0;
		}
		trả_về (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	nếu (p == NULL)
		len = strlen(cmd);
	không_thì
		len = p - cmd;

	/* return the partial matches */
	cho (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		nếu (clen < len)
			continue;

		nếu (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		nếu (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	trả_về n_found;
#không_thì
	trả_về 0;
#endif
}

static số_nguyên complete_cmdv(số_nguyên argc, char *const argv[], char last_char,
			 số_nguyên maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	trả_về complete_subcmdv(ll_entry_start(struct cmd_tbl, cmd),
				ll_entry_count(struct cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#không_thì
	trả_về 0;
#endif
}

static số_nguyên make_argv(char *s, số_nguyên argvsz, char *argv[])
{
	số_nguyên argc = 0;

	/* split into argv */
	trong_khi (argc < argvsz - 1) {

		/* skip any white space */
		trong_khi (isblank(*s))
			++s;

		nếu (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		trong_khi (*s && !isblank(*s))
			++s;

		nếu (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	trả_về argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep,
		       số_nguyên linemax, char *const argv[])
{
	số_nguyên ll = leader != NULL ? strlen(leader) : 0;
	số_nguyên sl = sep != NULL ? strlen(sep) : 0;
	số_nguyên len, i;

	nếu (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	trong_khi (*argv != NULL) {
		len = strlen(*argv) + sl;
		nếu (i + len >= linemax) {
			puts("\n");
			nếu (leader)
				puts(leader);
			i = ll - sl;
		} không_thì nếu (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static số_nguyên find_common_prefix(char *const argv[])
{
	số_nguyên i, len;
	char *anchor, *s, *t;

	nếu (*argv == NULL)
		trả_về 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	trong_khi ((t = *argv++) != NULL) {
		s = anchor;
		cho (i = 0; i < len; i++, t++, s++) {
			nếu (*t != *s)
				break;
		}
		len = s - anchor;
	}
	trả_về len;
}

số_nguyên cmd_auto_complete(const char *const prompt, char *buf, số_nguyên *np, số_nguyên *colp)
{
	char tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	số_nguyên n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	số_nguyên i, j, k, len, seplen, argc;
	số_nguyên cnt;
	char last_char;
	const char *ps_prompt;

	nếu (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		nếu (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} không_thì
		ps_prompt = CONFIG_SYS_PROMPT;

	nếu (strcmp(prompt, ps_prompt) != 0)
		trả_về 0;	/* not in normal console */

	cnt = strlen(buf);
	nếu (cnt >= 1)
		last_char = buf[cnt - 1];
	không_thì
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	nếu (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	nếu (i == 0) {
		nếu (argc > 1)	/* allow tab for non command */
			trả_về 0;
		putc('\a');
		trả_về 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	nếu (i == 1) { /* one match; perfect */
		nếu (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		không_thì
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} không_thì nếu (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		nếu (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		không_thì
			k = 0;

		j -= k;
		nếu (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	nếu (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		nếu (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			trả_về 1;
		}

		t = buf + cnt;
		cho (i = 0; i < len; i++)
			*t++ = *s++;
		nếu (sep != NULL)
			cho (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		nếu (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} không_thì {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	trả_về 1;
}

#endif

#ifdef CMD_DATA_SIZE
số_nguyên cmd_get_data_size(const char *arg, số_nguyên default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	số_nguyên len = strlen(arg);
	nếu (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		case 'b':
			trả_về 1;
		case 'w':
			trả_về 2;
		case 'l':
			trả_về 4;
		case 's':
			trả_về CMD_DATA_SIZE_STR;
		case 'q':
			nếu (MEM_SUPPORT_64BIT_DATA)
				trả_về 8;
			fallthrough;
		default:
			trả_về CMD_DATA_SIZE_ERR;
		}
	}
	trả_về default_size;
}
#endif

void fixup_cmdtable(struct cmd_tbl *cmdtp, số_nguyên size)
{
	số_nguyên	i;

	nếu (gd->reloc_off == 0)
		trả_về;

	cho (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(số_nguyên (*)(struct cmd_tbl *, số_nguyên, số_nguyên,
				 char * const [], số_nguyên *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (số_nguyên (*)(struct cmd_tbl *, số_nguyên, số_nguyên,
				      char *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;
		nếu (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		nếu (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		nếu (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(số_nguyên (*)(số_nguyên, char * const [], char, số_nguyên, char * []))addr;
		}
#endif
		cmdtp++;
	}
}

số_nguyên cmd_always_repeatable(struct cmd_tbl *cmdtp, số_nguyên flag, số_nguyên argc,
			  char *const argv[], số_nguyên *repeatable)
{
	*repeatable = 1;

	trả_về cmdtp->cmd(cmdtp, flag, argc, argv);
}

số_nguyên cmd_never_repeatable(struct cmd_tbl *cmdtp, số_nguyên flag, số_nguyên argc,
			 char *const argv[], số_nguyên *repeatable)
{
	*repeatable = 0;

	trả_về cmdtp->cmd(cmdtp, flag, argc, argv);
}

số_nguyên cmd_discard_repeatable(struct cmd_tbl *cmdtp, số_nguyên flag, số_nguyên argc,
			   char *const argv[])
{
	số_nguyên repeatable;

	trả_về cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static số_nguyên cmd_call(struct cmd_tbl *cmdtp, số_nguyên flag, số_nguyên argc,
		    char *const argv[], số_nguyên *repeatable)
{
	số_nguyên result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	nếu (result)
		debug("Command failed, result=%d\n", result);
	trả_về result;
}

enum command_ret_t cmd_process(số_nguyên flag, số_nguyên argc, char *const argv[],
			       số_nguyên *repeatable, ulong *ticks)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct cmd_tbl *cmdtp;

#nếu defined(CONFIG_SYS_XTRACE)
	char *xtrace;

	xtrace = env_get("xtrace");
	nếu (xtrace) {
		puts("+");
		cho (số_nguyên i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	nếu (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		trả_về 1;
	}

	/* found - check max args */
	nếu (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#nếu defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	không_thì nếu (cmdtp->cmd == do_bootd) {
		nếu (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} không_thì {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	nếu (!rc) {
		số_nguyên newrep;

		nếu (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		nếu (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	nếu (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	trả_về rc;
}

số_nguyên cmd_process_error(struct cmd_tbl *cmdtp, số_nguyên err)
{
	nếu (err == CMD_RET_USAGE)
		trả_về CMD_RET_USAGE;

	nếu (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		trả_về CMD_RET_FAILURE;
	}

	trả_về CMD_RET_SUCCESS;
}

số_nguyên cmd_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	số_nguyên ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	nếu (ret)
		trả_về CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	trả_về run_command_list(data, len, 0);
}
