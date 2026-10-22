// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#ଅନ୍ତର୍ଭାଵ <autoboot.h>
#ଅନ୍ତର୍ଭାଵ <button.h>
#ଅନ୍ତର୍ଭାଵ <bootstage.h>
#ଅନ୍ତର୍ଭାଵ <bootstd.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <fdtdec.h>
#ଅନ୍ତର୍ଭାଵ <init.h>
#ଅନ୍ତର୍ଭାଵ <net.h>
#ଅନ୍ତର୍ଭାଵ <version_string.h>
#ଅନ୍ତର୍ଭାଵ <efi_loader.h>
#ଅନ୍ତର୍ଭାଵ <event.h>

ସ୍ଥିର ଶୂନ୍ୟ run_preboot_environment_command(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *p;

	p = env_get("preboot");
	ଯଦି (p != ଖାଲି) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev = 0;

		ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		ଯଦି (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
ଶୂନ୍ୟ main_loop(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	ଯଦି (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	ଯଦି (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	ଯଦି (event_notify_null(EVT_POST_PREBOOT))
		ଫେରନ୍ତୁ;

	ଯଦି (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, ଖାଲି, ଖାଲି);

	ଯଦି (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		ଯଦି (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	ଯଦି (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	ଯଦି (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		ret = bootstd_prog_boot();
		printf("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
