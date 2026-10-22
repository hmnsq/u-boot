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

#അന്തര്ഭാവ <ansi.h>
#അന്തര്ഭാവ <bootstage.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <cli_hush.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <fdtdec.h>
#അന്തര്ഭാവ <hang.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <dm/ofnode.h>
#അന്തര്ഭാവ <linux/errno.h>

#ifdef CONFIG_CMDLINE

സ്ഥിതം വരിയിൽ bool use_hush_old(ശൂന്യം)
{
	തിരികെ IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
പൂർണ്ണസംഖ്യ run_command(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ flag)
{
#എങ്കിൽ !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	എങ്കിൽ (cli_simple_run_command(cmd, flag) == -1)
		തിരികെ 1;

	തിരികെ 0;
#അല്ലെങ്കിൽ
	എങ്കിൽ (use_hush_old()) {
		പൂർണ്ണസംഖ്യ hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		എങ്കിൽ (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		തിരികെ parse_string_outer(cmd, hush_flags);
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
	തിരികെ parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
പൂർണ്ണസംഖ്യ run_command_repeatable(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ flag)
{
#ifndef CONFIG_HUSH_PARSER
	തിരികെ cli_simple_run_command(cmd, flag);
#അല്ലെങ്കിൽ
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} അല്ലെങ്കിൽ {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	എങ്കിൽ (ret)
		തിരികെ -1;

	തിരികെ 0;
#endif
}
#അല്ലെങ്കിൽ
__weak പൂർണ്ണസംഖ്യ board_run_command(സ്ഥിരം അക്ഷരം *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	തിരികെ 1;
}
#endif /* CONFIG_CMDLINE */

പൂർണ്ണസംഖ്യ run_command_list(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ len, പൂർണ്ണസംഖ്യ flag)
{
	പൂർണ്ണസംഖ്യ need_buff = 1;
	അക്ഷരം *buff = (അക്ഷരം *)cmd;	/* cast away const */
	പൂർണ്ണസംഖ്യ rcode = 0;

	എങ്കിൽ (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#അല്ലെങ്കിൽ
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ശൂന്യ;
#endif
	}
	എങ്കിൽ (need_buff) {
		buff = memdup_nul(cmd, len);
		എങ്കിൽ (!buff)
			തിരികെ 1;
	}
#ifdef CONFIG_HUSH_PARSER
	എങ്കിൽ (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} അല്ലെങ്കിൽ {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#അല്ലെങ്കിൽ
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#അല്ലെങ്കിൽ
	rcode = board_run_command(buff);
#endif
#endif
	എങ്കിൽ (need_buff)
		സ്വതന്ത്രം(buff);

	തിരികെ rcode;
}

പൂർണ്ണസംഖ്യ run_commandf(സ്ഥിരം അക്ഷരം *fmt, ...)
{
	va_list args;
	പൂർണ്ണസംഖ്യ nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	എങ്കിൽ (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		തിരികെ -EIO;
	} അല്ലെങ്കിൽ എങ്കിൽ (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		തിരികെ -ENOSPC;
	}
	തിരികെ run_command(console_buffer, 0);
}

/****************************************************************************/

#എങ്കിൽ defined(CONFIG_CMD_RUN)
പൂർണ്ണസംഖ്യ do_run(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc, അക്ഷരം *സ്ഥിരം argv[])
{
	പൂർണ്ണസംഖ്യ i, ret;

	എങ്കിൽ (argc < 2)
		തിരികെ CMD_RET_USAGE;

	വേണ്ടി (i = 1; i < argc; ++i) {
		അക്ഷരം *arg;

		arg = env_get(argv[i]);
		എങ്കിൽ (arg == ശൂന്യ) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			തിരികെ 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		എങ്കിൽ (ret)
			തിരികെ ret;
	}
	തിരികെ 0;
}
#endif

#എങ്കിൽ CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(സ്ഥിരം അക്ഷരം **cmdp)
{
	/* Allow the fdt to override the boot command */
	സ്ഥിരം അക്ഷരം *env = ofnode_conf_read_str("bootcmd");
	എങ്കിൽ (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	തിരികെ ofnode_conf_read_int("bootsecure", 0);
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
ശൂന്യം cli_secure_boot_cmd(സ്ഥിരം അക്ഷരം *cmd)
{
#ifdef CONFIG_CMDLINE
	ഘടന cmd_tbl *cmdtp;
#endif
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		പോകുക err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	എങ്കിൽ (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		പോകുക err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (അക്ഷരം **)&cmd);

#അല്ലെങ്കിൽ
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

ശൂന്യം cli_loop(ശൂന്യം)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	എങ്കിൽ (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#എങ്കിൽ CONFIG_IS_ENABLED(HUSH_PARSER)
	എങ്കിൽ (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	അല്ലെങ്കിൽ എങ്കിൽ (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	വേണ്ടി (;;);
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_CMDLINE)
	cli_simple_loop();
#അല്ലെങ്കിൽ
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

ശൂന്യം cli_init(ശൂന്യം)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	എങ്കിൽ (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		എങ്കിൽ (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	എങ്കിൽ (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} അല്ലെങ്കിൽ എങ്കിൽ (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} അല്ലെങ്കിൽ {
		printf("No valid hush parser to use, cli will not initialized!\n");
		തിരികെ;
	}
#endif

#എങ്കിൽ defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	എങ്കിൽ (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
