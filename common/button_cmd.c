// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#অন্তর্ভাব <button.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

কাঠামো button_cmd {
	bool pressed;
	ধ্রুবক অক্ষর *btn_name;
	ধ্রুবক অক্ষর *cmd;
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
স্থির পূর্ণসংখ্যা get_button_cmd(পূর্ণসংখ্যা n, কাঠামো button_cmd *cmd)
{
	ধ্রুবক অক্ষর *cmd_str;
	কাঠামো udevice *btn = ফাঁকা;
	অক্ষর buf[24];

	snprintf(buf, আকার(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	যদি (!cmd->btn_name)
		ফেরত 0;

	button_get_by_label(cmd->btn_name, &btn);
	যদি (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		ফেরত 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	যদি (!cmd->pressed)
		ফেরত 1;

	snprintf(buf, আকার(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	যদি (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		ফেরত 0;
	}

	cmd->cmd = cmd_str;

	ফেরত 1;
}

শূন্য process_button_cmds(শূন্য)
{
	কাঠামো button_cmd cmd = {0};
	পূর্ণসংখ্যা i = 0;

	যখন (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		যদি (!cmd.pressed)
			চালু;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		ফেরত;
	}
}
