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

#অন্তর্ভাৱ <ansi.h>
#অন্তর্ভাৱ <bootstage.h>
#অন্তর্ভাৱ <cli.h>
#অন্তর্ভাৱ <cli_hush.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <fdtdec.h>
#অন্তর্ভাৱ <hang.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <dm/ofnode.h>
#অন্তর্ভাৱ <linux/errno.h>

#ifdef CONFIG_CMDLINE

স্থির অন্তঃস্থ bool use_hush_old(শূন্য)
{
	ঘূৰাই IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
পূর্ণাঙ্ক run_command(ধ্রুৱ ৱর্ণ *cmd, পূর্ণাঙ্ক flag)
{
#যদি !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	যদি (cli_simple_run_command(cmd, flag) == -1)
		ঘূৰাই 1;

	ঘূৰাই 0;
#অন্যথা
	যদি (use_hush_old()) {
		পূর্ণাঙ্ক hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		যদি (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ঘূৰাই parse_string_outer(cmd, hush_flags);
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
	ঘূৰাই parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
পূর্ণাঙ্ক run_command_repeatable(ধ্রুৱ ৱর্ণ *cmd, পূর্ণাঙ্ক flag)
{
#ifndef CONFIG_HUSH_PARSER
	ঘূৰাই cli_simple_run_command(cmd, flag);
#অন্যথা
	পূর্ণাঙ্ক ret;

	যদি (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} অন্যথা {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	যদি (ret)
		ঘূৰাই -1;

	ঘূৰাই 0;
#endif
}
#অন্যথা
__weak পূর্ণাঙ্ক board_run_command(ধ্রুৱ ৱর্ণ *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ঘূৰাই 1;
}
#endif /* CONFIG_CMDLINE */

পূর্ণাঙ্ক run_command_list(ধ্রুৱ ৱর্ণ *cmd, পূর্ণাঙ্ক len, পূর্ণাঙ্ক flag)
{
	পূর্ণাঙ্ক need_buff = 1;
	ৱর্ণ *buff = (ৱর্ণ *)cmd;	/* cast away const */
	পূর্ণাঙ্ক rcode = 0;

	যদি (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#অন্যথা
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != রিক্ত;
#endif
	}
	যদি (need_buff) {
		buff = memdup_nul(cmd, len);
		যদি (!buff)
			ঘূৰাই 1;
	}
#ifdef CONFIG_HUSH_PARSER
	যদি (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} অন্যথা {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#অন্যথা
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#অন্যথা
	rcode = board_run_command(buff);
#endif
#endif
	যদি (need_buff)
		মুক্ত(buff);

	ঘূৰাই rcode;
}

পূর্ণাঙ্ক run_commandf(ধ্রুৱ ৱর্ণ *fmt, ...)
{
	va_list args;
	পূর্ণাঙ্ক nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	যদি (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ঘূৰাই -EIO;
	} অন্যথা যদি (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ঘূৰাই -ENOSPC;
	}
	ঘূৰাই run_command(console_buffer, 0);
}

/****************************************************************************/

#যদি defined(CONFIG_CMD_RUN)
পূর্ণাঙ্ক do_run(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc, ৱর্ণ *ধ্রুৱ argv[])
{
	পূর্ণাঙ্ক i, ret;

	যদি (argc < 2)
		ঘূৰাই CMD_RET_USAGE;

	বাবে (i = 1; i < argc; ++i) {
		ৱর্ণ *arg;

		arg = env_get(argv[i]);
		যদি (arg == রিক্ত) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			ঘূৰাই 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		যদি (ret)
			ঘূৰাই ret;
	}
	ঘূৰাই 0;
}
#endif

#যদি CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ধ্রুৱ ৱর্ণ **cmdp)
{
	/* Allow the fdt to override the boot command */
	ধ্রুৱ ৱর্ণ *env = ofnode_conf_read_str("bootcmd");
	যদি (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ঘূৰাই ofnode_conf_read_int("bootsecure", 0);
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
শূন্য cli_secure_boot_cmd(ধ্রুৱ ৱর্ণ *cmd)
{
#ifdef CONFIG_CMDLINE
	সংরচনা cmd_tbl *cmdtp;
#endif
	পূর্ণাঙ্ক rc;

	যদি (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		গচ্ছ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	যদি (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		গচ্ছ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (ৱর্ণ **)&cmd);

#অন্যথা
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

শূন্য cli_loop(শূন্য)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	যদি (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#যদি CONFIG_IS_ENABLED(HUSH_PARSER)
	যদি (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	অন্যথা যদি (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	বাবে (;;);
#নোচেদ্যদি defined(CONFIG_CMDLINE)
	cli_simple_loop();
#অন্যথা
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

শূন্য cli_init(শূন্য)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	যদি (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		যদি (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		অন্যথা যদি (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	যদি (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} অন্যথা যদি (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} অন্যথা {
		printf("No valid hush parser to use, cli will not initialized!\n");
		ঘূৰাই;
	}
#endif

#যদি defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	যদি (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
