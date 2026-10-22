// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pr_fmt(fmt) "cli: %s: " fmt, __func__

#ਅਨ੍ਤਰ੍ਭਾਵ <ansi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootstage.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli_hush.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/ofnode.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE

ਸਥਿਰ ਸਤਰਅੰਦਰ bool use_hush_old(ਖਾਲੀ)
{
	ਵਾਪਸ IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
ਪੂਰਨ ਲੜੀਵਾਰ_command(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ flag)
{
#ਜੇ !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	ਜੇ (cli_simple_ਲੜੀਵਾਰ_command(cmd, flag) == -1)
		ਵਾਪਸ 1;

	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ਜੇ (use_hush_old()) {
		ਪੂਰਨ hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		ਜੇ (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ਵਾਪਸ parse_string_outer(cmd, hush_flags);
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
	ਵਾਪਸ parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#ਜੇ_ਅੰਤ
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
ਪੂਰਨ ਲੜੀਵਾਰ_command_repeatable(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ flag)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_HUSH_PARSER
	ਵਾਪਸ cli_simple_ਲੜੀਵਾਰ_command(cmd, flag);
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ ret;

	ਜੇ (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} ਨਹੀਂਤਾਂ {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	ਜੇ (ret)
		ਵਾਪਸ -1;

	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
}
#ਨਹੀਂਤਾਂ
__weak ਪੂਰਨ board_ਲੜੀਵਾਰ_command(ਅਚਲ ਅੱਖਰ *cmdline)
{
	ਲਿਖੋ("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ਵਾਪਸ 1;
}
#ਜੇ_ਅੰਤ /* CONFIG_CMDLINE */

ਪੂਰਨ ਲੜੀਵਾਰ_command_list(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ len, ਪੂਰਨ flag)
{
	ਪੂਰਨ need_buff = 1;
	ਅੱਖਰ *buff = (ਅੱਖਰ *)cmd;	/* cast away const */
	ਪੂਰਨ rcode = 0;

	ਜੇ (len == -1) {
		len = ਲੜੀ_ਲੰਬਾਈ(cmd);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#ਨਹੀਂਤਾਂ
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	}
	ਜੇ (need_buff) {
		buff = memdup_nul(cmd, len);
		ਜੇ (!buff)
			ਵਾਪਸ 1;
	}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_HUSH_PARSER
	ਜੇ (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} ਨਹੀਂਤਾਂ {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#ਨਹੀਂਤਾਂ
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
	rcode = cli_simple_ਲੜੀਵਾਰ_command_list(buff, flag);
#ਨਹੀਂਤਾਂ
	rcode = board_ਲੜੀਵਾਰ_command(buff);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
	ਜੇ (need_buff)
		ਮੁਕਤ(buff);

	ਵਾਪਸ rcode;
}

ਪੂਰਨ ਲੜੀਵਾਰ_commandf(ਅਚਲ ਅੱਖਰ *fmt, ...)
{
	va_list ਦਲੀਲਾਂ;
	ਪੂਰਨ nbytes;

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_ਬਫਰ, CONFIG_SYS_CBSIZE, fmt, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);

	ਜੇ (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ਵਾਪਸ -EIO;
	} ਨਹੀਂਤਾਂ ਜੇ (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ਵਾਪਸ -ENOSPC;
	}
	ਵਾਪਸ ਲੜੀਵਾਰ_command(console_ਬਫਰ, 0);
}

/****************************************************************************/

#ਜੇ defined(CONFIG_CMD_ਲੜੀਵਾਰ)
ਪੂਰਨ do_ਲੜੀਵਾਰ(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc, ਅੱਖਰ *ਅਚਲ argv[])
{
	ਪੂਰਨ i, ret;

	ਜੇ (argc < 2)
		ਵਾਪਸ CMD_RET_USAGE;

	ਲਈ (i = 1; i < argc; ++i) {
		ਅੱਖਰ *arg;

		arg = env_get(argv[i]);
		ਜੇ (arg == ਨਿਰਾ) {
			ਲਿਖੋ("## Error: \"%s\" not defined\n", argv[i]);
			ਵਾਪਸ 1;
		}

		ret = ਲੜੀਵਾਰ_command(arg, flag | CMD_FLAG_ENV);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ CONFIG_IS_ENABLED(ਦਾ_CONTROL)
bool cli_process_fdt(ਅਚਲ ਅੱਖਰ **cmdp)
{
	/* Allow the fdt to override the boot command */
	ਅਚਲ ਅੱਖਰ *env = ofnode_conf_read_str("bootcmd");
	ਜੇ (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ਵਾਪਸ ofnode_conf_read_int("bootsecure", 0);
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
ਖਾਲੀ cli_secure_boot_cmd(ਅਚਲ ਅੱਖਰ *cmd)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
	ਬਣਤਰ cmd_tbl *cmdtp;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ rc;

	ਜੇ (!cmd) {
		ਲਿਖੋ("## Error: Secure boot command not specified\n");
		ਜਾਓ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE
	cmdtp = ਲੱਭੋ_cmd(cmd);
	ਜੇ (!cmdtp) {
		ਲਿਖੋ("## Error: \"%s\" not defined\n", cmd);
		ਜਾਓ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (ਅੱਖਰ **)&cmd);

#ਨਹੀਂਤਾਂ
	rc = board_ਲੜੀਵਾਰ_command(cmd);
#ਜੇ_ਅੰਤ

	/* Shouldn't ever return from boot command. */
	ਲਿਖੋ("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#ਜੇ_ਅੰਤ /* CONFIG_IS_ENABLED(OF_CONTROL) */

ਖਾਲੀ cli_loop(ਖਾਲੀ)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	ਜੇ (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#ਜੇ CONFIG_IS_ENABLED(HUSH_PARSER)
	ਜੇ (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_ਲੜੀਵਾਰ_file();
	ਨਹੀਂਤਾਂ ਜੇ (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	ਲਿਖੋ("Problem\n");
	/* This point is never reached */
	ਲਈ (;;);
#ਜਾਂਜੇ defined(CONFIG_CMDLINE)
	cli_simple_loop();
#ਨਹੀਂਤਾਂ
	ਲਿਖੋ("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#ਜੇ_ਅੰਤ /*CONFIG_HUSH_PARSER*/
}

ਖਾਲੀ cli_init(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	ਜੇ (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		ਜੇ (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	ਜੇ (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_ਸ਼ੁਰੂ();
	} ਨਹੀਂਤਾਂ ਜੇ (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_ਸ਼ੁਰੂ_modern();
	} ਨਹੀਂਤਾਂ {
		ਲਿਖੋ("No valid hush parser to use, cli will not initialized!\n");
		ਵਾਪਸ;
	}
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#ਜੇ_ਅੰਤ

	ਜੇ (CONFIG_IS_ENABLED(VIDEO_ANSI))
		ਲਿਖੋ(ANSI_CURSOR_SHOW "\n");
}
