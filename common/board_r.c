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
#اگر_تعریف_شدہ CONFIG_MTD_NOR_FLASH
#شامل <flash.h>
#اگر_ختم
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

DECLARE_GLOBAL_ڈیٹا_PTR;

ulong monitor_flash_len;

__weak صحیح board_flash_wp_on(خالی)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	واپس 0;
}

__weak صحیح cpu_secondary_init_r(خالی)
{
	واپس 0;
}

ساکن صحیح initr_trace(خالی)
{
#اگر_تعریف_شدہ CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_بفر_سائز);
#اگر_ختم

	واپس 0;
}

ساکن صحیح initr_reloc(خالی)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_بھرا_MALLOC_INIT;

	واپس 0;
}

#اگر defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ساکن صحیح initr_caches(خالی)
{
	/* Enable caches */
	enable_caches();
	واپس 0;
}
#اگر_ختم

__weak صحیح fixup_cpu(خالی)
{
	واپس 0;
}

ساکن صحیح initr_reloc_global_ڈیٹا(خالی)
{
#اگر_تعریف_شدہ __ARM__
	monitor_flash_len = انتہا - __image_copy_start;
#وگرنہ_اگر defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)انتہا - (ulong)آغاز_2;
#وگرنہ_اگر !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#اگر_ختم
#اگر defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_بنیاد;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#اگر_ختم
#اگر_تعریف_شدہ CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#اگر_ختم

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	اگر (IS_ENABLED(CONFIG_کا_EMBED))
		fdtdec_تیاری_embed();

#اگر_تعریف_شدہ CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	اگر (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, تہی);

#اگر_ختم
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	اگر (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_سیٹ_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_سیٹ_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_سیٹ_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	واپس 0;
}

__weak صحیح arch_initr_trap(خالی)
{
	واپس 0;
}

#اگر defined(CONFIG_SYS_INIT_RAM_تالا) && defined(CONFIG_E500)
ساکن صحیح initr_unlock_ram_اندر_cache(خالی)
{
	unlock_ram_اندر_cache();	/* it's time to unlock D-cache in e500 */
	واپس 0;
}
#اگر_ختم

ساکن صحیح initr_barrier(خالی)
{
#اگر_تعریف_شدہ CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#اگر_ختم
	واپس 0;
}

ساکن صحیح initr_malloc(خالی)
{
	ulong آغاز;

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#اگر_ختم
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	آغاز = gd->relocaddr - کل_MALLOC_LEN;
	gd_سیٹ_malloc_آغاز(آغاز);
	mem_malloc_init(آغاز, کل_MALLOC_LEN);
	واپس 0;
}

ساکن صحیح initr_کا_زندہ(خالی)
{
	اگر (CONFIG_IS_ENABLED(کا_زندہ)) {
		صحیح ret;

		bootstage_آغاز(BOOTSTAGE_ID_ACCUM_کا_زندہ, "of_live");
		ret = کا_زندہ_build(gd->fdt_blob,
				    (ساخت device_گرہ **)gd_کا_جڑ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_کا_زندہ);
		اگر (ret)
			واپس ret;
	}

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_DM
ساکن صحیح initr_dm(خالی)
{
	صحیح ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_جڑ = تہی;
#اگر_تعریف_شدہ CONFIG_TIMER
	gd->timer = تہی;
#اگر_ختم
	bootstage_آغاز(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_جائزہ(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	اگر (ret)
		واپس ret;

	واپس dm_autoprobe();
}
#اگر_ختم

ساکن صحیح initr_dm_devices(خالی)
{
	صحیح ret;

	اگر (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		اگر (ret)
			واپس ret;
	}

	اگر (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		اگر (ret)
			واپس ret;
	}

	واپس 0;
}

ساکن صحیح initr_bootstage(خالی)
{
	bootstage_mark_name(BOOTSTAGE_ID_آغاز_UBOOT_R, "board_init_r");

	واپس 0;
}

__weak صحیح power_init_board(خالی)
{
	واپس 0;
}

ساکن صحیح initr_announce(خالی)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	واپس 0;
}

ساکن صحیح __maybe_unused initr_binman(خالی)
{
	صحیح ret;

	ret = binman_init();
	اگر (ret)
		لکھو("binman_init failed:%d\n", ret);

	واپس ret;
}

#اگر defined(CONFIG_MTD_NOR_FLASH)
__weak صحیح is_flash_دستیاب(خالی)
{
	واپس 1;
}

ساکن صحیح initr_flash(خالی)
{
	ulong flash_سائز = 0;
	ساخت bd_info *bd = gd->bd;

	اگر (!is_flash_دستیاب())
		واپس 0;

	puts("Flash: ");

	اگر (board_flash_wp_on())
		لکھو("Uninitialized - Write Protect On\n");
	ورنہ
		flash_سائز = flash_init();

	print_سائز(flash_سائز, "");
#اگر_تعریف_شدہ CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	اگر (env_get_yesno("flashchecksum") == 1) {
		ثابت uchar *flash_بنیاد = (ثابت uchar *)CFG_SYS_FLASH_بنیاد;

		لکھو("  CRC: %08X", چکری_جانچ_32(0,
					    flash_بنیاد,
					    flash_سائز));
	}
#اگر_ختم /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#اگر_تعریف_شدہ CFG_SYS_FLASH_بنیاد
	bd->bi_flashstart = CFG_SYS_FLASH_بنیاد;
#اگر_ختم
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_سائز;

#اگر defined(CONFIG_SYS_UPDATE_FLASH_سائز)
	/* Make a update of the Memctrl. */
	update_flash_سائز(flash_سائز);
#اگر_ختم

#اگر defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_متن_بنیاد + flash_سائز;
#وگرنہ_اگر CONFIG_SYS_MONITOR_بنیاد == CFG_SYS_FLASH_بنیاد
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#اگر_ختم
	واپس 0;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_CMD_NAND
/* go init the NAND */
ساکن صحیح initr_nand(خالی)
{
	puts("NAND:  ");
	nand_init();
	لکھو("%lu MiB\n", nand_سائز() / 1024);
	واپس 0;
}
#اگر_ختم

#اگر defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ساکن صحیح initr_onenand(خالی)
{
	puts("NAND:  ");
	onenand_init();
	واپس 0;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_MMC
ساکن صحیح initr_mmc(خالی)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	واپس 0;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_PVBLOCK
ساکن صحیح initr_pvblock(خالی)
{
	puts("PVBLOCK: ");
	pvblock_init();
	واپس 0;
}
#اگر_ختم

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
ساکن صحیح should_بھرو_env(خالی)
{
	اگر (IS_ENABLED(CONFIG_کا_CONTROL))
		واپس ofnode_conf_read_int("load-environment", 1);

	واپس 1;
}

ساکن صحیح initr_env(خالی)
{
	/* initialize environment */
	اگر (should_بھرو_env())
		env_relocate();
	ورنہ
		env_سیٹ_default(تہی, 0);

	env_import_fdt();

	اگر (IS_ENABLED(CONFIG_کا_CONTROL))
		env_سیٹ_hex("fdtcontroladdr",
			    (غیرعلامتی طویل)map_مقصد_sysmem(gd->fdt_blob));

	#اگر (IS_ENABLED(CONFIG_SAVE_پچھلا_BL_INITRAMFS_آغاز_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_پچھلا_BL_FDT_ADDR))
		save_پچھلا_bl_ڈیٹا();
	#اگر_ختم

	/* Initialize from environment */
	image_بھرو_addr = env_get_ulong("loadaddr", 16, image_بھرو_addr);

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_SYS_MALLOC_BOOTPARAMS
ساکن صحیح initr_malloc_bootparams(خالی)
{
	gd->bd->bi_boot_params = (ulong)حافظہ_دو(CONFIG_SYS_BOOTPARAMS_LEN);
	اگر (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		واپس -ENOMEM;
	}
	واپس 0;
}
#اگر_ختم

ساکن صحیح initr_boot_led_blink(خالی)
{
	led_boot_blink();

	واپس 0;
}

ساکن صحیح initr_boot_led_on(خالی)
{
	led_boot_on();

	واپس 0;
}

#اگر CONFIG_IS_ENABLED(NET)
ساکن صحیح initr_net(خالی)
{
	puts("Net:   ");
	eth_initialize();
#اگر defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#اگر_ختم
	واپس 0;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_POST
ساکن صحیح initr_post(خالی)
{
	post_تسلسل(تہی, POST_RAM | post_bootmode_get(0));
	واپس 0;
}
#اگر_ختم

#اگر defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
صحیح initr_mem(خالی)
{
	ulong pram = 0;
	حرف memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (طویل صحیح)((gd->ram_سائز / 1024) - pram));
	env_سیٹ("mem", memsz);

	واپس 0;
}
#اگر_ختم

ساکن صحیح initr_lmb(خالی)
{
	اگر (CONFIG_IS_ENABLED(LMB))
		واپس lmb_init();
	ورنہ
		واپس 0;
}

ساکن صحیح dm_announce(خالی)
{
	صحیح device_گنتی;
	صحیح uclass_گنتی;

	اگر (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_گنتی, &uclass_گنتی);
		لکھو("Core:  %d devices, %d uclasses", device_گنتی,
		       uclass_گنتی);
		اگر (CONFIG_IS_ENABLED(کا_REAL))
			لکھو(", devicetree: %s", fdtdec_get_srcname());
		اگر (CONFIG_IS_ENABLED(UPL))
			لکھو(", universal payload active");
		لکھو("\n");
		اگر (IS_ENABLED(CONFIG_کا_HAS_پیشگی_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			لکھو("Warning: Unexpected devicetree source (not from a prior stage)");
			لکھو("Warning: U-Boot may not function properly\n");
		}
	}

	واپس 0;
}

ساکن صحیح تسلسل_main_loop(خالی)
{
	صحیح ret;

#اگر_تعریف_شدہ CONFIG_SANDBOX
	sandbox_main_loop_init();
#اگر_ختم

	ret = event_notify_null(EVT_MAIN_LOOP);
	اگر (ret)
		واپس ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	برائے (;;)
		main_loop();
	واپس 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ساکن خالی initcall_تسلسل_r(خالی)
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
#اگر_ختم
	INITCALL(initr_reloc_global_ڈیٹا);
#اگر CONFIG_IS_ENABLED(SYS_INIT_RAM_تالا) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_اندر_cache);
#اگر_ختم
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#اگر CONFIG_IS_ENABLED(CONSOLE_ریکارڈ)
	INITCALL(console_ریکارڈ_init);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_حافظہ)
	INITCALL(noncached_init);
#اگر_ختم
	INITCALL(initr_کا_زندہ);
#اگر CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#اگر_ختم
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#اگر CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(سیٹ_cpu_clk_info);
#اگر_ختم
	INITCALL(initr_lmb);
#اگر CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_حافظہ_init);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#اگر_ختم
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#اگر CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#اگر_ختم
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#اگر CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#اگر_ختم
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#اگر_ختم
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#اگر_ختم
	INITCALL(power_init_board);
#اگر CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#اگر_ختم
	WATCHDOG_RESET();
#اگر IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#اگر_ختم
	INITCALL(initr_env);
#اگر CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#اگر_ختم
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#اگر CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#اگر_ختم
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#اگر_ختم
	INITCALL(stdio_جوڑو_devices);
	INITCALL(jumptable_init);
#اگر CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#اگر_ختم
	INITCALL(console_init_r);	/* fully init console as a device */
#اگر CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#اگر_ختم
	/* miscellaneous arch-dependent init */
#اگر CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#اگر_ختم
	/* miscellaneous platform-dependent init */
#اگر CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#اگر_ختم
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#اگر_ختم
	INITCALL(مداخلت_init);
#اگر defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#اگر_ختم
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#اگر CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#اگر_ختم
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_آخری_STAGE_INIT);
#اگر defined(CFG_PRAM)
	INITCALL(initr_mem);
#اگر_ختم
	INITCALL(initr_boot_led_on);
	INITCALL(تسلسل_main_loop);
}

خالی board_init_r(gd_t *new_gd, ulong dest_addr)
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
		arch_تیاری_gd(new_gd);

#اگر defined(CONFIG_RISCV)
	سیٹ_gd(new_gd);
#وگرنہ_اگر !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#اگر_ختم
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_تسلسل_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
