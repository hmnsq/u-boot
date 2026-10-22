// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#ଅନ୍ତର୍ଭାଵ <button.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

ସଂରଚନା button_cmd {
	bool pressed;
	ଅଚଳ ଅକ୍ଷର *btn_name;
	ଅଚଳ ଅକ୍ଷର *cmd;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_button_cmd(ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ସଂରଚନା button_cmd *cmd)
{
	ଅଚଳ ଅକ୍ଷର *cmd_str;
	ସଂରଚନା udevice *btn = ଖାଲି;
	ଅକ୍ଷର buf[24];

	snprintf(buf, ଆକାର(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	ଯଦି (!cmd->btn_name)
		ଫେରନ୍ତୁ 0;

	button_get_by_label(cmd->btn_name, &btn);
	ଯଦି (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		ଫେରନ୍ତୁ 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	ଯଦି (!cmd->pressed)
		ଫେରନ୍ତୁ 1;

	snprintf(buf, ଆକାର(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	ଯଦି (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		ଫେରନ୍ତୁ 0;
	}

	cmd->cmd = cmd_str;

	ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ process_button_cmds(ଶୂନ୍ୟ)
{
	ସଂରଚନା button_cmd cmd = {0};
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;

	ଯେତେବେଳେ (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		ଯଦି (!cmd.pressed)
			ଜାରି;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		ଫେରନ୍ତୁ;
	}
}
