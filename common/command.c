// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2009
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/*
 *  Command Processor Table
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <compiler.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/ctype.h>

DECLARE_GLOBAL_दत्त_PTR;

/*
 * Use puts() instead of printf() to avoid printf buffer overflow
 * for long help messages
 */

पूर्णाङ्क _do_help(संरचना cmd_tbl *cmd_start, पूर्णाङ्क cmd_वस्तूनि, संरचना cmd_tbl *cmdtp,
	     पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i;
	पूर्णाङ्क rcode = 0;

	यदि (argc == 1) {	/* show list of commands */
		संरचना cmd_tbl *cmd_array[cmd_वस्तूनि];
		पूर्णाङ्क i, j, swaps;

		/* Make array of commands from .uboot_cmd section */
		cmdtp = cmd_start;
		प्रति (i = 0; i < cmd_वस्तूनि; i++) {
			cmd_array[i] = cmdtp++;
		}

		/* Sort command list (trivial bubble sort) */
		प्रति (i = cmd_वस्तूनि - 1; i > 0; --i) {
			swaps = 0;
			प्रति (j = 0; j < i; ++j) {
				यदि (वाक्यं_तुलय(cmd_array[j]->name,
					   cmd_array[j + 1]->name) > 0) {
					संरचना cmd_tbl *tmp;
					tmp = cmd_array[j];
					cmd_array[j] = cmd_array[j + 1];
					cmd_array[j + 1] = tmp;
					++swaps;
				}
			}
			यदि (!swaps)
				भङ्ग;
		}

		/* print short help (usage) */
		प्रति (i = 0; i < cmd_वस्तूनि; i++) {
			ध्रुव वर्ण *usage = cmd_array[i]->usage;

			/* allow user abort */
			यदि (ctrlc())
				प्रत्यागम 1;
			यदि (usage == रिक्त)
				अनुवर्त;
			प्रारूप_लिख("%-*s- %s\n", CFG_SYS_HELP_CMD_विस्तारः,
			       cmd_array[i]->name, usage);
		}
		प्रत्यागम 0;
	}
	/*
	 * command help (long version)
	 */
	प्रति (i = 1; i < argc; ++i) {
		cmdtp = find_cmd_tbl(argv[i], cmd_start, cmd_वस्तूनि);
		यदि (cmdtp != रिक्त) {
			rcode |= cmd_usage(cmdtp);
		} अन्यथा {
			प्रारूप_लिख("Unknown command '%s' - try 'help' without arguments for list of all known commands\n\n",
			       argv[i]);
			rcode = 1;
		}
	}
	प्रत्यागम rcode;
}

/* find command table entry for a command */
संरचना cmd_tbl *find_cmd_tbl(ध्रुव वर्ण *cmd, संरचना cmd_tbl *सारणी,
			     पूर्णाङ्क सारणी_len)
{
#यदि_परिभाषितम् CONFIG_CMDLINE
	संरचना cmd_tbl *cmdtp;
	संरचना cmd_tbl *cmdtp_अस्थायी = सारणी;	/* Init value */
	ध्रुव वर्ण *p;
	पूर्णाङ्क len;
	पूर्णाङ्क n_प्राप्तम् = 0;

	यदि (!cmd)
		प्रत्यागम रिक्त;
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == रिक्त) ? वाक्यदैर्घ्यम् (cmd) : (p - cmd);

	प्रति (cmdtp = सारणी; cmdtp != सारणी + सारणी_len; cmdtp++) {
		यदि (वाक्ये_तुलय(cmd, cmdtp->name, len) == 0) {
			यदि (len == वाक्यदैर्घ्यम्(cmdtp->name))
				प्रत्यागम cmdtp;	/* full match */

			cmdtp_अस्थायी = cmdtp;	/* abbreviated command ? */
			n_प्राप्तम्++;
		}
	}
	यदि (n_प्राप्तम् == 1) {			/* exactly one match */
		प्रत्यागम cmdtp_अस्थायी;
	}
#यदि_अन्तः /* CONFIG_CMDLINE */

	प्रत्यागम रिक्त;	/* not found or ambiguous command */
}

संरचना cmd_tbl *find_cmd(ध्रुव वर्ण *cmd)
{
	संरचना cmd_tbl *start = ll_प्रविष्टिः_start(संरचना cmd_tbl, cmd);
	ध्रुव पूर्णाङ्क len = ll_प्रविष्टिः_गणनम्(संरचना cmd_tbl, cmd);
	प्रत्यागम find_cmd_tbl(cmd, start, len);
}

पूर्णाङ्क cmd_usage(ध्रुव संरचना cmd_tbl *cmdtp)
{
	प्रारूप_लिख("%s - %s\n\n", cmdtp->name, cmdtp->usage);

#यदि_परिभाषितम्	CONFIG_SYS_LONGHELP
	प्रारूप_लिख("Usage:\n%s ", cmdtp->name);

	यदि (!cmdtp->help) {
		पङ्क्तिं_लिख ("- No additional help available.\n");
		प्रत्यागम 1;
	}

	पङ्क्तिं_लिख(cmdtp->help);
	putc('\n');
#यदि_अन्तः	/* CONFIG_SYS_LONGHELP */
	प्रत्यागम 1;
}

#यदि_परिभाषितम् CONFIG_AUTO_COMPLETE
स्थिर वर्ण env_complete_buf[512];

पूर्णाङ्क var_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण चरमः_char, पूर्णाङ्क maxv,
		 वर्ण *cmdv[])
{
	पूर्णाङ्क space;

	space = चरमः_char == '\0' || isblank(चरमः_char);

	यदि (space && argc == 1)
		प्रत्यागम env_complete("", maxv, cmdv, परिमाण(env_complete_buf),
				    env_complete_buf, false);

	यदि (!space && argc == 2)
		प्रत्यागम env_complete(argv[1], maxv, cmdv,
				    परिमाण(env_complete_buf),
				    env_complete_buf, false);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क dollar_complete(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण चरमः_char,
			   पूर्णाङ्क maxv, वर्ण *cmdv[])
{
	/* Make sure the last argument starts with a $. */
	यदि (argc < 1 || argv[argc - 1][0] != '$' ||
	    चरमः_char == '\0' || isblank(चरमः_char))
		प्रत्यागम 0;

	प्रत्यागम env_complete(argv[argc - 1], maxv, cmdv, परिमाण(env_complete_buf),
			    env_complete_buf, true);
}

/*************************************************************************************/

पूर्णाङ्क complete_subcmdv(संरचना cmd_tbl *cmdtp, पूर्णाङ्क गणनम्, पूर्णाङ्क argc,
		     वर्ण *ध्रुव argv[], वर्ण चरमः_char,
		     पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#यदि_परिभाषितम् CONFIG_CMDLINE
	ध्रुव संरचना cmd_tbl *cmdend = cmdtp + गणनम्;
	ध्रुव वर्ण *p;
	पूर्णाङ्क len, clen;
	पूर्णाङ्क n_प्राप्तम् = 0;
	ध्रुव वर्ण *cmd;

	/* sanity? */
	यदि (maxv < 2)
		प्रत्यागम -2;

	cmdv[0] = रिक्त;

	यदि (argc == 0) {
		/* output full list of commands */
		प्रति (; cmdtp != cmdend; cmdtp++) {
			यदि (n_प्राप्तम् >= maxv - 2) {
				cmdv[n_प्राप्तम्++] = "...";
				भङ्ग;
			}
			cmdv[n_प्राप्तम्++] = cmdtp->name;
		}
		cmdv[n_प्राप्तम्] = रिक्त;
		प्रत्यागम n_प्राप्तम्;
	}

	/* more than one arg or one but the start of the next */
	यदि (argc > 1 || चरमः_char == '\0' || isblank(चरमः_char)) {
		cmdtp = find_cmd_tbl(argv[0], cmdtp, गणनम्);
		यदि (cmdtp == रिक्त || cmdtp->complete == रिक्त) {
			cmdv[0] = रिक्त;
			प्रत्यागम 0;
		}
		प्रत्यागम (*cmdtp->complete)(argc, argv, चरमः_char, maxv, cmdv);
	}

	cmd = argv[0];
	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	p = strchr(cmd, '.');
	यदि (p == रिक्त)
		len = वाक्यदैर्घ्यम्(cmd);
	अन्यथा
		len = p - cmd;

	/* return the partial matches */
	प्रति (; cmdtp != cmdend; cmdtp++) {

		clen = वाक्यदैर्घ्यम्(cmdtp->name);
		यदि (clen < len)
			अनुवर्त;

		यदि (memcmp(cmd, cmdtp->name, len) != 0)
			अनुवर्त;

		/* too many! */
		यदि (n_प्राप्तम् >= maxv - 2) {
			cmdv[n_प्राप्तम्++] = "...";
			भङ्ग;
		}

		cmdv[n_प्राप्तम्++] = cmdtp->name;
	}

	cmdv[n_प्राप्तम्] = रिक्त;
	प्रत्यागम n_प्राप्तम्;
#अन्यथा
	प्रत्यागम 0;
#यदि_अन्तः
}

स्थिर पूर्णाङ्क complete_cmdv(पूर्णाङ्क argc, वर्ण *ध्रुव argv[], वर्ण चरमः_char,
			 पूर्णाङ्क maxv, वर्ण *cmdv[])
{
#यदि_परिभाषितम् CONFIG_CMDLINE
	प्रत्यागम complete_subcmdv(ll_प्रविष्टिः_start(संरचना cmd_tbl, cmd),
				ll_प्रविष्टिः_गणनम्(संरचना cmd_tbl, cmd), argc, argv,
				चरमः_char, maxv, cmdv);
#अन्यथा
	प्रत्यागम 0;
#यदि_अन्तः
}

स्थिर पूर्णाङ्क make_argv(वर्ण *s, पूर्णाङ्क argvsz, वर्ण *argv[])
{
	पूर्णाङ्क argc = 0;

	/* split into argv */
	यावत् (argc < argvsz - 1) {

		/* skip any white space */
		यावत् (isblank(*s))
			++s;

		यदि (*s == '\0')	/* end of s, no more args	*/
			भङ्ग;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		यावत् (*s && !isblank(*s))
			++s;

		यदि (*s == '\0')		/* end of s, no more args	*/
			भङ्ग;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = रिक्त;

	प्रत्यागम argc;
}

स्थिर शून्य print_argv(ध्रुव वर्ण *banner, ध्रुव वर्ण *leader, ध्रुव वर्ण *sep,
		       पूर्णाङ्क linemax, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क ll = leader != रिक्त ? वाक्यदैर्घ्यम्(leader) : 0;
	पूर्णाङ्क sl = sep != रिक्त ? वाक्यदैर्घ्यम्(sep) : 0;
	पूर्णाङ्क len, i;

	यदि (banner) {
		पङ्क्तिं_लिख("\n");
		पङ्क्तिं_लिख(banner);
	}

	i = linemax;	/* force leader and newline */
	यावत् (*argv != रिक्त) {
		len = वाक्यदैर्घ्यम्(*argv) + sl;
		यदि (i + len >= linemax) {
			पङ्क्तिं_लिख("\n");
			यदि (leader)
				पङ्क्तिं_लिख(leader);
			i = ll - sl;
		} अन्यथा यदि (sep)
			पङ्क्तिं_लिख(sep);
		पङ्क्तिं_लिख(*argv++);
		i += len;
	}
	प्रारूप_लिख("\n");
}

स्थिर पूर्णाङ्क find_common_उपसर्गः(वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i, len;
	वर्ण *anchor, *s, *t;

	यदि (*argv == रिक्त)
		प्रत्यागम 0;

	/* begin with max */
	anchor = *argv++;
	len = वाक्यदैर्घ्यम्(anchor);
	यावत् ((t = *argv++) != रिक्त) {
		s = anchor;
		प्रति (i = 0; i < len; i++, t++, s++) {
			यदि (*t != *s)
				भङ्ग;
		}
		len = s - anchor;
	}
	प्रत्यागम len;
}

पूर्णाङ्क cmd_auto_complete(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, पूर्णाङ्क *np, पूर्णाङ्क *colp)
{
	वर्ण tmp_buf[CONFIG_SYS_CBSIZE + 1];	/* copy of console I/O buffer */
	पूर्णाङ्क n = *np, col = *colp;
	वर्ण *argv[CONFIG_SYS_MAXARGS + 1];		/* NULL terminated	*/
	वर्ण *cmdv[20];
	वर्ण *s, *t;
	ध्रुव वर्ण *sep;
	पूर्णाङ्क i, j, k, len, seplen, argc;
	पूर्णाङ्क cnt;
	वर्ण चरमः_char;
	ध्रुव वर्ण *ps_prompt;

	यदि (IS_ENABLED(CONFIG_CMDLINE_PS_SUPPORT)) {
		ps_prompt = env_get("PS1");

		यदि (!ps_prompt)
			ps_prompt = CONFIG_SYS_PROMPT;
	} अन्यथा
		ps_prompt = CONFIG_SYS_PROMPT;

	यदि (वाक्यं_तुलय(prompt, ps_prompt) != 0)
		प्रत्यागम 0;	/* not in normal console */

	cnt = वाक्यदैर्घ्यम्(buf);
	यदि (cnt >= 1)
		चरमः_char = buf[cnt - 1];
	अन्यथा
		चरमः_char = '\0';

	/* copy to secondary buffer which will be affected */
	वाक्यं_प्रतिलिख(tmp_buf, buf);

	/* separate into argv */
	argc = make_argv(tmp_buf, परिमाण(argv)/परिमाण(argv[0]), argv);

	/* first try a $ completion */
	i = dollar_complete(argc, argv, चरमः_char,
			    परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	यदि (!i) {
		/* do the completion and return the possible completions */
		i = complete_cmdv(argc, argv, चरमः_char,
				  परिमाण(cmdv) / परिमाण(cmdv[0]), cmdv);
	}

	/* no match; bell and out */
	यदि (i == 0) {
		यदि (argc > 1)	/* allow tab for non command */
			प्रत्यागम 0;
		putc('\a');
		प्रत्यागम 1;
	}

	s = रिक्त;
	len = 0;
	sep = रिक्त;
	seplen = 0;
	यदि (i == 1) { /* one match; perfect */
		यदि (चरमः_char != '\0' && !isblank(चरमः_char))
			k = वाक्यदैर्घ्यम्(argv[argc - 1]);
		अन्यथा
			k = 0;

		s = cmdv[0] + k;
		len = वाक्यदैर्घ्यम्(s);
		sep = " ";
		seplen = 1;
	} अन्यथा यदि (i > 1 && (j = find_common_उपसर्गः(cmdv)) != 0) { /*अधिकम्*/
		यदि (चरमः_char != '\0' && !isblank(चरमः_char))
			k = वाक्यदैर्घ्यम्(argv[argc - 1]);
		अन्यथा
			k = 0;

		j -= k;
		यदि (j > 0) {
			s = cmdv[0] + k;
			len = j;
		}
	}

	यदि (s != रिक्त) {
		k = len + seplen;
		/* make sure it fits */
		यदि (n + k >= CONFIG_SYS_CBSIZE - 2) {
			putc('\a');
			प्रत्यागम 1;
		}

		t = buf + cnt;
		प्रति (i = 0; i < len; i++)
			*t++ = *s++;
		यदि (sep != रिक्त)
			प्रति (i = 0; i < seplen; i++)
				*t++ = sep[i];
		*t = '\0';
		n += k;
		col += k;
		पङ्क्तिं_लिख(t - k);
		यदि (sep == रिक्त)
			putc('\a');
		*np = n;
		*colp = col;
	} अन्यथा {
		print_argv(रिक्त, "  ", " ", 78, cmdv);

		पङ्क्तिं_लिख(prompt);
		पङ्क्तिं_लिख(buf);
	}
	प्रत्यागम 1;
}

#यदि_अन्तः

#यदि_परिभाषितम् CMD_दत्त_आकार
पूर्णाङ्क cmd_get_दत्त_आकार(ध्रुव वर्ण *arg, पूर्णाङ्क default_आकार)
{
	/* Check for a size specification .b, .w or .l.
	 */
	पूर्णाङ्क len = वाक्यदैर्घ्यम्(arg);
	यदि (len >= 2 && arg[len-2] == '.') {
		विकल्प (arg[len-1]) {
		पक्ष 'b':
			प्रत्यागम 1;
		पक्ष 'w':
			प्रत्यागम 2;
		पक्ष 'l':
			प्रत्यागम 4;
		पक्ष 's':
			प्रत्यागम CMD_दत्त_आकार_STR;
		पक्ष 'q':
			यदि (MEM_SUPPORT_64_BIT_दत्त)
				प्रत्यागम 8;
			fallthrough;
		स्वभाव:
			प्रत्यागम CMD_दत्त_आकार_ERR;
		}
	}
	प्रत्यागम default_आकार;
}
#यदि_अन्तः

शून्य fixup_cmdtable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क आकार)
{
	पूर्णाङ्क	i;

	यदि (gd->reloc_off == 0)
		प्रत्यागम;

	प्रति (i = 0; i < आकार; i++) {
		ulong addr;

		addr = (ulong)(cmdtp->cmd_rep) + gd->reloc_off;
		cmdtp->cmd_rep =
			(पूर्णाङ्क (*)(संरचना cmd_tbl *, पूर्णाङ्क, पूर्णाङ्क,
				 वर्ण * ध्रुव [], पूर्णाङ्क *))addr;

		addr = (ulong)(cmdtp->cmd) + gd->reloc_off;
#यदि_परिभाषितम् DEBUG_COMMANDS
		प्रारूप_लिख("Command \"%s\": 0x%08lx => 0x%08lx\n",
		       cmdtp->name, (ulong)(cmdtp->cmd), addr);
#यदि_अन्तः
		cmdtp->cmd = (पूर्णाङ्क (*)(संरचना cmd_tbl *, पूर्णाङ्क, पूर्णाङ्क,
				      वर्ण *ध्रुव []))addr;
		addr = (ulong)(cmdtp->name) + gd->reloc_off;
		cmdtp->name = (वर्ण *)addr;
		यदि (cmdtp->usage) {
			addr = (ulong)(cmdtp->usage) + gd->reloc_off;
			cmdtp->usage = (वर्ण *)addr;
		}
#यदि_परिभाषितम्	CONFIG_SYS_LONGHELP
		यदि (cmdtp->help) {
			addr = (ulong)(cmdtp->help) + gd->reloc_off;
			cmdtp->help = (वर्ण *)addr;
		}
#यदि_अन्तः
#यदि_परिभाषितम् CONFIG_AUTO_COMPLETE
		यदि (cmdtp->complete) {
			addr = (ulong)(cmdtp->complete) + gd->reloc_off;
			cmdtp->complete =
				(पूर्णाङ्क (*)(पूर्णाङ्क, वर्ण * ध्रुव [], वर्ण, पूर्णाङ्क, वर्ण * []))addr;
		}
#यदि_अन्तः
		cmdtp++;
	}
}

पूर्णाङ्क cmd_always_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			  वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	*repeatable = 1;

	प्रत्यागम cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णाङ्क cmd_never_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			 वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	*repeatable = 0;

	प्रत्यागम cmdtp->cmd(cmdtp, flag, argc, argv);
}

पूर्णाङ्क cmd_discard_repeatable(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
			   वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क repeatable;

	प्रत्यागम cmdtp->cmd_rep(cmdtp, flag, argc, argv, &repeatable);
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
स्थिर पूर्णाङ्क cmd_call(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
		    वर्ण *ध्रुव argv[], पूर्णाङ्क *repeatable)
{
	पूर्णाङ्क परिणाम;

	परिणाम = cmdtp->cmd_rep(cmdtp, flag, argc, argv, repeatable);
	यदि (परिणाम)
		debug("Command failed, result=%d\n", परिणाम);
	प्रत्यागम परिणाम;
}

गणना command_ret_t cmd_process(पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[],
			       पूर्णाङ्क *repeatable, ulong *ticks)
{
	गणना command_ret_t rc = CMD_RET_SUCCESS;
	संरचना cmd_tbl *cmdtp;

#यदि defined(CONFIG_SYS_XTRACE)
	वर्ण *xtrace;

	xtrace = env_get("xtrace");
	यदि (xtrace) {
		पङ्क्तिं_लिख("+");
		प्रति (पूर्णाङ्क i = 0; i < argc; i++) {
			पङ्क्तिं_लिख(" ");
			पङ्क्तिं_लिख(argv[i]);
		}
		पङ्क्तिं_लिख("\n");
	}
#यदि_अन्तः

	/* Look up command in command table */
	cmdtp = find_cmd(argv[0]);
	यदि (cmdtp == रिक्त) {
		प्रारूप_लिख("Unknown command '%s' - try 'help'\n", argv[0]);
		प्रत्यागम 1;
	}

	/* found - check max args */
	यदि (argc > cmdtp->maxargs)
		rc = CMD_RET_USAGE;

#यदि defined(CONFIG_CMD_BOOTD)
	/* avoid "bootd" recursion */
	अन्यथा यदि (cmdtp->cmd == do_bootd) {
		यदि (flag & CMD_FLAG_BOOTD) {
			पङ्क्तिं_लिख("'bootd' recursion detected\n");
			rc = CMD_RET_FAILURE;
		} अन्यथा {
			flag |= CMD_FLAG_BOOTD;
		}
	}
#यदि_अन्तः

	/* If OK so far, then do the command */
	यदि (!rc) {
		पूर्णाङ्क newrep;

		यदि (ticks)
			*ticks = get_timer(0);
		rc = cmd_call(cmdtp, flag, argc, argv, &newrep);
		यदि (ticks)
			*ticks = get_timer(*ticks);
		*repeatable &= newrep;
	}
	यदि (rc == CMD_RET_USAGE)
		rc = cmd_usage(cmdtp);
	प्रत्यागम rc;
}

पूर्णाङ्क cmd_process_error(संरचना cmd_tbl *cmdtp, पूर्णाङ्क err)
{
	यदि (err == CMD_RET_USAGE)
		प्रत्यागम CMD_RET_USAGE;

	यदि (err) {
		प्रारूप_लिख("Command '%s' failed: Error %d\n", cmdtp->name, err);
		प्रत्यागम CMD_RET_FAILURE;
	}

	प्रत्यागम CMD_RET_SUCCESS;
}

पूर्णाङ्क cmd_source_script(ulong addr, ध्रुव वर्ण *fit_uname, ध्रुव वर्ण *confname)
{
	वर्ण *दत्त;
	शून्य *buf;
	uint len;
	पूर्णाङ्क ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &दत्त, &len);
	unmap_sysmem(buf);
	यदि (ret)
		प्रत्यागम CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	प्रत्यागम सततम्_command_list(दत्त, len, 0);
}
