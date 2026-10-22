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

#شامل <ansi.h>
#شامل <bootstage.h>
#شامل <cli.h>
#شامل <cli_hush.h>
#شامل <command.h>
#شامل <console.h>
#شامل <env.h>
#شامل <fdtdec.h>
#شامل <hang.h>
#شامل <malloc.h>
#شامل <asm/global_data.h>
#شامل <dm/ofnode.h>
#شامل <linux/errno.h>

#ifdef CONFIG_CMDLINE

ایستا درونخطی bool use_hush_old(تهی)
{
	بازگشت IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
صحیح run_command(ثابت نویسه *cmd, صحیح flag)
{
#اگر !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	اگر (cli_simple_run_command(cmd, flag) == -1)
		بازگشت 1;

	بازگشت 0;
#وگرنه
	اگر (use_hush_old()) {
		صحیح hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		اگر (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		بازگشت parse_string_outer(cmd, hush_flags);
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
	بازگشت parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
صحیح run_command_repeatable(ثابت نویسه *cmd, صحیح flag)
{
#ifndef CONFIG_HUSH_PARSER
	بازگشت cli_simple_run_command(cmd, flag);
#وگرنه
	صحیح ret;

	اگر (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} وگرنه {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	اگر (ret)
		بازگشت -1;

	بازگشت 0;
#endif
}
#وگرنه
__weak صحیح board_run_command(ثابت نویسه *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	بازگشت 1;
}
#endif /* CONFIG_CMDLINE */

صحیح run_command_list(ثابت نویسه *cmd, صحیح len, صحیح flag)
{
	صحیح need_buff = 1;
	نویسه *buff = (نویسه *)cmd;	/* cast away const */
	صحیح rcode = 0;

	اگر (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#وگرنه
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != پوچ;
#endif
	}
	اگر (need_buff) {
		buff = memdup_nul(cmd, len);
		اگر (!buff)
			بازگشت 1;
	}
#ifdef CONFIG_HUSH_PARSER
	اگر (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} وگرنه {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#وگرنه
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#وگرنه
	rcode = board_run_command(buff);
#endif
#endif
	اگر (need_buff)
		آزاد(buff);

	بازگشت rcode;
}

صحیح run_commandf(ثابت نویسه *fmt, ...)
{
	va_list args;
	صحیح nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	اگر (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		بازگشت -EIO;
	} وگرنه اگر (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		بازگشت -ENOSPC;
	}
	بازگشت run_command(console_buffer, 0);
}

/****************************************************************************/

#اگر defined(CONFIG_CMD_RUN)
صحیح do_run(ساختار cmd_tbl *cmdtp, صحیح flag, صحیح argc, نویسه *ثابت argv[])
{
	صحیح i, ret;

	اگر (argc < 2)
		بازگشت CMD_RET_USAGE;

	برای (i = 1; i < argc; ++i) {
		نویسه *arg;

		arg = env_get(argv[i]);
		اگر (arg == پوچ) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			بازگشت 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		اگر (ret)
			بازگشت ret;
	}
	بازگشت 0;
}
#endif

#اگر CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ثابت نویسه **cmdp)
{
	/* Allow the fdt to override the boot command */
	ثابت نویسه *env = ofnode_conf_read_str("bootcmd");
	اگر (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	بازگشت ofnode_conf_read_int("bootsecure", 0);
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
تهی cli_secure_boot_cmd(ثابت نویسه *cmd)
{
#ifdef CONFIG_CMDLINE
	ساختار cmd_tbl *cmdtp;
#endif
	صحیح rc;

	اگر (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		برو err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	اگر (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		برو err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (نویسه **)&cmd);

#وگرنه
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

تهی cli_loop(تهی)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	اگر (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#اگر CONFIG_IS_ENABLED(HUSH_PARSER)
	اگر (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	وگرنه اگر (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	برای (;;);
#وگرنهاگر defined(CONFIG_CMDLINE)
	cli_simple_loop();
#وگرنه
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

تهی cli_init(تهی)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	اگر (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		اگر (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		وگرنه اگر (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	اگر (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} وگرنه اگر (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} وگرنه {
		printf("No valid hush parser to use, cli will not initialized!\n");
		بازگشت;
	}
#endif

#اگر defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	اگر (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
