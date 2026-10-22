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

#অন্তর্ভাৱ <config.h>
#অন্তর্ভাৱ <api.h>
#অন্তর্ভাৱ <bootstage.h>
#অন্তর্ভাৱ <cpu_func.h>
#অন্তর্ভাৱ <cyclic.h>
#অন্তর্ভাৱ <display_options.h>
#অন্তর্ভাৱ <exports.h>
#ifdef CONFIG_MTD_NOR_FLASH
#অন্তর্ভাৱ <flash.h>
#endif
#অন্তর্ভাৱ <hang.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <irq_func.h>
#অন্তর্ভাৱ <lmb.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <net.h>
#অন্তর্ভাৱ <asm/cache.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <u-boot/crc.h>
#অন্তর্ভাৱ <binman.h>
#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <dm.h>
#অন্তর্ভাৱ <efi_loader.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <env_internal.h>
#অন্তর্ভাৱ <fdtdec.h>
#অন্তর্ভাৱ <init.h>
#অন্তর্ভাৱ <initcall.h>
#অন্তর্ভাৱ <kgdb.h>
#অন্তর্ভাৱ <irq_func.h>
#অন্তর্ভাৱ <led.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <miiphy.h>
#অন্তর্ভাৱ <mmc.h>
#অন্তর্ভাৱ <mux.h>
#অন্তর্ভাৱ <nand.h>
#অন্তর্ভাৱ <of_live.h>
#অন্তর্ভাৱ <onenand_uboot.h>
#অন্তর্ভাৱ <pvblock.h>
#অন্তর্ভাৱ <scsi.h>
#অন্তর্ভাৱ <serial.h>
#অন্তর্ভাৱ <stdio_dev.h>
#অন্তর্ভাৱ <timer.h>
#অন্তর্ভাৱ <trace.h>
#অন্তর্ভাৱ <watchdog.h>
#অন্তর্ভাৱ <xen.h>
#অন্তর্ভাৱ <asm/sections.h>
#অন্তর্ভাৱ <dm/root.h>
#অন্তর্ভাৱ <dm/ofnode.h>
#অন্তর্ভাৱ <linux/compiler.h>
#অন্তর্ভাৱ <linux/err.h>
#অন্তর্ভাৱ <wdt.h>
#অন্তর্ভাৱ <asm-generic/gpio.h>
#অন্তর্ভাৱ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak পূর্ণাঙ্ক board_flash_wp_on(শূন্য)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক cpu_secondary_init_r(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_trace(শূন্য)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_reloc(শূন্য)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	ঘূৰাই 0;
}

#যদি defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
স্থির পূর্ণাঙ্ক initr_caches(শূন্য)
{
	/* Enable caches */
	enable_caches();
	ঘূৰাই 0;
}
#endif

__weak পূর্ণাঙ্ক fixup_cpu(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_reloc_global_data(শূন্য)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#নোচেদ্যদি defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#নোচেদ্যদি !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#endif
#যদি defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
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
	যদি (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#ifdef CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	যদি (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, রিক্ত);

#endif
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	যদি (IS_ENABLED(CONFIG_MMU_PGPROT)) {
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

	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক arch_initr_trap(শূন্য)
{
	ঘূৰাই 0;
}

#যদি defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
স্থির পূর্ণাঙ্ক initr_unlock_ram_in_cache(শূন্য)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক initr_barrier(শূন্য)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_malloc(শূন্য)
{
	ulong start;

#যদি CONFIG_IS_ENABLED(SYS_MALLOC_F)
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
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_of_live(শূন্য)
{
	যদি (CONFIG_IS_ENABLED(OF_LIVE)) {
		পূর্ণাঙ্ক ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (সংরচনা device_গ্ৰন্থি **)gd_of_মূল_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		যদি (ret)
			ঘূৰাই ret;
	}

	ঘূৰাই 0;
}

#ifdef CONFIG_DM
স্থির পূর্ণাঙ্ক initr_dm(শূন্য)
{
	পূর্ণাঙ্ক ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_মূল = রিক্ত;
#ifdef CONFIG_TIMER
	gd->timer = রিক্ত;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	যদি (ret)
		ঘূৰাই ret;

	ঘূৰাই dm_autoprobe();
}
#endif

স্থির পূর্ণাঙ্ক initr_dm_devices(শূন্য)
{
	পূর্ণাঙ্ক ret;

	যদি (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		যদি (ret)
			ঘূৰাই ret;
	}

	যদি (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		যদি (ret)
			ঘূৰাই ret;
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_bootstage(শূন্য)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক power_init_board(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_announce(শূন্য)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক __maybe_unused initr_binman(শূন্য)
{
	পূর্ণাঙ্ক ret;

	ret = binman_init();
	যদি (ret)
		printf("binman_init failed:%d\n", ret);

	ঘূৰাই ret;
}

#যদি defined(CONFIG_MTD_NOR_FLASH)
__weak পূর্ণাঙ্ক is_flash_available(শূন্য)
{
	ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক initr_flash(শূন্য)
{
	ulong flash_size = 0;
	সংরচনা bd_info *bd = gd->bd;

	যদি (!is_flash_available())
		ঘূৰাই 0;

	puts("Flash: ");

	যদি (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	অন্যথা
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	যদি (env_get_yesno("flashchecksum") == 1) {
		ধ্রুৱ uchar *flash_base = (ধ্রুৱ uchar *)CFG_SYS_FLASH_BASE;

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

#যদি defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#যদি defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_size;
#নোচেদ্যদি CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	ঘূৰাই 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
স্থির পূর্ণাঙ্ক initr_nand(শূন্য)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	ঘূৰাই 0;
}
#endif

#যদি defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
স্থির পূর্ণাঙ্ক initr_onenand(শূন্য)
{
	puts("NAND:  ");
	onenand_init();
	ঘূৰাই 0;
}
#endif

#ifdef CONFIG_MMC
স্থির পূর্ণাঙ্ক initr_mmc(শূন্য)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	ঘূৰাই 0;
}
#endif

#ifdef CONFIG_PVBLOCK
স্থির পূর্ণাঙ্ক initr_pvblock(শূন্য)
{
	puts("PVBLOCK: ");
	pvblock_init();
	ঘূৰাই 0;
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
স্থির পূর্ণাঙ্ক should_load_env(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_OF_CONTROL))
		ঘূৰাই ofnode_conf_read_int("load-environment", 1);

	ঘূৰাই 1;
}

স্থির পূর্ণাঙ্ক initr_env(শূন্য)
{
	/* initialize environment */
	যদি (should_load_env())
		env_relocate();
	অন্যথা
		env_set_default(রিক্ত, 0);

	env_import_fdt();

	যদি (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (unsigned long)map_to_sysmem(gd->fdt_blob));

	#যদি (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_data();
	#endif

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	ঘূৰাই 0;
}

#ifdef CONFIG_SYS_MALLOC_BOOTPARAMS
স্থির পূর্ণাঙ্ক initr_malloc_bootparams(শূন্য)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	যদি (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		ঘূৰাই -ENOMEM;
	}
	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক initr_boot_led_blink(শূন্য)
{
	led_boot_blink();

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initr_boot_led_on(শূন্য)
{
	led_boot_on();

	ঘূৰাই 0;
}

#যদি CONFIG_IS_ENABLED(NET)
স্থির পূর্ণাঙ্ক initr_net(শূন্য)
{
	puts("Net:   ");
	eth_initialize();
#যদি defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	ঘূৰাই 0;
}
#endif

#ifdef CONFIG_POST
স্থির পূর্ণাঙ্ক initr_post(শূন্য)
{
	post_run(রিক্ত, POST_RAM | post_bootmode_get(0));
	ঘূৰাই 0;
}
#endif

#যদি defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
পূর্ণাঙ্ক initr_mem(শূন্য)
{
	ulong pram = 0;
	ৱর্ণ memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (long পূর্ণাঙ্ক)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক initr_lmb(শূন্য)
{
	যদি (CONFIG_IS_ENABLED(LMB))
		ঘূৰাই lmb_init();
	অন্যথা
		ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক dm_announce(শূন্য)
{
	পূর্ণাঙ্ক device_গুনতি;
	পূর্ণাঙ্ক uclass_গুনতি;

	যদি (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_গুনতি, &uclass_গুনতি);
		printf("Core:  %d devices, %d uclasses", device_গুনতি,
		       uclass_গুনতি);
		যদি (CONFIG_IS_ENABLED(OF_REAL))
			printf(", devicetree: %s", fdtdec_get_srcname());
		যদি (CONFIG_IS_ENABLED(UPL))
			printf(", universal payload active");
		printf("\n");
		যদি (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			printf("Warning: Unexpected devicetree source (not from a prior stage)");
			printf("Warning: U-Boot may not function properly\n");
		}
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক run_main_loop(শূন্য)
{
	পূর্ণাঙ্ক ret;

#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif

	ret = event_notify_null(EVT_MAIN_LOOP);
	যদি (ret)
		ঘূৰাই ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	বাবে (;;)
		main_loop();
	ঘূৰাই 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

স্থির শূন্য initcall_run_r(শূন্য)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#যদি CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	INITCALL(initr_reloc_global_data);
#যদি CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#endif
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#যদি CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#endif
#যদি CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#endif
	INITCALL(initr_of_live);
#যদি CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#endif
#যদি CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#endif
#যদি CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#যদি CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#endif
	INITCALL(initr_lmb);
#যদি CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#endif
#যদি CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#endif
#যদি CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#endif
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#যদি CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#endif
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#যদি CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#endif
	WATCHDOG_RESET();
#যদি CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#endif
	WATCHDOG_RESET();
#যদি CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#endif
#যদি CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#endif
	INITCALL(power_init_board);
#যদি CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#endif
	WATCHDOG_RESET();
#যদি IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#endif
#যদি CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#endif
#যদি CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#endif
#যদি CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#endif
#যদি CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#endif
#যদি CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#endif
#যদি CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#endif
	INITCALL(initr_env);
#যদি CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#endif
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#যদি CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#endif
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#যদি CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#endif
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#যদি CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#endif
	INITCALL(console_init_r);	/* fully init console as a device */
#যদি CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#endif
	/* miscellaneous arch-dependent init */
#যদি CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#endif
	/* miscellaneous platform-dependent init */
#যদি CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#endif
	WATCHDOG_RESET();
#যদি CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#endif
	INITCALL(interrupt_init);
#যদি defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#endif
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#যদি CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#endif
#যদি CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#endif
#যদি CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#endif
#যদি CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#endif
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#যদি defined(CFG_PRAM)
	INITCALL(initr_mem);
#endif
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

শূন্য board_init_r(gd_t *new_gd, ulong dest_addr)
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
	যদি (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#যদি defined(CONFIG_RISCV)
	set_gd(new_gd);
#নোচেদ্যদি !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
