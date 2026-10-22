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
#که_تعریف_شوی CONFIG_MTD_NOR_FLASH
#ضمن <flash.h>
#د_که_پای
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

DECLARE_GLOBAL_معلومات_PTR;

ulong monitor_flash_len;

__weak صحيح board_flash_wp_on(فراغ)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	أرجع 0;
}

__weak صحيح cpu_secondary_init_r(فراغ)
{
	أرجع 0;
}

ساكن صحيح initr_trace(فراغ)
{
#که_تعریف_شوی CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_بفر_اندازه);
#د_که_پای

	أرجع 0;
}

ساكن صحيح initr_reloc(فراغ)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_ډک_MALLOC_INIT;

	أرجع 0;
}

#إذا defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ساكن صحيح initr_caches(فراغ)
{
	/* Enable caches */
	enable_caches();
	أرجع 0;
}
#د_که_پای

__weak صحيح fixup_cpu(فراغ)
{
	أرجع 0;
}

ساكن صحيح initr_reloc_global_معلومات(فراغ)
{
#که_تعریف_شوی __ARM__
	monitor_flash_len = پای_ټکی - __image_copy_start;
#وإلاإذا defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)پای_ټکی - (ulong)پیل_2;
#وإلاإذا !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#د_که_پای
#إذا defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_بنسټ;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#د_که_پای
#که_تعریف_شوی CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#د_که_پای

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	إذا (IS_ENABLED(CONFIG_د_EMBED))
		fdtdec_تیاری_embed();

#که_تعریف_شوی CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	إذا (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, عدم);

#د_که_پای
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	إذا (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_سېټ_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_سېټ_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_سېټ_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	أرجع 0;
}

__weak صحيح arch_initr_trap(فراغ)
{
	أرجع 0;
}

#إذا defined(CONFIG_SYS_INIT_RAM_قلف) && defined(CONFIG_E500)
ساكن صحيح initr_unlock_ram_دننه_cache(فراغ)
{
	unlock_ram_دننه_cache();	/* it's time to unlock D-cache in e500 */
	أرجع 0;
}
#د_که_پای

ساكن صحيح initr_barrier(فراغ)
{
#که_تعریف_شوی CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#د_که_پای
	أرجع 0;
}

ساكن صحيح initr_malloc(فراغ)
{
	ulong پیل;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#د_که_پای
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	پیل = gd->relocaddr - ټول_MALLOC_LEN;
	gd_سېټ_malloc_پیل(پیل);
	mem_malloc_init(پیل, ټول_MALLOC_LEN);
	أرجع 0;
}

ساكن صحيح initr_د_ژوندی(فراغ)
{
	إذا (CONFIG_IS_ENABLED(د_ژوندی)) {
		صحيح ret;

		bootstage_پیل(BOOTSTAGE_ID_ACCUM_د_ژوندی, "of_live");
		ret = د_ژوندی_build(gd->fdt_blob,
				    (بنية device_غوټه **)gd_د_ریښه_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_د_ژوندی);
		إذا (ret)
			أرجع ret;
	}

	أرجع 0;
}

#که_تعریف_شوی CONFIG_DM
ساكن صحيح initr_dm(فراغ)
{
	صحيح ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_ریښه = عدم;
#که_تعریف_شوی CONFIG_TIMER
	gd->timer = عدم;
#د_که_پای
	bootstage_پیل(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_لټون(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	إذا (ret)
		أرجع ret;

	أرجع dm_autoprobe();
}
#د_که_پای

ساكن صحيح initr_dm_devices(فراغ)
{
	صحيح ret;

	إذا (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		إذا (ret)
			أرجع ret;
	}

	إذا (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		إذا (ret)
			أرجع ret;
	}

	أرجع 0;
}

ساكن صحيح initr_bootstage(فراغ)
{
	bootstage_mark_name(BOOTSTAGE_ID_پیل_UBOOT_R, "board_init_r");

	أرجع 0;
}

__weak صحيح power_init_board(فراغ)
{
	أرجع 0;
}

ساكن صحيح initr_announce(فراغ)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	أرجع 0;
}

ساكن صحيح __maybe_unused initr_binman(فراغ)
{
	صحيح ret;

	ret = binman_init();
	إذا (ret)
		چاپ_کړه("binman_init failed:%d\n", ret);

	أرجع ret;
}

#إذا defined(CONFIG_MTD_NOR_FLASH)
__weak صحيح is_flash_شته(فراغ)
{
	أرجع 1;
}

ساكن صحيح initr_flash(فراغ)
{
	ulong flash_اندازه = 0;
	بنية bd_info *bd = gd->bd;

	إذا (!is_flash_شته())
		أرجع 0;

	puts("Flash: ");

	إذا (board_flash_wp_on())
		چاپ_کړه("Uninitialized - Write Protect On\n");
	وإلا
		flash_اندازه = flash_init();

	print_اندازه(flash_اندازه, "");
#که_تعریف_شوی CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	إذا (env_get_yesno("flashchecksum") == 1) {
		ثابت uchar *flash_بنسټ = (ثابت uchar *)CFG_SYS_FLASH_بنسټ;

		چاپ_کړه("  CRC: %08X", دوراني_ازموینه_32(0,
					    flash_بنسټ,
					    flash_اندازه));
	}
#د_که_پای /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#که_تعریف_شوی CFG_SYS_FLASH_بنسټ
	bd->bi_flashstart = CFG_SYS_FLASH_بنسټ;
#د_که_پای
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_اندازه;

#إذا defined(CONFIG_SYS_UPDATE_FLASH_اندازه)
	/* Make a update of the Memctrl. */
	update_flash_اندازه(flash_اندازه);
#د_که_پای

#إذا defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_متن_بنسټ + flash_اندازه;
#وإلاإذا CONFIG_SYS_MONITOR_بنسټ == CFG_SYS_FLASH_بنسټ
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#د_که_پای
	أرجع 0;
}
#د_که_پای

#که_تعریف_شوی CONFIG_CMD_NAND
/* go init the NAND */
ساكن صحيح initr_nand(فراغ)
{
	puts("NAND:  ");
	nand_init();
	چاپ_کړه("%lu MiB\n", nand_اندازه() / 1024);
	أرجع 0;
}
#د_که_پای

#إذا defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ساكن صحيح initr_onenand(فراغ)
{
	puts("NAND:  ");
	onenand_init();
	أرجع 0;
}
#د_که_پای

#که_تعریف_شوی CONFIG_MMC
ساكن صحيح initr_mmc(فراغ)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	أرجع 0;
}
#د_که_پای

#که_تعریف_شوی CONFIG_PVBLOCK
ساكن صحيح initr_pvblock(فراغ)
{
	puts("PVBLOCK: ");
	pvblock_init();
	أرجع 0;
}
#د_که_پای

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
ساكن صحيح should_بارول_env(فراغ)
{
	إذا (IS_ENABLED(CONFIG_د_CONTROL))
		أرجع ofnode_conf_read_int("load-environment", 1);

	أرجع 1;
}

ساكن صحيح initr_env(فراغ)
{
	/* initialize environment */
	إذا (should_بارول_env())
		env_relocate();
	وإلا
		env_سېټ_default(عدم, 0);

	env_import_fdt();

	إذا (IS_ENABLED(CONFIG_د_CONTROL))
		env_سېټ_hex("fdtcontroladdr",
			    (بینښه اوږد)map_منزل_sysmem(gd->fdt_blob));

	#إذا (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_پیل_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_معلومات();
	#د_که_پای

	/* Initialize from environment */
	image_بارول_addr = env_get_ulong("loadaddr", 16, image_بارول_addr);

	أرجع 0;
}

#که_تعریف_شوی CONFIG_SYS_MALLOC_BOOTPARAMS
ساكن صحيح initr_malloc_bootparams(فراغ)
{
	gd->bd->bi_boot_params = (ulong)حافظه_ورکړه(CONFIG_SYS_BOOTPARAMS_LEN);
	إذا (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		أرجع -ENOMEM;
	}
	أرجع 0;
}
#د_که_پای

ساكن صحيح initr_boot_led_blink(فراغ)
{
	led_boot_blink();

	أرجع 0;
}

ساكن صحيح initr_boot_led_on(فراغ)
{
	led_boot_on();

	أرجع 0;
}

#إذا CONFIG_IS_ENABLED(NET)
ساكن صحيح initr_net(فراغ)
{
	puts("Net:   ");
	eth_initialize();
#إذا defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#د_که_پای
	أرجع 0;
}
#د_که_پای

#که_تعریف_شوی CONFIG_POST
ساكن صحيح initr_post(فراغ)
{
	post_پرلپسې(عدم, POST_RAM | post_bootmode_get(0));
	أرجع 0;
}
#د_که_پای

#إذا defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
صحيح initr_mem(فراغ)
{
	ulong pram = 0;
	حرف memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (اوږد صحيح)((gd->ram_اندازه / 1024) - pram));
	env_سېټ("mem", memsz);

	أرجع 0;
}
#د_که_پای

ساكن صحيح initr_lmb(فراغ)
{
	إذا (CONFIG_IS_ENABLED(LMB))
		أرجع lmb_init();
	وإلا
		أرجع 0;
}

ساكن صحيح dm_announce(فراغ)
{
	صحيح device_شمېر;
	صحيح uclass_شمېر;

	إذا (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_شمېر, &uclass_شمېر);
		چاپ_کړه("Core:  %d devices, %d uclasses", device_شمېر,
		       uclass_شمېر);
		إذا (CONFIG_IS_ENABLED(د_REAL))
			چاپ_کړه(", devicetree: %s", fdtdec_get_srcname());
		إذا (CONFIG_IS_ENABLED(UPL))
			چاپ_کړه(", universal payload active");
		چاپ_کړه("\n");
		إذا (IS_ENABLED(CONFIG_د_HAS_مخکینی_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			چاپ_کړه("Warning: Unexpected devicetree source (not from a prior stage)");
			چاپ_کړه("Warning: U-Boot may not function properly\n");
		}
	}

	أرجع 0;
}

ساكن صحيح پرلپسې_main_loop(فراغ)
{
	صحيح ret;

#که_تعریف_شوی CONFIG_SANDBOX
	sandbox_main_loop_init();
#د_که_پای

	ret = event_notify_null(EVT_MAIN_LOOP);
	إذا (ret)
		أرجع ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	لكل (;;)
		main_loop();
	أرجع 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ساكن فراغ initcall_پرلپسې_r(فراغ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#إذا CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#د_که_پای
	INITCALL(initr_reloc_global_معلومات);
#إذا CONFIG_IS_ENABLED(SYS_INIT_RAM_قلف) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_دننه_cache);
#د_که_پای
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#إذا CONFIG_IS_ENABLED(CONSOLE_ریکارډ)
	INITCALL(console_ریکارډ_init);
#د_که_پای
#إذا CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_حافظه)
	INITCALL(noncached_init);
#د_که_پای
	INITCALL(initr_د_ژوندی);
#إذا CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#د_که_پای
#إذا CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#د_که_پای
#إذا CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#د_که_پای
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#إذا CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(سېټ_cpu_clk_info);
#د_که_پای
	INITCALL(initr_lmb);
#إذا CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_حافظه_init);
#د_که_پای
#إذا CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#د_که_پای
#إذا CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#د_که_پای
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#إذا CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#د_که_پای
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#إذا CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#د_که_پای
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#د_که_پای
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#د_که_پای
#إذا CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#د_که_پای
	INITCALL(power_init_board);
#إذا CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#د_که_پای
	WATCHDOG_RESET();
#إذا IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#د_که_پای
#إذا CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#د_که_پای
#إذا CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#د_که_پای
#إذا CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#د_که_پای
#إذا CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#د_که_پای
#إذا CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#د_که_پای
#إذا CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#د_که_پای
	INITCALL(initr_env);
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#د_که_پای
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#إذا CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#د_که_پای
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#د_که_پای
	INITCALL(stdio_جمع_کړه_devices);
	INITCALL(jumptable_init);
#إذا CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#د_که_پای
	INITCALL(console_init_r);	/* fully init console as a device */
#إذا CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#د_که_پای
	/* miscellaneous arch-dependent init */
#إذا CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#د_که_پای
	/* miscellaneous platform-dependent init */
#إذا CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#د_که_پای
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#د_که_پای
	INITCALL(مداخله_init);
#إذا defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#د_که_پای
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#إذا CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#د_که_پای
#إذا CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#د_که_پای
#إذا CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#د_که_پای
#إذا CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#د_که_پای
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_وروستی_STAGE_INIT);
#إذا defined(CFG_PRAM)
	INITCALL(initr_mem);
#د_که_پای
	INITCALL(initr_boot_led_on);
	INITCALL(پرلپسې_main_loop);
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
	إذا (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_تیاری_gd(new_gd);

#إذا defined(CONFIG_RISCV)
	سېټ_gd(new_gd);
#وإلاإذا !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#د_که_پای
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_پرلپسې_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
