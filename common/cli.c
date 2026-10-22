// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#تعریف_کرو pr_fmt(fmt) "cli: %s: " fmt, __func__

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

#اگر_تعریف_شدہ CONFIG_CMDLINE

ساکن اندرونی bool use_hush_old(خالی)
{
	واپس IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
صحیح تسلسل_command(ثابت حرف *cmd, صحیح flag)
{
#اگر !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	اگر (cli_simple_تسلسل_command(cmd, flag) == -1)
		واپس 1;

	واپس 0;
#ورنہ
	اگر (use_hush_old()) {
		صحیح hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		اگر (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		واپس parse_string_outer(cmd, hush_flags);
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
	واپس parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#اگر_ختم
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
صحیح تسلسل_command_repeatable(ثابت حرف *cmd, صحیح flag)
{
#اگر_غیر_تعریف_شدہ CONFIG_HUSH_PARSER
	واپس cli_simple_تسلسل_command(cmd, flag);
#ورنہ
	صحیح ret;

	اگر (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} ورنہ {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	اگر (ret)
		واپس -1;

	واپس 0;
#اگر_ختم
}
#ورنہ
__weak صحیح board_تسلسل_command(ثابت حرف *cmdline)
{
	لکھو("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	واپس 1;
}
#اگر_ختم /* CONFIG_CMDLINE */

صحیح تسلسل_command_list(ثابت حرف *cmd, صحیح len, صحیح flag)
{
	صحیح need_buff = 1;
	حرف *buff = (حرف *)cmd;	/* cast away const */
	صحیح rcode = 0;

	اگر (len == -1) {
		len = لڑی_لمبائی(cmd);
#اگر_تعریف_شدہ CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#ورنہ
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != تہی;
#اگر_ختم
	}
	اگر (need_buff) {
		buff = memdup_nul(cmd, len);
		اگر (!buff)
			واپس 1;
	}
#اگر_تعریف_شدہ CONFIG_HUSH_PARSER
	اگر (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} ورنہ {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#ورنہ
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#اگر_تعریف_شدہ CONFIG_CMDLINE
	rcode = cli_simple_تسلسل_command_list(buff, flag);
#ورنہ
	rcode = board_تسلسل_command(buff);
#اگر_ختم
#اگر_ختم
	اگر (need_buff)
		آزاد(buff);

	واپس rcode;
}

صحیح تسلسل_commandf(ثابت حرف *fmt, ...)
{
	va_list دلائل;
	صحیح nbytes;

	va_آغاز(دلائل, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_بفر, CONFIG_SYS_CBSIZE, fmt, دلائل);
	va_انتہا(دلائل);

	اگر (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		واپس -EIO;
	} ورنہ اگر (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		واپس -ENOSPC;
	}
	واپس تسلسل_command(console_بفر, 0);
}

/****************************************************************************/

#اگر defined(CONFIG_CMD_تسلسل)
صحیح do_تسلسل(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc, حرف *ثابت argv[])
{
	صحیح i, ret;

	اگر (argc < 2)
		واپس CMD_RET_USAGE;

	برائے (i = 1; i < argc; ++i) {
		حرف *arg;

		arg = env_get(argv[i]);
		اگر (arg == تہی) {
			لکھو("## Error: \"%s\" not defined\n", argv[i]);
			واپس 1;
		}

		ret = تسلسل_command(arg, flag | CMD_FLAG_ENV);
		اگر (ret)
			واپس ret;
	}
	واپس 0;
}
#اگر_ختم

#اگر CONFIG_IS_ENABLED(کا_CONTROL)
bool cli_process_fdt(ثابت حرف **cmdp)
{
	/* Allow the fdt to override the boot command */
	ثابت حرف *env = ofnode_conf_read_str("bootcmd");
	اگر (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	واپس ofnode_conf_read_int("bootsecure", 0);
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
خالی cli_secure_boot_cmd(ثابت حرف *cmd)
{
#اگر_تعریف_شدہ CONFIG_CMDLINE
	ساخت cmd_tbl *cmdtp;
#اگر_ختم
	صحیح rc;

	اگر (!cmd) {
		لکھو("## Error: Secure boot command not specified\n");
		جاؤ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#اگر_تعریف_شدہ CONFIG_CMDLINE
	cmdtp = ڈھونڈو_cmd(cmd);
	اگر (!cmdtp) {
		لکھو("## Error: \"%s\" not defined\n", cmd);
		جاؤ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (حرف **)&cmd);

#ورنہ
	rc = board_تسلسل_command(cmd);
#اگر_ختم

	/* Shouldn't ever return from boot command. */
	لکھو("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#اگر_ختم /* CONFIG_IS_ENABLED(OF_CONTROL) */

خالی cli_loop(خالی)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	اگر (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#اگر CONFIG_IS_ENABLED(HUSH_PARSER)
	اگر (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_تسلسل_file();
	ورنہ اگر (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	لکھو("Problem\n");
	/* This point is never reached */
	برائے (;;);
#وگرنہ_اگر defined(CONFIG_CMDLINE)
	cli_simple_loop();
#ورنہ
	لکھو("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#اگر_ختم /*CONFIG_HUSH_PARSER*/
}

خالی cli_init(خالی)
{
#اگر_تعریف_شدہ CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	اگر (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		اگر (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		ورنہ اگر (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	اگر (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_آغاز();
	} ورنہ اگر (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_آغاز_modern();
	} ورنہ {
		لکھو("No valid hush parser to use, cli will not initialized!\n");
		واپس;
	}
#اگر_ختم

#اگر defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#اگر_ختم

	اگر (CONFIG_IS_ENABLED(VIDEO_ANSI))
		لکھو(ANSI_CURSOR_SHOW "\n");
}
