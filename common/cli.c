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

स्थिर ओळीत bool use_hush_old(शून्य)
{
	परत IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
पूर्णांक run_command(स्थिरांक अक्षर *cmd, पूर्णांक flag)
{
#जर !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	जर (cli_simple_run_command(cmd, flag) == -1)
		परत 1;

	परत 0;
#अन्यथा
	जर (use_hush_old()) {
		पूर्णांक hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		जर (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		परत parse_string_outer(cmd, hush_flags);
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
	परत parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
पूर्णांक run_command_repeatable(स्थिरांक अक्षर *cmd, पूर्णांक flag)
{
#ifndef CONFIG_HUSH_PARSER
	परत cli_simple_run_command(cmd, flag);
#अन्यथा
	पूर्णांक ret;

	जर (use_hush_old()) {
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
	जर (ret)
		परत -1;

	परत 0;
#endif
}
#अन्यथा
__weak पूर्णांक board_run_command(स्थिरांक अक्षर *cmdline)
{
	म_लिखो("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	परत 1;
}
#endif /* CONFIG_CMDLINE */

पूर्णांक run_command_list(स्थिरांक अक्षर *cmd, पूर्णांक len, पूर्णांक flag)
{
	पूर्णांक need_buff = 1;
	अक्षर *buff = (अक्षर *)cmd;	/* cast away const */
	पूर्णांक rcode = 0;

	जर (len == -1) {
		len = म_माप(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#अन्यथा
		/* the built-in parser will change our string if it sees \n */
		need_buff = म_अक्षर(cmd, '\n') != रिक्त;
#endif
	}
	जर (need_buff) {
		buff = memdup_nul(cmd, len);
		जर (!buff)
			परत 1;
	}
#ifdef CONFIG_HUSH_PARSER
	जर (use_hush_old()) {
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
	जर (need_buff)
		मुक्त(buff);

	परत rcode;
}

पूर्णांक run_commandf(स्थिरांक अक्षर *fmt, ...)
{
	बहु_सूची args;
	पूर्णांक nbytes;

	बहु_शुरू(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	बहु_पूर्ण(args);

	जर (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		परत -EIO;
	} अन्यथा जर (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		परत -ENOSPC;
	}
	परत run_command(console_buffer, 0);
}

/****************************************************************************/

#जर defined(CONFIG_CMD_RUN)
पूर्णांक do_run(संरचना cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिरांक argv[])
{
	पूर्णांक i, ret;

	जर (argc < 2)
		परत CMD_RET_USAGE;

	साठी (i = 1; i < argc; ++i) {
		अक्षर *arg;

		arg = env_get(argv[i]);
		जर (arg == रिक्त) {
			म_लिखो("## Error: \"%s\" not defined\n", argv[i]);
			परत 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		जर (ret)
			परत ret;
	}
	परत 0;
}
#endif

#जर CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(स्थिरांक अक्षर **cmdp)
{
	/* Allow the fdt to override the boot command */
	स्थिरांक अक्षर *env = ofnode_conf_read_str("bootcmd");
	जर (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	परत ofnode_conf_read_int("bootsecure", 0);
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
शून्य cli_secure_boot_cmd(स्थिरांक अक्षर *cmd)
{
#ifdef CONFIG_CMDLINE
	संरचना cmd_tbl *cmdtp;
#endif
	पूर्णांक rc;

	जर (!cmd) {
		म_लिखो("## Error: Secure boot command not specified\n");
		जा err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	जर (!cmdtp) {
		म_लिखो("## Error: \"%s\" not defined\n", cmd);
		जा err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (अक्षर **)&cmd);

#अन्यथा
	rc = board_run_command(cmd);
#endif

	/* Shouldn't ever return from boot command. */
	म_लिखो("## Error: \"%s\" returned (code %d)\n", cmd, rc);

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

	जर (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#जर CONFIG_IS_ENABLED(HUSH_PARSER)
	जर (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	अन्यथा जर (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	म_लिखो("Problem\n");
	/* This point is never reached */
	साठी (;;);
#किंवाजर defined(CONFIG_CMDLINE)
	cli_simple_loop();
#अन्यथा
	म_लिखो("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

शून्य cli_init(शून्य)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	जर (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		जर (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		अन्यथा जर (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	जर (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} अन्यथा जर (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} अन्यथा {
		म_लिखो("No valid hush parser to use, cli will not initialized!\n");
		परत;
	}
#endif

#जर defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	जर (CONFIG_IS_ENABLED(VIDEO_ANSI))
		म_लिखो(ANSI_CURSOR_SHOW "\n");
}
