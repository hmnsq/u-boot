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

#અન્તર્ભાવ <ansi.h>
#અન્તર્ભાવ <bootstage.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <cli_hush.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <fdtdec.h>
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <dm/ofnode.h>
#અન્તર્ભાવ <linux/errno.h>

#ifdef CONFIG_CMDLINE

સ્થિર અંતઃરેખ bool use_hush_old(શૂન્ય)
{
	પાછા IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
પૂર્ણાંક run_command(અચલ અક્ષર *cmd, પૂર્ણાંક flag)
{
#જો !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	જો (cli_simple_run_command(cmd, flag) == -1)
		પાછા 1;

	પાછા 0;
#નહીંતર
	જો (use_hush_old()) {
		પૂર્ણાંક hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		જો (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		પાછા parse_string_outer(cmd, hush_flags);
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
	પાછા parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
પૂર્ણાંક run_command_repeatable(અચલ અક્ષર *cmd, પૂર્ણાંક flag)
{
#ifndef CONFIG_HUSH_PARSER
	પાછા cli_simple_run_command(cmd, flag);
#નહીંતર
	પૂર્ણાંક ret;

	જો (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} નહીંતર {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	જો (ret)
		પાછા -1;

	પાછા 0;
#endif
}
#નહીંતર
__weak પૂર્ણાંક board_run_command(અચલ અક્ષર *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	પાછા 1;
}
#endif /* CONFIG_CMDLINE */

પૂર્ણાંક run_command_list(અચલ અક્ષર *cmd, પૂર્ણાંક len, પૂર્ણાંક flag)
{
	પૂર્ણાંક need_buff = 1;
	અક્ષર *buff = (અક્ષર *)cmd;	/* cast away const */
	પૂર્ણાંક rcode = 0;

	જો (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#નહીંતર
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ખાલી;
#endif
	}
	જો (need_buff) {
		buff = memdup_nul(cmd, len);
		જો (!buff)
			પાછા 1;
	}
#ifdef CONFIG_HUSH_PARSER
	જો (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} નહીંતર {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#નહીંતર
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#નહીંતર
	rcode = board_run_command(buff);
#endif
#endif
	જો (need_buff)
		મુક્ત(buff);

	પાછા rcode;
}

પૂર્ણાંક run_commandf(અચલ અક્ષર *fmt, ...)
{
	va_list args;
	પૂર્ણાંક nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	જો (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		પાછા -EIO;
	} નહીંતર જો (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		પાછા -ENOSPC;
	}
	પાછા run_command(console_buffer, 0);
}

/****************************************************************************/

#જો defined(CONFIG_CMD_RUN)
પૂર્ણાંક do_run(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc, અક્ષર *અચલ argv[])
{
	પૂર્ણાંક i, ret;

	જો (argc < 2)
		પાછા CMD_RET_USAGE;

	માટે (i = 1; i < argc; ++i) {
		અક્ષર *arg;

		arg = env_get(argv[i]);
		જો (arg == ખાલી) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			પાછા 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		જો (ret)
			પાછા ret;
	}
	પાછા 0;
}
#endif

#જો CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(અચલ અક્ષર **cmdp)
{
	/* Allow the fdt to override the boot command */
	અચલ અક્ષર *env = ofnode_conf_read_str("bootcmd");
	જો (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	પાછા ofnode_conf_read_int("bootsecure", 0);
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
શૂન્ય cli_secure_boot_cmd(અચલ અક્ષર *cmd)
{
#ifdef CONFIG_CMDLINE
	સંરચના cmd_tbl *cmdtp;
#endif
	પૂર્ણાંક rc;

	જો (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		જાઓ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	જો (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		જાઓ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (અક્ષર **)&cmd);

#નહીંતર
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

શૂન્ય cli_loop(શૂન્ય)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	જો (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#જો CONFIG_IS_ENABLED(HUSH_PARSER)
	જો (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	નહીંતર જો (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	માટે (;;);
#અથવાજો defined(CONFIG_CMDLINE)
	cli_simple_loop();
#નહીંતર
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

શૂન્ય cli_init(શૂન્ય)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	જો (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		જો (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		નહીંતર જો (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	જો (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} નહીંતર જો (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} નહીંતર {
		printf("No valid hush parser to use, cli will not initialized!\n");
		પાછા;
	}
#endif

#જો defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	જો (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
