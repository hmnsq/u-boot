// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#अन्तर्भाव <autoboot.h>
#अन्तर्भाव <button.h>
#अन्तर्भाव <bootstage.h>
#अन्तर्भाव <bootstd.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <console.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <fdtdec.h>
#अन्तर्भाव <init.h>
#अन्तर्भाव <net.h>
#अन्तर्भाव <version_string.h>
#अन्तर्भाव <efi_loader.h>
#अन्तर्भाव <event.h>

स्थिर शून्य सततम्_preboot_environment_command(शून्य)
{
	वर्ण *p;

	p = env_get("preboot");
	यदि (p != रिक्त) {
		पूर्णाङ्क पूर्व = 0;

		यदि (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			पूर्व = disable_ctrlc(1); /* disable Ctrl-C checking */

		सततम्_command_list(p, -1, 0);

		यदि (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(पूर्व);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
शून्य main_loop(शून्य)
{
	ध्रुव वर्ण *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	यदि (IS_ENABLED(CONFIG_संस्करणम्_VARIABLE))
		env_समुच्चयः("ver", संस्करणम्_string);  /* set version variable */

	cli_init();

	यदि (IS_ENABLED(CONFIG_USE_PREBOOT))
		सततम्_preboot_environment_command();

	यदि (event_notify_null(EVT_POST_PREBOOT))
		प्रत्यागम;

	यदि (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, रिक्त, रिक्त);

	यदि (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		यदि (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	यदि (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	यदि (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		पूर्णाङ्क ret;

		ret = bootstd_prog_boot();
		प्रारूप_लिख("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
