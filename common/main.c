// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#అన్తర్భావ <autoboot.h>
#అన్తర్భావ <button.h>
#అన్తర్భావ <bootstage.h>
#అన్తర్భావ <bootstd.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <init.h>
#అన్తర్భావ <net.h>
#అన్తర్భావ <version_string.h>
#అన్తర్భావ <efi_loader.h>
#అన్తర్భావ <event.h>

స్థిర శూన్య నిరంతరం_preboot_environment_command(శూన్య)
{
	అక్షరం *p;

	p = env_get("preboot");
	ఒకవేళ (p != ఖాళీ) {
		పూర్ణాంకం prev = 0;

		ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		నిరంతరం_command_list(p, -1, 0);

		ఒకవేళ (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
శూన్య main_loop(శూన్య)
{
	స్థిరరాశి అక్షరం *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	ఒకవేళ (IS_ENABLED(CONFIG_సంస్కరణ_VARIABLE))
		env_సమితి("ver", సంస్కరణ_string);  /* set version variable */

	cli_init();

	ఒకవేళ (IS_ENABLED(CONFIG_USE_PREBOOT))
		నిరంతరం_preboot_environment_command();

	ఒకవేళ (event_notify_null(EVT_POST_PREBOOT))
		తిరిగి;

	ఒకవేళ (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, ఖాళీ, ఖాళీ);

	ఒకవేళ (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		ఒకవేళ (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	ఒకవేళ (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	ఒకవేళ (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		పూర్ణాంకం ret;

		ret = bootstd_prog_boot();
		రాయి("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
