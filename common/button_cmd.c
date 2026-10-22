// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#அந்தர்பாவ <button.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <stdio.h>

/* Some sane limit "just in case" */
#define MAX_BTN_CMDS 32

கட்டமைப்பு button_cmd {
	bool pressed;
	மாறிலி எழுத்து *btn_name;
	மாறிலி எழுத்து *cmd;
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
நிலையான முழு get_button_cmd(முழு n, கட்டமைப்பு button_cmd *cmd)
{
	மாறிலி எழுத்து *cmd_str;
	கட்டமைப்பு udevice *btn = வெற்று;
	எழுத்து buf[24];

	snprintf(buf, அளவு(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	எனில் (!cmd->btn_name)
		திரும்பு 0;

	button_get_by_label(cmd->btn_name, &btn);
	எனில் (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		திரும்பு 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	எனில் (!cmd->pressed)
		திரும்பு 1;

	snprintf(buf, அளவு(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	எனில் (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		திரும்பு 0;
	}

	cmd->cmd = cmd_str;

	திரும்பு 1;
}

வெறுமை process_button_cmds(வெறுமை)
{
	கட்டமைப்பு button_cmd cmd = {0};
	முழு i = 0;

	போது (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		எனில் (!cmd.pressed)
			தொடர்;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		run_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		திரும்பு;
	}
}
