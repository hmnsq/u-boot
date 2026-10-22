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

#شامل <config.h>
#شامل <api.h>
#شامل <bootstage.h>
#شامل <cpu_func.h>
#شامل <cyclic.h>
#شامل <display_options.h>
#شامل <exports.h>
#ifdef CONFIG_MTD_NOR_FLASH
#شامل <flash.h>
#endif
#شامل <hang.h>
#شامل <image.h>
#شامل <irq_func.h>
#شامل <lmb.h>
#شامل <log.h>
#شامل <net.h>
#شامل <asm/cache.h>
#شامل <asm/global_data.h>
#شامل <u-boot/crc.h>
#شامل <binman.h>
#شامل <command.h>
#شامل <console.h>
#شامل <dm.h>
#شامل <efi_loader.h>
#شامل <env.h>
#شامل <env_internal.h>
#شامل <fdtdec.h>
#شامل <init.h>
#شامل <initcall.h>
#شامل <kgdb.h>
#شامل <irq_func.h>
#شامل <led.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <miiphy.h>
#شامل <mmc.h>
#شامل <mux.h>
#شامل <nand.h>
#شامل <of_live.h>
#شامل <onenand_uboot.h>
#شامل <pvblock.h>
#شامل <scsi.h>
#شامل <serial.h>
#شامل <stdio_dev.h>
#شامل <timer.h>
#شامل <trace.h>
#شامل <watchdog.h>
#شامل <xen.h>
#شامل <asm/sections.h>
#شامل <dm/root.h>
#شامل <dm/ofnode.h>
#شامل <linux/compiler.h>
#شامل <linux/err.h>
#شامل <wdt.h>
#شامل <asm-generic/gpio.h>
#شامل <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak صحیح board_flash_wp_on(تهی)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	بازگشت 0;
}

__weak صحیح cpu_secondary_init_r(تهی)
{
	بازگشت 0;
}

ایستا صحیح initr_trace(تهی)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	بازگشت 0;
}

ایستا صحیح initr_reloc(تهی)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	بازگشت 0;
}

#اگر defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ایستا صحیح initr_caches(تهی)
{
	/* Enable caches */
	enable_caches();
	بازگشت 0;
}
#endif

__weak صحیح fixup_cpu(تهی)
{
	بازگشت 0;
}

ایستا صحیح initr_reloc_global_data(تهی)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#وگرنهاگر defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#وگرنهاگر !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#endif
#اگر defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
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
	اگر (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#ifdef CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	اگر (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, پوچ);

#endif
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	اگر (IS_ENABLED(CONFIG_MMU_PGPROT)) {
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

	بازگشت 0;
}

__weak صحیح arch_initr_trap(تهی)
{
	بازگشت 0;
}

#اگر defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
ایستا صحیح initr_unlock_ram_in_cache(تهی)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	بازگشت 0;
}
#endif

ایستا صحیح initr_barrier(تهی)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	بازگشت 0;
}

ایستا صحیح initr_malloc(تهی)
{
	ulong start;

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
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
	بازگشت 0;
}

ایستا صحیح initr_of_live(تهی)
{
	اگر (CONFIG_IS_ENABLED(OF_LIVE)) {
		صحیح ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (ساختار device_گره **)gd_of_ریشه_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		اگر (ret)
			بازگشت ret;
	}

	بازگشت 0;
}

#ifdef CONFIG_DM
ایستا صحیح initr_dm(تهی)
{
	صحیح ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_ریشه = پوچ;
#ifdef CONFIG_TIMER
	gd->timer = پوچ;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	اگر (ret)
		بازگشت ret;

	بازگشت dm_autoprobe();
}
#endif

ایستا صحیح initr_dm_devices(تهی)
{
	صحیح ret;

	اگر (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		اگر (ret)
			بازگشت ret;
	}

	اگر (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		اگر (ret)
			بازگشت ret;
	}

	بازگشت 0;
}

ایستا صحیح initr_bootstage(تهی)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	بازگشت 0;
}

__weak صحیح power_init_board(تهی)
{
	بازگشت 0;
}

ایستا صحیح initr_announce(تهی)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	بازگشت 0;
}

ایستا صحیح __maybe_unused initr_binman(تهی)
{
	صحیح ret;

	ret = binman_init();
	اگر (ret)
		printf("binman_init failed:%d\n", ret);

	بازگشت ret;
}

#اگر defined(CONFIG_MTD_NOR_FLASH)
__weak صحیح is_flash_available(تهی)
{
	بازگشت 1;
}

ایستا صحیح initr_flash(تهی)
{
	ulong flash_size = 0;
	ساختار bd_info *bd = gd->bd;

	اگر (!is_flash_available())
		بازگشت 0;

	puts("Flash: ");

	اگر (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	وگرنه
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	اگر (env_get_yesno("flashchecksum") == 1) {
		ثابت uchar *flash_base = (ثابت uchar *)CFG_SYS_FLASH_BASE;

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

#اگر defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#اگر defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_size;
#وگرنهاگر CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	بازگشت 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
ایستا صحیح initr_nand(تهی)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	بازگشت 0;
}
#endif

#اگر defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ایستا صحیح initr_onenand(تهی)
{
	puts("NAND:  ");
	onenand_init();
	بازگشت 0;
}
#endif

#ifdef CONFIG_MMC
ایستا صحیح initr_mmc(تهی)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	بازگشت 0;
}
#endif

#ifdef CONFIG_PVBLOCK
ایستا صحیح initr_pvblock(تهی)
{
	puts("PVBLOCK: ");
	pvblock_init();
	بازگشت 0;
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
ایستا صحیح should_load_env(تهی)
{
	اگر (IS_ENABLED(CONFIG_OF_CONTROL))
		بازگشت ofnode_conf_read_int("load-environment", 1);

	بازگشت 1;
}

ایستا صحیح initr_env(تهی)
{
	/* initialize environment */
	اگر (should_load_env())
		env_relocate();
	وگرنه
		env_set_default(پوچ, 0);

	env_import_fdt();

	اگر (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (unsigned long)map_to_sysmem(gd->fdt_blob));

	#اگر (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_data();
	#endif

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	بازگشت 0;
}

#ifdef CONFIG_SYS_MALLOC_BOOTPARAMS
ایستا صحیح initr_malloc_bootparams(تهی)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	اگر (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		بازگشت -ENOMEM;
	}
	بازگشت 0;
}
#endif

ایستا صحیح initr_boot_led_blink(تهی)
{
	led_boot_blink();

	بازگشت 0;
}

ایستا صحیح initr_boot_led_on(تهی)
{
	led_boot_on();

	بازگشت 0;
}

#اگر CONFIG_IS_ENABLED(NET)
ایستا صحیح initr_net(تهی)
{
	puts("Net:   ");
	eth_initialize();
#اگر defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	بازگشت 0;
}
#endif

#ifdef CONFIG_POST
ایستا صحیح initr_post(تهی)
{
	post_run(پوچ, POST_RAM | post_bootmode_get(0));
	بازگشت 0;
}
#endif

#اگر defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
صحیح initr_mem(تهی)
{
	ulong pram = 0;
	نویسه memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (long صحیح)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	بازگشت 0;
}
#endif

ایستا صحیح initr_lmb(تهی)
{
	اگر (CONFIG_IS_ENABLED(LMB))
		بازگشت lmb_init();
	وگرنه
		بازگشت 0;
}

ایستا صحیح dm_announce(تهی)
{
	صحیح device_شمار;
	صحیح uclass_شمار;

	اگر (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_شمار, &uclass_شمار);
		printf("Core:  %d devices, %d uclasses", device_شمار,
		       uclass_شمار);
		اگر (CONFIG_IS_ENABLED(OF_REAL))
			printf(", devicetree: %s", fdtdec_get_srcname());
		اگر (CONFIG_IS_ENABLED(UPL))
			printf(", universal payload active");
		printf("\n");
		اگر (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			printf("Warning: Unexpected devicetree source (not from a prior stage)");
			printf("Warning: U-Boot may not function properly\n");
		}
	}

	بازگشت 0;
}

ایستا صحیح run_main_loop(تهی)
{
	صحیح ret;

#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif

	ret = event_notify_null(EVT_MAIN_LOOP);
	اگر (ret)
		بازگشت ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	برای (;;)
		main_loop();
	بازگشت 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ایستا تهی initcall_run_r(تهی)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#اگر CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	INITCALL(initr_reloc_global_data);
#اگر CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#endif
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#اگر CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#endif
#اگر CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#endif
	INITCALL(initr_of_live);
#اگر CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#endif
#اگر CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#endif
#اگر CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#اگر CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#endif
	INITCALL(initr_lmb);
#اگر CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#endif
#اگر CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#endif
#اگر CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#endif
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#اگر CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#endif
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#اگر CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#endif
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#endif
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#endif
#اگر CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#endif
	INITCALL(power_init_board);
#اگر CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#endif
	WATCHDOG_RESET();
#اگر IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#endif
#اگر CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#endif
#اگر CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#endif
#اگر CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#endif
#اگر CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#endif
#اگر CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#endif
#اگر CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#endif
	INITCALL(initr_env);
#اگر CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#endif
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#اگر CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#endif
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#endif
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#اگر CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#endif
	INITCALL(console_init_r);	/* fully init console as a device */
#اگر CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#endif
	/* miscellaneous arch-dependent init */
#اگر CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#endif
	/* miscellaneous platform-dependent init */
#اگر CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#endif
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#endif
	INITCALL(interrupt_init);
#اگر defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#endif
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#اگر CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#endif
#اگر CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#endif
#اگر CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#endif
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#endif
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#اگر defined(CFG_PRAM)
	INITCALL(initr_mem);
#endif
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

تهی board_init_r(gd_t *new_gd, ulong dest_addr)
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
	اگر (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#اگر defined(CONFIG_RISCV)
	set_gd(new_gd);
#وگرنهاگر !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
