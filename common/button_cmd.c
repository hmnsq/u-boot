// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#ضمن <button.h>
#ضمن <command.h>
#ضمن <env.h>
#ضمن <log.h>
#ضمن <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

بنية button_cmd {
	bool pressed;
	ثابت حرف *btn_name;
	ثابت حرف *cmd;
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
ساكن صحيح get_button_cmd(صحيح n, بنية button_cmd *cmd)
{
	ثابت حرف *cmd_str;
	بنية udevice *btn = عدم;
	حرف buf[24];

	snprintf(buf, حجم(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	𐤀𐤌 (!cmd->btn_name)
		أرجع 0;

	button_get_by_label(cmd->btn_name, &btn);
	𐤀𐤌 (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		أرجع 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	𐤀𐤌 (!cmd->pressed)
		أرجع 1;

	snprintf(buf, حجم(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	𐤀𐤌 (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		أرجع 0;
	}

	cmd->cmd = cmd_str;

	أرجع 1;
}

فراغ process_button_cmds(فراغ)
{
	بنية button_cmd cmd = {0};
	صحيح i = 0;

	طالما (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		𐤀𐤌 (!cmd.pressed)
			استمر;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		أرجع;
	}
}
