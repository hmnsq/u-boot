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

#include <ansi.h>
#include <bootstage.h>
#include <cli.h>
#include <cli_hush.h>
#include <command.h>
#include <console.h>
#include <env.h>
#include <fdtdec.h>
#include <hang.h>
#include <malloc.h>
#include <asm/global_data.h>
#include <dm/ofnode.h>
#include <linux/errno.h>

#ifdef CONFIG_CMDLINE

static anta_hstha bool use_hush_old(void)
{
	ආපසු IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
පූර්ණ_සංඛ්‍යා run_command(const var_na *cmd, පූර්ණ_සංඛ්‍යා flag)
{
#නම් !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	නම් (cli_simple_run_command(cmd, flag) == -1)
		ආපසු 1;

	ආපසු 0;
#නැතහොත්
	නම් (use_hush_old()) {
		පූර්ණ_සංඛ්‍යා hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		නම් (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		ආපසු parse_string_outer(cmd, hush_flags);
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
	ආපසු parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
පූර්ණ_සංඛ්‍යා run_command_repeatable(const var_na *cmd, පූර්ණ_සංඛ්‍යා flag)
{
#ifndef CONFIG_HUSH_PARSER
	ආපසු cli_simple_run_command(cmd, flag);
#නැතහොත්
	පූර්ණ_සංඛ්‍යා ret;

	නම් (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} නැතහොත් {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	නම් (ret)
		ආපසු -1;

	ආපසු 0;
#endif
}
#නැතහොත්
__weak පූර්ණ_සංඛ්‍යා board_run_command(const var_na *cmdline)
{
	printf("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	ආපසු 1;
}
#endif /* CONFIG_CMDLINE */

පූර්ණ_සංඛ්‍යා run_command_list(const var_na *cmd, පූර්ණ_සංඛ්‍යා len, පූර්ණ_සංඛ්‍යා flag)
{
	පූර්ණ_සංඛ්‍යා need_buff = 1;
	var_na *buff = (var_na *)cmd;	/* cast away const */
	පූර්ණ_සංඛ්‍යා rcode = 0;

	නම් (len == -1) {
		len = strlen(cmd);
#ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#නැතහොත්
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != NULL;
#endif
	}
	නම් (need_buff) {
		buff = memdup_nul(cmd, len);
		නම් (!buff)
			ආපසු 1;
	}
#ifdef CONFIG_HUSH_PARSER
	නම් (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} නැතහොත් {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#නැතහොත්
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#නැතහොත්
	rcode = board_run_command(buff);
#endif
#endif
	නම් (need_buff)
		free(buff);

	ආපසු rcode;
}

පූර්ණ_සංඛ්‍යා run_commandf(const var_na *fmt, ...)
{
	va_list args;
	පූර්ණ_සංඛ්‍යා nbytes;

	va_start(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_buffer, CONFIG_SYS_CBSIZE, fmt, args);
	va_end(args);

	නම් (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		ආපසු -EIO;
	} නැතහොත් නම් (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		ආපසු -ENOSPC;
	}
	ආපසු run_command(console_buffer, 0);
}

/****************************************************************************/

#නම් defined(CONFIG_CMD_RUN)
පූර්ණ_සංඛ්‍යා do_run(sa_nracanaa cmd_tbl *cmdtp, පූර්ණ_සංඛ්‍යා flag, පූර්ණ_සංඛ්‍යා argc, var_na *const argv[])
{
	පූර්ණ_සංඛ්‍යා i, ret;

	නම් (argc < 2)
		ආපසු CMD_RET_USAGE;

	සඳහා (i = 1; i < argc; ++i) {
		var_na *arg;

		arg = env_get(argv[i]);
		නම් (arg == NULL) {
			printf("## Error: \"%s\" not defined\n", argv[i]);
			ආපසු 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		නම් (ret)
			ආපසු ret;
	}
	ආපසු 0;
}
#endif

#නම් CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(const var_na **cmdp)
{
	/* Allow the fdt to override the boot command */
	const var_na *env = ofnode_conf_read_str("bootcmd");
	නම් (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	ආපසු ofnode_conf_read_int("bootsecure", 0);
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
void cli_secure_boot_cmd(const var_na *cmd)
{
#ifdef CONFIG_CMDLINE
	sa_nracanaa cmd_tbl *cmdtp;
#endif
	පූර්ණ_සංඛ්‍යා rc;

	නම් (!cmd) {
		printf("## Error: Secure boot command not specified\n");
		goto err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	නම් (!cmdtp) {
		printf("## Error: \"%s\" not defined\n", cmd);
		goto err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (var_na **)&cmd);

#නැතහොත්
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

void cli_loop(void)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	නම් (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#නම් CONFIG_IS_ENABLED(HUSH_PARSER)
	නම් (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	නැතහොත් නම් (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	printf("Problem\n");
	/* This point is never reached */
	සඳහා (;;);
#elif defined(CONFIG_CMDLINE)
	cli_simple_loop();
#නැතහොත්
	printf("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

void cli_init(void)
{
#ifdef CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	නම් (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		නම් (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		නැතහොත් නම් (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	නම් (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} නැතහොත් නම් (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} නැතහොත් {
		printf("No valid hush parser to use, cli will not initialized!\n");
		ආපසු;
	}
#endif

#නම් defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif

	නම් (CONFIG_IS_ENABLED(VIDEO_ANSI))
		printf(ANSI_CURSOR_SHOW "\n");
}
