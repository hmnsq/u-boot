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

целое _do_help(struct cmd_tbl *cmd_start, целое cmd_items, struct cmd_tbl *cmdtp,
	     целое flag, целое argc, char *const argv[])
{
	целое i;
	целое rcode = 0;

	если (argc == 1) {	/* show list of commands */
		struct cmd_tbl *cmd_array[cmd_items];
		целое i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		для (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		для (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			для (j = 0; j < i; ++j) {
				если (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			если (!swaps)
				break;
		}

		/* print short help (usage) */
		для (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			если (ctrlc())
				вернуть 1;
			если (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		вернуть 0;
	}
	/*
	 * command help (long version)
	 */
	для (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		если (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} иначе {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	вернуть rcode;
}

/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     целое table_len)
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *cmdtp;
	struct cmd_tbl *cmdtp_temp = table;	/* Init value */
	const char *p;
	целое len;
	целое n_found = 0;

	если (!cmd)
		вернуть NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	для (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		если (strncmp(cmd, cmdtp->name, len) == 0) {
			если (len == strlen(cmdtp->name))
				вернуть cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	если (n_found == 1) {			/* exactly one match */
		вернуть cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	вернуть NULL;	/* not found or ambiguous command */
}

struct cmd_tbl *find_cmd(const char *cmd)
{
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const целое len = ll_entry_count(struct cmd_tbl, cmd);
	вернуть find_cmd_tbl(cmd, start, len);
}

целое cmd_usage(const struct cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	если (!cmdtp->help) {
		puts ("- No additional help available.\n");
		вернуть 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	вернуть 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static char env_complete_buf[512];

целое var_complete(целое argc, char *const argv[], char last_char, целое maxv,
		 char *cmdv[])
{
	целое space;

	space = last_char == '\0' || isblank(last_char);

	если (space && argc == 1)
		вернуть env_complete("", maxv, cmdv, sizeof(env_complete_buf),
				    env_complete_buf, false);

	если (!space && argc == 2)
		вернуть env_complete(argv[1], maxv, cmdv,
				    sizeof(env_complete_buf),
				    env_complete_buf, false);

	вернуть 0;
}

static целое dollar_complete(целое argc, char *const argv[], char last_char,
			   целое maxv, char *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	если (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		вернуть 0;

	вернуть env_complete(argv[argc - 1], maxv, cmdv, sizeof(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

целое complete_subcmdv(struct cmd_tbl *cmdtp, целое count, целое argc,
		     char *const argv[], char last_char,
		     целое maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const struct cmd_tbl *cmdend = cmdtp + count;
	const char *p;
	целое len, clen;
	целое n_found = 0;
	const char *cmd;

	/* sanity? */
	если (maxv < 2)
		вернуть -2;

	cmdv[0] = NULL;

	если (argc == 0) {
		/* output full list of commands */
		для (; cmdtp != cmdend; cmdtp++) {
			если (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		вернуть n_found;
	}

	/* more than one arg or one but the start of the next */
	если (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		если (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			вернуть 0;
		}
		вернуть (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	если (p == NULL)
		len = strlen(cmd);
	иначе
		len = p - cmd;

	/* return the partial matches */
	для (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		если (clen < len)
			continue;

		если (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		если (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	вернуть n_found;
#иначе
	вернуть 0;
#endif
}

static целое complete_cmdv(целое argc, char *const argv[], char last_char,
			 целое maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	вернуть complete_subcmdv(ll_entry_start(struct cmd_tbl, cmd),
				ll_entry_count(struct cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#иначе
	вернуть 0;
#endif
}

static целое make_argv(char *s, целое argvsz, char *argv[])
{
	целое argc = 0;

	/* split into argv */
	пока (argc < argvsz - 1) {

		/* skip any white space */
		пока (isblank(*s))
			++s;

		если (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		пока (*s && !isblank(*s))
			++s;

		если (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	вернуть argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep,
		       целое linemax, char *const argv[])
{
	целое ll = leader != NULL ? strlen(leader) : 0;
	целое sl = sep != NULL ? strlen(sep) : 0;
	целое len, i;

	если (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	пока (*argv != NULL) {
		len = strlen(*argv) + sl;
		если (i + len >= linemax) {
			puts("\n");
			если (leader)
				puts(leader);
			i = ll - sl;
		} иначе если (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static целое find_common_prefix(char *const argv[])
{
	целое i, len;
	char *anchor, *s, *t;

	если (*argv == NULL)
		вернуть 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	пока ((t = *argv++) != NULL) {
		s = anchor;
		для (i = 0; i < len; i++, t++, s++) {
			если (*t != *s)
				break;
		}
		len = s - anchor;
	}
	вернуть len;
}

целое cmd_auto_complete(const char *const prompt, char *buf, целое *np, целое *colp)
{
	char tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	целое n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	целое i, j, k, len, seplen, argc;
	целое cnt;
	char last_char;
	const char *ps_prompt;

	если (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		если (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} иначе
		ps_prompt = CONFIG_SYS_PROMPT;

	если (strcmp(prompt, ps_prompt) != 0)
		вернуть 0;	/* not in normal console */

	cnt = strlen(buf);
	если (cnt >= 1)
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
	если (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	если (i == 0) {
		если (argc > 1)	/* allow tab for non command */
			вернуть 0;
		putc('\a');
		вернуть 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	если (i == 1) { /* one match; perfect */
		если (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		иначе
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} иначе если (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		если (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		иначе
			k = 0;

		j -= k;
		если (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	если (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		если (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			вернуть 1;
		}

		t = buf + cnt;
		для (i = 0; i < len; i++)
			*t++ = *s++;
		если (sep != NULL)
			для (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		если (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} иначе {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	вернуть 1;
}

#endif

#ifdef CMD_DATA_SIZE
целое cmd_get_data_size(const char *arg, целое default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	целое len = strlen(arg);
	если (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		case 'b':
			вернуть 1;
		case 'w':
			вернуть 2;
		case 'l':
			вернуть 4;
		case 's':
			вернуть CMD_DATA_SIZE_STR;
		case 'q':
			если (MEM_SUPPORT_64BIT_DATA)
				вернуть 8;
			fallthrough;
		default:
			вернуть CMD_DATA_SIZE_ERR;
		}
	}
	вернуть default_size;
}
#endif

void fixup_cmdtable(struct cmd_tbl *cmdtp, целое size)
{
	целое	i;

	если (gd->reloc_off == 0)
		вернуть;

	для (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(целое (*)(struct cmd_tbl *, целое, целое,
				 char * const [], целое *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (целое (*)(struct cmd_tbl *, целое, целое,
				      char *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;
		если (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		если (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		если (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(целое (*)(целое, char * const [], char, целое, char * []))addr;
		}
#endif
		cmdtp++;
	}
}

целое cmd_always_repeatable(struct cmd_tbl *cmdtp, целое flag, целое argc,
			  char *const argv[], целое *repeatable)
{
	*repeatable = 1;

	вернуть cmdtp->cmd(cmdtp, flag, argc, argv);
}

целое cmd_never_repeatable(struct cmd_tbl *cmdtp, целое flag, целое argc,
			 char *const argv[], целое *repeatable)
{
	*repeatable = 0;

	вернуть cmdtp->cmd(cmdtp, flag, argc, argv);
}

целое cmd_discard_repeatable(struct cmd_tbl *cmdtp, целое flag, целое argc,
			   char *const argv[])
{
	целое repeatable;

	вернуть cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static целое cmd_call(struct cmd_tbl *cmdtp, целое flag, целое argc,
		    char *const argv[], целое *repeatable)
{
	целое result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	если (result)
		debug("Command failed, result=%d\n", result);
	вернуть result;
}

enum command_ret_t cmd_process(целое flag, целое argc, char *const argv[],
			       целое *repeatable, ulong *ticks)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct cmd_tbl *cmdtp;

#если defined(CONFIG_SYS_XTRACE)
	char *xtrace;

	xtrace = env_get("xtrace");
	если (xtrace) {
		puts("+");
		для (целое i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	если (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		вернуть 1;
	}

	/* found - check max args */
	если (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#если defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	иначе если (cmdtp->cmd == do_bootd) {
		если (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} иначе {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	если (!rc) {
		целое newrep;

		если (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		если (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	если (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	вернуть rc;
}

целое cmd_process_error(struct cmd_tbl *cmdtp, целое err)
{
	если (err == CMD_RET_USAGE)
		вернуть CMD_RET_USAGE;

	если (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		вернуть CMD_RET_FAILURE;
	}

	вернуть CMD_RET_SUCCESS;
}

целое cmd_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	целое ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	если (ret)
		вернуть CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	вернуть run_command_list(data, len, 0);
}
