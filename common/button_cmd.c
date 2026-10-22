// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#അന്തര്ഭാവ <button.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

ഘടന button_cmd {
	bool pressed;
	സ്ഥിരം അക്ഷരം *btn_name;
	സ്ഥിരം അക്ഷരം *cmd;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ get_button_cmd(പൂർണ്ണസംഖ്യ n, ഘടന button_cmd *cmd)
{
	സ്ഥിരം അക്ഷരം *cmd_str;
	ഘടന udevice *btn = ശൂന്യ;
	അക്ഷരം buf[24];

	snprintf(buf, വലുപ്പം(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	എങ്കിൽ (!cmd->btn_name)
		തിരികെ 0;

	button_get_by_label(cmd->btn_name, &btn);
	എങ്കിൽ (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		തിരികെ 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	എങ്കിൽ (!cmd->pressed)
		തിരികെ 1;

	snprintf(buf, വലുപ്പം(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	എങ്കിൽ (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		തിരികെ 0;
	}

	cmd->cmd = cmd_str;

	തിരികെ 1;
}

ശൂന്യം process_button_cmds(ശൂന്യം)
{
	ഘടന button_cmd cmd = {0};
	പൂർണ്ണസംഖ്യ i = 0;

	ആകുമ്പോൾ (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		എങ്കിൽ (!cmd.pressed)
			തുടരുക;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		തിരികെ;
	}
}
