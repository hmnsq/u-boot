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

정수 _do_help(struct cmd_tbl *cmd_start, 정수 cmd_items, struct cmd_tbl *cmdtp,
	     정수 flag, 정수 argc, char *const argv[])
{
	정수 i;
	정수 rcode = 0;

	만약 (argc == 1) {	/* show list of commands */
		struct cmd_tbl *cmd_array[cmd_items];
		정수 i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		위해 (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		위해 (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			위해 (j = 0; j < i; ++j) {
				만약 (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			만약 (!swaps)
				break;
		}

		/* print short help (usage) */
		위해 (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			만약 (ctrlc())
				반환 1;
			만약 (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		반환 0;
	}
	/*
	 * command help (long version)
	 */
	위해 (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		만약 (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} 아니면 {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	반환 rcode;
}

/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     정수 table_len)
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *cmdtp;
	struct cmd_tbl *cmdtp_temp = table;	/* Init value */
	const char *p;
	정수 len;
	정수 n_found = 0;

	만약 (!cmd)
		반환 NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	위해 (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		만약 (strncmp(cmd, cmdtp->name, len) == 0) {
			만약 (len == strlen(cmdtp->name))
				반환 cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	만약 (n_found == 1) {			/* exactly one match */
		반환 cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	반환 NULL;	/* not found or ambiguous command */
}

struct cmd_tbl *find_cmd(const char *cmd)
{
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const 정수 len = ll_entry_count(struct cmd_tbl, cmd);
	반환 find_cmd_tbl(cmd, start, len);
}

정수 cmd_usage(const struct cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	만약 (!cmdtp->help) {
		puts ("- No additional help available.\n");
		반환 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	반환 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static char env_complete_buf[512];

정수 var_complete(정수 argc, char *const argv[], char last_char, 정수 maxv,
		 char *cmdv[])
{
	정수 space;

	space = last_char == '\0' || isblank(last_char);

	만약 (space && argc == 1)
		반환 env_complete("", maxv, cmdv, sizeof(env_complete_buf),
				    env_complete_buf, false);

	만약 (!space && argc == 2)
		반환 env_complete(argv[1], maxv, cmdv,
				    sizeof(env_complete_buf),
				    env_complete_buf, false);

	반환 0;
}

static 정수 dollar_complete(정수 argc, char *const argv[], char last_char,
			   정수 maxv, char *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	만약 (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		반환 0;

	반환 env_complete(argv[argc - 1], maxv, cmdv, sizeof(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

정수 complete_subcmdv(struct cmd_tbl *cmdtp, 정수 count, 정수 argc,
		     char *const argv[], char last_char,
		     정수 maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const struct cmd_tbl *cmdend = cmdtp + count;
	const char *p;
	정수 len, clen;
	정수 n_found = 0;
	const char *cmd;

	/* sanity? */
	만약 (maxv < 2)
		반환 -2;

	cmdv[0] = NULL;

	만약 (argc == 0) {
		/* output full list of commands */
		위해 (; cmdtp != cmdend; cmdtp++) {
			만약 (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		반환 n_found;
	}

	/* more than one arg or one but the start of the next */
	만약 (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		만약 (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			반환 0;
		}
		반환 (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	만약 (p == NULL)
		len = strlen(cmd);
	아니면
		len = p - cmd;

	/* return the partial matches */
	위해 (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		만약 (clen < len)
			continue;

		만약 (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		만약 (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	반환 n_found;
#아니면
	반환 0;
#endif
}

static 정수 complete_cmdv(정수 argc, char *const argv[], char last_char,
			 정수 maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	반환 complete_subcmdv(ll_entry_start(struct cmd_tbl, cmd),
				ll_entry_count(struct cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#아니면
	반환 0;
#endif
}

static 정수 make_argv(char *s, 정수 argvsz, char *argv[])
{
	정수 argc = 0;

	/* split into argv */
	동안 (argc < argvsz - 1) {

		/* skip any white space */
		동안 (isblank(*s))
			++s;

		만약 (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		동안 (*s && !isblank(*s))
			++s;

		만약 (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	반환 argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep,
		       정수 linemax, char *const argv[])
{
	정수 ll = leader != NULL ? strlen(leader) : 0;
	정수 sl = sep != NULL ? strlen(sep) : 0;
	정수 len, i;

	만약 (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	동안 (*argv != NULL) {
		len = strlen(*argv) + sl;
		만약 (i + len >= linemax) {
			puts("\n");
			만약 (leader)
				puts(leader);
			i = ll - sl;
		} 아니면 만약 (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static 정수 find_common_prefix(char *const argv[])
{
	정수 i, len;
	char *anchor, *s, *t;

	만약 (*argv == NULL)
		반환 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	동안 ((t = *argv++) != NULL) {
		s = anchor;
		위해 (i = 0; i < len; i++, t++, s++) {
			만약 (*t != *s)
				break;
		}
		len = s - anchor;
	}
	반환 len;
}

정수 cmd_auto_complete(const char *const prompt, char *buf, 정수 *np, 정수 *colp)
{
	char tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	정수 n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	정수 i, j, k, len, seplen, argc;
	정수 cnt;
	char last_char;
	const char *ps_prompt;

	만약 (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		만약 (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} 아니면
		ps_prompt = CONFIG_SYS_PROMPT;

	만약 (strcmp(prompt, ps_prompt) != 0)
		반환 0;	/* not in normal console */

	cnt = strlen(buf);
	만약 (cnt >= 1)
		last_char = buf[cnt - 1];
	아니면
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	만약 (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	만약 (i == 0) {
		만약 (argc > 1)	/* allow tab for non command */
			반환 0;
		putc('\a');
		반환 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	만약 (i == 1) { /* one match; perfect */
		만약 (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		아니면
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} 아니면 만약 (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		만약 (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		아니면
			k = 0;

		j -= k;
		만약 (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	만약 (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		만약 (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			반환 1;
		}

		t = buf + cnt;
		위해 (i = 0; i < len; i++)
			*t++ = *s++;
		만약 (sep != NULL)
			위해 (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		만약 (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} 아니면 {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	반환 1;
}

#endif

#ifdef CMD_DATA_SIZE
정수 cmd_get_data_size(const char *arg, 정수 default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	정수 len = strlen(arg);
	만약 (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		case 'b':
			반환 1;
		case 'w':
			반환 2;
		case 'l':
			반환 4;
		case 's':
			반환 CMD_DATA_SIZE_STR;
		case 'q':
			만약 (MEM_SUPPORT_64BIT_DATA)
				반환 8;
			fallthrough;
		default:
			반환 CMD_DATA_SIZE_ERR;
		}
	}
	반환 default_size;
}
#endif

void fixup_cmdtable(struct cmd_tbl *cmdtp, 정수 size)
{
	정수	i;

	만약 (gd->reloc_off == 0)
		반환;

	위해 (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(정수 (*)(struct cmd_tbl *, 정수, 정수,
				 char * const [], 정수 *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (정수 (*)(struct cmd_tbl *, 정수, 정수,
				      char *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;
		만약 (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		만약 (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		만약 (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(정수 (*)(정수, char * const [], char, 정수, char * []))addr;
		}
#endif
		cmdtp++;
	}
}

정수 cmd_always_repeatable(struct cmd_tbl *cmdtp, 정수 flag, 정수 argc,
			  char *const argv[], 정수 *repeatable)
{
	*repeatable = 1;

	반환 cmdtp->cmd(cmdtp, flag, argc, argv);
}

정수 cmd_never_repeatable(struct cmd_tbl *cmdtp, 정수 flag, 정수 argc,
			 char *const argv[], 정수 *repeatable)
{
	*repeatable = 0;

	반환 cmdtp->cmd(cmdtp, flag, argc, argv);
}

정수 cmd_discard_repeatable(struct cmd_tbl *cmdtp, 정수 flag, 정수 argc,
			   char *const argv[])
{
	정수 repeatable;

	반환 cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static 정수 cmd_call(struct cmd_tbl *cmdtp, 정수 flag, 정수 argc,
		    char *const argv[], 정수 *repeatable)
{
	정수 result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	만약 (result)
		debug("Command failed, result=%d\n", result);
	반환 result;
}

enum command_ret_t cmd_process(정수 flag, 정수 argc, char *const argv[],
			       정수 *repeatable, ulong *ticks)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct cmd_tbl *cmdtp;

#만약 defined(CONFIG_SYS_XTRACE)
	char *xtrace;

	xtrace = env_get("xtrace");
	만약 (xtrace) {
		puts("+");
		위해 (정수 i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	만약 (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		반환 1;
	}

	/* found - check max args */
	만약 (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#만약 defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	아니면 만약 (cmdtp->cmd == do_bootd) {
		만약 (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} 아니면 {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	만약 (!rc) {
		정수 newrep;

		만약 (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		만약 (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	만약 (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	반환 rc;
}

정수 cmd_process_error(struct cmd_tbl *cmdtp, 정수 err)
{
	만약 (err == CMD_RET_USAGE)
		반환 CMD_RET_USAGE;

	만약 (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		반환 CMD_RET_FAILURE;
	}

	반환 CMD_RET_SUCCESS;
}

정수 cmd_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	정수 ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	만약 (ret)
		반환 CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	반환 run_command_list(data, len, 0);
}
