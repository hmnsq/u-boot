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

#ضمن <config.h>
#ضمن <api.h>
#ضمن <bootstage.h>
#ضمن <cpu_func.h>
#ضمن <cyclic.h>
#ضمن <display_options.h>
#ضمن <exports.h>
#ifdef CONFIG_MTD_NOR_FLASH
#ضمن <flash.h>
#endif
#ضمن <hang.h>
#ضمن <image.h>
#ضمن <irq_func.h>
#ضمن <lmb.h>
#ضمن <log.h>
#ضمن <net.h>
#ضمن <asm/cache.h>
#ضمن <asm/global_data.h>
#ضمن <u-boot/crc.h>
#ضمن <binman.h>
#ضمن <command.h>
#ضمن <console.h>
#ضمن <dm.h>
#ضمن <efi_loader.h>
#ضمن <env.h>
#ضمن <env_internal.h>
#ضمن <fdtdec.h>
#ضمن <init.h>
#ضمن <initcall.h>
#ضمن <kgdb.h>
#ضمن <irq_func.h>
#ضمن <led.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <miiphy.h>
#ضمن <mmc.h>
#ضمن <mux.h>
#ضمن <nand.h>
#ضمن <of_live.h>
#ضمن <onenand_uboot.h>
#ضمن <pvblock.h>
#ضمن <scsi.h>
#ضمن <serial.h>
#ضمن <stdio_dev.h>
#ضمن <timer.h>
#ضمن <trace.h>
#ضمن <watchdog.h>
#ضمن <xen.h>
#ضمن <asm/sections.h>
#ضمن <dm/root.h>
#ضمن <dm/ofnode.h>
#ضمن <linux/compiler.h>
#ضمن <linux/err.h>
#ضمن <wdt.h>
#ضمن <asm-generic/gpio.h>
#ضمن <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak صحيح board_flash_wp_on(فراغ)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	החזר 0;
}

__weak صحيح cpu_secondary_init_r(فراغ)
{
	החזר 0;
}

ساكن صحيح initr_trace(فراغ)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	החזר 0;
}

ساكن صحيح initr_reloc(فراغ)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	החזר 0;
}

#אם defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ساكن صحيح initr_caches(فراغ)
{
	/* Enable caches */
	enable_caches();
	החזר 0;
}
#endif

__weak صحيح fixup_cpu(فراغ)
{
	החזר 0;
}

ساكن صحيح initr_reloc_global_data(فراغ)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#وإلاإذا defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#وإلاإذا !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#endif
#אם defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
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
	אם (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#ifdef CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	אם (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, عدم);

#endif
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	אם (IS_ENABLED(CONFIG_MMU_PGPROT)) {
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

	החזר 0;
}

__weak صحيح arch_initr_trap(فراغ)
{
	החזר 0;
}

#אם defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
ساكن صحيح initr_unlock_ram_in_cache(فراغ)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	החזר 0;
}
#endif

ساكن صحيح initr_barrier(فراغ)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	החזר 0;
}

ساكن صحيح initr_malloc(فراغ)
{
	ulong start;

#אם CONFIG_IS_ENABLED(SYS_MALLOC_F)
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
	החזר 0;
}

ساكن صحيح initr_of_live(فراغ)
{
	אם (CONFIG_IS_ENABLED(OF_LIVE)) {
		صحيح ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (بنية device_node **)gd_of_root_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		אם (ret)
			החזר ret;
	}

	החזר 0;
}

#ifdef CONFIG_DM
ساكن صحيح initr_dm(فراغ)
{
	صحيح ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_root = عدم;
#ifdef CONFIG_TIMER
	gd->timer = عدم;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	אם (ret)
		החזר ret;

	החזר dm_autoprobe();
}
#endif

ساكن صحيح initr_dm_devices(فراغ)
{
	صحيح ret;

	אם (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		אם (ret)
			החזר ret;
	}

	אם (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		אם (ret)
			החזר ret;
	}

	החזר 0;
}

ساكن صحيح initr_bootstage(فراغ)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	החזר 0;
}

__weak صحيح power_init_board(فراغ)
{
	החזר 0;
}

ساكن صحيح initr_announce(فراغ)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	החזר 0;
}

ساكن صحيح __maybe_unused initr_binman(فراغ)
{
	صحيح ret;

	ret = binman_init();
	אם (ret)
		printf("binman_init failed:%d\n", ret);

	החזר ret;
}

#אם defined(CONFIG_MTD_NOR_FLASH)
__weak صحيح is_flash_available(فراغ)
{
	החזר 1;
}

ساكن صحيح initr_flash(فراغ)
{
	ulong flash_size = 0;
	بنية bd_info *bd = gd->bd;

	אם (!is_flash_available())
		החזר 0;

	puts("Flash: ");

	אם (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	אחרת
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	אם (env_get_yesno("flashchecksum") == 1) {
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

#אם defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#אם defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_size;
#وإلاإذا CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	החזר 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
ساكن صحيح initr_nand(فراغ)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	החזר 0;
}
#endif

#אם defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ساكن صحيح initr_onenand(فراغ)
{
	puts("NAND:  ");
	onenand_init();
	החזר 0;
}
#endif

#ifdef CONFIG_MMC
ساكن صحيح initr_mmc(فراغ)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	החזר 0;
}
#endif

#ifdef CONFIG_PVBLOCK
ساكن صحيح initr_pvblock(فراغ)
{
	puts("PVBLOCK: ");
	pvblock_init();
	החזר 0;
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
ساكن صحيح should_load_env(فراغ)
{
	אם (IS_ENABLED(CONFIG_OF_CONTROL))
		החזר ofnode_conf_read_int("load-environment", 1);

	החזר 1;
}

ساكن صحيح initr_env(فراغ)
{
	/* initialize environment */
	אם (should_load_env())
		env_relocate();
	אחרת
		env_set_default(عدم, 0);

	env_import_fdt();

	אם (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (unsigned long)map_to_sysmem(gd->fdt_blob));

	#אם (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_data();
	#endif

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	החזר 0;
}

#ifdef CONFIG_SYS_MALLOC_BOOTPARAMS
ساكن صحيح initr_malloc_bootparams(فراغ)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	אם (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		החזר -ENOMEM;
	}
	החזר 0;
}
#endif

ساكن صحيح initr_boot_led_blink(فراغ)
{
	led_boot_blink();

	החזר 0;
}

ساكن صحيح initr_boot_led_on(فراغ)
{
	led_boot_on();

	החזר 0;
}

#אם CONFIG_IS_ENABLED(NET)
ساكن صحيح initr_net(فراغ)
{
	puts("Net:   ");
	eth_initialize();
#אם defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	החזר 0;
}
#endif

#ifdef CONFIG_POST
ساكن صحيح initr_post(فراغ)
{
	post_run(عدم, POST_RAM | post_bootmode_get(0));
	החזר 0;
}
#endif

#אם defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
صحيح initr_mem(فراغ)
{
	ulong pram = 0;
	حرف memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (long صحيح)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	החזר 0;
}
#endif

ساكن صحيح initr_lmb(فراغ)
{
	אם (CONFIG_IS_ENABLED(LMB))
		החזר lmb_init();
	אחרת
		החזר 0;
}

ساكن صحيح dm_announce(فراغ)
{
	صحيح device_count;
	صحيح uclass_count;

	אם (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_count, &uclass_count);
		printf("Core:  %d devices, %d uclasses", device_count,
		       uclass_count);
		אם (CONFIG_IS_ENABLED(OF_REAL))
			printf(", devicetree: %s", fdtdec_get_srcname());
		אם (CONFIG_IS_ENABLED(UPL))
			printf(", universal payload active");
		printf("\n");
		אם (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			printf("Warning: Unexpected devicetree source (not from a prior stage)");
			printf("Warning: U-Boot may not function properly\n");
		}
	}

	החזר 0;
}

ساكن صحيح run_main_loop(فراغ)
{
	صحيح ret;

#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif

	ret = event_notify_null(EVT_MAIN_LOOP);
	אם (ret)
		החזר ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	עבור (;;)
		main_loop();
	החזר 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ساكن فراغ initcall_run_r(فراغ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#אם CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	INITCALL(initr_reloc_global_data);
#אם CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#endif
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#אם CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#endif
#אם CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#endif
	INITCALL(initr_of_live);
#אם CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#endif
#אם CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#endif
#אם CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#אם CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#endif
	INITCALL(initr_lmb);
#אם CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#endif
#אם CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#endif
#אם CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#endif
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#אם CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#endif
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#אם CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#endif
	WATCHDOG_RESET();
#אם CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#endif
	WATCHDOG_RESET();
#אם CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#endif
#אם CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#endif
	INITCALL(power_init_board);
#אם CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#endif
	WATCHDOG_RESET();
#אם IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#endif
#אם CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#endif
#אם CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#endif
#אם CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#endif
#אם CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#endif
#אם CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#endif
#אם CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#endif
	INITCALL(initr_env);
#אם CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#endif
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#אם CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#endif
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#אם CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#endif
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#אם CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#endif
	INITCALL(console_init_r);	/* fully init console as a device */
#אם CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#endif
	/* miscellaneous arch-dependent init */
#אם CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#endif
	/* miscellaneous platform-dependent init */
#אם CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#endif
	WATCHDOG_RESET();
#אם CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#endif
	INITCALL(interrupt_init);
#אם defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#endif
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#אם CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#endif
#אם CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#endif
#אם CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#endif
#אם CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#endif
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#אם defined(CFG_PRAM)
	INITCALL(initr_mem);
#endif
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

فراغ board_init_r(gd_t *new_gd, ulong dest_addr)
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
	אם (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#אם defined(CONFIG_RISCV)
	set_gd(new_gd);
#وإلاإذا !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
