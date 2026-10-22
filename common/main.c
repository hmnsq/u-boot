// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#ضمن <autoboot.h>
#ضمن <button.h>
#ضمن <bootstage.h>
#ضمن <bootstd.h>
#ضمن <cli.h>
#ضمن <command.h>
#ضمن <console.h>
#ضمن <env.h>
#ضمن <fdtdec.h>
#ضمن <init.h>
#ضمن <net.h>
#ضمن <version_string.h>
#ضمن <efi_loader.h>
#ضمن <event.h>

ساكن فراغ run_preboot_environment_command(فراغ)
{
	حرف *p;

	p = env_get("preboot");
	𐡄𐡍 (p != عدم) {
		صحيح prev = 0;

		𐡄𐡍 (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			prev = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		𐡄𐡍 (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(prev);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
فراغ main_loop(فراغ)
{
	ثابت حرف *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	𐡄𐡍 (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	𐡄𐡍 (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	𐡄𐡍 (event_notify_null(EVT_POST_PREBOOT))
		أرجع;

	𐡄𐡍 (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, عدم, عدم);

	𐡄𐡍 (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		𐡄𐡍 (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	𐡄𐡍 (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	𐡄𐡍 (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		صحيح ret;

		ret = bootstd_prog_boot();
		printf("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
