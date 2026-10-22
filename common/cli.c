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
	प्रत्यागम IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
#यदि !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	यदि (cli_simple_run_command(cmd, flag) == -1)
		प्रत्यागम 1;

	प्रत्यागम 0;
#अन्यथा
	यदि (use_hush_old()) {
		पूर्णाङ्क hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		यदि (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		प्रत्यागम parse_string_outer(cmd, hush_flags);
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
	प्रत्यागम parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
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
	प्रत्यागम cli_simple_run_command(cmd, flag);
#अन्यथा
	पूर्णाङ्क ret;

	यदि (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} अन्यथा {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	यदि (ret)
		प्रत्यागम -1;

	प्रत्यागम 0;
#endif
}
#अन्यथा
__weak पूर्णाङ्क board_run_command(ध्रुव वर्ण *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	प्रत्यागम 1;
}
#endif /* CONFIG_CMDLINE */

पूर्णाङ्क run_command_list(ध्रुव वर्ण *cmd, पूर्णाङ्क len, पूर्णाङ्क flag)
{
	पूर्णाङ्क need_buff = 1;
	वर्ण *buff = (वर्ण *)cmd;	/* cast away const */
	पूर्णाङ्क rcode = 0;

	यदि (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#अन्यथा
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != रिक्त;
#endif
	}
	यदि (need_buff) {
		buff = memdup_nul(cmd, len);
		यदि (!buff)
			प्रत्यागम 1;
	}
#ifdef CONFIG_HUSH_PARSER
	यदि (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} अन्यथा {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#अन्यथा
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#अन्यथा
	rcode = board_run_command(buff);
#endif
#endif
	यदि (need_buff)
		मुक्त(buff);

	प्रत्यागम rcode;
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

	यदि (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		प्रत्यागम -EIO;
	} अन्यथा यदि (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		प्रत्यागम -ENOSPC;
	}
	प्रत्यागम run_command(console_buffer, 0);
}

/****************************************************************************/

#यदि defined(CONFIG_CMD_RUN)
पूर्णाङ्क do_run(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i, ret;

	यदि (argc < 2)
		प्रत्यागम CMD_RET_USAGE;

	प्रति (i = 1; i < argc; ++i) {
		वर्ण *arg;

		arg = env_get(argv[i]);
		यदि (arg == रिक्त) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			प्रत्यागम 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		यदि (ret)
			प्रत्यागम ret;
	}
	प्रत्यागम 0;
}
#endif

#यदि CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ध्रुव वर्ण **cmdp)
{
	/* Allow the fdt to override the boot command */
	ध्रुव वर्ण *env = ofnode_conf_read_str("bootcmd");
	यदि (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	प्रत्यागम ofnode_conf_read_int("bootsecure", 0);
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

	यदि (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		गच्छ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	यदि (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		गच्छ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (वर्ण **)&cmd);

#अन्यथा
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

	यदि (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#यदि CONFIG_IS_ENABLED(HUSH_PARSER)
	यदि (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	अन्यथा यदि (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	प्रति (;;);
#नोचेद्यदि defined(CONFIG_CMDLINE)
	cli_simple_loop();
#अन्यथा
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

शून्य cli_init(शून्य)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	यदि (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		यदि (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		अन्यथा यदि (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	यदि (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} अन्यथा यदि (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} अन्यथा {
		printf("No valid hush parser to use, cli will not initialized!\n");
		प्रत्यागम;
	}
#endif

#यदि defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	यदि (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
