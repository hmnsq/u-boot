// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#அந்தர்பாவ <autoboot.h>
#அந்தர்பாவ <button.h>
#அந்தர்பாவ <bootstage.h>
#அந்தர்பாவ <bootstd.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <fdtdec.h>
#அந்தர்பாவ <init.h>
#அந்தர்பாவ <net.h>
#அந்தர்பாவ <version_string.h>
#அந்தர்பாவ <efi_loader.h>
#அந்தர்பாவ <event.h>

நிலையான வெறுமை run_preboot_environment_command(வெறுமை)
{
	எழுத்து *p;

	p = env_get("preboot");
	எனில் (p != வெற்று) {
		முழு prev = 0;

		எனில் (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		எனில் (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
வெறுமை main_loop(வெறுமை)
{
	மாறிலி எழுத்து *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	எனில் (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	எனில் (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	எனில் (event_notify_null(EVT_POST_PREBOOT))
		திரும்பு;

	எனில் (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, வெற்று, வெற்று);

	எனில் (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		எனில் (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	எனில் (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	எனில் (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		முழு ret;

		ret = bootstd_prog_boot();
		printf("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
