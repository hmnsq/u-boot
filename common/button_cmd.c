// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#అన్తర్భావ <button.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <stdio.h>

/* Some sane limit "just in case" */
#నిర్వచించు MAX_BTN_CMDS 32

నిర్మాణం button_cmd {
	bool pressed;
	స్థిరరాశి అక్షరం *btn_name;
	స్థిరరాశి అక్షరం *cmd;
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
స్థిర పూర్ణాంకం get_button_cmd(పూర్ణాంకం n, నిర్మాణం button_cmd *cmd)
{
	స్థిరరాశి అక్షరం *cmd_str;
	నిర్మాణం udevice *btn = ఖాళీ;
	అక్షరం buf[24];

	snprintf(buf, పరిమాణం(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	ఒకవేళ (!cmd->btn_name)
		తిరిగి 0;

	button_get_by_label(cmd->btn_name, &btn);
	ఒకవేళ (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		తిరిగి 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	ఒకవేళ (!cmd->pressed)
		తిరిగి 1;

	snprintf(buf, పరిమాణం(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	ఒకవేళ (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		తిరిగి 0;
	}

	cmd->cmd = cmd_str;

	తిరిగి 1;
}

శూన్య process_button_cmds(శూన్య)
{
	నిర్మాణం button_cmd cmd = {0};
	పూర్ణాంకం i = 0;

	అయితే (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		ఒకవేళ (!cmd.pressed)
			కొనసాగు;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		నిరంతరం_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		తిరిగి;
	}
}
