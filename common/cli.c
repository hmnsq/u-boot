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

#ضمن <ansi.h>
#ضمن <bootstage.h>
#ضمن <cli.h>
#ضمن <cli_hush.h>
#ضمن <command.h>
#ضمن <console.h>
#ضمن <env.h>
#ضمن <fdtdec.h>
#ضمن <hang.h>
#ضمن <malloc.h>
#ضمن <asm/global_data.h>
#ضمن <dm/ofnode.h>
#ضمن <linux/errno.h>

#ifdef CONFIG_CMDLINE

ساكن مضمن bool use_hush_old(فراغ)
{
	ܦܢܐ IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
صحيح run_command(ثابت حرف *cmd, صحيح flag)
{
#ܐܢ !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	ܐܢ (cli_simple_run_command(cmd, flag) == -1)
		ܦܢܐ 1;

	ܦܢܐ 0;
#ܐܚܪܢܐ
	ܐܢ (use_hush_old()) {
		صحيح hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		ܐܢ (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ܦܢܐ parse_string_outer(cmd, hush_flags);
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
	ܦܢܐ parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
صحيح run_command_repeatable(ثابت حرف *cmd, صحيح flag)
{
#ifndef CONFIG_HUSH_PARSER
	ܦܢܐ cli_simple_run_command(cmd, flag);
#ܐܚܪܢܐ
	صحيح ret;

	ܐܢ (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} ܐܚܪܢܐ {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	ܐܢ (ret)
		ܦܢܐ -1;

	ܦܢܐ 0;
#endif
}
#ܐܚܪܢܐ
__weak صحيح board_run_command(ثابت حرف *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ܦܢܐ 1;
}
#endif /* CONFIG_CMDLINE */

صحيح run_command_list(ثابت حرف *cmd, صحيح len, صحيح flag)
{
	صحيح need_buff = 1;
	حرف *buff = (حرف *)cmd;	/* cast away const */
	صحيح rcode = 0;

	ܐܢ (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#ܐܚܪܢܐ
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != عدم;
#endif
	}
	ܐܢ (need_buff) {
		buff = memdup_nul(cmd, len);
		ܐܢ (!buff)
			ܦܢܐ 1;
	}
#ifdef CONFIG_HUSH_PARSER
	ܐܢ (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} ܐܚܪܢܐ {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#ܐܚܪܢܐ
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#ܐܚܪܢܐ
	rcode = board_run_command(buff);
#endif
#endif
	ܐܢ (need_buff)
		حرر(buff);

	ܦܢܐ rcode;
}

صحيح run_commandf(ثابت حرف *fmt, ...)
{
	va_list args;
	صحيح nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	ܐܢ (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ܦܢܐ -EIO;
	} ܐܚܪܢܐ ܐܢ (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ܦܢܐ -ENOSPC;
	}
	ܦܢܐ run_command(console_buffer, 0);
}

/****************************************************************************/

#ܐܢ defined(CONFIG_CMD_RUN)
صحيح do_run(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc, حرف *ثابت argv[])
{
	صحيح i, ret;

	ܐܢ (argc < 2)
		ܦܢܐ CMD_RET_USAGE;

	لكل (i = 1; i < argc; ++i) {
		حرف *arg;

		arg = env_get(argv[i]);
		ܐܢ (arg == عدم) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			ܦܢܐ 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		ܐܢ (ret)
			ܦܢܐ ret;
	}
	ܦܢܐ 0;
}
#endif

#ܐܢ CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ثابت حرف **cmdp)
{
	/* Allow the fdt to override the boot command */
	ثابت حرف *env = ofnode_conf_read_str("bootcmd");
	ܐܢ (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ܦܢܐ ofnode_conf_read_int("bootsecure", 0);
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
فراغ cli_secure_boot_cmd(ثابت حرف *cmd)
{
#ifdef CONFIG_CMDLINE
	بنية cmd_tbl *cmdtp;
#endif
	صحيح rc;

	ܐܢ (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		اذهب err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	ܐܢ (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		اذهب err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (حرف **)&cmd);

#ܐܚܪܢܐ
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

فراغ cli_loop(فراغ)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	ܐܢ (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#ܐܢ CONFIG_IS_ENABLED(HUSH_PARSER)
	ܐܢ (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	ܐܚܪܢܐ ܐܢ (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	لكل (;;);
#وإلاإذا defined(CONFIG_CMDLINE)
	cli_simple_loop();
#ܐܚܪܢܐ
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

فراغ cli_init(فراغ)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	ܐܢ (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		ܐܢ (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		ܐܚܪܢܐ ܐܢ (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	ܐܢ (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} ܐܚܪܢܐ ܐܢ (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} ܐܚܪܢܐ {
		printf("No valid hush parser to use, cli will not initialized!\n");
		ܦܢܐ;
	}
#endif

#ܐܢ defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	ܐܢ (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
