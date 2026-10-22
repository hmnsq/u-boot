// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#অন্তর্ভাৱ <autoboot.h>
#অন্তর্ভাৱ <button.h>
#অন্তর্ভাৱ <bootstage.h>
#অন্তর্ভাৱ <bootstd.h>
#অন্তর্ভাৱ <cli.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <fdtdec.h>
#অন্তর্ভাৱ <init.h>
#অন্তর্ভাৱ <net.h>
#অন্তর্ভাৱ <version_string.h>
#অন্তর্ভাৱ <efi_loader.h>
#অন্তর্ভাৱ <event.h>

স্থির শূন্য run_preboot_environment_command(শূন্য)
{
	ৱর্ণ *p;

	p = env_get("preboot");
	যদি (p != রিক্ত) {
		পূর্ণাঙ্ক prev = 0;

		যদি (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		যদি (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
শূন্য main_loop(শূন্য)
{
	ধ্রুৱ ৱর্ণ *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	যদি (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	যদি (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	যদি (event_notify_null(EVT_POST_PREBOOT))
		ঘূৰাই;

	যদি (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, রিক্ত, রিক্ত);

	যদি (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		যদি (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	যদি (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	যদি (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		পূর্ণাঙ্ক ret;

		ret = bootstd_prog_boot();
		printf("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
