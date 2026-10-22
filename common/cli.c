// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#تعریف_کړه pr_fmt(fmt) "cli: %s: " fmt, __func__

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

#که_تعریف_شوی CONFIG_CMDLINE

ساكن مضمن bool use_hush_old(فراغ)
{
	أرجع IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
صحيح پرلپسې_command(ثابت حرف *cmd, صحيح flag)
{
#إذا !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	إذا (cli_simple_پرلپسې_command(cmd, flag) == -1)
		أرجع 1;

	أرجع 0;
#وإلا
	إذا (use_hush_old()) {
		صحيح hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		إذا (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		أرجع parse_string_outer(cmd, hush_flags);
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
	أرجع parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#د_که_پای
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
صحيح پرلپسې_command_repeatable(ثابت حرف *cmd, صحيح flag)
{
#که_نه_تعریف_شوی CONFIG_HUSH_PARSER
	أرجع cli_simple_پرلپسې_command(cmd, flag);
#وإلا
	صحيح ret;

	إذا (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} وإلا {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	إذا (ret)
		أرجع -1;

	أرجع 0;
#د_که_پای
}
#وإلا
__weak صحيح board_پرلپسې_command(ثابت حرف *cmdline)
{
	چاپ_کړه("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	أرجع 1;
}
#د_که_پای /* CONFIG_CMDLINE */

صحيح پرلپسې_command_list(ثابت حرف *cmd, صحيح len, صحيح flag)
{
	صحيح need_buff = 1;
	حرف *buff = (حرف *)cmd;	/* cast away const */
	صحيح rcode = 0;

	إذا (len == -1) {
		len = مزي_اوږدوالی(cmd);
#که_تعریف_شوی CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#وإلا
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != عدم;
#د_که_پای
	}
	إذا (need_buff) {
		buff = memdup_nul(cmd, len);
		إذا (!buff)
			أرجع 1;
	}
#که_تعریف_شوی CONFIG_HUSH_PARSER
	إذا (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} وإلا {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#وإلا
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#که_تعریف_شوی CONFIG_CMDLINE
	rcode = cli_simple_پرلپسې_command_list(buff, flag);
#وإلا
	rcode = board_پرلپسې_command(buff);
#د_که_پای
#د_که_پای
	إذا (need_buff)
		حرر(buff);

	أرجع rcode;
}

صحيح پرلپسې_commandf(ثابت حرف *fmt, ...)
{
	va_list دلیلونه;
	صحيح nbytes;

	va_پیل(دلیلونه, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_بفر, CONFIG_SYS_CBSIZE, fmt, دلیلونه);
	va_پای_ټکی(دلیلونه);

	إذا (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		أرجع -EIO;
	} وإلا إذا (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		أرجع -ENOSPC;
	}
	أرجع پرلپسې_command(console_بفر, 0);
}

/****************************************************************************/

#إذا defined(CONFIG_CMD_پرلپسې)
صحيح do_پرلپسې(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc, حرف *ثابت argv[])
{
	صحيح i, ret;

	إذا (argc < 2)
		أرجع CMD_RET_USAGE;

	لكل (i = 1; i < argc; ++i) {
		حرف *arg;

		arg = env_get(argv[i]);
		إذا (arg == عدم) {
			چاپ_کړه("## Error: \"%s\" not defined\n", argv[i]);
			أرجع 1;
		}

		ret = پرلپسې_command(arg, flag | CMD_FLAG_ENV);
		إذا (ret)
			أرجع ret;
	}
	أرجع 0;
}
#د_که_پای

#إذا CONFIG_IS_ENABLED(د_CONTROL)
bool cli_process_fdt(ثابت حرف **cmdp)
{
	/* Allow the fdt to override the boot command */
	ثابت حرف *env = ofnode_conf_read_str("bootcmd");
	إذا (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	أرجع ofnode_conf_read_int("bootsecure", 0);
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
#که_تعریف_شوی CONFIG_CMDLINE
	بنية cmd_tbl *cmdtp;
#د_که_پای
	صحيح rc;

	إذا (!cmd) {
		چاپ_کړه("## Error: Secure boot command not specified\n");
		اذهب err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#که_تعریف_شوی CONFIG_CMDLINE
	cmdtp = پیدا_کړه_cmd(cmd);
	إذا (!cmdtp) {
		چاپ_کړه("## Error: \"%s\" not defined\n", cmd);
		اذهب err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (حرف **)&cmd);

#وإلا
	rc = board_پرلپسې_command(cmd);
#د_که_پای

	/* Shouldn't ever return from boot command. */
	چاپ_کړه("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#د_که_پای /* CONFIG_IS_ENABLED(OF_CONTROL) */

فراغ cli_loop(فراغ)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	إذا (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#إذا CONFIG_IS_ENABLED(HUSH_PARSER)
	إذا (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_پرلپسې_file();
	وإلا إذا (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	چاپ_کړه("Problem\n");
	/* This point is never reached */
	لكل (;;);
#وإلاإذا defined(CONFIG_CMDLINE)
	cli_simple_loop();
#وإلا
	چاپ_کړه("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#د_که_پای /*CONFIG_HUSH_PARSER*/
}

فراغ cli_init(فراغ)
{
#که_تعریف_شوی CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	إذا (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		إذا (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		وإلا إذا (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	إذا (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_پیل();
	} وإلا إذا (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_پیل_modern();
	} وإلا {
		چاپ_کړه("No valid hush parser to use, cli will not initialized!\n");
		أرجع;
	}
#د_که_پای

#إذا defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#د_که_پای

	إذا (CONFIG_IS_ENABLED(VIDEO_ANSI))
		چاپ_کړه(ANSI_CURSOR_SHOW "\n");
}
