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

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <api.h>
#அந்தர்பாவ <bootstage.h>
#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <cyclic.h>
#அந்தர்பாவ <display_options.h>
#அந்தர்பாவ <exports.h>
#ifdef CONFIG_MTD_NOR_FLASH
#அந்தர்பாவ <flash.h>
#endif
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <irq_func.h>
#அந்தர்பாவ <lmb.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <net.h>
#அந்தர்பாவ <asm/cache.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <u-boot/crc.h>
#அந்தர்பாவ <binman.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <efi_loader.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <env_internal.h>
#அந்தர்பாவ <fdtdec.h>
#அந்தர்பாவ <init.h>
#அந்தர்பாவ <initcall.h>
#அந்தர்பாவ <kgdb.h>
#அந்தர்பாவ <irq_func.h>
#அந்தர்பாவ <led.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <miiphy.h>
#அந்தர்பாவ <mmc.h>
#அந்தர்பாவ <mux.h>
#அந்தர்பாவ <nand.h>
#அந்தர்பாவ <of_live.h>
#அந்தர்பாவ <onenand_uboot.h>
#அந்தர்பாவ <pvblock.h>
#அந்தர்பாவ <scsi.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <stdio_dev.h>
#அந்தர்பாவ <timer.h>
#அந்தர்பாவ <trace.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <xen.h>
#அந்தர்பாவ <asm/sections.h>
#அந்தர்பாவ <dm/root.h>
#அந்தர்பாவ <dm/ofnode.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <linux/err.h>
#அந்தர்பாவ <wdt.h>
#அந்தர்பாவ <asm-generic/gpio.h>
#அந்தர்பாவ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak முழு board_flash_wp_on(வெறுமை)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	திரும்பு 0;
}

__weak முழு cpu_secondary_init_r(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு initr_trace(வெறுமை)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	திரும்பு 0;
}

நிலையான முழு initr_reloc(வெறுமை)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	திரும்பு 0;
}

#எனில் defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
நிலையான முழு initr_caches(வெறுமை)
{
	/* Enable caches */
	enable_caches();
	திரும்பு 0;
}
#endif

__weak முழு fixup_cpu(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு initr_reloc_global_data(வெறுமை)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#அல்லதெனில் defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#அல்லதெனில் !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#endif
#எனில் defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
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
#endif
#ifdef CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#endif

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	எனில் (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#ifdef CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	எனில் (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, வெற்று);

#endif
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	எனில் (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_set_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_set_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_set_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	திரும்பு 0;
}

__weak முழு arch_initr_trap(வெறுமை)
{
	திரும்பு 0;
}

#எனில் defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
நிலையான முழு initr_unlock_ram_in_cache(வெறுமை)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	திரும்பு 0;
}
#endif

நிலையான முழு initr_barrier(வெறுமை)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	திரும்பு 0;
}

நிலையான முழு initr_malloc(வெறுமை)
{
	ulong start;

#எனில் CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#endif
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	start = gd->relocaddr - TOTAL_MALLOC_LEN;
	gd_set_malloc_start(start);
	mem_malloc_init(start, TOTAL_MALLOC_LEN);
	திரும்பு 0;
}

நிலையான முழு initr_of_live(வெறுமை)
{
	எனில் (CONFIG_IS_ENABLED(OF_LIVE)) {
		முழு ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (கட்டமைப்பு device_முனை **)gd_of_வேர்_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		எனில் (ret)
			திரும்பு ret;
	}

	திரும்பு 0;
}

#ifdef CONFIG_DM
நிலையான முழு initr_dm(வெறுமை)
{
	முழு ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_வேர் = வெற்று;
#ifdef CONFIG_TIMER
	gd->timer = வெற்று;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	எனில் (ret)
		திரும்பு ret;

	திரும்பு dm_autoprobe();
}
#endif

நிலையான முழு initr_dm_devices(வெறுமை)
{
	முழு ret;

	எனில் (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		எனில் (ret)
			திரும்பு ret;
	}

	எனில் (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		எனில் (ret)
			திரும்பு ret;
	}

	திரும்பு 0;
}

நிலையான முழு initr_bootstage(வெறுமை)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	திரும்பு 0;
}

__weak முழு power_init_board(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு initr_announce(வெறுமை)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	திரும்பு 0;
}

நிலையான முழு __maybe_unused initr_binman(வெறுமை)
{
	முழு ret;

	ret = binman_init();
	எனில் (ret)
		printf("binman_init failed:%d\n", ret);

	திரும்பு ret;
}

#எனில் defined(CONFIG_MTD_NOR_FLASH)
__weak முழு is_flash_available(வெறுமை)
{
	திரும்பு 1;
}

நிலையான முழு initr_flash(வெறுமை)
{
	ulong flash_size = 0;
	கட்டமைப்பு bd_info *bd = gd->bd;

	எனில் (!is_flash_available())
		திரும்பு 0;

	puts("Flash: ");

	எனில் (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	இல்லையெனில்
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	எனில் (env_get_yesno("flashchecksum") == 1) {
		மாறிலி uchar *flash_base = (மாறிலி uchar *)CFG_SYS_FLASH_BASE;

		printf("  CRC: %08X", crc32(0,
					    flash_base,
					    flash_size));
	}
#endif /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#ifdef CFG_SYS_FLASH_BASE
	bd->bi_flashstart = CFG_SYS_FLASH_BASE;
#endif
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_size;

#எனில் defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#எனில் defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_size;
#அல்லதெனில் CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	திரும்பு 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
நிலையான முழு initr_nand(வெறுமை)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	திரும்பு 0;
}
#endif

#எனில் defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
நிலையான முழு initr_onenand(வெறுமை)
{
	puts("NAND:  ");
	onenand_init();
	திரும்பு 0;
}
#endif

#ifdef CONFIG_MMC
நிலையான முழு initr_mmc(வெறுமை)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	திரும்பு 0;
}
#endif

#ifdef CONFIG_PVBLOCK
நிலையான முழு initr_pvblock(வெறுமை)
{
	puts("PVBLOCK: ");
	pvblock_init();
	திரும்பு 0;
}
#endif

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
நிலையான முழு should_load_env(வெறுமை)
{
	எனில் (IS_ENABLED(CONFIG_OF_CONTROL))
		திரும்பு ofnode_conf_read_int("load-environment", 1);

	திரும்பு 1;
}

நிலையான முழு initr_env(வெறுமை)
{
	/* initialize environment */
	எனில் (should_load_env())
		env_relocate();
	இல்லையெனில்
		env_set_default(வெற்று, 0);

	env_import_fdt();

	எனில் (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (unsigned long)map_to_sysmem(gd->fdt_blob));

	#எனில் (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_data();
	#endif

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	திரும்பு 0;
}

#ifdef CONFIG_SYS_MALLOC_BOOTPARAMS
நிலையான முழு initr_malloc_bootparams(வெறுமை)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	எனில் (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		திரும்பு -ENOMEM;
	}
	திரும்பு 0;
}
#endif

நிலையான முழு initr_boot_led_blink(வெறுமை)
{
	led_boot_blink();

	திரும்பு 0;
}

நிலையான முழு initr_boot_led_on(வெறுமை)
{
	led_boot_on();

	திரும்பு 0;
}

#எனில் CONFIG_IS_ENABLED(NET)
நிலையான முழு initr_net(வெறுமை)
{
	puts("Net:   ");
	eth_initialize();
#எனில் defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	திரும்பு 0;
}
#endif

#ifdef CONFIG_POST
நிலையான முழு initr_post(வெறுமை)
{
	post_run(வெற்று, POST_RAM | post_bootmode_get(0));
	திரும்பு 0;
}
#endif

#எனில் defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
முழு initr_mem(வெறுமை)
{
	ulong pram = 0;
	எழுத்து memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (long முழு)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	திரும்பு 0;
}
#endif

நிலையான முழு initr_lmb(வெறுமை)
{
	எனில் (CONFIG_IS_ENABLED(LMB))
		திரும்பு lmb_init();
	இல்லையெனில்
		திரும்பு 0;
}

நிலையான முழு dm_announce(வெறுமை)
{
	முழு device_மொத்தம்;
	முழு uclass_மொத்தம்;

	எனில் (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_மொத்தம், &uclass_மொத்தம்);
		printf("Core:  %d devices, %d uclasses", device_மொத்தம்,
		       uclass_மொத்தம்);
		எனில் (CONFIG_IS_ENABLED(OF_REAL))
			printf(", devicetree: %s", fdtdec_get_srcname());
		எனில் (CONFIG_IS_ENABLED(UPL))
			printf(", universal payload active");
		printf("\n");
		எனில் (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			printf("Warning: Unexpected devicetree source (not from a prior stage)");
			printf("Warning: U-Boot may not function properly\n");
		}
	}

	திரும்பு 0;
}

நிலையான முழு run_main_loop(வெறுமை)
{
	முழு ret;

#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif

	ret = event_notify_null(EVT_MAIN_LOOP);
	எனில் (ret)
		திரும்பு ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	க்கு (;;)
		main_loop();
	திரும்பு 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

நிலையான வெறுமை initcall_run_r(வெறுமை)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#எனில் CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	INITCALL(initr_reloc_global_data);
#எனில் CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#endif
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#எனில் CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#endif
#எனில் CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#endif
	INITCALL(initr_of_live);
#எனில் CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#endif
#எனில் CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#endif
#எனில் CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#எனில் CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#endif
	INITCALL(initr_lmb);
#எனில் CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#endif
#எனில் CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#endif
#எனில் CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#endif
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#எனில் CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#endif
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#எனில் CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#endif
	WATCHDOG_RESET();
#எனில் CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#endif
	WATCHDOG_RESET();
#எனில் CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#endif
#எனில் CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#endif
	INITCALL(power_init_board);
#எனில் CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#endif
	WATCHDOG_RESET();
#எனில் IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#endif
#எனில் CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#endif
#எனில் CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#endif
#எனில் CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#endif
#எனில் CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#endif
#எனில் CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#endif
#எனில் CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#endif
	INITCALL(initr_env);
#எனில் CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#endif
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#எனில் CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#endif
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#எனில் CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#endif
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#எனில் CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#endif
	INITCALL(console_init_r);	/* fully init console as a device */
#எனில் CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#endif
	/* miscellaneous arch-dependent init */
#எனில் CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#endif
	/* miscellaneous platform-dependent init */
#எனில் CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#endif
	WATCHDOG_RESET();
#எனில் CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#endif
	INITCALL(interrupt_init);
#எனில் defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#endif
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#எனில் CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#endif
#எனில் CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#endif
#எனில் CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#endif
#எனில் CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#endif
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#எனில் defined(CFG_PRAM)
	INITCALL(initr_mem);
#endif
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

வெறுமை board_init_r(gd_t *new_gd, ulong dest_addr)
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
	எனில் (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#எனில் defined(CONFIG_RISCV)
	set_gd(new_gd);
#அல்லதெனில் !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
