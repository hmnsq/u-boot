// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#ਅਨ੍ਤਰ੍ਭਾਵ <autoboot.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <button.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootstage.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootstd.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <version_string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <efi_loader.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <event.h>

ਸਥਿਰ ਖਾਲੀ ਲੜੀਵਾਰ_preboot_environment_command(ਖਾਲੀ)
{
	ਅੱਖਰ *p;

	p = env_get("preboot");
	ਜੇ (p != ਨਿਰਾ) {
		ਪੂਰਨ prev = 0;

		ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		ਲੜੀਵਾਰ_command_list(p, -1, 0);

		ਜੇ (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
ਖਾਲੀ main_loop(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	ਜੇ (IS_ENABLED(CONFIG_ਸੰਸਕਰਣ_VARIABLE))
		env_ਸੈੱਟ("ver", ਸੰਸਕਰਣ_string);  /* set version variable */

	cli_init();

	ਜੇ (IS_ENABLED(CONFIG_USE_PREBOOT))
		ਲੜੀਵਾਰ_preboot_environment_command();

	ਜੇ (event_notify_null(EVT_POST_PREBOOT))
		ਵਾਪਸ;

	ਜੇ (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, ਨਿਰਾ, ਨਿਰਾ);

	ਜੇ (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		ਜੇ (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	ਜੇ (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	ਜੇ (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		ਪੂਰਨ ret;

		ret = bootstd_prog_boot();
		ਲਿਖੋ("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
