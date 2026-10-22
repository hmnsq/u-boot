// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ವ್ಯಾಖ್ಯಾನಿಸು pr_fmt(fmt) "cli: %s: " fmt, __func__

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

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE

ಸ್ಥಿರ ಸಾಲೊಳಗೆ bool use_hush_old(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
ಪೂರ್ಣಾಂಕ ಸತತ_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ flag)
{
#ಆದರೆ !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	ಆದರೆ (cli_simple_ಸತತ_command(cmd, flag) == -1)
		ಹಿಂತಿರುಗಿ 1;

	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (use_hush_old()) {
		ಪೂರ್ಣಾಂಕ hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		ಆದರೆ (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ಹಿಂತಿರುಗಿ parse_string_outer(cmd, hush_flags);
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
	ಹಿಂತಿರುಗಿ parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#ಆದರೆ_ಅಂತ್ಯ
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
ಪೂರ್ಣಾಂಕ ಸತತ_command_repeatable(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ flag)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_HUSH_PARSER
	ಹಿಂತಿರುಗಿ cli_simple_ಸತತ_command(cmd, flag);
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ -1;

	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
}
#ಇಲ್ಲದಿದ್ದರೆ
__weak ಪೂರ್ಣಾಂಕ board_ಸತತ_command(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmdline)
{
	ಬರೆ("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ಹಿಂತಿರುಗಿ 1;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_CMDLINE */

ಪೂರ್ಣಾಂಕ ಸತತ_command_list(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ len, ಪೂರ್ಣಾಂಕ flag)
{
	ಪೂರ್ಣಾಂಕ need_buff = 1;
	ಅಕ್ಷರ *buff = (ಅಕ್ಷರ *)cmd;	/* cast away const */
	ಪೂರ್ಣಾಂಕ rcode = 0;

	ಆದರೆ (len == -1) {
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cmd);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#ಇಲ್ಲದಿದ್ದರೆ
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	}
	ಆದರೆ (need_buff) {
		buff = memdup_nul(cmd, len);
		ಆದರೆ (!buff)
			ಹಿಂತಿರುಗಿ 1;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_HUSH_PARSER
	ಆದರೆ (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} ಇಲ್ಲದಿದ್ದರೆ {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#ಇಲ್ಲದಿದ್ದರೆ
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
	rcode = cli_simple_ಸತತ_command_list(buff, flag);
#ಇಲ್ಲದಿದ್ದರೆ
	rcode = board_ಸತತ_command(buff);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (need_buff)
		ಬಿಡುಗಡೆ(buff);

	ಹಿಂತಿರುಗಿ rcode;
}

ಪೂರ್ಣಾಂಕ ಸತತ_commandf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
{
	va_list ವಾದಗಳು;
	ಪೂರ್ಣಾಂಕ nbytes;

	va_ಆರಂಭ(ವಾದಗಳು, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_ಬಫರ್, CONFIG_SYS_CBSIZE, fmt, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);

	ಆದರೆ (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ಹಿಂತಿರುಗಿ -EIO;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}
	ಹಿಂತಿರುಗಿ ಸತತ_command(console_ಬಫರ್, 0);
}

/****************************************************************************/

#ಆದರೆ defined(CONFIG_CMD_ಸತತ)
ಪೂರ್ಣಾಂಕ do_ಸತತ(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ಪೂರ್ಣಾಂಕ i, ret;

	ಆದರೆ (argc < 2)
		ಹಿಂತಿರುಗಿ CMD_RET_USAGE;

	ಗಾಗಿ (i = 1; i < argc; ++i) {
		ಅಕ್ಷರ *arg;

		arg = env_get(argv[i]);
		ಆದರೆ (arg == ಖಾಲಿ) {
			ಬರೆ("## Error: \"%s\" not defined\n", argv[i]);
			ಹಿಂತಿರುಗಿ 1;
		}

		ret = ಸತತ_command(arg, flag | CMD_FLAG_ENV);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ CONFIG_IS_ENABLED(ದ_CONTROL)
bool cli_process_fdt(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **cmdp)
{
	/* Allow the fdt to override the boot command */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *env = ofnode_conf_read_str("bootcmd");
	ಆದರೆ (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ಹಿಂತಿರುಗಿ ofnode_conf_read_int("bootsecure", 0);
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
ಶೂನ್ಯ cli_secure_boot_cmd(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
	ರಚನೆ cmd_tbl *cmdtp;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (!cmd) {
		ಬರೆ("## Error: Secure boot command not specified\n");
		ಹೋಗು err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE
	cmdtp = ಕಂಡುಹಿಡಿ_cmd(cmd);
	ಆದರೆ (!cmdtp) {
		ಬರೆ("## Error: \"%s\" not defined\n", cmd);
		ಹೋಗು err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (ಅಕ್ಷರ **)&cmd);

#ಇಲ್ಲದಿದ್ದರೆ
	rc = board_ಸತತ_command(cmd);
#ಆದರೆ_ಅಂತ್ಯ

	/* Shouldn't ever return from boot command. */
	ಬರೆ("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_IS_ENABLED(OF_CONTROL) */

ಶೂನ್ಯ cli_loop(ಶೂನ್ಯ)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	ಆದರೆ (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#ಆದರೆ CONFIG_IS_ENABLED(HUSH_PARSER)
	ಆದರೆ (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_ಸತತ_file();
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	ಬರೆ("Problem\n");
	/* This point is never reached */
	ಗಾಗಿ (;;);
#ಅಥವಾಆದರೆ defined(CONFIG_CMDLINE)
	cli_simple_loop();
#ಇಲ್ಲದಿದ್ದರೆ
	ಬರೆ("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#ಆದರೆ_ಅಂತ್ಯ /*CONFIG_HUSH_PARSER*/
}

ಶೂನ್ಯ cli_init(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	ಆದರೆ (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		ಆದರೆ (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	ಆದರೆ (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_ಆರಂಭ();
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_ಆರಂಭ_modern();
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಬರೆ("No valid hush parser to use, cli will not initialized!\n");
		ಹಿಂತಿರುಗಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (CONFIG_IS_ENABLED(VIDEO_ANSI))
		ಬರೆ(ANSI_CURSOR_SHOW "\n");
}
