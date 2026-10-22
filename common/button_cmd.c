// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#include <button.h>
#include <command.h>
#include <env.h>
#include <log.h>
#include <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

sa_nracanaa button_cmd {
	bool pressed;
	const var_na *btn_name;
	const var_na *cmd;
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
static puur_naa_nka get_button_cmd(puur_naa_nka n, sa_nracanaa button_cmd *cmd)
{
	const var_na *cmd_str;
	sa_nracanaa udevice *btn = NULL;
	var_na buf[24];

	snprintf(buf, parimaa_na(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	ᰀᰧ (!cmd->btn_name)
		ᰀᰧᰌ 0;

	button_get_by_label(cmd->btn_name, &btn);
	ᰀᰧ (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		ᰀᰧᰌ 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	ᰀᰧ (!cmd->pressed)
		ᰀᰧᰌ 1;

	snprintf(buf, parimaa_na(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	ᰀᰧ (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		ᰀᰧᰌ 0;
	}

	cmd->cmd = cmd_str;

	ᰀᰧᰌ 1;
}

void process_button_cmds(void)
{
	sa_nracanaa button_cmd cmd = {0};
	puur_naa_nka i = 0;

	ᰀᰬ (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		ᰀᰧ (!cmd.pressed)
			continue;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		ᰀᰧᰌ;
	}
}
