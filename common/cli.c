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

#ଅନ୍ତର୍ଭାଵ <ansi.h>
#ଅନ୍ତର୍ଭାଵ <bootstage.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <cli_hush.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <fdtdec.h>
#ଅନ୍ତର୍ଭାଵ <hang.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <dm/ofnode.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>

#ifdef CONFIG_CMDLINE

ସ୍ଥିର ଧାଡ଼ିଭିତରେ bool use_hush_old(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_command(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
{
#ଯଦି !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	ଯଦି (cli_simple_run_command(cmd, flag) == -1)
		ଫେରନ୍ତୁ 1;

	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ଯଦି (use_hush_old()) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		ଯଦି (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ଫେରନ୍ତୁ parse_string_outer(cmd, hush_flags);
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
	ଫେରନ୍ତୁ parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_command_repeatable(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
{
#ifndef CONFIG_HUSH_PARSER
	ଫେରନ୍ତୁ cli_simple_run_command(cmd, flag);
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} ଅନ୍ୟଥା {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	ଯଦି (ret)
		ଫେରନ୍ତୁ -1;

	ଫେରନ୍ତୁ 0;
#endif
}
#ଅନ୍ୟଥା
__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା board_run_command(ଅଚଳ ଅକ୍ଷର *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ଫେରନ୍ତୁ 1;
}
#endif /* CONFIG_CMDLINE */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_command_list(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା need_buff = 1;
	ଅକ୍ଷର *buff = (ଅକ୍ଷର *)cmd;	/* cast away const */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = 0;

	ଯଦି (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#ଅନ୍ୟଥା
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ଖାଲି;
#endif
	}
	ଯଦି (need_buff) {
		buff = memdup_nul(cmd, len);
		ଯଦି (!buff)
			ଫେରନ୍ତୁ 1;
	}
#ifdef CONFIG_HUSH_PARSER
	ଯଦି (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} ଅନ୍ୟଥା {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#ଅନ୍ୟଥା
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#ଅନ୍ୟଥା
	rcode = board_run_command(buff);
#endif
#endif
	ଯଦି (need_buff)
		ମୁକ୍ତ(buff);

	ଫେରନ୍ତୁ rcode;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_commandf(ଅଚଳ ଅକ୍ଷର *fmt, ...)
{
	va_list args;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	ଯଦି (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ଫେରନ୍ତୁ -EIO;
	} ଅନ୍ୟଥା ଯଦି (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ଫେରନ୍ତୁ -ENOSPC;
	}
	ଫେରନ୍ତୁ run_command(console_buffer, 0);
}

/****************************************************************************/

#ଯଦି defined(CONFIG_CMD_RUN)
ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_run(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ret;

	ଯଦି (argc < 2)
		ଫେରନ୍ତୁ CMD_RET_USAGE;

	ପାଇଁ (i = 1; i < argc; ++i) {
		ଅକ୍ଷର *arg;

		arg = env_get(argv[i]);
		ଯଦି (arg == ଖାଲି) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			ଫେରନ୍ତୁ 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}
	ଫେରନ୍ତୁ 0;
}
#endif

#ଯଦି CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(ଅଚଳ ଅକ୍ଷର **cmdp)
{
	/* Allow the fdt to override the boot command */
	ଅଚଳ ଅକ୍ଷର *env = ofnode_conf_read_str("bootcmd");
	ଯଦି (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ଫେରନ୍ତୁ ofnode_conf_read_int("bootsecure", 0);
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
ଶୂନ୍ୟ cli_secure_boot_cmd(ଅଚଳ ଅକ୍ଷର *cmd)
{
#ifdef CONFIG_CMDLINE
	ସଂରଚନା cmd_tbl *cmdtp;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		ଯାଅ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	ଯଦି (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		ଯାଅ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (ଅକ୍ଷର **)&cmd);

#ଅନ୍ୟଥା
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

ଶୂନ୍ୟ cli_loop(ଶୂନ୍ୟ)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	ଯଦି (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#ଯଦି CONFIG_IS_ENABLED(HUSH_PARSER)
	ଯଦି (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	ଅନ୍ୟଥା ଯଦି (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	ପାଇଁ (;;);
#ନଚେତଯଦି defined(CONFIG_CMDLINE)
	cli_simple_loop();
#ଅନ୍ୟଥା
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

ଶୂନ୍ୟ cli_init(ଶୂନ୍ୟ)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	ଯଦି (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		ଯଦି (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		ଅନ୍ୟଥା ଯଦି (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	ଯଦି (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} ଅନ୍ୟଥା ଯଦି (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} ଅନ୍ୟଥା {
		printf("No valid hush parser to use, cli will not initialized!\n");
		ଫେରନ୍ତୁ;
	}
#endif

#ଯଦି defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	ଯଦି (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
