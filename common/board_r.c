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

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <api.h>
#અન્તર્ભાવ <bootstage.h>
#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <cyclic.h>
#અન્તર્ભાવ <display_options.h>
#અન્તર્ભાવ <exports.h>
#ifdef CONFIG_MTD_NOR_FLASH
#અન્તર્ભાવ <flash.h>
#endif
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <irq_func.h>
#અન્તર્ભાવ <lmb.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <net.h>
#અન્તર્ભાવ <asm/cache.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <u-boot/crc.h>
#અન્તર્ભાવ <binman.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <efi_loader.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <env_internal.h>
#અન્તર્ભાવ <fdtdec.h>
#અન્તર્ભાવ <init.h>
#અન્તર્ભાવ <initcall.h>
#અન્તર્ભાવ <kgdb.h>
#અન્તર્ભાવ <irq_func.h>
#અન્તર્ભાવ <led.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <miiphy.h>
#અન્તર્ભાવ <mmc.h>
#અન્તર્ભાવ <mux.h>
#અન્તર્ભાવ <nand.h>
#અન્તર્ભાવ <of_live.h>
#અન્તર્ભાવ <onenand_uboot.h>
#અન્તર્ભાવ <pvblock.h>
#અન્તર્ભાવ <scsi.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <stdio_dev.h>
#અન્તર્ભાવ <timer.h>
#અન્તર્ભાવ <trace.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <xen.h>
#અન્તર્ભાવ <asm/sections.h>
#અન્તર્ભાવ <dm/root.h>
#અન્તર્ભાવ <dm/ofnode.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <linux/err.h>
#અન્તર્ભાવ <wdt.h>
#અન્તર્ભાવ <asm-generic/gpio.h>
#અન્તર્ભાવ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak પૂર્ણાંક board_flash_wp_on(શૂન્ય)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	પાછા 0;
}

__weak પૂર્ણાંક cpu_secondary_init_r(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_trace(શૂન્ય)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_reloc(શૂન્ય)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	પાછા 0;
}

#જો defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
સ્થિર પૂર્ણાંક initr_caches(શૂન્ય)
{
	/* Enable caches */
	enable_caches();
	પાછા 0;
}
#endif

__weak પૂર્ણાંક fixup_cpu(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_reloc_global_data(શૂન્ય)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#અથવાજો defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#અથવાજો !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#endif
#જો defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
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
	જો (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#ifdef CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	જો (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, ખાલી);

#endif
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	જો (IS_ENABLED(CONFIG_MMU_PGPROT)) {
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

	પાછા 0;
}

__weak પૂર્ણાંક arch_initr_trap(શૂન્ય)
{
	પાછા 0;
}

#જો defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
સ્થિર પૂર્ણાંક initr_unlock_ram_in_cache(શૂન્ય)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક initr_barrier(શૂન્ય)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_malloc(શૂન્ય)
{
	ulong start;

#જો CONFIG_IS_ENABLED(SYS_MALLOC_F)
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
	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_of_live(શૂન્ય)
{
	જો (CONFIG_IS_ENABLED(OF_LIVE)) {
		પૂર્ણાંક ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (સંરચના device_ગાંઠ **)gd_of_મૂળ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		જો (ret)
			પાછા ret;
	}

	પાછા 0;
}

#ifdef CONFIG_DM
સ્થિર પૂર્ણાંક initr_dm(શૂન્ય)
{
	પૂર્ણાંક ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_મૂળ = ખાલી;
#ifdef CONFIG_TIMER
	gd->timer = ખાલી;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	જો (ret)
		પાછા ret;

	પાછા dm_autoprobe();
}
#endif

સ્થિર પૂર્ણાંક initr_dm_devices(શૂન્ય)
{
	પૂર્ણાંક ret;

	જો (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		જો (ret)
			પાછા ret;
	}

	જો (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		જો (ret)
			પાછા ret;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_bootstage(શૂન્ય)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	પાછા 0;
}

__weak પૂર્ણાંક power_init_board(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_announce(શૂન્ય)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	પાછા 0;
}

સ્થિર પૂર્ણાંક __maybe_unused initr_binman(શૂન્ય)
{
	પૂર્ણાંક ret;

	ret = binman_init();
	જો (ret)
		printf("binman_init failed:%d\n", ret);

	પાછા ret;
}

#જો defined(CONFIG_MTD_NOR_FLASH)
__weak પૂર્ણાંક is_flash_available(શૂન્ય)
{
	પાછા 1;
}

સ્થિર પૂર્ણાંક initr_flash(શૂન્ય)
{
	ulong flash_size = 0;
	સંરચના bd_info *bd = gd->bd;

	જો (!is_flash_available())
		પાછા 0;

	puts("Flash: ");

	જો (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	નહીંતર
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	જો (env_get_yesno("flashchecksum") == 1) {
		અચલ uchar *flash_base = (અચલ uchar *)CFG_SYS_FLASH_BASE;

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

#જો defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#જો defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_size;
#અથવાજો CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	પાછા 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
સ્થિર પૂર્ણાંક initr_nand(શૂન્ય)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	પાછા 0;
}
#endif

#જો defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
સ્થિર પૂર્ણાંક initr_onenand(શૂન્ય)
{
	puts("NAND:  ");
	onenand_init();
	પાછા 0;
}
#endif

#ifdef CONFIG_MMC
સ્થિર પૂર્ણાંક initr_mmc(શૂન્ય)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	પાછા 0;
}
#endif

#ifdef CONFIG_PVBLOCK
સ્થિર પૂર્ણાંક initr_pvblock(શૂન્ય)
{
	puts("PVBLOCK: ");
	pvblock_init();
	પાછા 0;
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
સ્થિર પૂર્ણાંક should_load_env(શૂન્ય)
{
	જો (IS_ENABLED(CONFIG_OF_CONTROL))
		પાછા ofnode_conf_read_int("load-environment", 1);

	પાછા 1;
}

સ્થિર પૂર્ણાંક initr_env(શૂન્ય)
{
	/* initialize environment */
	જો (should_load_env())
		env_relocate();
	નહીંતર
		env_set_default(ખાલી, 0);

	env_import_fdt();

	જો (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (unsigned long)map_to_sysmem(gd->fdt_blob));

	#જો (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_data();
	#endif

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	પાછા 0;
}

#ifdef CONFIG_SYS_MALLOC_BOOTPARAMS
સ્થિર પૂર્ણાંક initr_malloc_bootparams(શૂન્ય)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	જો (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		પાછા -ENOMEM;
	}
	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક initr_boot_led_blink(શૂન્ય)
{
	led_boot_blink();

	પાછા 0;
}

સ્થિર પૂર્ણાંક initr_boot_led_on(શૂન્ય)
{
	led_boot_on();

	પાછા 0;
}

#જો CONFIG_IS_ENABLED(NET)
સ્થિર પૂર્ણાંક initr_net(શૂન્ય)
{
	puts("Net:   ");
	eth_initialize();
#જો defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	પાછા 0;
}
#endif

#ifdef CONFIG_POST
સ્થિર પૂર્ણાંક initr_post(શૂન્ય)
{
	post_run(ખાલી, POST_RAM | post_bootmode_get(0));
	પાછા 0;
}
#endif

#જો defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
પૂર્ણાંક initr_mem(શૂન્ય)
{
	ulong pram = 0;
	અક્ષર memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (long પૂર્ણાંક)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક initr_lmb(શૂન્ય)
{
	જો (CONFIG_IS_ENABLED(LMB))
		પાછા lmb_init();
	નહીંતર
		પાછા 0;
}

સ્થિર પૂર્ણાંક dm_announce(શૂન્ય)
{
	પૂર્ણાંક device_ગણતરી;
	પૂર્ણાંક uclass_ગણતરી;

	જો (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_ગણતરી, &uclass_ગણતરી);
		printf("Core:  %d devices, %d uclasses", device_ગણતરી,
		       uclass_ગણતરી);
		જો (CONFIG_IS_ENABLED(OF_REAL))
			printf(", devicetree: %s", fdtdec_get_srcname());
		જો (CONFIG_IS_ENABLED(UPL))
			printf(", universal payload active");
		printf("\n");
		જો (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			printf("Warning: Unexpected devicetree source (not from a prior stage)");
			printf("Warning: U-Boot may not function properly\n");
		}
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક run_main_loop(શૂન્ય)
{
	પૂર્ણાંક ret;

#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif

	ret = event_notify_null(EVT_MAIN_LOOP);
	જો (ret)
		પાછા ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	માટે (;;)
		main_loop();
	પાછા 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

સ્થિર શૂન્ય initcall_run_r(શૂન્ય)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#જો CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	INITCALL(initr_reloc_global_data);
#જો CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#endif
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#જો CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#endif
#જો CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#endif
	INITCALL(initr_of_live);
#જો CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#endif
#જો CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#endif
#જો CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#જો CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#endif
	INITCALL(initr_lmb);
#જો CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#endif
#જો CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#endif
#જો CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#endif
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#જો CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#endif
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#જો CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#endif
	WATCHDOG_RESET();
#જો CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#endif
	WATCHDOG_RESET();
#જો CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#endif
#જો CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#endif
	INITCALL(power_init_board);
#જો CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#endif
	WATCHDOG_RESET();
#જો IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#endif
#જો CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#endif
#જો CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#endif
#જો CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#endif
#જો CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#endif
#જો CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#endif
#જો CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#endif
	INITCALL(initr_env);
#જો CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#endif
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#જો CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#endif
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#જો CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#endif
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#જો CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#endif
	INITCALL(console_init_r);	/* fully init console as a device */
#જો CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#endif
	/* miscellaneous arch-dependent init */
#જો CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#endif
	/* miscellaneous platform-dependent init */
#જો CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#endif
	WATCHDOG_RESET();
#જો CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#endif
	INITCALL(interrupt_init);
#જો defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#endif
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#જો CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#endif
#જો CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#endif
#જો CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#endif
#જો CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#endif
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#જો defined(CFG_PRAM)
	INITCALL(initr_mem);
#endif
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

શૂન્ય board_init_r(gd_t *new_gd, ulong dest_addr)
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
	જો (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#જો defined(CONFIG_RISCV)
	set_gd(new_gd);
#અથવાજો !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
