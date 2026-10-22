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

ціле _do_help(struct cmd_tbl *cmd_start, ціле cmd_items, struct cmd_tbl *cmdtp,
	     ціле flag, ціле argc, char *const argv[])
{
	ціле i;
	ціле rcode = 0;

	якщо (argc == 1) {	/* show list of commands */
		struct cmd_tbl *cmd_array[cmd_items];
		ціле i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		для (i = 0; i < cmd_items; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		для (i = cmd_items - 1; i > 0; --i) {
			swaps = 0;
			для (j = 0; j < i; ++j) {
				якщо (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					struct cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			якщо (!swaps)
				break;
		}

		/* print short help (usage) */
		для (i = 0; i < cmd_items; i++) {
			const char *usage = cmd_array[i]->usage;

			/* allow user abort */
			якщо (ctrlc())
				повернути 1;
			якщо (usage == NULL)
				continue;
			printf("%-*s- %s\n", CFG_SYS_HELP_CMD_WIDTH,
			       cmd_array[i]->name, usage);
		}
		повернути 0;
	}
	/*
	 * command help (long version)
	 */
	для (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_items);
		якщо (cmdtp != NULL) {
			rcode |= cmd_usage(cmdtp);
		} інакше {
			printf("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	повернути rcode;
}

/* find command table entry for a command */
struct cmd_tbl *find_cmd_tbl(const char *cmd, struct cmd_tbl *table,
			     ціле table_len)
{
#ifdef CONFIG_CMDLINE
	struct cmd_tbl *cmdtp;
	struct cmd_tbl *cmdtp_temp = table;	/* Init value */
	const char *p;
	ціле len;
	ціле n_found = 0;

	якщо (!cmd)
		повернути NULL;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	для (cmdtp = table; cmdtp != table + table_len; cmdtp++) {
		якщо (strncmp(cmd, cmdtp->name, len) == 0) {
			якщо (len == strlen(cmdtp->name))
				повернути cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	якщо (n_found == 1) {			/* exactly one match */
		повернути cmdtp_temp;
	}
#endif /* CONFIG_CMDLINE */

	повернути NULL;	/* not found or ambiguous command */
}

struct cmd_tbl *find_cmd(const char *cmd)
{
	struct cmd_tbl *start = ll_entry_start(struct cmd_tbl, cmd);
	const ціле len = ll_entry_count(struct cmd_tbl, cmd);
	повернути find_cmd_tbl(cmd, start, len);
}

ціле cmd_usage(const struct cmd_tbl *cmdtp)
{
	printf("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ifdef	CONFIG_SYS_LONGHELP
	printf("Usage:\n%s ", cmdtp->name);

	якщо (!cmdtp->help) {
		puts ("- No additional help available.\n");
		повернути 1;
	}

	puts(cmdtp->help);
	putc('\n');
#endif	/* CONFIG_SYS_LONGHELP */
	повернути 1;
}

#ifdef CONFIG_AUTO_COMPLETE
static char env_complete_buf[512];

ціле var_complete(ціле argc, char *const argv[], char last_char, ціле maxv,
		 char *cmdv[])
{
	ціле space;

	space = last_char == '\0' || isblank(last_char);

	якщо (space && argc == 1)
		повернути env_complete("", maxv, cmdv, sizeof(env_complete_buf),
				    env_complete_buf, false);

	якщо (!space && argc == 2)
		повернути env_complete(argv[1], maxv, cmdv,
				    sizeof(env_complete_buf),
				    env_complete_buf, false);

	повернути 0;
}

static ціле dollar_complete(ціле argc, char *const argv[], char last_char,
			   ціле maxv, char *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	якщо (argc < 1 || argv[argc - 1][0] != '$' ||
	    last_char == '\0' || isblank(last_char))
		повернути 0;

	повернути env_complete(argv[argc - 1], maxv, cmdv, sizeof(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ціле complete_subcmdv(struct cmd_tbl *cmdtp, ціле count, ціле argc,
		     char *const argv[], char last_char,
		     ціле maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	const struct cmd_tbl *cmdend = cmdtp + count;
	const char *p;
	ціле len, clen;
	ціле n_found = 0;
	const char *cmd;

	/* sanity? */
	якщо (maxv < 2)
		повернути -2;

	cmdv[0] = NULL;

	якщо (argc == 0) {
		/* output full list of commands */
		для (; cmdtp != cmdend; cmdtp++) {
			якщо (n_found >= maxv - 2) {
				cmdv[n_found++] = "...";
				break;
			}
			cmdv[n_found++] = cmdtp->name;
		}
		cmdv[n_found] = NULL;
		повернути n_found;
	}

	/* more than one arg or one but the start of the next */
	якщо (argc > 1 || last_char == '\0' || isblank(last_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, count);
		якщо (cmdtp == NULL || cmdtp->complete == NULL) {
			cmdv[0] = NULL;
			повернути 0;
		}
		повернути (*cmdtp->complete)(argc, argv, last_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	якщо (p == NULL)
		len = strlen(cmd);
	інакше
		len = p - cmd;

	/* return the partial matches */
	для (; cmdtp != cmdend; cmdtp++) {

		clen = strlen(cmdtp->name);
		якщо (clen < len)
			continue;

		якщо (memcmp(cmd, cmdtp->name, len) != 0)
			continue;

		/* too many! */
		якщо (n_found >= maxv - 2) {
			cmdv[n_found++] = "...";
			break;
		}

		cmdv[n_found++] = cmdtp->name;
	}

	cmdv[n_found] = NULL;
	повернути n_found;
#інакше
	повернути 0;
#endif
}

static ціле complete_cmdv(ціле argc, char *const argv[], char last_char,
			 ціле maxv, char *cmdv[])
{
#ifdef CONFIG_CMDLINE
	повернути complete_subcmdv(ll_entry_start(struct cmd_tbl, cmd),
				ll_entry_count(struct cmd_tbl, cmd), argc, argv,
				last_char, maxv, cmdv);
#інакше
	повернути 0;
#endif
}

static ціле make_argv(char *s, ціле argvsz, char *argv[])
{
	ціле argc = 0;

	/* split into argv */
	поки (argc < argvsz - 1) {

		/* skip any white space */
		поки (isblank(*s))
			++s;

		якщо (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		поки (*s && !isblank(*s))
			++s;

		якщо (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	повернути argc;
}

static void print_argv(const char *banner, const char *leader, const char *sep,
		       ціле linemax, char *const argv[])
{
	ціле ll = leader != NULL ? strlen(leader) : 0;
	ціле sl = sep != NULL ? strlen(sep) : 0;
	ціле len, i;

	якщо (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	поки (*argv != NULL) {
		len = strlen(*argv) + sl;
		якщо (i + len >= linemax) {
			puts("\n");
			якщо (leader)
				puts(leader);
			i = ll - sl;
		} інакше якщо (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	printf("\n");
}

static ціле find_common_prefix(char *const argv[])
{
	ціле i, len;
	char *anchor, *s, *t;

	якщо (*argv == NULL)
		повернути 0;

	/* begin with max */
	anchor = *argv++;
	len = strlen(anchor);
	поки ((t = *argv++) != NULL) {
		s = anchor;
		для (i = 0; i < len; i++, t++, s++) {
			якщо (*t != *s)
				break;
		}
		len = s - anchor;
	}
	повернути len;
}

ціле cmd_auto_complete(const char *const prompt, char *buf, ціле *np, ціле *colp)
{
	char tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ціле n = *np, col = *colp;
	char *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	char *cmdv[20];
	char *s, *t;
	const char *sep;
	ціле i, j, k, len, seplen, argc;
	ціле cnt;
	char last_char;
	const char *ps_prompt;

	якщо (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		якщо (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} інакше
		ps_prompt = CONFIG_SYS_PROMPT;

	якщо (strcmp(prompt, ps_prompt) != 0)
		повернути 0;	/* not in normal console */

	cnt = strlen(buf);
	якщо (cnt >= 1)
		last_char = buf[cnt - 1];
	інакше
		last_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, sizeof(argv)/sizeof(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, last_char,
			    sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	якщо (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, last_char,
				  sizeof(cmdv) / sizeof(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	якщо (i == 0) {
		якщо (argc > 1)	/* allow tab for non command */
			повернути 0;
		putc('\a');
		повернути 1;
	}

	s = NULL;
	len = 0;
	sep = NULL;
	seplen = 0;
	якщо (i == 1) { /* one match; perfect */
		якщо (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		інакше
			k = 0;

		s = cmdv[0] + k;
		len = strlen(s);
		sep = " ";
		seplen = 1;
	} інакше якщо (i > 1 && (j = find_common_prefix(cmdv)) != 0) { /* more */
		якщо (last_char != '\0' && !isblank(last_char))
			k = strlen(argv[argc - 1]);
		інакше
			k = 0;

		j -= k;
		якщо (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	якщо (s != NULL) {
		k = len + seplen;
		/* make sure it fits */
		якщо (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			повернути 1;
		}

		t = buf + cnt;
		для (i = 0; i < len; i++)
			*t++ = *s++;
		якщо (sep != NULL)
			для (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		puts(t - k);
		якщо (sep == NULL)
			putc('\a');
		*np = n;
		*colp = col;
	} інакше {
		print_argv(NULL, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	повернути 1;
}

#endif

#ifdef CMD_DATA_SIZE
ціле cmd_get_data_size(const char *arg, ціле default_size)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ціле len = strlen(arg);
	якщо (len >= 2 && arg[len-2] == '.') {
		switch (arg[len-1]) {
		case 'b':
			повернути 1;
		case 'w':
			повернути 2;
		case 'l':
			повернути 4;
		case 's':
			повернути CMD_DATA_SIZE_STR;
		case 'q':
			якщо (MEM_SUPPORT_64BIT_DATA)
				повернути 8;
			fallthrough;
		default:
			повернути CMD_DATA_SIZE_ERR;
		}
	}
	повернути default_size;
}
#endif

void fixup_cmdtable(struct cmd_tbl *cmdtp, ціле size)
{
	ціле	i;

	якщо (gd->reloc_off == 0)
		повернути;

	для (i = 0; i < size; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ціле (*)(struct cmd_tbl *, ціле, ціле,
				 char * const [], ціле *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ifdef DEBUG_COMMANDS
		printf("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#endif
		cmdtp->cmd = (ціле (*)(struct cmd_tbl *, ціле, ціле,
				      char *const []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (char *)addr;
		якщо (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (char *)addr;
		}
#ifdef	CONFIG_SYS_LONGHELP
		якщо (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (char *)addr;
		}
#endif
#ifdef CONFIG_AUTO_COMPLETE
		якщо (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ціле (*)(ціле, char * const [], char, ціле, char * []))addr;
		}
#endif
		cmdtp++;
	}
}

ціле cmd_always_repeatable(struct cmd_tbl *cmdtp, ціле flag, ціле argc,
			  char *const argv[], ціле *repeatable)
{
	*repeatable = 1;

	повернути cmdtp->cmd(cmdtp, flag, argc, argv);
}

ціле cmd_never_repeatable(struct cmd_tbl *cmdtp, ціле flag, ціле argc,
			 char *const argv[], ціле *repeatable)
{
	*repeatable = 0;

	повернути cmdtp->cmd(cmdtp, flag, argc, argv);
}

ціле cmd_discard_repeatable(struct cmd_tbl *cmdtp, ціле flag, ціле argc,
			   char *const argv[])
{
	ціле repeatable;

	повернути cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
static ціле cmd_call(struct cmd_tbl *cmdtp, ціле flag, ціле argc,
		    char *const argv[], ціле *repeatable)
{
	ціле result;

	result = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	якщо (result)
		debug("Command failed, result=%d\n", result);
	повернути result;
}

enum command_ret_t cmd_process(ціле flag, ціле argc, char *const argv[],
			       ціле *repeatable, ulong *ticks)
{
	enum command_ret_t rc = CMD_RET_SUCCESS;
	struct cmd_tbl *cmdtp;

#якщо defined(CONFIG_SYS_XTRACE)
	char *xtrace;

	xtrace = env_get("xtrace");
	якщо (xtrace) {
		puts("+");
		для (ціле i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#endif

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	якщо (cmdtp == NULL) {
		printf("Unknown command '%s' - try 'help'\n", argv[0]);
		повернути 1;
	}

	/* found - check max args */
	якщо (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#якщо defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	інакше якщо (cmdtp->cmd == do_bootd) {
		якщо (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} інакше {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#endif

	/* If OK so far, then do the command */
	якщо (!rc) {
		ціле newrep;

		якщо (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		якщо (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	якщо (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	повернути rc;
}

ціле cmd_process_error(struct cmd_tbl *cmdtp, ціле err)
{
	якщо (err == CMD_RET_USAGE)
		повернути CMD_RET_USAGE;

	якщо (err) {
		printf("Command '%s' failed: Error %d\n", cmdtp->name, err);
		повернути CMD_RET_FAILURE;
	}

	повернути CMD_RET_SUCCESS;
}

ціле cmd_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	ціле ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	якщо (ret)
		повернути CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	повернути run_command_list(data, len, 0);
}
