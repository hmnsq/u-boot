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

整数 _do_help(struct cmd_tbl *cmd_start, 整数 cmd_items, struct cmd_tbl *cmdtp,
	     整数 flag, 整数 argc, char *const argv[])
{
	整数 i;
	整数 rcode = 0;

	如果 (argc == 1) {	/* show list of commands */
		struct cmd_tbl *cmd_array[cmd_items];
		整数 i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		为 (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		为 (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			为 (j = 0; j < i; ++j) {
				如果 (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			如果 (!swaps)
				break;
		}

		/* print short help (usage) */
		为 (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			如果 (ctrlc())
				返回 1;
			如果 (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		返回 0;
	}
	/*
	 * command help (long version)
	 */
	为 (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		如果 (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} 否则 {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	返回 rcode;
}

/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     整数 table_len)
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *cmdtp;
	struct cmd_tbl *cmdtp_temp = table;	/* Init value */
	const char *p;
	整数 len;
	整数 n_found = 0;

	如果 (!cmd)
		返回 NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	为 (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		如果 (strncmp(cmd, cmdtp->name, len) == 0) {
			如果 (len == strlen(cmdtp->name))
				返回 cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	如果 (n_found == 1) {			/* exactly one match */
		返回 cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	返回 NULL;	/* not found or ambiguous command */
}

struct cmd_tbl *find_cmd(const char *cmd)
{
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const 整数 len = ll_entry_count(struct cmd_tbl, cmd);
	返回 find_cmd_tbl(cmd, start, len);
}

整数 cmd_usage(const struct cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	如果 (!cmdtp->help) {
		puts ("- No additional help available.\n");
		返回 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	返回 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static char env_complete_buf[512];

整数 var_complete(整数 argc, char *const argv[], char last_char, 整数 maxv,
		 char *cmdv[])
{
	整数 space;

	space = last_char == '\0' || isblank(last_char);

	如果 (space && argc == 1)
		返回 env_complete("", maxv, cmdv, sizeof(env_complete_buf),
				    env_complete_buf, false);

	如果 (!space && argc == 2)
		返回 env_complete(argv[1], maxv, cmdv,
				    sizeof(env_complete_buf),
				    env_complete_buf, false);

	返回 0;
}

static 整数 dollar_complete(整数 argc, char *const argv[], char last_char,
			   整数 maxv, char *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	如果 (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		返回 0;

	返回 env_complete(argv[argc - 1], maxv, cmdv, sizeof(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

整数 complete_subcmdv(struct cmd_tbl *cmdtp, 整数 count, 整数 argc,
		     char *const argv[], char last_char,
		     整数 maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const struct cmd_tbl *cmdend = cmdtp + count;
	const char *p;
	整数 len, clen;
	整数 n_found = 0;
	const char *cmd;

	/* sanity? */
	如果 (maxv < 2)
		返回 -2;

	cmdv[0] = NULL;

	如果 (argc == 0) {
		/* output full list of commands */
		为 (; cmdtp != cmdend; cmdtp++) {
			如果 (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		返回 n_found;
	}

	/* more than one arg or one but the start of the next */
	如果 (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		如果 (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			返回 0;
		}
		返回 (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	如果 (p == NULL)
		len = strlen(cmd);
	否则
		len = p - cmd;

	/* return the partial matches */
	为 (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		如果 (clen < len)
			continue;

		如果 (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		如果 (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	返回 n_found;
#否则
	返回 0;
#endif
}

static 整数 complete_cmdv(整数 argc, char *const argv[], char last_char,
			 整数 maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	返回 complete_subcmdv(ll_entry_start(struct cmd_tbl, cmd),
				ll_entry_count(struct cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#否则
	返回 0;
#endif
}

static 整数 make_argv(char *s, 整数 argvsz, char *argv[])
{
	整数 argc = 0;

	/* split into argv */
	当 (argc < argvsz - 1) {

		/* skip any white space */
		当 (isblank(*s))
			++s;

		如果 (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		当 (*s && !isblank(*s))
			++s;

		如果 (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	返回 argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep,
		       整数 linemax, char *const argv[])
{
	整数 ll = leader != NULL ? strlen(leader) : 0;
	整数 sl = sep != NULL ? strlen(sep) : 0;
	整数 len, i;

	如果 (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	当 (*argv != NULL) {
		len = strlen(*argv) + sl;
		如果 (i + len >= linemax) {
			puts("\n");
			如果 (leader)
				puts(leader);
			i = ll - sl;
		} 否则 如果 (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static 整数 find_common_prefix(char *const argv[])
{
	整数 i, len;
	char *anchor, *s, *t;

	如果 (*argv == NULL)
		返回 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	当 ((t = *argv++) != NULL) {
		s = anchor;
		为 (i = 0; i < len; i++, t++, s++) {
			如果 (*t != *s)
				break;
		}
		len = s - anchor;
	}
	返回 len;
}

整数 cmd_auto_complete(const char *const prompt, char *buf, 整数 *np, 整数 *colp)
{
	char tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	整数 n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	整数 i, j, k, len, seplen, argc;
	整数 cnt;
	char last_char;
	const char *ps_prompt;

	如果 (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		如果 (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} 否则
		ps_prompt = CONFIG_SYS_PROMPT;

	如果 (strcmp(prompt, ps_prompt) != 0)
		返回 0;	/* not in normal console */

	cnt = strlen(buf);
	如果 (cnt >= 1)
		last_char = buf[cnt - 1];
	否则
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	如果 (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	如果 (i == 0) {
		如果 (argc > 1)	/* allow tab for non command */
			返回 0;
		putc('\a');
		返回 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	如果 (i == 1) { /* one match; perfect */
		如果 (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		否则
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} 否则 如果 (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		如果 (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		否则
			k = 0;

		j -= k;
		如果 (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	如果 (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		如果 (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			返回 1;
		}

		t = buf + cnt;
		为 (i = 0; i < len; i++)
			*t++ = *s++;
		如果 (sep != NULL)
			为 (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		如果 (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} 否则 {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	返回 1;
}

#endif

#ifdef CMD_DATA_SIZE
整数 cmd_get_data_size(const char *arg, 整数 default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	整数 len = strlen(arg);
	如果 (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		case 'b':
			返回 1;
		case 'w':
			返回 2;
		case 'l':
			返回 4;
		case 's':
			返回 CMD_DATA_SIZE_STR;
		case 'q':
			如果 (MEM_SUPPORT_64BIT_DATA)
				返回 8;
			fallthrough;
		default:
			返回 CMD_DATA_SIZE_ERR;
		}
	}
	返回 default_size;
}
#endif

void fixup_cmdtable(struct cmd_tbl *cmdtp, 整数 size)
{
	整数	i;

	如果 (gd->reloc_off == 0)
		返回;

	为 (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(整数 (*)(struct cmd_tbl *, 整数, 整数,
				 char * const [], 整数 *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (整数 (*)(struct cmd_tbl *, 整数, 整数,
				      char *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;
		如果 (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		如果 (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		如果 (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(整数 (*)(整数, char * const [], char, 整数, char * []))addr;
		}
#endif
		cmdtp++;
	}
}

整数 cmd_always_repeatable(struct cmd_tbl *cmdtp, 整数 flag, 整数 argc,
			  char *const argv[], 整数 *repeatable)
{
	*repeatable = 1;

	返回 cmdtp->cmd(cmdtp, flag, argc, argv);
}

整数 cmd_never_repeatable(struct cmd_tbl *cmdtp, 整数 flag, 整数 argc,
			 char *const argv[], 整数 *repeatable)
{
	*repeatable = 0;

	返回 cmdtp->cmd(cmdtp, flag, argc, argv);
}

整数 cmd_discard_repeatable(struct cmd_tbl *cmdtp, 整数 flag, 整数 argc,
			   char *const argv[])
{
	整数 repeatable;

	返回 cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static 整数 cmd_call(struct cmd_tbl *cmdtp, 整数 flag, 整数 argc,
		    char *const argv[], 整数 *repeatable)
{
	整数 result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	如果 (result)
		debug("Command failed, result=%d\n", result);
	返回 result;
}

enum command_ret_t cmd_process(整数 flag, 整数 argc, char *const argv[],
			       整数 *repeatable, ulong *ticks)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct cmd_tbl *cmdtp;

#如果 defined(CONFIG_SYS_XTRACE)
	char *xtrace;

	xtrace = env_get("xtrace");
	如果 (xtrace) {
		puts("+");
		为 (整数 i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	如果 (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		返回 1;
	}

	/* found - check max args */
	如果 (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#如果 defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	否则 如果 (cmdtp->cmd == do_bootd) {
		如果 (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} 否则 {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	如果 (!rc) {
		整数 newrep;

		如果 (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		如果 (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	如果 (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	返回 rc;
}

整数 cmd_process_error(struct cmd_tbl *cmdtp, 整数 err)
{
	如果 (err == CMD_RET_USAGE)
		返回 CMD_RET_USAGE;

	如果 (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		返回 CMD_RET_FAILURE;
	}

	返回 CMD_RET_SUCCESS;
}

整数 cmd_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	整数 ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	如果 (ret)
		返回 CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	返回 run_command_list(data, len, 0);
}
