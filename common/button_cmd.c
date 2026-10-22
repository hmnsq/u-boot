// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#شامل <button.h>
#شامل <command.h>
#شامل <env.h>
#شامل <log.h>
#شامل <stdio.h>

/* Some sane limit "just in case" */
#تعریف_کرو MAX_BTN_CMDS 32

ساخت button_cmd {
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
ساکن صحیح get_button_cmd(صحیح n, ساخت button_cmd *cmd)
{
	ثابت حرف *cmd_str;
	ساخت udevice *btn = تہی;
	حرف buf[24];

	snprintf(buf, حجم(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	اگر (!cmd->btn_name)
		واپس 0;

	button_get_by_label(cmd->btn_name, &btn);
	اگر (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		واپس 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	اگر (!cmd->pressed)
		واپس 1;

	snprintf(buf, حجم(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	اگر (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		واپس 0;
	}

	cmd->cmd = cmd_str;

	واپس 1;
}

خالی process_button_cmds(خالی)
{
	ساخت button_cmd cmd = {0};
	صحیح i = 0;

	جب (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		اگر (!cmd.pressed)
			جاری;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		تسلسل_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		واپس;
	}
}
