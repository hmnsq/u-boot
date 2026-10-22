// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#અન્તર્ભાવ <button.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

સંરચના button_cmd {
	bool pressed;
	અચલ અક્ષર *btn_name;
	અચલ અક્ષર *cmd;
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
સ્થિર પૂર્ણાંક get_button_cmd(પૂર્ણાંક n, સંરચના button_cmd *cmd)
{
	અચલ અક્ષર *cmd_str;
	સંરચના udevice *btn = ખાલી;
	અક્ષર buf[24];

	snprintf(buf, કદ(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	જો (!cmd->btn_name)
		પાછા 0;

	button_get_by_label(cmd->btn_name, &btn);
	જો (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		પાછા 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	જો (!cmd->pressed)
		પાછા 1;

	snprintf(buf, કદ(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	જો (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		પાછા 0;
	}

	cmd->cmd = cmd_str;

	પાછા 1;
}

શૂન્ય process_button_cmds(શૂન્ય)
{
	સંરચના button_cmd cmd = {0};
	પૂર્ણાંક i = 0;

	જ્યારે (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		જો (!cmd.pressed)
			ચાલુ;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		પાછા;
	}
}
