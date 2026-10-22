// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#define pr_fmt(fmt) "cli: %s: " fmt, __func__

#अन्तर्भाव <ansi.h>
#अन्तर्भाव <bootstage.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <cli_hush.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <hang.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <dm/ofnode.h>
#अन्तर्भाव <linux/errno.h>

#ifdef CONFIG_CMDLINE

स्थिर अन्तःस्थ bool use_hush_old(शून्य)
{
	पटिनिवत्त IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
	gd->flags & GD_FLG_HUSH_OLD_PARSER :
	IS_ENABLED(CONFIG_HUSH_OLD_PARSER);
}

/*
 * Run a command using the selected parser.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 on success, or != 0 on error.
 */
पूर्णाङ्क run_command(ध्रुव वर्ण *cmd, पूर्णाङ्क flag)
{
#सचे !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	सचे (cli_simple_run_command(cmd, flag) == -1)
		पटिनिवत्त 1;

	पटिनिवत्त 0;
#अञ्ञथा
	सचे (use_hush_old()) {
		पूर्णाङ्क hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		सचे (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		पटिनिवत्त parse_string_outer(cmd, hush_flags);
	}
	/*
	 * Possible values for flags are the following:
	 * FLAG_EXIT_FROM_LOOP: This flags ensures we exit from loop in
	 * parse_and_run_stream() after first iteration while normal
	 * behavior, * i.e. when called from cli_loop(), is to loop
	 * infinitely.
	 * FLAG_PARSE_SEMICOLON: modern Hush parses ';' and does not stop
	 * first time it sees one. So, I think we do not need this flag.
	 * FLAG_REPARSING: For the moment, I do not understand the goal
	 * of this flag.
	 * FLAG_CONT_ON_NEWLINE: This flag seems to be used to continue
	 * parsing even when reading '\n' when coming from
	 * run_command(). In this case, modern Hush reads until it finds
	 * '\0'. So, I think we do not need this flag.
	 */
	पटिनिवत्त parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
पूर्णाङ्क run_command_repeatable(ध्रुव वर्ण *cmd, पूर्णाङ्क flag)
{
#ifndef CONFIG_HUSH_PARSER
	पटिनिवत्त cli_simple_run_command(cmd, flag);
#अञ्ञथा
	पूर्णाङ्क ret;

	सचे (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} अञ्ञथा {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	सचे (ret)
		पटिनिवत्त -1;

	पटिनिवत्त 0;
#endif
}
#अञ्ञथा
__weak पूर्णाङ्क board_run_command(ध्रुव वर्ण *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	पटिनिवत्त 1;
}
#endif /* CONFIG_CMDLINE */

पूर्णाङ्क run_command_list(ध्रुव वर्ण *cmd, पूर्णाङ्क len, पूर्णाङ्क flag)
{
	पूर्णाङ्क need_buff = 1;
	वर्ण *buff = (वर्ण *)cmd;	/* cast away const */
	पूर्णाङ्क rcode = 0;

	सचे (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#अञ्ञथा
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != रिक्त;
#endif
	}
	सचे (need_buff) {
		buff = memdup_nul(cmd, len);
		सचे (!buff)
			पटिनिवत्त 1;
	}
#ifdef CONFIG_HUSH_PARSER
	सचे (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} अञ्ञथा {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#अञ्ञथा
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#अञ्ञथा
	rcode = board_run_command(buff);
#endif
#endif
	सचे (need_buff)
		मुक्त(buff);

	पटिनिवत्त rcode;
}

पूर्णाङ्क run_commandf(ध्रुव वर्ण *fmt, ...)
{
	va_list args;
	पूर्णाङ्क nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	सचे (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		पटिनिवत्त -EIO;
	} अञ्ञथा सचे (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		पटिनिवत्त -ENOSPC;
	}
	पटिनिवत्त run_command(console_buffer, 0);
}

/****************************************************************************/

#सचे defined(CONFIG_CMD_RUN)
पूर्णाङ्क do_run(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i, ret;

	सचे (argc < 2)
		पटिनिवत्त CMD_RET_USAGE;

	अत्थाय (i = 1; i < argc; ++i) {
		वर्ण *arg;

		arg = env_get(argv[i]);
		सचे (arg == रिक्त) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			पटिनिवत्त 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		सचे (ret)
			पटिनिवत्त ret;
	}
	पटिनिवत्त 0;
}
#endif

#सचे CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ध्रुव वर्ण **cmdp)
{
	/* Allow the fdt to override the boot command */
	ध्रुव वर्ण *env = ofnode_conf_read_str("bootcmd");
	सचे (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	पटिनिवत्त ofnode_conf_read_int("bootsecure", 0);
}

/*
 * Runs the given boot command securely.  Specifically:
 * - Doesn't run the command with the shell (run_command or parse_string_outer),
 *   since that's a lot of code surface that an attacker might exploit.
 *   Because of this, we don't do any argument parsing--the secure boot command
 *   has to be a full-fledged u-boot command.
 * - Doesn't check for keypresses before booting, since that could be a
 *   security hole; also disables Ctrl-C.
 * - Doesn't allow the command to return.
 *
 * Upon any failures, this function will drop into an infinite loop after
 * printing the error message to console.
 */
शून्य cli_secure_boot_cmd(ध्रुव वर्ण *cmd)
{
#ifdef CONFIG_CMDLINE
	संरचना cmd_tbl *cmdtp;
#endif
	पूर्णाङ्क rc;

	सचे (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		गच्छ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	सचे (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		गच्छ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (वर्ण **)&cmd);

#अञ्ञथा
	rc = board_run_command(cmd);
#endif

	/* Shouldn't ever return from boot command. */
	printf("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

शून्य cli_loop(शून्य)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	सचे (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#सचे CONFIG_IS_ENABLED(HUSH_PARSER)
	सचे (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	अञ्ञथा सचे (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	अत्थाय (;;);
#नोचेद्यदि defined(CONFIG_CMDLINE)
	cli_simple_loop();
#अञ्ञथा
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

शून्य cli_init(शून्य)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	सचे (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		सचे (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		अञ्ञथा सचे (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	सचे (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} अञ्ञथा सचे (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} अञ्ञथा {
		printf("No valid hush parser to use, cli will not initialized!\n");
		पटिनिवत्त;
	}
#endif

#सचे defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	सचे (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
