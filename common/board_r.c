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

#అన్తర్భావ <config.h>
#అన్తర్భావ <api.h>
#అన్తర్భావ <bootstage.h>
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <cyclic.h>
#అన్తర్భావ <display_options.h>
#అన్తర్భావ <exports.h>
#నిర్వచితమైతే CONFIG_MTD_NOR_FLASH
#అన్తర్భావ <flash.h>
#అయితే_ముగింపు
#అన్తర్భావ <hang.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <irq_func.h>
#అన్తర్భావ <lmb.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <net.h>
#అన్తర్భావ <asm/cache.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <u-boot/crc.h>
#అన్తర్భావ <binman.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <efi_loader.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <env_internal.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <init.h>
#అన్తర్భావ <initcall.h>
#అన్తర్భావ <kgdb.h>
#అన్తర్భావ <irq_func.h>
#అన్తర్భావ <led.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <miiphy.h>
#అన్తర్భావ <mmc.h>
#అన్తర్భావ <mux.h>
#అన్తర్భావ <nand.h>
#అన్తర్భావ <of_live.h>
#అన్తర్భావ <onenand_uboot.h>
#అన్తర్భావ <pvblock.h>
#అన్తర్భావ <scsi.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <stdio_dev.h>
#అన్తర్భావ <timer.h>
#అన్తర్భావ <trace.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <xen.h>
#అన్తర్భావ <asm/sections.h>
#అన్తర్భావ <dm/root.h>
#అన్తర్భావ <dm/ofnode.h>
#అన్తర్భావ <linux/compiler.h>
#అన్తర్భావ <linux/err.h>
#అన్తర్భావ <wdt.h>
#అన్తర్భావ <asm-generic/gpio.h>
#అన్తర్భావ <relocate.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

ulong monitor_flash_len;

__weak పూర్ణాంకం board_flash_wp_on(శూన్య)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	తిరిగి 0;
}

__weak పూర్ణాంకం cpu_secondary_init_r(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_trace(శూన్య)
{
#నిర్వచితమైతే CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_బఫరు_కొలత);
#అయితే_ముగింపు

	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_reloc(శూన్య)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_నిండిన_MALLOC_INIT;

	తిరిగి 0;
}

#ఒకవేళ defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
స్థిర పూర్ణాంకం initr_caches(శూన్య)
{
	/* Enable caches */
	enable_caches();
	తిరిగి 0;
}
#అయితే_ముగింపు

__weak పూర్ణాంకం fixup_cpu(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_reloc_global_దత్తాంశం(శూన్య)
{
#నిర్వచితమైతే __ARM__
	monitor_flash_len = చివర - __image_copy_start;
#లేదావేళ defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)చివర - (ulong)ప్రారంభం_2;
#లేదావేళ !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#అయితే_ముగింపు
#ఒకవేళ defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_పునాది;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#అయితే_ముగింపు
#నిర్వచితమైతే CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#అయితే_ముగింపు

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_EMBED))
		fdtdec_అమరిక_embed();

#నిర్వచితమైతే CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	ఒకవేళ (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, ఖాళీ);

#అయితే_ముగింపు
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	ఒకవేళ (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_సమితి_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_సమితి_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_సమితి_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	తిరిగి 0;
}

__weak పూర్ణాంకం arch_initr_trap(శూన్య)
{
	తిరిగి 0;
}

#ఒకవేళ defined(CONFIG_SYS_INIT_RAM_తాళం) && defined(CONFIG_E500)
స్థిర పూర్ణాంకం initr_unlock_ram_లోపల_cache(శూన్య)
{
	unlock_ram_లోపల_cache();	/* it's time to unlock D-cache in e500 */
	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం initr_barrier(శూన్య)
{
#నిర్వచితమైతే CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#అయితే_ముగింపు
	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_malloc(శూన్య)
{
	ulong ప్రారంభం;

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#అయితే_ముగింపు
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	ప్రారంభం = gd->relocaddr - మొత్తం_MALLOC_LEN;
	gd_సమితి_malloc_ప్రారంభం(ప్రారంభం);
	mem_malloc_init(ప్రారంభం, మొత్తం_MALLOC_LEN);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_యొక్క_సజీవ(శూన్య)
{
	ఒకవేళ (CONFIG_IS_ENABLED(యొక్క_సజీవ)) {
		పూర్ణాంకం ret;

		bootstage_ప్రారంభం(BOOTSTAGE_ID_ACCUM_యొక్క_సజీవ, "of_live");
		ret = యొక్క_సజీవ_build(gd->fdt_blob,
				    (నిర్మాణం device_కణుపు **)gd_యొక్క_మూలం_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_యొక్క_సజీవ);
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_DM
స్థిర పూర్ణాంకం initr_dm(శూన్య)
{
	పూర్ణాంకం ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_మూలం = ఖాళీ;
#నిర్వచితమైతే CONFIG_TIMER
	gd->timer = ఖాళీ;
#అయితే_ముగింపు
	bootstage_ప్రారంభం(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_పరిశీలన(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	ఒకవేళ (ret)
		తిరిగి ret;

	తిరిగి dm_autoprobe();
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం initr_dm_devices(శూన్య)
{
	పూర్ణాంకం ret;

	ఒకవేళ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	ఒకవేళ (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_bootstage(శూన్య)
{
	bootstage_mark_name(BOOTSTAGE_ID_ప్రారంభం_UBOOT_R, "board_init_r");

	తిరిగి 0;
}

__weak పూర్ణాంకం power_init_board(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_announce(శూన్య)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం __maybe_unused initr_binman(శూన్య)
{
	పూర్ణాంకం ret;

	ret = binman_init();
	ఒకవేళ (ret)
		రాయి("binman_init failed:%d\n", ret);

	తిరిగి ret;
}

#ఒకవేళ defined(CONFIG_MTD_NOR_FLASH)
__weak పూర్ణాంకం is_flash_అందుబాటు(శూన్య)
{
	తిరిగి 1;
}

స్థిర పూర్ణాంకం initr_flash(శూన్య)
{
	ulong flash_కొలత = 0;
	నిర్మాణం bd_info *bd = gd->bd;

	ఒకవేళ (!is_flash_అందుబాటు())
		తిరిగి 0;

	puts("Flash: ");

	ఒకవేళ (board_flash_wp_on())
		రాయి("Uninitialized - Write Protect On\n");
	లేకపోతే
		flash_కొలత = flash_init();

	print_కొలత(flash_కొలత, "");
#నిర్వచితమైతే CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	ఒకవేళ (env_get_yesno("flashchecksum") == 1) {
		స్థిరరాశి uchar *flash_పునాది = (స్థిరరాశి uchar *)CFG_SYS_FLASH_పునాది;

		రాయి("  CRC: %08X", చక్రీయ_తనిఖీ_32(0,
					    flash_పునాది,
					    flash_కొలత));
	}
#అయితే_ముగింపు /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#నిర్వచితమైతే CFG_SYS_FLASH_పునాది
	bd->bi_flashstart = CFG_SYS_FLASH_పునాది;
#అయితే_ముగింపు
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_కొలత;

#ఒకవేళ defined(CONFIG_SYS_UPDATE_FLASH_కొలత)
	/* Make a update of the Memctrl. */
	update_flash_కొలత(flash_కొలత);
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_పాఠ్యం_పునాది + flash_కొలత;
#లేదావేళ CONFIG_SYS_MONITOR_పునాది == CFG_SYS_FLASH_పునాది
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#అయితే_ముగింపు
	తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_CMD_NAND
/* go init the NAND */
స్థిర పూర్ణాంకం initr_nand(శూన్య)
{
	puts("NAND:  ");
	nand_init();
	రాయి("%lu MiB\n", nand_కొలత() / 1024);
	తిరిగి 0;
}
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
స్థిర పూర్ణాంకం initr_onenand(శూన్య)
{
	puts("NAND:  ");
	onenand_init();
	తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_MMC
స్థిర పూర్ణాంకం initr_mmc(శూన్య)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_PVBLOCK
స్థిర పూర్ణాంకం initr_pvblock(శూన్య)
{
	puts("PVBLOCK: ");
	pvblock_init();
	తిరిగి 0;
}
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం should_ఎక్కించు_env(శూన్య)
{
	ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_CONTROL))
		తిరిగి ofnode_conf_read_int("load-environment", 1);

	తిరిగి 1;
}

స్థిర పూర్ణాంకం initr_env(శూన్య)
{
	/* initialize environment */
	ఒకవేళ (should_ఎక్కించు_env())
		env_relocate();
	లేకపోతే
		env_సమితి_default(ఖాళీ, 0);

	env_import_fdt();

	ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_CONTROL))
		env_సమితి_hex("fdtcontroladdr",
			    (గుర్తులేని దీర్ఘ)map_గమ్యం_sysmem(gd->fdt_blob));

	#ఒకవేళ (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_ప్రారంభం_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_దత్తాంశం();
	#అయితే_ముగింపు

	/* Initialize from environment */
	image_ఎక్కించు_addr = env_get_ulong("loadaddr", 16, image_ఎక్కించు_addr);

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_SYS_MALLOC_BOOTPARAMS
స్థిర పూర్ణాంకం initr_malloc_bootparams(శూన్య)
{
	gd->bd->bi_boot_params = (ulong)జ్ఞాపకం_ఇవ్వు(CONFIG_SYS_BOOTPARAMS_LEN);
	ఒకవేళ (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		తిరిగి -ENOMEM;
	}
	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం initr_boot_led_blink(శూన్య)
{
	led_boot_blink();

	తిరిగి 0;
}

స్థిర పూర్ణాంకం initr_boot_led_on(శూన్య)
{
	led_boot_on();

	తిరిగి 0;
}

#ఒకవేళ CONFIG_IS_ENABLED(NET)
స్థిర పూర్ణాంకం initr_net(శూన్య)
{
	puts("Net:   ");
	eth_initialize();
#ఒకవేళ defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#అయితే_ముగింపు
	తిరిగి 0;
}
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_POST
స్థిర పూర్ణాంకం initr_post(శూన్య)
{
	post_నిరంతరం(ఖాళీ, POST_RAM | post_bootmode_get(0));
	తిరిగి 0;
}
#అయితే_ముగింపు

#ఒకవేళ defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
పూర్ణాంకం initr_mem(శూన్య)
{
	ulong pram = 0;
	అక్షరం memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (దీర్ఘ పూర్ణాంకం)((gd->ram_కొలత / 1024) - pram));
	env_సమితి("mem", memsz);

	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం initr_lmb(శూన్య)
{
	ఒకవేళ (CONFIG_IS_ENABLED(LMB))
		తిరిగి lmb_init();
	లేకపోతే
		తిరిగి 0;
}

స్థిర పూర్ణాంకం dm_announce(శూన్య)
{
	పూర్ణాంకం device_లెక్క;
	పూర్ణాంకం uclass_లెక్క;

	ఒకవేళ (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_లెక్క, &uclass_లెక్క);
		రాయి("Core:  %d devices, %d uclasses", device_లెక్క,
		       uclass_లెక్క);
		ఒకవేళ (CONFIG_IS_ENABLED(యొక్క_REAL))
			రాయి(", devicetree: %s", fdtdec_get_srcname());
		ఒకవేళ (CONFIG_IS_ENABLED(UPL))
			రాయి(", universal payload active");
		రాయి("\n");
		ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_HAS_పూర్వ_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			రాయి("Warning: Unexpected devicetree source (not from a prior stage)");
			రాయి("Warning: U-Boot may not function properly\n");
		}
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం నిరంతరం_main_loop(శూన్య)
{
	పూర్ణాంకం ret;

#నిర్వచితమైతే CONFIG_SANDBOX
	sandbox_main_loop_init();
#అయితే_ముగింపు

	ret = event_notify_null(EVT_MAIN_LOOP);
	ఒకవేళ (ret)
		తిరిగి ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	కొరకు (;;)
		main_loop();
	తిరిగి 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

స్థిర శూన్య initcall_నిరంతరం_r(శూన్య)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#ఒకవేళ CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#అయితే_ముగింపు
	INITCALL(initr_reloc_global_దత్తాంశం);
#ఒకవేళ CONFIG_IS_ENABLED(SYS_INIT_RAM_తాళం) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_లోపల_cache);
#అయితే_ముగింపు
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_రికార్డు)
	INITCALL(console_రికార్డు_init);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_జ్ఞాపకం)
	INITCALL(noncached_init);
#అయితే_ముగింపు
	INITCALL(initr_యొక్క_సజీవ);
#ఒకవేళ CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#అయితే_ముగింపు
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ఒకవేళ CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(సమితి_cpu_clk_info);
#అయితే_ముగింపు
	INITCALL(initr_lmb);
#ఒకవేళ CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_జ్ఞాపకం_init);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#అయితే_ముగింపు
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#ఒకవేళ CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#అయితే_ముగింపు
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#ఒకవేళ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#అయితే_ముగింపు
	WATCHDOG_RESET();
#ఒకవేళ CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#అయితే_ముగింపు
	WATCHDOG_RESET();
#ఒకవేళ CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#అయితే_ముగింపు
	INITCALL(power_init_board);
#ఒకవేళ CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#అయితే_ముగింపు
	WATCHDOG_RESET();
#ఒకవేళ IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#అయితే_ముగింపు
	INITCALL(initr_env);
#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#అయితే_ముగింపు
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#ఒకవేళ CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#అయితే_ముగింపు
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#ఒకవేళ CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#అయితే_ముగింపు
	INITCALL(stdio_కూడు_devices);
	INITCALL(jumptable_init);
#ఒకవేళ CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#అయితే_ముగింపు
	INITCALL(console_init_r);	/* fully init console as a device */
#ఒకవేళ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#అయితే_ముగింపు
	/* miscellaneous arch-dependent init */
#ఒకవేళ CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#అయితే_ముగింపు
	/* miscellaneous platform-dependent init */
#ఒకవేళ CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#అయితే_ముగింపు
	WATCHDOG_RESET();
#ఒకవేళ CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#అయితే_ముగింపు
	INITCALL(అంతరాయం_init);
#ఒకవేళ defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#అయితే_ముగింపు
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#ఒకవేళ CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#అయితే_ముగింపు
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_చివరి_STAGE_INIT);
#ఒకవేళ defined(CFG_PRAM)
	INITCALL(initr_mem);
#అయితే_ముగింపు
	INITCALL(initr_boot_led_on);
	INITCALL(నిరంతరం_main_loop);
}

శూన్య board_init_r(gd_t *new_gd, ulong dest_addr)
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
	ఒకవేళ (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_అమరిక_gd(new_gd);

#ఒకవేళ defined(CONFIG_RISCV)
	సమితి_gd(new_gd);
#లేదావేళ !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#అయితే_ముగింపు
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_నిరంతరం_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
