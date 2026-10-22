// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#నిర్వచించు pr_fmt(fmt) "cli: %s: " fmt, __func__

#అన్తర్భావ <ansi.h>
#అన్తర్భావ <bootstage.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <cli_hush.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <hang.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <dm/ofnode.h>
#అన్తర్భావ <linux/errno.h>

#నిర్వచితమైతే CONFIG_CMDLINE

స్థిర అంతఃవరుస bool use_hush_old(శూన్య)
{
	తిరిగి IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
పూర్ణాంకం నిరంతరం_command(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం flag)
{
#ఒకవేళ !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	ఒకవేళ (cli_simple_నిరంతరం_command(cmd, flag) == -1)
		తిరిగి 1;

	తిరిగి 0;
#లేకపోతే
	ఒకవేళ (use_hush_old()) {
		పూర్ణాంకం hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		ఒకవేళ (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		తిరిగి parse_string_outer(cmd, hush_flags);
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
	తిరిగి parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#అయితే_ముగింపు
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
పూర్ణాంకం నిరంతరం_command_repeatable(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం flag)
{
#నిర్వచితం_కాకపోతే CONFIG_HUSH_PARSER
	తిరిగి cli_simple_నిరంతరం_command(cmd, flag);
#లేకపోతే
	పూర్ణాంకం ret;

	ఒకవేళ (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} లేకపోతే {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	ఒకవేళ (ret)
		తిరిగి -1;

	తిరిగి 0;
#అయితే_ముగింపు
}
#లేకపోతే
__weak పూర్ణాంకం board_నిరంతరం_command(స్థిరరాశి అక్షరం *cmdline)
{
	రాయి("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	తిరిగి 1;
}
#అయితే_ముగింపు /* CONFIG_CMDLINE */

పూర్ణాంకం నిరంతరం_command_list(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం len, పూర్ణాంకం flag)
{
	పూర్ణాంకం need_buff = 1;
	అక్షరం *buff = (అక్షరం *)cmd;	/* cast away const */
	పూర్ణాంకం rcode = 0;

	ఒకవేళ (len == -1) {
		len = అక్షరశ్రేణి_పొడవు(cmd);
#నిర్వచితమైతే CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#లేకపోతే
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ఖాళీ;
#అయితే_ముగింపు
	}
	ఒకవేళ (need_buff) {
		buff = memdup_nul(cmd, len);
		ఒకవేళ (!buff)
			తిరిగి 1;
	}
#నిర్వచితమైతే CONFIG_HUSH_PARSER
	ఒకవేళ (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} లేకపోతే {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#లేకపోతే
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#నిర్వచితమైతే CONFIG_CMDLINE
	rcode = cli_simple_నిరంతరం_command_list(buff, flag);
#లేకపోతే
	rcode = board_నిరంతరం_command(buff);
#అయితే_ముగింపు
#అయితే_ముగింపు
	ఒకవేళ (need_buff)
		విడుదల(buff);

	తిరిగి rcode;
}

పూర్ణాంకం నిరంతరం_commandf(స్థిరరాశి అక్షరం *fmt, ...)
{
	va_list వాదనలు;
	పూర్ణాంకం nbytes;

	va_ప్రారంభం(వాదనలు, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_బఫరు, CONFIG_SYS_CBSIZE, fmt, వాదనలు);
	va_చివర(వాదనలు);

	ఒకవేళ (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		తిరిగి -EIO;
	} లేకపోతే ఒకవేళ (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		తిరిగి -ENOSPC;
	}
	తిరిగి నిరంతరం_command(console_బఫరు, 0);
}

/****************************************************************************/

#ఒకవేళ defined(CONFIG_CMD_నిరంతరం)
పూర్ణాంకం do_నిరంతరం(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[])
{
	పూర్ణాంకం i, ret;

	ఒకవేళ (argc < 2)
		తిరిగి CMD_RET_USAGE;

	కొరకు (i = 1; i < argc; ++i) {
		అక్షరం *arg;

		arg = env_get(argv[i]);
		ఒకవేళ (arg == ఖాళీ) {
			రాయి("## Error: \"%s\" not defined\n", argv[i]);
			తిరిగి 1;
		}

		ret = నిరంతరం_command(arg, flag | CMD_FLAG_ENV);
		ఒకవేళ (ret)
			తిరిగి ret;
	}
	తిరిగి 0;
}
#అయితే_ముగింపు

#ఒకవేళ CONFIG_IS_ENABLED(యొక్క_CONTROL)
bool cli_process_fdt(స్థిరరాశి అక్షరం **cmdp)
{
	/* Allow the fdt to override the boot command */
	స్థిరరాశి అక్షరం *env = ofnode_conf_read_str("bootcmd");
	ఒకవేళ (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	తిరిగి ofnode_conf_read_int("bootsecure", 0);
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
శూన్య cli_secure_boot_cmd(స్థిరరాశి అక్షరం *cmd)
{
#నిర్వచితమైతే CONFIG_CMDLINE
	నిర్మాణం cmd_tbl *cmdtp;
#అయితే_ముగింపు
	పూర్ణాంకం rc;

	ఒకవేళ (!cmd) {
		రాయి("## Error: Secure boot command not specified\n");
		వెళ్ళు err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#నిర్వచితమైతే CONFIG_CMDLINE
	cmdtp = కనుగొను_cmd(cmd);
	ఒకవేళ (!cmdtp) {
		రాయి("## Error: \"%s\" not defined\n", cmd);
		వెళ్ళు err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (అక్షరం **)&cmd);

#లేకపోతే
	rc = board_నిరంతరం_command(cmd);
#అయితే_ముగింపు

	/* Shouldn't ever return from boot command. */
	రాయి("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#అయితే_ముగింపు /* CONFIG_IS_ENABLED(OF_CONTROL) */

శూన్య cli_loop(శూన్య)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	ఒకవేళ (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#ఒకవేళ CONFIG_IS_ENABLED(HUSH_PARSER)
	ఒకవేళ (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_నిరంతరం_file();
	లేకపోతే ఒకవేళ (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	రాయి("Problem\n");
	/* This point is never reached */
	కొరకు (;;);
#లేదావేళ defined(CONFIG_CMDLINE)
	cli_simple_loop();
#లేకపోతే
	రాయి("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#అయితే_ముగింపు /*CONFIG_HUSH_PARSER*/
}

శూన్య cli_init(శూన్య)
{
#నిర్వచితమైతే CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	ఒకవేళ (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		ఒకవేళ (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	ఒకవేళ (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_ప్రారంభం();
	} లేకపోతే ఒకవేళ (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_ప్రారంభం_modern();
	} లేకపోతే {
		రాయి("No valid hush parser to use, cli will not initialized!\n");
		తిరిగి;
	}
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#అయితే_ముగింపు

	ఒకవేళ (CONFIG_IS_ENABLED(VIDEO_ANSI))
		రాయి(ANSI_CURSOR_SHOW "\n");
}
