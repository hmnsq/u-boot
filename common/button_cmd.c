// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#অন্তর্ভাৱ <button.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

সংরচনা button_cmd {
	bool pressed;
	ধ্রুৱ ৱর্ণ *btn_name;
	ধ্রুৱ ৱর্ণ *cmd;
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
স্থির পূর্ণাঙ্ক get_button_cmd(পূর্ণাঙ্ক n, সংরচনা button_cmd *cmd)
{
	ধ্রুৱ ৱর্ণ *cmd_str;
	সংরচনা udevice *btn = রিক্ত;
	ৱর্ণ buf[24];

	snprintf(buf, পরিমাণ(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	যদি (!cmd->btn_name)
		ঘূৰাই 0;

	button_get_by_label(cmd->btn_name, &btn);
	যদি (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		ঘূৰাই 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	যদি (!cmd->pressed)
		ঘূৰাই 1;

	snprintf(buf, পরিমাণ(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	যদি (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		ঘূৰাই 0;
	}

	cmd->cmd = cmd_str;

	ঘূৰাই 1;
}

শূন্য process_button_cmds(শূন্য)
{
	সংরচনা button_cmd cmd = {0};
	পূর্ণাঙ্ক i = 0;

	যেতিয়ালৈ (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		যদি (!cmd.pressed)
			অনুৱর্ত;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		ঘূৰাই;
	}
}
