// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2023 Linaro Ltd.
 *   Author: Casey Connolly <casey.connolly@linaro.org>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <button.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>

/* Some sane limit "just in case" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_BTN_CMDS 32

ਬਣਤਰ button_cmd {
	bool pressed;
	ਅਚਲ ਅੱਖਰ *btn_name;
	ਅਚਲ ਅੱਖਰ *cmd;
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
ਸਥਿਰ ਪੂਰਨ get_button_cmd(ਪੂਰਨ n, ਬਣਤਰ button_cmd *cmd)
{
	ਅਚਲ ਅੱਖਰ *cmd_str;
	ਬਣਤਰ udevice *btn = ਨਿਰਾ;
	ਅੱਖਰ buf[24];

	snprintf(buf, ਆਕਾਰ(buf), "button_cmd_%d_name", n);
	cmd->btn_name = env_get(buf);
	ਜੇ (!cmd->btn_name)
		ਵਾਪਸ 0;

	button_get_by_label(cmd->btn_name, &btn);
	ਜੇ (!btn) {
		log_err("No button labelled '%s'\n", cmd->btn_name);
		ਵਾਪਸ 0;
	}

	cmd->pressed = button_get_state(btn) == BUTTON_ON;
	/* If the button isn't pressed then cmd->cmd will be unused so don't waste
	 * cycles reading it
	 */
	ਜੇ (!cmd->pressed)
		ਵਾਪਸ 1;

	snprintf(buf, ਆਕਾਰ(buf), "button_cmd_%d", n);
	cmd_str = env_get(buf);
	ਜੇ (!cmd_str) {
		log_err("No command set for button '%s'\n", cmd->btn_name);
		ਵਾਪਸ 0;
	}

	cmd->cmd = cmd_str;

	ਵਾਪਸ 1;
}

ਖਾਲੀ process_button_cmds(ਖਾਲੀ)
{
	ਬਣਤਰ button_cmd cmd = {0};
	ਪੂਰਨ i = 0;

	ਜਦੋਂ (get_button_cmd(i++, &cmd) && i < MAX_BTN_CMDS) {
		ਜੇ (!cmd.pressed)
			ਜਾਰੀ;

		log_info("BTN '%s'> %s\n", cmd.btn_name, cmd.cmd);
		ਲੜੀਵਾਰ_command(cmd.cmd, CMD_FLAG_ENV);
		/* Don't run commands for multiple buttons */
		ਵਾਪਸ;
	}
}
