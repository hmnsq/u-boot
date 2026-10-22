// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#شامل <autoboot.h>
#شامل <button.h>
#شامل <bootstage.h>
#شامل <bootstd.h>
#شامل <cli.h>
#شامل <command.h>
#شامل <console.h>
#شامل <env.h>
#شامل <fdtdec.h>
#شامل <init.h>
#شامل <net.h>
#شامل <version_string.h>
#شامل <efi_loader.h>
#شامل <event.h>

ساکن خالی تسلسل_preboot_environment_command(خالی)
{
	حرف *p;

	p = env_get("preboot");
	اگر (p != تہی) {
		صحیح پچھلا = 0;

		اگر (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			پچھلا = disable_ctrlc(1); /* disable Ctrl-C checking */

		تسلسل_command_list(p, -1, 0);

		اگر (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(پچھلا);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
خالی main_loop(خالی)
{
	ثابت حرف *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	اگر (IS_ENABLED(CONFIG_نسخہ_VARIABLE))
		env_سیٹ("ver", نسخہ_string);  /* set version variable */

	cli_init();

	اگر (IS_ENABLED(CONFIG_USE_PREBOOT))
		تسلسل_preboot_environment_command();

	اگر (event_notify_null(EVT_POST_PREBOOT))
		واپس;

	اگر (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, تہی, تہی);

	اگر (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		اگر (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	اگر (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	اگر (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		صحیح ret;

		ret = bootstd_prog_boot();
		لکھو("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
