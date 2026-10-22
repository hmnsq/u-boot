// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#घोषणा pr_fmt(fmt) "cli: %s: " fmt, __func__

#समावेश <ansi.h>
#समावेश <bootstage.h>
#समावेश <cli.h>
#समावेश <cli_hush.h>
#समावेश <command.h>
#समावेश <console.h>
#समावेश <env.h>
#समावेश <fdtdec.h>
#समावेश <hang.h>
#समावेश <malloc.h>
#समावेश <asm/global_data.h>
#समावेश <dm/ofnode.h>
#समावेश <linux/errno.h>

#अगर_घोषित CONFIG_CMDLINE

अटल अंतरभूत bool use_hush_old(व्योम)
{
	वापस IS_ENABLED(CONFIG_HUSH_SELECTABLE) ?
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
पूर्णांक run_command(स्थिर अक्षर *cmd, पूर्णांक flag)
{
#अगर !IS_ENABLED(CONFIG_HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	अगर (cli_simple_run_command(cmd, flag) == -1)
		वापस 1;

	वापस 0;
#अन्यथा
	अगर (use_hush_old()) {
		पूर्णांक hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

		अगर (flag & CMD_FLAG_ENV)
			hush_flags |= FLAG_CONT_ON_NEWLINE;
		वापस parse_string_outer(cmd, hush_flags);
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
	वापस parse_string_outer_modern(cmd, FLAG_EXIT_FROM_LOOP);
#पूर्ण_अगर
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * Return: 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
पूर्णांक run_command_repeatable(स्थिर अक्षर *cmd, पूर्णांक flag)
{
#अगर_अघोषित CONFIG_HUSH_PARSER
	वापस cli_simple_run_command(cmd, flag);
#अन्यथा
	पूर्णांक ret;

	अगर (use_hush_old()) {
		ret = parse_string_outer(cmd,
					 FLAG_PARSE_SEMICOLON
					 | FLAG_EXIT_FROM_LOOP);
	} अन्यथा {
		ret = parse_string_outer_modern(cmd,
					      FLAG_PARSE_SEMICOLON
					      | FLAG_EXIT_FROM_LOOP);
	}

	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	अगर (ret)
		वापस -1;

	वापस 0;
#पूर्ण_अगर
}
#अन्यथा
__weak पूर्णांक board_run_command(स्थिर अक्षर *cmdline)
{
	म_लिखो("## Commands are disabled. Please enable CONFIG_CMDLINE.\n");

	वापस 1;
}
#पूर्ण_अगर /* CONFIG_CMDLINE */

पूर्णांक run_command_list(स्थिर अक्षर *cmd, पूर्णांक len, पूर्णांक flag)
{
	पूर्णांक need_buff = 1;
	अक्षर *buff = (अक्षर *)cmd;	/* cast away const */
	पूर्णांक rcode = 0;

	अगर (len == -1) {
		len = म_माप(cmd);
#अगर_घोषित CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#अन्यथा
		/* the built-in parser will change our string if it sees \n */
		need_buff = म_अक्षर(cmd, '\n') != शून्य;
#पूर्ण_अगर
	}
	अगर (need_buff) {
		buff = memdup_nul(cmd, len);
		अगर (!buff)
			वापस 1;
	}
#अगर_घोषित CONFIG_HUSH_PARSER
	अगर (use_hush_old()) {
		rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
	} अन्यथा {
		rcode = parse_string_outer_modern(buff, FLAG_PARSE_SEMICOLON);
	}
#अन्यथा
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#अगर_घोषित CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#अन्यथा
	rcode = board_run_command(buff);
#पूर्ण_अगर
#पूर्ण_अगर
	अगर (need_buff)
		मुक्त(buff);

	वापस rcode;
}

पूर्णांक run_commandf(स्थिर अक्षर *fmt, ...)
{
	बहु_सूची args;
	पूर्णांक nbytes;

	बहु_शुरू(args, fmt);
	/*
	 * Limit the console_buffer space being used to CONFIG_SYS_CBSIZE,
	 * because its last byte is used to fit the replacement of \0 by \n\0
	 * in underlying hush parser
	 */
	nbytes = vsnprintf(console_बफर, CONFIG_SYS_CBSIZE, fmt, args);
	बहु_पूर्ण(args);

	अगर (nbytes < 0) {
		pr_debug("I/O internal error occurred.\n");
		वापस -EIO;
	} अन्यथा अगर (nbytes >= CONFIG_SYS_CBSIZE) {
		pr_debug("'fmt' size:%d exceeds the limit(%d)\n",
			 nbytes, CONFIG_SYS_CBSIZE);
		वापस -ENOSPC;
	}
	वापस run_command(console_बफर, 0);
}

/****************************************************************************/

#अगर defined(CONFIG_CMD_RUN)
पूर्णांक do_run(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिर argv[])
{
	पूर्णांक i, ret;

	अगर (argc < 2)
		वापस CMD_RET_USAGE;

	क्रम (i = 1; i < argc; ++i) {
		अक्षर *arg;

		arg = env_get(argv[i]);
		अगर (arg == शून्य) {
			म_लिखो("## Error: \"%s\" not defined\n", argv[i]);
			वापस 1;
		}

		ret = run_command(arg, flag | CMD_FLAG_ENV);
		अगर (ret)
			वापस ret;
	}
	वापस 0;
}
#पूर्ण_अगर

#अगर CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(स्थिर अक्षर **cmdp)
{
	/* Allow the fdt to override the boot command */
	स्थिर अक्षर *env = ofnode_conf_read_str("bootcmd");
	अगर (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	वापस ofnode_conf_read_int("bootsecure", 0);
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
व्योम cli_secure_boot_cmd(स्थिर अक्षर *cmd)
{
#अगर_घोषित CONFIG_CMDLINE
	काष्ठा cmd_tbl *cmdtp;
#पूर्ण_अगर
	पूर्णांक rc;

	अगर (!cmd) {
		म_लिखो("## Error: Secure boot command not specified\n");
		जाओ err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#अगर_घोषित CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	अगर (!cmdtp) {
		म_लिखो("## Error: \"%s\" not defined\n", cmd);
		जाओ err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (अक्षर **)&cmd);

#अन्यथा
	rc = board_run_command(cmd);
#पूर्ण_अगर

	/* Shouldn't ever return from boot command. */
	म_लिखो("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#पूर्ण_अगर /* CONFIG_IS_ENABLED(OF_CONTROL) */

व्योम cli_loop(व्योम)
{
	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);

	अगर (IS_ENABLED(CONFIG_CMDLINE_FLUSH_STDIN))
		console_flush_stdin();

#अगर CONFIG_IS_ENABLED(HUSH_PARSER)
	अगर (gd->flags & GD_FLG_HUSH_MODERN_PARSER)
		parse_and_run_file();
	अन्यथा अगर (gd->flags & GD_FLG_HUSH_OLD_PARSER)
		parse_file_outer();

	म_लिखो("Problem\n");
	/* This point is never reached */
	क्रम (;;);
#या_अगर defined(CONFIG_CMDLINE)
	cli_simple_loop();
#अन्यथा
	म_लिखो("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#पूर्ण_अगर /*CONFIG_HUSH_PARSER*/
}

व्योम cli_init(व्योम)
{
#अगर_घोषित CONFIG_HUSH_PARSER
	/* This if block is used to initialize hush parser gd flag. */
	अगर (!(gd->flags & GD_FLG_HUSH_OLD_PARSER)
		&& !(gd->flags & GD_FLG_HUSH_MODERN_PARSER)) {
		अगर (CONFIG_IS_ENABLED(HUSH_OLD_PARSER))
			gd->flags |= GD_FLG_HUSH_OLD_PARSER;
		अन्यथा अगर (CONFIG_IS_ENABLED(HUSH_MODERN_PARSER))
			gd->flags |= GD_FLG_HUSH_MODERN_PARSER;
	}

	अगर (gd->flags & GD_FLG_HUSH_OLD_PARSER) {
		u_boot_hush_start();
	} अन्यथा अगर (gd->flags & GD_FLG_HUSH_MODERN_PARSER) {
		u_boot_hush_start_modern();
	} अन्यथा {
		म_लिखो("No valid hush parser to use, cli will not initialized!\n");
		वापस;
	}
#पूर्ण_अगर

#अगर defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#पूर्ण_अगर

	अगर (CONFIG_IS_ENABLED(VIDEO_ANSI))
		म_लिखो(ANSI_CURSOR_SHOW "\n");
}
