// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#समावेश <button.h>
#समावेश <command.h>
#समावेश <env.h>
#समावेश <log.h>
#समावेश <stdio.h>

/* Some sane limit "just in case" */
#घोषणा MAX_BTN_CMDS 32

काष्ठा button_cmd {
	bool pressed;
	स्थिर अक्षर *btn_name;
	स्थिर अक्षर *cmd;
};

/*
 * Button commands are set via environment variables, e.g.:
 * button_cmd_N_name=Volume Up
 * button_cmd_N=fastboot usb 0
 *
 * This function will retrieve the command for the given button N
 * and populate the cmd struct with the command string and pressed
 * state of the button.
 *
 * Returns 1 if a command was found, 0 otherwise.
 */
अटल पूर्णांक get_button_cmd(पूर्णांक n, काष्ठा button_cmd *cmd)
{
	स्थिर अक्षर *cmd_str;
	काष्ठा udevice *btn = शून्य;
	अक्षर buf[24];

	snprintf(buf, माप(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	अगर (!cmd->btn_name)
		वापस 0;

	button_get_by_label(cmd->btn_name, &btn);
	अगर (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		वापस 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	अगर (!cmd->pressed)
		वापस 1;

	snprintf(buf, माप(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	अगर (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		वापस 0;
	}

	cmd->cmd = cmd_str;

	वापस 1;
}

व्योम process_button_cmds(व्योम)
{
	काष्ठा button_cmd cmd = {0};
	पूर्णांक i = 0;

	जबतक (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		अगर (!cmd.pressed)
			जारी;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		वापस;
	}
}
