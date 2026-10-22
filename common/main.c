// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define	DEBUG	*/

#समावेश <autoboot.h>
#समावेश <button.h>
#समावेश <bootstage.h>
#समावेश <bootstd.h>
#समावेश <cli.h>
#समावेश <command.h>
#समावेश <console.h>
#समावेश <env.h>
#समावेश <fdtdec.h>
#समावेश <init.h>
#समावेश <net.h>
#समावेश <version_string.h>
#समावेश <efi_loader.h>
#समावेश <event.h>

अटल व्योम run_preboot_environment_command(व्योम)
{
	अक्षर *p;

	p = env_get("preboot");
	अगर (p != शून्य) {
		पूर्णांक पूर्व = 0;

		अगर (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			पूर्व = disable_ctrlc(1); /* disable Ctrl-C checking */

		run_command_list(p, -1, 0);

		अगर (IS_ENABLED(CONFIG_AUTOBOOT_KEYED))
			disable_ctrlc(पूर्व);	/* restore Ctrl-C checking */
	}
}

/* We come here after U-Boot is initialised and ready to process commands */
व्योम main_loop(व्योम)
{
	स्थिर अक्षर *s;

	bootstage_mark_name(BOOTSTAGE_ID_MAIN_LOOP, "main_loop");

	अगर (IS_ENABLED(CONFIG_VERSION_VARIABLE))
		env_set("ver", version_string);  /* set version variable */

	cli_init();

	अगर (IS_ENABLED(CONFIG_USE_PREBOOT))
		run_preboot_environment_command();

	अगर (event_notify_null(EVT_POST_PREBOOT))
		वापस;

	अगर (IS_ENABLED(CONFIG_UPDATE_TFTP))
		update_tftp(0UL, शून्य, शून्य);

	अगर (IS_ENABLED(CONFIG_EFI_CAPSULE_ON_DISK_EARLY)) {
		/* efi_init_early() already called */
		अगर (efi_init_obj_list() == EFI_SUCCESS)
			efi_launch_capsules();
	}

	process_button_cmds();

	s = bootdelay_process();
	अगर (cli_process_fdt(&s))
		cli_secure_boot_cmd(s);

	autoboot_command(s);

	/* if standard boot if enabled, assume that it will be able to boot */
	अगर (IS_ENABLED(CONFIG_BOOTSTD_PROG)) {
		पूर्णांक ret;

		ret = bootstd_prog_boot();
		म_लिखो("Standard boot failed (err=%dE)\n", ret);
		panic("Failed to boot");
	}

	cli_loop();

	panic("No CLI available");
}
