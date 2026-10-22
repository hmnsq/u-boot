// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#ಅನ್ತರ್ಭಾವ <autoboot.h>
#ಅನ್ತರ್ಭಾವ <button.h>
#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <bootstd.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <net.h>
#ಅನ್ತರ್ಭಾವ <version_string.h>
#ಅನ್ತರ್ಭಾವ <efi_loader.h>
#ಅನ್ತರ್ಭಾವ <event.h>

ಸ್ಥಿರ ಶೂನ್ಯ run_preboot_environment_command(ಶೂನ್ಯ)
{
	ವರ್ಣ *p;

	p = env_get("preboot");
	ಯದಿ (p != ರಿಕ್ತ) {
		ಪೂರ್ಣಾಙ್ಕ prev = 0;

		ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		ಯದಿ (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
ಶೂನ್ಯ main_loop(ಶೂನ್ಯ)
{
	ಧ್ರುವ ವರ್ಣ *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	ಯದಿ (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	ಯದಿ (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	ಯದಿ (event_notify_null(EVT_POST_PREBOOT))
		ಪ್ರತ್ಯಾಗಮ;

	ಯದಿ (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, ರಿಕ್ತ, ರಿಕ್ತ);

	ಯದಿ (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		ಯದಿ (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	ಯದಿ (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	ಯದಿ (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = bootstd_prog_boot();
		printf("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
