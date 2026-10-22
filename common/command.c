// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#అన్తర్భావ <config.h>
#అన్తర్భావ <compiler.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/ctype.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

పూర్ణాంకం _do_help(నిర్మాణం cmd_tbl *cmd_ప్రారంభం, పూర్ణాంకం cmd_వస్తువులు, నిర్మాణం cmd_tbl *cmdtp,
	     పూర్ణాంకం flag, పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[])
{
	పూర్ణాంకం i;
	పూర్ణాంకం rcode = 0;

	ఒకవేళ (argc == 1) {	/* show list of commands */
		నిర్మాణం cmd_tbl *cmd_array[cmd_వస్తువులు];
		పూర్ణాంకం i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_ప్రారంభం;
		కొరకు (i = 0; i < cmd_వస్తువులు; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		కొరకు (i = cmd_వస్తువులు - 1; i > 0; --i) {
			swaps = 0;
			కొరకు (j = 0; j < i; ++j) {
				ఒకవేళ (strcmp(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					నిర్మాణం cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			ఒకవేళ (!swaps)
				ఆపు;
		}

		/* print short help (usage) */
		కొరకు (i = 0; i < cmd_వస్తువులు; i++) {
			స్థిరరాశి అక్షరం *usage = cmd_array[i]->usage;

			/* allow user abort */
			ఒకవేళ (ctrlc())
				తిరిగి 1;
			ఒకవేళ (usage == ఖాళీ)
				కొనసాగు;
			రాయి("%-*s- %s\n", CFG_SYS_HELP_CMD_వెడల్పు,
			       cmd_array[i]->name, usage);
		}
		తిరిగి 0;
	}
	/*
	 * command help (long version)
	 */
	కొరకు (i = 1; i < argc; ++i) {
		cmdtp = కనుగొను_cmd_tbl(argv[i], cmd_ప్రారంభం, cmd_వస్తువులు);
		ఒకవేళ (cmdtp != ఖాళీ) {
			rcode |= cmd_usage(cmdtp);
		} లేకపోతే {
			రాయి("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	తిరిగి rcode;
}

/* find command table entry for a command */
నిర్మాణం cmd_tbl *కనుగొను_cmd_tbl(స్థిరరాశి అక్షరం *cmd, నిర్మాణం cmd_tbl *పట్టిక,
			     పూర్ణాంకం పట్టిక_len)
{
#నిర్వచితమైతే CONFIG_CMDLINE
	నిర్మాణం cmd_tbl *cmdtp;
	నిర్మాణం cmd_tbl *cmdtp_తాత్కాలిక = పట్టిక;	/* Init value */
	స్థిరరాశి అక్షరం *p;
	పూర్ణాంకం len;
	పూర్ణాంకం n_దొరికింది = 0;

	ఒకవేళ (!cmd)
		తిరిగి ఖాళీ;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == ఖాళీ) ? అక్షరశ్రేణి_పొడవు (cmd) : (p - cmd);

	కొరకు (cmdtp = పట్టిక; cmdtp != పట్టిక + పట్టిక_len; cmdtp++) {
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(cmd, cmdtp->name, len) == 0) {
			ఒకవేళ (len == అక్షరశ్రేణి_పొడవు(cmdtp->name))
				తిరిగి cmdtp;	/* full match */

			cmdtp_తాత్కాలిక = cmdtp;	/* abbreviated command ? */
			n_దొరికింది++;
		}
	}
	ఒకవేళ (n_దొరికింది == 1) {			/* exactly one match */
		తిరిగి cmdtp_తాత్కాలిక;
	}
#అయితే_ముగింపు /* CONFIG_CMDLINE */

	తిరిగి ఖాళీ;	/* not found or ambiguous command */
}

నిర్మాణం cmd_tbl *కనుగొను_cmd(స్థిరరాశి అక్షరం *cmd)
{
	నిర్మాణం cmd_tbl *ప్రారంభం = ll_నమోదు_ప్రారంభం(నిర్మాణం cmd_tbl, cmd);
	స్థిరరాశి పూర్ణాంకం len = ll_నమోదు_లెక్క(నిర్మాణం cmd_tbl, cmd);
	తిరిగి కనుగొను_cmd_tbl(cmd, ప్రారంభం, len);
}

పూర్ణాంకం cmd_usage(స్థిరరాశి నిర్మాణం cmd_tbl *cmdtp)
{
	రాయి("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#నిర్వచితమైతే	CONFIG_SYS_LONGHELP
	రాయి("Usage:\n%s ", cmdtp->name);

	ఒకవేళ (!cmdtp->help) {
		puts ("- No additional help available.\n");
		తిరిగి 1;
	}

	puts(cmdtp->help);
	putc('\n');
#అయితే_ముగింపు	/* CONFIG_SYS_LONGHELP */
	తిరిగి 1;
}

#నిర్వచితమైతే CONFIG_AUTO_COMPLETE
స్థిర అక్షరం env_complete_buf[512];

పూర్ణాంకం var_complete(పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[], అక్షరం చివరి_char, పూర్ణాంకం maxv,
		 అక్షరం *cmdv[])
{
	పూర్ణాంకం space;

	space = చివరి_char == '\0' || isblank(చివరి_char);

	ఒకవేళ (space && argc == 1)
		తిరిగి env_complete("", maxv, cmdv, పరిమాణం(env_complete_buf),
				    env_complete_buf, false);

	ఒకవేళ (!space && argc == 2)
		తిరిగి env_complete(argv[1], maxv, cmdv,
				    పరిమాణం(env_complete_buf),
				    env_complete_buf, false);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం dollar_complete(పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[], అక్షరం చివరి_char,
			   పూర్ణాంకం maxv, అక్షరం *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	ఒకవేళ (argc < 1 || argv[argc - 1][0] != '$' ||
	    చివరి_char == '\0' || isblank(చివరి_char))
		తిరిగి 0;

	తిరిగి env_complete(argv[argc - 1], maxv, cmdv, పరిమాణం(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

పూర్ణాంకం complete_subcmdv(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం లెక్క, పూర్ణాంకం argc,
		     అక్షరం *స్థిరరాశి argv[], అక్షరం చివరి_char,
		     పూర్ణాంకం maxv, అక్షరం *cmdv[])
{
#నిర్వచితమైతే CONFIG_CMDLINE
	స్థిరరాశి నిర్మాణం cmd_tbl *cmdend = cmdtp + లెక్క;
	స్థిరరాశి అక్షరం *p;
	పూర్ణాంకం len, clen;
	పూర్ణాంకం n_దొరికింది = 0;
	స్థిరరాశి అక్షరం *cmd;

	/* sanity? */
	ఒకవేళ (maxv < 2)
		తిరిగి -2;

	cmdv[0] = ఖాళీ;

	ఒకవేళ (argc == 0) {
		/* output full list of commands */
		కొరకు (; cmdtp != cmdend; cmdtp++) {
			ఒకవేళ (n_దొరికింది >= maxv - 2) {
				cmdv[n_దొరికింది++] = "...";
				ఆపు;
			}
			cmdv[n_దొరికింది++] = cmdtp->name;
		}
		cmdv[n_దొరికింది] = ఖాళీ;
		తిరిగి n_దొరికింది;
	}

	/* more than one arg or one but the start of the next */
	ఒకవేళ (argc > 1 || చివరి_char == '\0' || isblank(చివరి_char)) {
		cmdtp = కనుగొను_cmd_tbl(argv[0], cmdtp, లెక్క);
		ఒకవేళ (cmdtp == ఖాళీ || cmdtp->complete == ఖాళీ) {
			cmdv[0] = ఖాళీ;
			తిరిగి 0;
		}
		తిరిగి (*cmdtp->complete)(argc, argv, చివరి_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	ఒకవేళ (p == ఖాళీ)
		len = అక్షరశ్రేణి_పొడవు(cmd);
	లేకపోతే
		len = p - cmd;

	/* return the partial matches */
	కొరకు (; cmdtp != cmdend; cmdtp++) {

		clen = అక్షరశ్రేణి_పొడవు(cmdtp->name);
		ఒకవేళ (clen < len)
			కొనసాగు;

		ఒకవేళ (memcmp(cmd, cmdtp->name, len) != 0)
			కొనసాగు;

		/* too many! */
		ఒకవేళ (n_దొరికింది >= maxv - 2) {
			cmdv[n_దొరికింది++] = "...";
			ఆపు;
		}

		cmdv[n_దొరికింది++] = cmdtp->name;
	}

	cmdv[n_దొరికింది] = ఖాళీ;
	తిరిగి n_దొరికింది;
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం complete_cmdv(పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[], అక్షరం చివరి_char,
			 పూర్ణాంకం maxv, అక్షరం *cmdv[])
{
#నిర్వచితమైతే CONFIG_CMDLINE
	తిరిగి complete_subcmdv(ll_నమోదు_ప్రారంభం(నిర్మాణం cmd_tbl, cmd),
				ll_నమోదు_లెక్క(నిర్మాణం cmd_tbl, cmd), argc, argv,
				చివరి_char, maxv, cmdv);
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం make_argv(అక్షరం *s, పూర్ణాంకం argvsz, అక్షరం *argv[])
{
	పూర్ణాంకం argc = 0;

	/* split into argv */
	అయితే (argc < argvsz - 1) {

		/* skip any white space */
		అయితే (isblank(*s))
			++s;

		ఒకవేళ (*s == '\0')	/* end of s, no more args	*/
			ఆపు;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		అయితే (*s && !isblank(*s))
			++s;

		ఒకవేళ (*s == '\0')		/* end of s, no more args	*/
			ఆపు;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = ఖాళీ;

	తిరిగి argc;
}

స్థిర శూన్య print_argv(స్థిరరాశి అక్షరం *banner, స్థిరరాశి అక్షరం *leader, స్థిరరాశి అక్షరం *sep,
		       పూర్ణాంకం linemax, అక్షరం *స్థిరరాశి argv[])
{
	పూర్ణాంకం ll = leader != ఖాళీ ? అక్షరశ్రేణి_పొడవు(leader) : 0;
	పూర్ణాంకం sl = sep != ఖాళీ ? అక్షరశ్రేణి_పొడవు(sep) : 0;
	పూర్ణాంకం len, i;

	ఒకవేళ (banner) {
		puts("\n");
		puts(banner);
	}

	i = linemax;	/* force leader and newline */
	అయితే (*argv != ఖాళీ) {
		len = అక్షరశ్రేణి_పొడవు(*argv) + sl;
		ఒకవేళ (i + len >= linemax) {
			puts("\n");
			ఒకవేళ (leader)
				puts(leader);
			i = ll - sl;
		} లేకపోతే ఒకవేళ (sep)
			puts(sep);
		puts(*argv++);
		i += len;
	}
	రాయి("\n");
}

స్థిర పూర్ణాంకం కనుగొను_common_ఉపసర్గ(అక్షరం *స్థిరరాశి argv[])
{
	పూర్ణాంకం i, len;
	అక్షరం *anchor, *s, *t;

	ఒకవేళ (*argv == ఖాళీ)
		తిరిగి 0;

	/* begin with max */
	anchor = *argv++;
	len = అక్షరశ్రేణి_పొడవు(anchor);
	అయితే ((t = *argv++) != ఖాళీ) {
		s = anchor;
		కొరకు (i = 0; i < len; i++, t++, s++) {
			ఒకవేళ (*t != *s)
				ఆపు;
		}
		len = s - anchor;
	}
	తిరిగి len;
}

పూర్ణాంకం cmd_auto_complete(స్థిరరాశి అక్షరం *స్థిరరాశి prompt, అక్షరం *buf, పూర్ణాంకం *np, పూర్ణాంకం *colp)
{
	అక్షరం tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	పూర్ణాంకం n = *np, నిలువు = *colp;
	అక్షరం *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	అక్షరం *cmdv[20];
	అక్షరం *s, *t;
	స్థిరరాశి అక్షరం *sep;
	పూర్ణాంకం i, j, k, len, seplen, argc;
	పూర్ణాంకం cnt;
	అక్షరం చివరి_char;
	స్థిరరాశి అక్షరం *ps_prompt;

	ఒకవేళ (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		ఒకవేళ (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} లేకపోతే
		ps_prompt = CONFIG_SYS_PROMPT;

	ఒకవేళ (strcmp(prompt, ps_prompt) != 0)
		తిరిగి 0;	/* not in normal console */

	cnt = అక్షరశ్రేణి_పొడవు(buf);
	ఒకవేళ (cnt >= 1)
		చివరి_char = buf[cnt - 1];
	లేకపోతే
		చివరి_char = '\0';

	/* copy to secondary buffer which will be affected */
	strcpy(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, పరిమాణం(argv)/పరిమాణం(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, చివరి_char,
			    పరిమాణం(cmdv) / పరిమాణం(cmdv[0]), cmdv);
	ఒకవేళ (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, చివరి_char,
				  పరిమాణం(cmdv) / పరిమాణం(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	ఒకవేళ (i == 0) {
		ఒకవేళ (argc > 1)	/* allow tab for non command */
			తిరిగి 0;
		putc('\a');
		తిరిగి 1;
	}

	s = ఖాళీ;
	len = 0;
	sep = ఖాళీ;
	seplen = 0;
	ఒకవేళ (i == 1) { /* one match; perfect */
		ఒకవేళ (చివరి_char != '\0' && !isblank(చివరి_char))
			k = అక్షరశ్రేణి_పొడవు(argv[argc - 1]);
		లేకపోతే
			k = 0;

		s = cmdv[0] + k;
		len = అక్షరశ్రేణి_పొడవు(s);
		sep = " ";
		seplen = 1;
	} లేకపోతే ఒకవేళ (i > 1 && (j = కనుగొను_common_ఉపసర్గ(cmdv)) != 0) { /*మరిన్ని*/
		ఒకవేళ (చివరి_char != '\0' && !isblank(చివరి_char))
			k = అక్షరశ్రేణి_పొడవు(argv[argc - 1]);
		లేకపోతే
			k = 0;

		j -= k;
		ఒకవేళ (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	ఒకవేళ (s != ఖాళీ) {
		k = len + seplen;
		/* make sure it fits */
		ఒకవేళ (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			తిరిగి 1;
		}

		t = buf + cnt;
		కొరకు (i = 0; i < len; i++)
			*t++ = *s++;
		ఒకవేళ (sep != ఖాళీ)
			కొరకు (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		నిలువు += k;
		puts(t - k);
		ఒకవేళ (sep == ఖాళీ)
			putc('\a');
		*np = n;
		*colp = నిలువు;
	} లేకపోతే {
		print_argv(ఖాళీ, "  ", " ", 78, cmdv);

		puts(prompt);
		puts(buf);
	}
	తిరిగి 1;
}

#అయితే_ముగింపు

#నిర్వచితమైతే CMD_దత్తాంశం_కొలత
పూర్ణాంకం cmd_get_దత్తాంశం_కొలత(స్థిరరాశి అక్షరం *arg, పూర్ణాంకం default_కొలత)
{
	/* Check for a size specification .b, .w or .l.
	 */
	పూర్ణాంకం len = అక్షరశ్రేణి_పొడవు(arg);
	ఒకవేళ (len >= 2 && arg[len-2] == '.') {
		ఎంపిక (arg[len-1]) {
		స్థితి 'b':
			తిరిగి 1;
		స్థితి 'w':
			తిరిగి 2;
		స్థితి 'l':
			తిరిగి 4;
		స్థితి 's':
			తిరిగి CMD_దత్తాంశం_కొలత_STR;
		స్థితి 'q':
			ఒకవేళ (MEM_SUPPORT_64_బిట్టు_దత్తాంశం)
				తిరిగి 8;
			fallthrough;
		అప్రమేయ:
			తిరిగి CMD_దత్తాంశం_కొలత_ERR;
		}
	}
	తిరిగి default_కొలత;
}
#అయితే_ముగింపు

శూన్య fixup_cmdtable(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం కొలత)
{
	పూర్ణాంకం	i;

	ఒకవేళ (gd->reloc_off == 0)
		తిరిగి;

	కొరకు (i = 0; i < కొలత; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(పూర్ణాంకం (*)(నిర్మాణం cmd_tbl *, పూర్ణాంకం, పూర్ణాంకం,
				 అక్షరం * స్థిరరాశి [], పూర్ణాంకం *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#నిర్వచితమైతే DEBUG_COMMANDS
		రాయి("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#అయితే_ముగింపు
		cmdtp->cmd = (పూర్ణాంకం (*)(నిర్మాణం cmd_tbl *, పూర్ణాంకం, పూర్ణాంకం,
				      అక్షరం *స్థిరరాశి []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (అక్షరం *)addr;
		ఒకవేళ (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (అక్షరం *)addr;
		}
#నిర్వచితమైతే	CONFIG_SYS_LONGHELP
		ఒకవేళ (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (అక్షరం *)addr;
		}
#అయితే_ముగింపు
#నిర్వచితమైతే CONFIG_AUTO_COMPLETE
		ఒకవేళ (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(పూర్ణాంకం (*)(పూర్ణాంకం, అక్షరం * స్థిరరాశి [], అక్షరం, పూర్ణాంకం, అక్షరం * []))addr;
		}
#అయితే_ముగింపు
		cmdtp++;
	}
}

పూర్ణాంకం cmd_always_repeatable(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc,
			  అక్షరం *స్థిరరాశి argv[], పూర్ణాంకం *repeatable)
{
	*repeatable = 1;

	తిరిగి cmdtp->cmd(cmdtp, flag, argc, argv);
}

పూర్ణాంకం cmd_never_repeatable(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc,
			 అక్షరం *స్థిరరాశి argv[], పూర్ణాంకం *repeatable)
{
	*repeatable = 0;

	తిరిగి cmdtp->cmd(cmdtp, flag, argc, argv);
}

పూర్ణాంకం cmd_discard_repeatable(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc,
			   అక్షరం *స్థిరరాశి argv[])
{
	పూర్ణాంకం repeatable;

	తిరిగి cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
స్థిర పూర్ణాంకం cmd_call(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc,
		    అక్షరం *స్థిరరాశి argv[], పూర్ణాంకం *repeatable)
{
	పూర్ణాంకం ఫలితం;

	ఫలితం = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	ఒకవేళ (ఫలితం)
		debug("Command failed, result=%d\n", ఫలితం);
	తిరిగి ఫలితం;
}

గణన command_ret_t cmd_process(పూర్ణాంకం flag, పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[],
			       పూర్ణాంకం *repeatable, ulong *ticks)
{
	గణన command_ret_t rc = CMD_RET_SUCCESS;
	నిర్మాణం cmd_tbl *cmdtp;

#ఒకవేళ defined(CONFIG_SYS_XTRACE)
	అక్షరం *xtrace;

	xtrace = env_get("xtrace");
	ఒకవేళ (xtrace) {
		puts("+");
		కొరకు (పూర్ణాంకం i = 0; i < argc; i++) {
			puts(" ");
			puts(argv[i]);
		}
		puts("\n");
	}
#అయితే_ముగింపు

	/* Look up command in command table */
	cmdtp = కనుగొను_cmd(argv[0]);
	ఒకవేళ (cmdtp == ఖాళీ) {
		రాయి("Unknown command '%s' - try 'help'\n", argv[0]);
		తిరిగి 1;
	}

	/* found - check max args */
	ఒకవేళ (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#ఒకవేళ defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	లేకపోతే ఒకవేళ (cmdtp->cmd == do_bootd) {
		ఒకవేళ (flag & CMD_FLAG_BOOTD) {
			puts("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} లేకపోతే {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#అయితే_ముగింపు

	/* If OK so far, then do the command */
	ఒకవేళ (!rc) {
		పూర్ణాంకం newrep;

		ఒకవేళ (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		ఒకవేళ (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	ఒకవేళ (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	తిరిగి rc;
}

పూర్ణాంకం cmd_process_error(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం err)
{
	ఒకవేళ (err == CMD_RET_USAGE)
		తిరిగి CMD_RET_USAGE;

	ఒకవేళ (err) {
		రాయి("Command '%s' failed: Error %d\n", cmdtp->name, err);
		తిరిగి CMD_RET_FAILURE;
	}

	తిరిగి CMD_RET_SUCCESS;
}

పూర్ణాంకం cmd_ఆధారం_script(ulong addr, స్థిరరాశి అక్షరం *fit_uname, స్థిరరాశి అక్షరం *confname)
{
	అక్షరం *దత్తాంశం;
	శూన్య *buf;
	uint len;
	పూర్ణాంకం ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &దత్తాంశం, &len);
	unmap_sysmem(buf);
	ఒకవేళ (ret)
		తిరిగి CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	తిరిగి నిరంతరం_command_list(దత్తాంశం, len, 0);
}
