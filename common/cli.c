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

#அந்தர்பாவ <ansi.h>
#அந்தர்பாவ <bootstage.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <cli_hush.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <fdtdec.h>
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <dm/ofnode.h>
#அந்தர்பாவ <linux/errno.h>

#ifdef CONFIG_CMDLINE

நிலையான உள்வரி bool use_hush_old(வெறுமை)
{
	திரும்பு IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
முழு run_command(மாறிலி எழுத்து *cmd, முழு flag)
{
#எனில் !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	எனில் (cli_simple_run_command(cmd, flag) == -1)
		திரும்பு 1;

	திரும்பு 0;
#இல்லையெனில்
	எனில் (use_hush_old()) {
		முழு hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		எனில் (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		திரும்பு parse_string_outer(cmd, hush_flags);
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
	திரும்பு parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
முழு run_command_repeatable(மாறிலி எழுத்து *cmd, முழு flag)
{
#ifndef CONFIG_HUSH_PARSER
	திரும்பு cli_simple_run_command(cmd, flag);
#இல்லையெனில்
	முழு ret;

	எனில் (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} இல்லையெனில் {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	எனில் (ret)
		திரும்பு -1;

	திரும்பு 0;
#endif
}
#இல்லையெனில்
__weak முழு board_run_command(மாறிலி எழுத்து *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	திரும்பு 1;
}
#endif /* CONFIG_CMDLINE */

முழு run_command_list(மாறிலி எழுத்து *cmd, முழு len, முழு flag)
{
	முழு need_buff = 1;
	எழுத்து *buff = (எழுத்து *)cmd;	/* cast away const */
	முழு rcode = 0;

	எனில் (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#இல்லையெனில்
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != வெற்று;
#endif
	}
	எனில் (need_buff) {
		buff = memdup_nul(cmd, len);
		எனில் (!buff)
			திரும்பு 1;
	}
#ifdef CONFIG_HUSH_PARSER
	எனில் (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} இல்லையெனில் {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#இல்லையெனில்
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#இல்லையெனில்
	rcode = board_run_command(buff);
#endif
#endif
	எனில் (need_buff)
		விடுவி(buff);

	திரும்பு rcode;
}

முழு run_commandf(மாறிலி எழுத்து *fmt, ...)
{
	va_list args;
	முழு nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	எனில் (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		திரும்பு -EIO;
	} இல்லையெனில் எனில் (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		திரும்பு -ENOSPC;
	}
	திரும்பு run_command(console_buffer, 0);
}

/****************************************************************************/

#எனில் defined(CONFIG_CMD_RUN)
முழு do_run(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc, எழுத்து *மாறிலி argv[])
{
	முழு i, ret;

	எனில் (argc < 2)
		திரும்பு CMD_RET_USAGE;

	க்கு (i = 1; i < argc; ++i) {
		எழுத்து *arg;

		arg = env_get(argv[i]);
		எனில் (arg == வெற்று) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			திரும்பு 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		எனில் (ret)
			திரும்பு ret;
	}
	திரும்பு 0;
}
#endif

#எனில் CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(மாறிலி எழுத்து **cmdp)
{
	/* Allow the fdt to override the boot command */
	மாறிலி எழுத்து *env = ofnode_conf_read_str("bootcmd");
	எனில் (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	திரும்பு ofnode_conf_read_int("bootsecure", 0);
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
வெறுமை cli_secure_boot_cmd(மாறிலி எழுத்து *cmd)
{
#ifdef CONFIG_CMDLINE
	கட்டமைப்பு cmd_tbl *cmdtp;
#endif
	முழு rc;

	எனில் (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		செல் err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	எனில் (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		செல் err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (எழுத்து **)&cmd);

#இல்லையெனில்
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

வெறுமை cli_loop(வெறுமை)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	எனில் (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#எனில் CONFIG_IS_ENABLED(HUSH_PARSER)
	எனில் (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	இல்லையெனில் எனில் (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	க்கு (;;);
#அல்லதெனில் defined(CONFIG_CMDLINE)
	cli_simple_loop();
#இல்லையெனில்
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

வெறுமை cli_init(வெறுமை)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	எனில் (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		எனில் (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		இல்லையெனில் எனில் (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	எனில் (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} இல்லையெனில் எனில் (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} இல்லையெனில் {
		printf("No valid hush parser to use, cli will not initialized!\n");
		திரும்பு;
	}
#endif

#எனில் defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	எனில் (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
