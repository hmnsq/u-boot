// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2011 The Chromium OS Authors.
 * (C) Copyright 2002-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 */

#समावेश <config.h>
#समावेश <api.h>
#समावेश <bootstage.h>
#समावेश <cpu_func.h>
#समावेश <cyclic.h>
#समावेश <display_options.h>
#समावेश <exports.h>
#अगर_घोषित CONFIG_MTD_NOR_FLASH
#समावेश <flash.h>
#पूर्ण_अगर
#समावेश <hang.h>
#समावेश <image.h>
#समावेश <irq_func.h>
#समावेश <lmb.h>
#समावेश <log.h>
#समावेश <net.h>
#समावेश <asm/cache.h>
#समावेश <asm/global_data.h>
#समावेश <u-boot/crc.h>
#समावेश <binman.h>
#समावेश <command.h>
#समावेश <console.h>
#समावेश <dm.h>
#समावेश <efi_loader.h>
#समावेश <env.h>
#समावेश <env_internal.h>
#समावेश <fdtdec.h>
#समावेश <init.h>
#समावेश <initcall.h>
#समावेश <kgdb.h>
#समावेश <irq_func.h>
#समावेश <led.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <miiphy.h>
#समावेश <mmc.h>
#समावेश <mux.h>
#समावेश <nand.h>
#समावेश <of_live.h>
#समावेश <onenand_uboot.h>
#समावेश <pvblock.h>
#समावेश <scsi.h>
#समावेश <serial.h>
#समावेश <stdio_dev.h>
#समावेश <timer.h>
#समावेश <trace.h>
#समावेश <watchdog.h>
#समावेश <xen.h>
#समावेश <asm/sections.h>
#समावेश <dm/root.h>
#समावेश <dm/ofnode.h>
#समावेश <linux/compiler.h>
#समावेश <linux/err.h>
#समावेश <wdt.h>
#समावेश <asm-generic/gpio.h>
#समावेश <relocate.h>

DECLARE_GLOBAL_तथ्य_PTR;

ulong monitor_flash_len;

__weak पूर्णांक board_flash_wp_on(व्योम)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	वापस 0;
}

__weak पूर्णांक cpu_secondary_init_r(व्योम)
{
	वापस 0;
}

अटल पूर्णांक initr_trace(व्योम)
{
#अगर_घोषित CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_बफर_आकार);
#पूर्ण_अगर

	वापस 0;
}

अटल पूर्णांक initr_reloc(व्योम)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	वापस 0;
}

#अगर defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
अटल पूर्णांक initr_caches(व्योम)
{
	/* Enable caches */
	enable_caches();
	वापस 0;
}
#पूर्ण_अगर

__weak पूर्णांक fixup_cpu(व्योम)
{
	वापस 0;
}

अटल पूर्णांक initr_reloc_global_तथ्य(व्योम)
{
#अगर_घोषित __ARM__
	monitor_flash_len = _end - __image_copy_start;
#या_अगर defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#या_अगर !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#पूर्ण_अगर
#अगर defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_BASE;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#पूर्ण_अगर
#अगर_घोषित CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#पूर्ण_अगर

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	अगर (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#अगर_घोषित CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	अगर (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, शून्य);

#पूर्ण_अगर
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	अगर (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_set_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (माप_प्रकार)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_set_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (माप_प्रकार)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_set_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (माप_प्रकार)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	वापस 0;
}

__weak पूर्णांक arch_initr_trap(व्योम)
{
	वापस 0;
}

#अगर defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
अटल पूर्णांक initr_unlock_ram_in_cache(व्योम)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक initr_barrier(व्योम)
{
#अगर_घोषित CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	यंत्र("sync ; isync");
#पूर्ण_अगर
	वापस 0;
}

अटल पूर्णांक initr_malloc(व्योम)
{
	ulong start;

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#पूर्ण_अगर
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	start = gd->relocaddr - योग_MALLOC_LEN;
	gd_set_malloc_start(start);
	mem_malloc_init(start, योग_MALLOC_LEN);
	वापस 0;
}

अटल पूर्णांक initr_of_live(व्योम)
{
	अगर (CONFIG_IS_ENABLED(OF_LIVE)) {
		पूर्णांक ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (काष्ठा device_गाँठ **)gd_of_मूल_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		अगर (ret)
			वापस ret;
	}

	वापस 0;
}

#अगर_घोषित CONFIG_DM
अटल पूर्णांक initr_dm(व्योम)
{
	पूर्णांक ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_मूल = शून्य;
#अगर_घोषित CONFIG_TIMER
	gd->timer = शून्य;
#पूर्ण_अगर
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	अगर (ret)
		वापस ret;

	वापस dm_autoprobe();
}
#पूर्ण_अगर

अटल पूर्णांक initr_dm_devices(व्योम)
{
	पूर्णांक ret;

	अगर (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		अगर (ret)
			वापस ret;
	}

	अगर (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		अगर (ret)
			वापस ret;
	}

	वापस 0;
}

अटल पूर्णांक initr_bootstage(व्योम)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	वापस 0;
}

__weak पूर्णांक power_init_board(व्योम)
{
	वापस 0;
}

अटल पूर्णांक initr_announce(व्योम)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	वापस 0;
}

अटल पूर्णांक __maybe_unused initr_binman(व्योम)
{
	पूर्णांक ret;

	ret = binman_init();
	अगर (ret)
		म_लिखो("binman_init failed:%d\n", ret);

	वापस ret;
}

#अगर defined(CONFIG_MTD_NOR_FLASH)
__weak पूर्णांक is_flash_available(व्योम)
{
	वापस 1;
}

अटल पूर्णांक initr_flash(व्योम)
{
	ulong flash_आकार = 0;
	काष्ठा bd_info *bd = gd->bd;

	अगर (!is_flash_available())
		वापस 0;

	माला_दो("Flash: ");

	अगर (board_flash_wp_on())
		म_लिखो("Uninitialized - Write Protect On\n");
	अन्यथा
		flash_आकार = flash_init();

	print_आकार(flash_आकार, "");
#अगर_घोषित CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	अगर (env_get_yesno("flashchecksum") == 1) {
		स्थिर uchar *flash_base = (स्थिर uchar *)CFG_SYS_FLASH_BASE;

		म_लिखो("  CRC: %08X", crc32(0,
					    flash_base,
					    flash_आकार));
	}
#पूर्ण_अगर /* CONFIG_SYS_FLASH_CHECKSUM */
	अ_दो('\n');

	/* update start of FLASH memory    */
#अगर_घोषित CFG_SYS_FLASH_BASE
	bd->bi_flashstart = CFG_SYS_FLASH_BASE;
#पूर्ण_अगर
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_आकार;

#अगर defined(CONFIG_SYS_UPDATE_FLASH_आकार)
	/* Make a update of the Memctrl. */
	update_flash_आकार(flash_आकार);
#पूर्ण_अगर

#अगर defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_आकार;
#या_अगर CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#पूर्ण_अगर
	वापस 0;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_CMD_NAND
/* go init the NAND */
अटल पूर्णांक initr_nand(व्योम)
{
	माला_दो("NAND:  ");
	nand_init();
	म_लिखो("%lu MiB\n", nand_आकार() / 1024);
	वापस 0;
}
#पूर्ण_अगर

#अगर defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
अटल पूर्णांक initr_onenand(व्योम)
{
	माला_दो("NAND:  ");
	onenand_init();
	वापस 0;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_MMC
अटल पूर्णांक initr_mmc(व्योम)
{
	माला_दो("MMC:   ");
	mmc_initialize(gd->bd);
	वापस 0;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_PVBLOCK
अटल पूर्णांक initr_pvblock(व्योम)
{
	माला_दो("PVBLOCK: ");
	pvblock_init();
	वापस 0;
}
#पूर्ण_अगर

/*
 * Tell if it's OK to load the environment early in boot.
 *
 * If CONFIG_OF_CONTROL is defined, we'll check with the FDT to see
 * if this is OK (defaulting to saying it's OK).
 *
 * NOTE: Loading the environment early can be a bad idea if security is
 *       important, since no verification is done on the environment.
 *
 * Return: 0 if environment should not be loaded, !=0 if it is ok to load
 */
अटल पूर्णांक should_load_env(व्योम)
{
	अगर (IS_ENABLED(CONFIG_OF_CONTROL))
		वापस ofnode_conf_read_int("load-environment", 1);

	वापस 1;
}

अटल पूर्णांक initr_env(व्योम)
{
	/* initialize environment */
	अगर (should_load_env())
		env_relocate();
	अन्यथा
		env_set_default(शून्य, 0);

	env_import_fdt();

	अगर (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (अचिन्हित दीर्घ)map_to_sysmem(gd->fdt_blob));

	#अगर (IS_ENABLED(CONFIG_SAVE_पूर्व_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_पूर्व_BL_FDT_ADDR))
		save_पूर्व_bl_तथ्य();
	#पूर्ण_अगर

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	वापस 0;
}

#अगर_घोषित CONFIG_SYS_MALLOC_BOOTPARAMS
अटल पूर्णांक initr_malloc_bootparams(व्योम)
{
	gd->bd->bi_boot_params = (ulong)दो_स्मृति(CONFIG_SYS_BOOTPARAMS_LEN);
	अगर (!gd->bd->bi_boot_params) {
		माला_दो("WARNING: Cannot allocate space for boot parameters\n");
		वापस -ENOMEM;
	}
	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक initr_boot_led_blink(व्योम)
{
	led_boot_blink();

	वापस 0;
}

अटल पूर्णांक initr_boot_led_on(व्योम)
{
	led_boot_on();

	वापस 0;
}

#अगर CONFIG_IS_ENABLED(NET)
अटल पूर्णांक initr_net(व्योम)
{
	माला_दो("Net:   ");
	eth_initialize();
#अगर defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#पूर्ण_अगर
	वापस 0;
}
#पूर्ण_अगर

#अगर_घोषित CONFIG_POST
अटल पूर्णांक initr_post(व्योम)
{
	post_run(शून्य, POST_RAM | post_bootmode_get(0));
	वापस 0;
}
#पूर्ण_अगर

#अगर defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
पूर्णांक initr_mem(व्योम)
{
	ulong pram = 0;
	अक्षर memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	प्र_लिखो(memsz, "%ldk", (दीर्घ पूर्णांक)((gd->ram_आकार / 1024) - pram));
	env_set("mem", memsz);

	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक initr_lmb(व्योम)
{
	अगर (CONFIG_IS_ENABLED(LMB))
		वापस lmb_init();
	अन्यथा
		वापस 0;
}

अटल पूर्णांक dm_announce(व्योम)
{
	पूर्णांक device_गिनती;
	पूर्णांक uclass_गिनती;

	अगर (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_गिनती, &uclass_गिनती);
		म_लिखो("Core:  %d devices, %d uclasses", device_गिनती,
		       uclass_गिनती);
		अगर (CONFIG_IS_ENABLED(OF_REAL))
			म_लिखो(", devicetree: %s", fdtdec_get_srcname());
		अगर (CONFIG_IS_ENABLED(UPL))
			म_लिखो(", universal payload active");
		म_लिखो("\n");
		अगर (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			म_लिखो("Warning: Unexpected devicetree source (not from a prior stage)");
			म_लिखो("Warning: U-Boot may not function properly\n");
		}
	}

	वापस 0;
}

अटल पूर्णांक run_main_loop(व्योम)
{
	पूर्णांक ret;

#अगर_घोषित CONFIG_SANDBOX
	sandbox_main_loop_init();
#पूर्ण_अगर

	ret = event_notify_null(EVT_MAIN_LOOP);
	अगर (ret)
		वापस ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	क्रम (;;)
		main_loop();
	वापस 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

अटल व्योम initcall_run_r(व्योम)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#अगर CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#पूर्ण_अगर
	INITCALL(initr_reloc_global_तथ्य);
#अगर CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#पूर्ण_अगर
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#अगर CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#पूर्ण_अगर
	INITCALL(initr_of_live);
#अगर CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#पूर्ण_अगर
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#अगर CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#पूर्ण_अगर
	INITCALL(initr_lmb);
#अगर CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#पूर्ण_अगर
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#अगर CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#पूर्ण_अगर
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#अगर CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#पूर्ण_अगर
	WATCHDOG_RESET();
#अगर CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#पूर्ण_अगर
	WATCHDOG_RESET();
#अगर CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#पूर्ण_अगर
	INITCALL(power_init_board);
#अगर CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#पूर्ण_अगर
	WATCHDOG_RESET();
#अगर IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#पूर्ण_अगर
	INITCALL(initr_env);
#अगर CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#पूर्ण_अगर
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#अगर CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#पूर्ण_अगर
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#अगर CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#पूर्ण_अगर
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#अगर CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#पूर्ण_अगर
	INITCALL(console_init_r);	/* fully init console as a device */
#अगर CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#पूर्ण_अगर
	/* miscellaneous arch-dependent init */
#अगर CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#पूर्ण_अगर
	/* miscellaneous platform-dependent init */
#अगर CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#पूर्ण_अगर
	WATCHDOG_RESET();
#अगर CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#पूर्ण_अगर
	INITCALL(interrupt_init);
#अगर defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#पूर्ण_अगर
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#अगर CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#पूर्ण_अगर
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#अगर defined(CFG_PRAM)
	INITCALL(initr_mem);
#पूर्ण_अगर
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

व्योम board_init_r(gd_t *new_gd, ulong dest_addr)
{
	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);

	/*
	 * Set up the new global data pointer. So far only x86 does this
	 * here.
	 * TODO(sjg@chromium.org): Consider doing this for all archs, or
	 * dropping the new_gd parameter.
	 */
	अगर (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#अगर defined(CONFIG_RISCV)
	set_gd(new_gd);
#या_अगर !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#पूर्ण_अगर
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
