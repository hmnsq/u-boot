// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <compiler.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

ಪೂರ್ಣಾಂಕ _do_help(ರಚನೆ cmd_tbl *cmd_ಆರಂಭ, ಪೂರ್ಣಾಂಕ cmd_ವಸ್ತುಗಳು, ರಚನೆ cmd_tbl *cmdtp,
	     ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ಪೂರ್ಣಾಂಕ i;
	ಪೂರ್ಣಾಂಕ rcode = 0;

	ಆದರೆ (argc == 1) {	/* show list of commands */
		ರಚನೆ cmd_tbl *cmd_array[cmd_ವಸ್ತುಗಳು];
		ಪೂರ್ಣಾಂಕ i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_ಆರಂಭ;
		ಗಾಗಿ (i = 0; i < cmd_ವಸ್ತುಗಳು; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		ಗಾಗಿ (i = cmd_ವಸ್ತುಗಳು - 1; i > 0; --i) {
			swaps = 0;
			ಗಾಗಿ (j = 0; j < i; ++j) {
				ಆದರೆ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					ರಚನೆ cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ಆದರೆ (!swaps)
				ನಿಲ್ಲಿಸು;
		}

		/* print short help (usage) */
		ಗಾಗಿ (i = 0; i < cmd_ವಸ್ತುಗಳು; i++) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *usage = cmd_array[i]->usage;

			/* allow user abort */
			ಆದರೆ (ctrlc())
				ಹಿಂತಿರುಗಿ 1;
			ಆದರೆ (usage == ಖಾಲಿ)
				ಮುಂದುವರಿ;
			ಬರೆ("%-*s- %s\n", CFG_SYS_HELP_CMD_ಅಗಲ,
			       cmd_array[i]->name, usage);
		}
		ಹಿಂತಿರುಗಿ 0;
	}
	/*
	 * command help (long version)
	 */
	ಗಾಗಿ (i = 1; i < argc; ++i) {
		cmdtp = ಕಂಡುಹಿಡಿ_cmd_tbl(argv[i], cmd_ಆರಂಭ, cmd_ವಸ್ತುಗಳು);
		ಆದರೆ (cmdtp != ಖಾಲಿ) {
			rcode |= cmd_usage(cmdtp);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಬರೆ("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	ಹಿಂತಿರುಗಿ rcode;
}

/* find command table entry for a command */
ರಚನೆ cmd_tbl *ಕಂಡುಹಿಡಿ_cmd_tbl(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ರಚನೆ cmd_tbl *ಕೋಷ್ಟಕ,
			     ಪೂರ್ಣಾಂಕ ಕೋಷ್ಟಕ_len)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
	ರಚನೆ cmd_tbl *cmdtp;
	ರಚನೆ cmd_tbl *cmdtp_ತಾತ್ಕಾಲಿಕ = ಕೋಷ್ಟಕ;	/* Init value */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ len;
	ಪೂರ್ಣಾಂಕ n_ಸಿಕ್ಕಿತು = 0;

	ಆದರೆ (!cmd)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ಖಾಲಿ) ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ (cmd) : (p - cmd);

	ಗಾಗಿ (cmdtp = ಕೋಷ್ಟಕ; cmdtp != ಕೋಷ್ಟಕ + ಕೋಷ್ಟಕ_len; cmdtp++) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(cmd, cmdtp->name, len) == 0) {
			ಆದರೆ (len == ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmdtp->name))
				ಹಿಂತಿರುಗಿ cmdtp;	/* full match */

			cmdtp_ತಾತ್ಕಾಲಿಕ = cmdtp;	/* abbreviated command ? */
			n_ಸಿಕ್ಕಿತು++;
		}
	}
	ಆದರೆ (n_ಸಿಕ್ಕಿತು == 1) {			/* exactly one match */
		ಹಿಂತಿರುಗಿ cmdtp_ತಾತ್ಕಾಲಿಕ;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_CMDLINE */

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;	/* not found or ambiguous command */
}

ರಚನೆ cmd_tbl *ಕಂಡುಹಿಡಿ_cmd(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd)
{
	ರಚನೆ cmd_tbl *ಆರಂಭ = ll_ನಮೂದು_ಆರಂಭ(ರಚನೆ cmd_tbl, cmd);
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ len = ll_ನಮೂದು_ಎಣಿಕೆ(ರಚನೆ cmd_tbl, cmd);
	ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_cmd_tbl(cmd, ಆರಂಭ, len);
}

ಪೂರ್ಣಾಂಕ cmd_usage(ಸ್ಥಿರಾಂಕ ರಚನೆ cmd_tbl *cmdtp)
{
	ಬರೆ("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ	CONFIG_SYS_LONGHELP
	ಬರೆ("Usage:\n%s ", cmdtp->name);

	ಆದರೆ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		ಹಿಂತಿರುಗಿ 1;
	}

	puts(cmdtp->help);
	putc('\n');
#ಆದರೆ_ಅಂತ್ಯ	/* CONFIG_SYS_LONGHELP */
	ಹಿಂತಿರುಗಿ 1;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AUTO_COMPLETE
ಸ್ಥಿರ ಅಕ್ಷರ env_complete_buf[512];

ಪೂರ್ಣಾಂಕ var_complete(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಅಕ್ಷರ ಕೊನೆಯ_char, ಪೂರ್ಣಾಂಕ maxv,
		 ಅಕ್ಷರ *cmdv[])
{
	ಪೂರ್ಣಾಂಕ space;

	space = ಕೊನೆಯ_char == '\0' || isblank(ಕೊನೆಯ_char);

	ಆದರೆ (space && argc == 1)
		ಹಿಂತಿರುಗಿ env_complete("", maxv, cmdv, ಗಾತ್ರ(env_complete_buf),
				    env_complete_buf, false);

	ಆದರೆ (!space && argc == 2)
		ಹಿಂತಿರುಗಿ env_complete(argv[1], maxv, cmdv,
				    ಗಾತ್ರ(env_complete_buf),
				    env_complete_buf, false);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dollar_complete(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಅಕ್ಷರ ಕೊನೆಯ_char,
			   ಪೂರ್ಣಾಂಕ maxv, ಅಕ್ಷರ *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ಆದರೆ (argc < 1 || argv[argc - 1][0] != '$' ||
	    ಕೊನೆಯ_char == '\0' || isblank(ಕೊನೆಯ_char))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ env_complete(argv[argc - 1], maxv, cmdv, ಗಾತ್ರ(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

ಪೂರ್ಣಾಂಕ complete_subcmdv(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ argc,
		     ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಅಕ್ಷರ ಕೊನೆಯ_char,
		     ಪೂರ್ಣಾಂಕ maxv, ಅಕ್ಷರ *cmdv[])
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
	ಸ್ಥಿರಾಂಕ ರಚನೆ cmd_tbl *cmdend = cmdtp + ಎಣಿಕೆ_2;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ len, clen;
	ಪೂರ್ಣಾಂಕ n_ಸಿಕ್ಕಿತು = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd;

	/* sanity? */
	ಆದರೆ (maxv < 2)
		ಹಿಂತಿರುಗಿ -2;

	cmdv[0] = ಖಾಲಿ;

	ಆದರೆ (argc == 0) {
		/* output full list of commands */
		ಗಾಗಿ (; cmdtp != cmdend; cmdtp++) {
			ಆದರೆ (n_ಸಿಕ್ಕಿತು >= maxv - 2) {
				cmdv[n_ಸಿಕ್ಕಿತು++] = "...";
				ನಿಲ್ಲಿಸು;
			}
			cmdv[n_ಸಿಕ್ಕಿತು++] = cmdtp->name;
		}
		cmdv[n_ಸಿಕ್ಕಿತು] = ಖಾಲಿ;
		ಹಿಂತಿರುಗಿ n_ಸಿಕ್ಕಿತು;
	}

	/* more than one arg or one but the start of the next */
	ಆದರೆ (argc > 1 || ಕೊನೆಯ_char == '\0' || isblank(ಕೊನೆಯ_char)) {
		cmdtp = ಕಂಡುಹಿಡಿ_cmd_tbl(argv[0], cmdtp, ಎಣಿಕೆ_2);
		ಆದರೆ (cmdtp == ಖಾಲಿ || cmdtp->complete == ಖಾಲಿ) {
			cmdv[0] = ಖಾಲಿ;
			ಹಿಂತಿರುಗಿ 0;
		}
		ಹಿಂತಿರುಗಿ (*cmdtp->complete)(argc, argv, ಕೊನೆಯ_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ಆದರೆ (p == ಖಾಲಿ)
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmd);
	ಇಲ್ಲದಿದ್ದರೆ
		len = p - cmd;

	/* return the partial matches */
	ಗಾಗಿ (; cmdtp != cmdend; cmdtp++) {

		clen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmdtp->name);
		ಆದರೆ (clen < len)
			ಮುಂದುವರಿ;

		ಆದರೆ (memcmp(cmd, cmdtp->name, len) != 0)
			ಮುಂದುವರಿ;

		/* too many! */
		ಆದರೆ (n_ಸಿಕ್ಕಿತು >= maxv - 2) {
			cmdv[n_ಸಿಕ್ಕಿತು++] = "...";
			ನಿಲ್ಲಿಸು;
		}

		cmdv[n_ಸಿಕ್ಕಿತು++] = cmdtp->name;
	}

	cmdv[n_ಸಿಕ್ಕಿತು] = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ n_ಸಿಕ್ಕಿತು;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ complete_cmdv(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಅಕ್ಷರ ಕೊನೆಯ_char,
			 ಪೂರ್ಣಾಂಕ maxv, ಅಕ್ಷರ *cmdv[])
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
	ಹಿಂತಿರುಗಿ complete_subcmdv(ll_ನಮೂದು_ಆರಂಭ(ರಚನೆ cmd_tbl, cmd),
				ll_ನಮೂದು_ಎಣಿಕೆ(ರಚನೆ cmd_tbl, cmd), argc, argv,
				ಕೊನೆಯ_char, maxv, cmdv);
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ make_argv(ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ argvsz, ಅಕ್ಷರ *argv[])
{
	ಪೂರ್ಣಾಂಕ argc = 0;

	/* split into argv */
	ಆಗ (argc < argvsz - 1) {

		/* skip any white space */
		ಆಗ (isblank(*s))
			++s;

		ಆದರೆ (*s == '\0')	/* end of s, no more args	*/
			ನಿಲ್ಲಿಸು;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		ಆಗ (*s && !isblank(*s))
			++s;

		ಆದರೆ (*s == '\0')		/* end of s, no more args	*/
			ನಿಲ್ಲಿಸು;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ಖಾಲಿ;

	ಹಿಂತಿರುಗಿ argc;
}

ಸ್ಥಿರ ಶೂನ್ಯ print_argv(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *banner, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *leader, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sep,
		       ಪೂರ್ಣಾಂಕ linemax, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ಪೂರ್ಣಾಂಕ ll = leader != ಖಾಲಿ ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(leader) : 0;
	ಪೂರ್ಣಾಂಕ sl = sep != ಖಾಲಿ ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(sep) : 0;
	ಪೂರ್ಣಾಂಕ len, i;

	ಆದರೆ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	ಆಗ (*argv != ಖಾಲಿ) {
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*argv) + sl;
		ಆದರೆ (i + len >= linemax) {
			puts("\n");
			ಆದರೆ (leader)
				puts(leader);
			i = ll - sl;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	ಬರೆ("\n");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_common_ಪೂರ್ವಪ್ರತ್ಯಯ(ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ಪೂರ್ಣಾಂಕ i, len;
	ಅಕ್ಷರ *anchor, *s, *t;

	ಆದರೆ (*argv == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	/* begin with max */
	anchor = *argv++;
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(anchor);
	ಆಗ ((t = *argv++) != ಖಾಲಿ) {
		s = anchor;
		ಗಾಗಿ (i = 0; i < len; i++, t++, s++) {
			ಆದರೆ (*t != *s)
				ನಿಲ್ಲಿಸು;
		}
		len = s - anchor;
	}
	ಹಿಂತಿರುಗಿ len;
}

ಪೂರ್ಣಾಂಕ cmd_auto_complete(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ prompt, ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ *np, ಪೂರ್ಣಾಂಕ *colp)
{
	ಅಕ್ಷರ tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	ಪೂರ್ಣಾಂಕ n = *np, ಲಂಬಸಾಲು = *colp;
	ಅಕ್ಷರ *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	ಅಕ್ಷರ *cmdv[20];
	ಅಕ್ಷರ *s, *t;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *sep;
	ಪೂರ್ಣಾಂಕ i, j, k, len, seplen, argc;
	ಪೂರ್ಣಾಂಕ cnt;
	ಅಕ್ಷರ ಕೊನೆಯ_char;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ps_prompt;

	ಆದರೆ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ಆದರೆ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} ಇಲ್ಲದಿದ್ದರೆ
		ps_prompt = CONFIG_SYS_PROMPT;

	ಆದರೆ (strcmp(prompt, ps_prompt) != 0)
		ಹಿಂತಿರುಗಿ 0;	/* not in normal console */

	cnt = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf);
	ಆದರೆ (cnt >= 1)
		ಕೊನೆಯ_char = buf[cnt - 1];
	ಇಲ್ಲದಿದ್ದರೆ
		ಕೊನೆಯ_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, ಗಾತ್ರ(argv)/ಗಾತ್ರ(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, ಕೊನೆಯ_char,
			    ಗಾತ್ರ(cmdv) / ಗಾತ್ರ(cmdv[0]), cmdv);
	ಆದರೆ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, ಕೊನೆಯ_char,
				  ಗಾತ್ರ(cmdv) / ಗಾತ್ರ(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ಆದರೆ (i == 0) {
		ಆದರೆ (argc > 1)	/* allow tab for non command */
			ಹಿಂತಿರುಗಿ 0;
		putc('\a');
		ಹಿಂತಿರುಗಿ 1;
	}

	s = ಖಾಲಿ;
	len = 0;
	sep = ಖಾಲಿ;
	seplen = 0;
	ಆದರೆ (i == 1) { /* one match; perfect */
		ಆದರೆ (ಕೊನೆಯ_char != '\0' && !isblank(ಕೊನೆಯ_char))
			k = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[argc - 1]);
		ಇಲ್ಲದಿದ್ದರೆ
			k = 0;

		s = cmdv[0] + k;
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s);
		sep = " ";
		seplen = 1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (i > 1 && (j = ಕಂಡುಹಿಡಿ_common_ಪೂರ್ವಪ್ರತ್ಯಯ(cmdv)) != 0) { /*ಇನ್ನಷ್ಟು*/
		ಆದರೆ (ಕೊನೆಯ_char != '\0' && !isblank(ಕೊನೆಯ_char))
			k = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(argv[argc - 1]);
		ಇಲ್ಲದಿದ್ದರೆ
			k = 0;

		j -= k;
		ಆದರೆ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ಆದರೆ (s != ಖಾಲಿ) {
		k = len + seplen;
		/* make sure it fits */
		ಆದರೆ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			ಹಿಂತಿರುಗಿ 1;
		}

		t = buf + cnt;
		ಗಾಗಿ (i = 0; i < len; i++)
			*t++ = *s++;
		ಆದರೆ (sep != ಖಾಲಿ)
			ಗಾಗಿ (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		ಲಂಬಸಾಲು += k;
		puts(t - k);
		ಆದರೆ (sep == ಖಾಲಿ)
			putc('\a');
		*np = n;
		*colp = ಲಂಬಸಾಲು;
	} ಇಲ್ಲದಿದ್ದರೆ {
		print_argv(ಖಾಲಿ, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	ಹಿಂತಿರುಗಿ 1;
}

#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CMD_ದತ್ತಾಂಶ_ಗಾತ್ರ
ಪೂರ್ಣಾಂಕ cmd_get_ದತ್ತಾಂಶ_ಗಾತ್ರ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಪೂರ್ಣಾಂಕ default_ಗಾತ್ರ)
{
	/* Check for a size specification .b, .w or .l.
	 */
	ಪೂರ್ಣಾಂಕ len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(arg);
	ಆದರೆ (len >= 2 && arg[len-2] == '.') {
		ಆಯ್ಕೆ (arg[len-1]) {
		ಸ್ಥಿತಿ 'b':
			ಹಿಂತಿರುಗಿ 1;
		ಸ್ಥಿತಿ 'w':
			ಹಿಂತಿರುಗಿ 2;
		ಸ್ಥಿತಿ 'l':
			ಹಿಂತಿರುಗಿ 4;
		ಸ್ಥಿತಿ 's':
			ಹಿಂತಿರುಗಿ CMD_ದತ್ತಾಂಶ_ಗಾತ್ರ_STR;
		ಸ್ಥಿತಿ 'q':
			ಆದರೆ (MEM_SUPPORT_64_ಬಿಟ್_ದತ್ತಾಂಶ)
				ಹಿಂತಿರುಗಿ 8;
			fallthrough;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಹಿಂತಿರುಗಿ CMD_ದತ್ತಾಂಶ_ಗಾತ್ರ_ERR;
		}
	}
	ಹಿಂತಿರುಗಿ default_ಗಾತ್ರ;
}
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ fixup_cmdtable(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2)
{
	ಪೂರ್ಣಾಂಕ	i;

	ಆದರೆ (gd->reloc_off == 0)
		ಹಿಂತಿರುಗಿ;

	ಗಾಗಿ (i = 0; i < ಗಾತ್ರ_2; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(ಪೂರ್ಣಾಂಕ (*)(ರಚನೆ cmd_tbl *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ,
				 ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ [], ಪೂರ್ಣಾಂಕ *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG_COMMANDS
		ಬರೆ("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#ಆದರೆ_ಅಂತ್ಯ
		cmdtp->cmd = (ಪೂರ್ಣಾಂಕ (*)(ರಚನೆ cmd_tbl *, ಪೂರ್ಣಾಂಕ, ಪೂರ್ಣಾಂಕ,
				      ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (ಅಕ್ಷರ *)addr;
		ಆದರೆ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (ಅಕ್ಷರ *)addr;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ	CONFIG_SYS_LONGHELP
		ಆದರೆ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (ಅಕ್ಷರ *)addr;
		}
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_AUTO_COMPLETE
		ಆದರೆ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(ಪೂರ್ಣಾಂಕ (*)(ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ [], ಅಕ್ಷರ, ಪೂರ್ಣಾಂಕ, ಅಕ್ಷರ * []))addr;
		}
#ಆದರೆ_ಅಂತ್ಯ
		cmdtp++;
	}
}

ಪೂರ್ಣಾಂಕ cmd_always_repeatable(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc,
			  ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಪೂರ್ಣಾಂಕ *repeatable)
{
	*repeatable = 1;

	ಹಿಂತಿರುಗಿ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ಪೂರ್ಣಾಂಕ cmd_never_repeatable(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc,
			 ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಪೂರ್ಣಾಂಕ *repeatable)
{
	*repeatable = 0;

	ಹಿಂತಿರುಗಿ cmdtp->cmd(cmdtp, flag, argc, argv);
}

ಪೂರ್ಣಾಂಕ cmd_discard_repeatable(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc,
			   ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ಪೂರ್ಣಾಂಕ repeatable;

	ಹಿಂತಿರುಗಿ cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cmd_call(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc,
		    ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[], ಪೂರ್ಣಾಂಕ *repeatable)
{
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ;

	ಫಲಿತಾಂಶ = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ಆದರೆ (ಫಲಿತಾಂಶ)
		debug("Command failed, result=%d\n", ಫಲಿತಾಂಶ);
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಎಣಿಕೆ command_ret_t cmd_process(ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[],
			       ಪೂರ್ಣಾಂಕ *repeatable, ulong *ticks)
{
	ಎಣಿಕೆ command_ret_t rc = CMD_RET_SUCCESS;
	ರಚನೆ cmd_tbl *cmdtp;

#ಆದರೆ defined(CONFIG_SYS_XTRACE)
	ಅಕ್ಷರ *xtrace;

	xtrace = env_get("xtrace");
	ಆದರೆ (xtrace) {
		puts("+");
		ಗಾಗಿ (ಪೂರ್ಣಾಂಕ i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* Look up command in command table */
	cmdtp = ಕಂಡುಹಿಡಿ_cmd(argv[0]);
	ಆದರೆ (cmdtp == ಖಾಲಿ) {
		ಬರೆ("Unknown command '%s' - try 'help'\n", argv[0]);
		ಹಿಂತಿರುಗಿ 1;
	}

	/* found - check max args */
	ಆದರೆ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ಆದರೆ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (cmdtp->cmd == do_bootd) {
		ಆದರೆ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} ಇಲ್ಲದಿದ್ದರೆ {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* If OK so far, then do the command */
	ಆದರೆ (!rc) {
		ಪೂರ್ಣಾಂಕ newrep;

		ಆದರೆ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ಆದರೆ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ಆದರೆ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	ಹಿಂತಿರುಗಿ rc;
}

ಪೂರ್ಣಾಂಕ cmd_process_error(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ err)
{
	ಆದರೆ (err == CMD_RET_USAGE)
		ಹಿಂತಿರುಗಿ CMD_RET_USAGE;

	ಆದರೆ (err) {
		ಬರೆ("Command '%s' failed: Error %d\n", cmdtp->name, err);
		ಹಿಂತಿರುಗಿ CMD_RET_FAILURE;
	}

	ಹಿಂತಿರುಗಿ CMD_RET_SUCCESS;
}

ಪೂರ್ಣಾಂಕ cmd_ಮೂಲ_script(ulong addr, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fit_uname, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *confname)
{
	ಅಕ್ಷರ *ದತ್ತಾಂಶ;
	ಶೂನ್ಯ *buf;
	uint len;
	ಪೂರ್ಣಾಂಕ ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &ದತ್ತಾಂಶ, &len);
	unmap_sysmem(buf);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	ಹಿಂತಿರುಗಿ ಸತತ_command_list(ದತ್ತಾಂಶ, len, 0);
}
