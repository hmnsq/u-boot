// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#અન્તર્ભાવ <autoboot.h>
#અન્તર્ભાવ <button.h>
#અન્તર્ભાવ <bootstage.h>
#અન્તર્ભાવ <bootstd.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <fdtdec.h>
#અન્તર્ભાવ <init.h>
#અન્તર્ભાવ <net.h>
#અન્તર્ભાવ <version_string.h>
#અન્તર્ભાવ <efi_loader.h>
#અન્તર્ભાવ <event.h>

સ્થિર શૂન્ય run_preboot_environment_command(શૂન્ય)
{
	અક્ષર *p;

	p = env_get("preboot");
	જો (p != ખાલી) {
		પૂર્ણાંક prev = 0;

		જો (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		જો (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
શૂન્ય main_loop(શૂન્ય)
{
	અચલ અક્ષર *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	જો (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	જો (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	જો (event_notify_null(EVT_POST_PREBOOT))
		પાછા;

	જો (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, ખાલી, ખાલી);

	જો (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		જો (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	જો (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	જો (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		પૂર્ણાંક ret;

		ret = bootstd_prog_boot();
		printf("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
