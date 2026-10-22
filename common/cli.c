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

#ಅನ್ತರ್ಭಾವ <ansi.h>
#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <cli_hush.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <dm/ofnode.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>

#ifdef CONFIG_CMDLINE

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ bool use_hush_old(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
ಪೂರ್ಣಾಙ್ಕ run_command(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ flag)
{
#ಯದಿ !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	ಯದಿ (cli_simple_run_command(cmd, flag) == -1)
		ಪ್ರತ್ಯಾಗಮ 1;

	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	ಯದಿ (use_hush_old()) {
		ಪೂರ್ಣಾಙ್ಕ hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		ಯದಿ (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ಪ್ರತ್ಯಾಗಮ parse_string_outer(cmd, hush_flags);
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
	ಪ್ರತ್ಯಾಗಮ parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
ಪೂರ್ಣಾಙ್ಕ run_command_repeatable(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ flag)
{
#ifndef CONFIG_HUSH_PARSER
	ಪ್ರತ್ಯಾಗಮ cli_simple_run_command(cmd, flag);
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} ಅನ್ಯಥಾ {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ -1;

	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}
#ಅನ್ಯಥಾ
__weak ಪೂರ್ಣಾಙ್ಕ board_run_command(ಧ್ರುವ ವರ್ಣ *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ಪ್ರತ್ಯಾಗಮ 1;
}
#endif /* CONFIG_CMDLINE */

ಪೂರ್ಣಾಙ್ಕ run_command_list(ಧ್ರುವ ವರ್ಣ *cmd, ಪೂರ್ಣಾಙ್ಕ len, ಪೂರ್ಣಾಙ್ಕ flag)
{
	ಪೂರ್ಣಾಙ್ಕ need_buff = 1;
	ವರ್ಣ *buff = (ವರ್ಣ *)cmd;	/* cast away const */
	ಪೂರ್ಣಾಙ್ಕ rcode = 0;

	ಯದಿ (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#ಅನ್ಯಥಾ
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ರಿಕ್ತ;
#endif
	}
	ಯದಿ (need_buff) {
		buff = memdup_nul(cmd, len);
		ಯದಿ (!buff)
			ಪ್ರತ್ಯಾಗಮ 1;
	}
#ifdef CONFIG_HUSH_PARSER
	ಯದಿ (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} ಅನ್ಯಥಾ {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#ಅನ್ಯಥಾ
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#ಅನ್ಯಥಾ
	rcode = board_run_command(buff);
#endif
#endif
	ಯದಿ (need_buff)
		ಮುಕ್ತ(buff);

	ಪ್ರತ್ಯಾಗಮ rcode;
}

ಪೂರ್ಣಾಙ್ಕ run_commandf(ಧ್ರುವ ವರ್ಣ *fmt, ...)
{
	va_list args;
	ಪೂರ್ಣಾಙ್ಕ nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	ಯದಿ (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ಪ್ರತ್ಯಾಗಮ -EIO;
	} ಅನ್ಯಥಾ ಯದಿ (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ಪ್ರತ್ಯಾಗಮ -ENOSPC;
	}
	ಪ್ರತ್ಯಾಗಮ run_command(console_buffer, 0);
}

/****************************************************************************/

#ಯದಿ defined(CONFIG_CMD_RUN)
ಪೂರ್ಣಾಙ್ಕ do_run(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[])
{
	ಪೂರ್ಣಾಙ್ಕ i, ret;

	ಯದಿ (argc < 2)
		ಪ್ರತ್ಯಾಗಮ CMD_RET_USAGE;

	ಪ್ರತಿ (i = 1; i < argc; ++i) {
		ವರ್ಣ *arg;

		arg = env_get(argv[i]);
		ಯದಿ (arg == ರಿಕ್ತ) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			ಪ್ರತ್ಯಾಗಮ 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

#ಯದಿ CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ಧ್ರುವ ವರ್ಣ **cmdp)
{
	/* Allow the fdt to override the boot command */
	ಧ್ರುವ ವರ್ಣ *env = ofnode_conf_read_str("bootcmd");
	ಯದಿ (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ಪ್ರತ್ಯಾಗಮ ofnode_conf_read_int("bootsecure", 0);
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
ಶೂನ್ಯ cli_secure_boot_cmd(ಧ್ರುವ ವರ್ಣ *cmd)
{
#ifdef CONFIG_CMDLINE
	ಸಂರಚನಾ cmd_tbl *cmdtp;
#endif
	ಪೂರ್ಣಾಙ್ಕ rc;

	ಯದಿ (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		ಗಚ್ಛ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	ಯದಿ (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		ಗಚ್ಛ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (ವರ್ಣ **)&cmd);

#ಅನ್ಯಥಾ
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

ಶೂನ್ಯ cli_loop(ಶೂನ್ಯ)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	ಯದಿ (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#ಯದಿ CONFIG_IS_ENABLED(HUSH_PARSER)
	ಯದಿ (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	ಅನ್ಯಥಾ ಯದಿ (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	ಪ್ರತಿ (;;);
#ನೋಚೇದ್ಯದಿ defined(CONFIG_CMDLINE)
	cli_simple_loop();
#ಅನ್ಯಥಾ
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

ಶೂನ್ಯ cli_init(ಶೂನ್ಯ)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	ಯದಿ (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		ಯದಿ (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		ಅನ್ಯಥಾ ಯದಿ (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	ಯದಿ (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} ಅನ್ಯಥಾ ಯದಿ (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} ಅನ್ಯಥಾ {
		printf("No valid hush parser to use, cli will not initialized!\n");
		ಪ್ರತ್ಯಾಗಮ;
	}
#endif

#ಯದಿ defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	ಯದಿ (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
