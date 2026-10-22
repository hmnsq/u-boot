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
#جيڪڏهن_وصف_ٿيل CONFIG_MTD_NOR_FLASH
#ضمن <flash.h>
#جيڪڏهن_پورو
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
#جيڪڏهن_وصف_ٿيل CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_بفر_سائيز);
#جيڪڏهن_پورو

	أرجع 0;
}

ساكن صحيح initr_reloc(فراغ)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_ڀريل_MALLOC_INIT;

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
#جيڪڏهن_پورو

__weak صحيح fixup_cpu(فراغ)
{
	أرجع 0;
}

ساكن صحيح initr_reloc_global_معلومات(فراغ)
{
#جيڪڏهن_وصف_ٿيل __ARM__
	monitor_flash_len = پڇاڙي - __image_copy_start;
#وإلاإذا defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)پڇاڙي - (ulong)شروعات_2;
#وإلاإذا !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#جيڪڏهن_پورو
#إذا defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_بنياد;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#جيڪڏهن_پورو

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	إذا (IS_ENABLED(CONFIG_جو_EMBED))
		fdtdec_تياري_embed();

#جيڪڏهن_وصف_ٿيل CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	إذا (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, عدم);

#جيڪڏهن_پورو
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	إذا (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_سيٽ_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_سيٽ_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_سيٽ_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	أرجع 0;
}

__weak صحيح arch_initr_trap(فراغ)
{
	أرجع 0;
}

#إذا defined(CONFIG_SYS_INIT_RAM_تالو) && defined(CONFIG_E500)
ساكن صحيح initr_unlock_ram_اندريون_cache(فراغ)
{
	unlock_ram_اندريون_cache();	/* it's time to unlock D-cache in e500 */
	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح initr_barrier(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#جيڪڏهن_پورو
	أرجع 0;
}

ساكن صحيح initr_malloc(فراغ)
{
	ulong شروعات;

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#جيڪڏهن_پورو
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	شروعات = gd->relocaddr - ڪل_MALLOC_LEN;
	gd_سيٽ_malloc_شروعات(شروعات);
	mem_malloc_init(شروعات, ڪل_MALLOC_LEN);
	أرجع 0;
}

ساكن صحيح initr_جو_جيئرو(فراغ)
{
	إذا (CONFIG_IS_ENABLED(جو_جيئرو)) {
		صحيح ret;

		bootstage_شروعات(BOOTSTAGE_ID_ACCUM_جو_جيئرو, "of_live");
		ret = جو_جيئرو_build(gd->fdt_blob,
				    (بنية device_ڳنڍ **)gd_جو_پاڙ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_جو_جيئرو);
		إذا (ret)
			أرجع ret;
	}

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_DM
ساكن صحيح initr_dm(فراغ)
{
	صحيح ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_پاڙ = عدم;
#جيڪڏهن_وصف_ٿيل CONFIG_TIMER
	gd->timer = عدم;
#جيڪڏهن_پورو
	bootstage_شروعات(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_جاچ(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	إذا (ret)
		أرجع ret;

	أرجع dm_autoprobe();
}
#جيڪڏهن_پورو

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
	bootstage_mark_name(BOOTSTAGE_ID_شروعات_UBOOT_R, "board_init_r");

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
		لکو("binman_init failed:%d\n", ret);

	أرجع ret;
}

#إذا defined(CONFIG_MTD_NOR_FLASH)
__weak صحيح is_flash_موجود(فراغ)
{
	أرجع 1;
}

ساكن صحيح initr_flash(فراغ)
{
	ulong flash_سائيز = 0;
	بنية bd_info *bd = gd->bd;

	إذا (!is_flash_موجود())
		أرجع 0;

	puts("Flash: ");

	إذا (board_flash_wp_on())
		لکو("Uninitialized - Write Protect On\n");
	وإلا
		flash_سائيز = flash_init();

	print_سائيز(flash_سائيز, "");
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	إذا (env_get_yesno("flashchecksum") == 1) {
		ثابت uchar *flash_بنياد = (ثابت uchar *)CFG_SYS_FLASH_بنياد;

		لکو("  CRC: %08X", چڪري_جاچ_32(0,
					    flash_بنياد,
					    flash_سائيز));
	}
#جيڪڏهن_پورو /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#جيڪڏهن_وصف_ٿيل CFG_SYS_FLASH_بنياد
	bd->bi_flashstart = CFG_SYS_FLASH_بنياد;
#جيڪڏهن_پورو
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_سائيز;

#إذا defined(CONFIG_SYS_UPDATE_FLASH_سائيز)
	/* Make a update of the Memctrl. */
	update_flash_سائيز(flash_سائيز);
#جيڪڏهن_پورو

#إذا defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_متن_بنياد + flash_سائيز;
#وإلاإذا CONFIG_SYS_MONITOR_بنياد == CFG_SYS_FLASH_بنياد
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#جيڪڏهن_پورو
	أرجع 0;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_CMD_NAND
/* go init the NAND */
ساكن صحيح initr_nand(فراغ)
{
	puts("NAND:  ");
	nand_init();
	لکو("%lu MiB\n", nand_سائيز() / 1024);
	أرجع 0;
}
#جيڪڏهن_پورو

#إذا defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ساكن صحيح initr_onenand(فراغ)
{
	puts("NAND:  ");
	onenand_init();
	أرجع 0;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_MMC
ساكن صحيح initr_mmc(فراغ)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	أرجع 0;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_PVBLOCK
ساكن صحيح initr_pvblock(فراغ)
{
	puts("PVBLOCK: ");
	pvblock_init();
	أرجع 0;
}
#جيڪڏهن_پورو

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
ساكن صحيح should_ڀريو_env(فراغ)
{
	إذا (IS_ENABLED(CONFIG_جو_CONTROL))
		أرجع ofnode_conf_read_int("load-environment", 1);

	أرجع 1;
}

ساكن صحيح initr_env(فراغ)
{
	/* initialize environment */
	إذا (should_ڀريو_env())
		env_relocate();
	وإلا
		env_سيٽ_default(عدم, 0);

	env_import_fdt();

	إذا (IS_ENABLED(CONFIG_جو_CONTROL))
		env_سيٽ_hex("fdtcontroladdr",
			    (بنانشان ڊگهو)map_منزل_sysmem(gd->fdt_blob));

	#إذا (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_شروعات_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_معلومات();
	#جيڪڏهن_پورو

	/* Initialize from environment */
	image_ڀريو_addr = env_get_ulong("loadaddr", 16, image_ڀريو_addr);

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_MALLOC_BOOTPARAMS
ساكن صحيح initr_malloc_bootparams(فراغ)
{
	gd->bd->bi_boot_params = (ulong)يادگيري_ڏيو(CONFIG_SYS_BOOTPARAMS_LEN);
	إذا (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		أرجع -ENOMEM;
	}
	أرجع 0;
}
#جيڪڏهن_پورو

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
#جيڪڏهن_پورو
	أرجع 0;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_POST
ساكن صحيح initr_post(فراغ)
{
	post_لاڳيتو(عدم, POST_RAM | post_bootmode_get(0));
	أرجع 0;
}
#جيڪڏهن_پورو

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
	sprintf(memsz, "%ldk", (ڊگهو صحيح)((gd->ram_سائيز / 1024) - pram));
	env_سيٽ("mem", memsz);

	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح initr_lmb(فراغ)
{
	إذا (CONFIG_IS_ENABLED(LMB))
		أرجع lmb_init();
	وإلا
		أرجع 0;
}

ساكن صحيح dm_announce(فراغ)
{
	صحيح device_ڳڻپ;
	صحيح uclass_ڳڻپ;

	إذا (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_ڳڻپ, &uclass_ڳڻپ);
		لکو("Core:  %d devices, %d uclasses", device_ڳڻپ,
		       uclass_ڳڻپ);
		إذا (CONFIG_IS_ENABLED(جو_REAL))
			لکو(", devicetree: %s", fdtdec_get_srcname());
		إذا (CONFIG_IS_ENABLED(UPL))
			لکو(", universal payload active");
		لکو("\n");
		إذا (IS_ENABLED(CONFIG_جو_HAS_اڳواٽ_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			لکو("Warning: Unexpected devicetree source (not from a prior stage)");
			لکو("Warning: U-Boot may not function properly\n");
		}
	}

	أرجع 0;
}

ساكن صحيح لاڳيتو_main_loop(فراغ)
{
	صحيح ret;

#جيڪڏهن_وصف_ٿيل CONFIG_SANDBOX
	sandbox_main_loop_init();
#جيڪڏهن_پورو

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

ساكن فراغ initcall_لاڳيتو_r(فراغ)
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
#جيڪڏهن_پورو
	INITCALL(initr_reloc_global_معلومات);
#إذا CONFIG_IS_ENABLED(SYS_INIT_RAM_تالو) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_اندريون_cache);
#جيڪڏهن_پورو
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#إذا CONFIG_IS_ENABLED(CONSOLE_رڪارڊ)
	INITCALL(console_رڪارڊ_init);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_يادگيري)
	INITCALL(noncached_init);
#جيڪڏهن_پورو
	INITCALL(initr_جو_جيئرو);
#إذا CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#جيڪڏهن_پورو
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#إذا CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(سيٽ_cpu_clk_info);
#جيڪڏهن_پورو
	INITCALL(initr_lmb);
#إذا CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_يادگيري_init);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#جيڪڏهن_پورو
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#إذا CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#إذا CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#جيڪڏهن_پورو
	INITCALL(power_init_board);
#إذا CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
#إذا IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#جيڪڏهن_پورو
	INITCALL(initr_env);
#إذا CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#إذا CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#جيڪڏهن_پورو
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#جيڪڏهن_پورو
	INITCALL(stdio_جوڙيو_devices);
	INITCALL(jumptable_init);
#إذا CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#جيڪڏهن_پورو
	INITCALL(console_init_r);	/* fully init console as a device */
#إذا CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#جيڪڏهن_پورو
	/* miscellaneous arch-dependent init */
#إذا CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#جيڪڏهن_پورو
	/* miscellaneous platform-dependent init */
#إذا CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#جيڪڏهن_پورو
	INITCALL(مداخلت_init);
#إذا defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#جيڪڏهن_پورو
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#إذا CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_آخري_STAGE_INIT);
#إذا defined(CFG_PRAM)
	INITCALL(initr_mem);
#جيڪڏهن_پورو
	INITCALL(initr_boot_led_on);
	INITCALL(لاڳيتو_main_loop);
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
		arch_تياري_gd(new_gd);

#إذا defined(CONFIG_RISCV)
	سيٽ_gd(new_gd);
#وإلاإذا !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#جيڪڏهن_پورو
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_لاڳيتو_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
