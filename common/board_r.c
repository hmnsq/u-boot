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

#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <api.h>
#ଅନ୍ତର୍ଭାଵ <bootstage.h>
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <cyclic.h>
#ଅନ୍ତର୍ଭାଵ <display_options.h>
#ଅନ୍ତର୍ଭାଵ <exports.h>
#ifdef CONFIG_MTD_NOR_FLASH
#ଅନ୍ତର୍ଭାଵ <flash.h>
#endif
#ଅନ୍ତର୍ଭାଵ <hang.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <irq_func.h>
#ଅନ୍ତର୍ଭାଵ <lmb.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <net.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/crc.h>
#ଅନ୍ତର୍ଭାଵ <binman.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <efi_loader.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <env_internal.h>
#ଅନ୍ତର୍ଭାଵ <fdtdec.h>
#ଅନ୍ତର୍ଭାଵ <init.h>
#ଅନ୍ତର୍ଭାଵ <initcall.h>
#ଅନ୍ତର୍ଭାଵ <kgdb.h>
#ଅନ୍ତର୍ଭାଵ <irq_func.h>
#ଅନ୍ତର୍ଭାଵ <led.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <miiphy.h>
#ଅନ୍ତର୍ଭାଵ <mmc.h>
#ଅନ୍ତର୍ଭାଵ <mux.h>
#ଅନ୍ତର୍ଭାଵ <nand.h>
#ଅନ୍ତର୍ଭାଵ <of_live.h>
#ଅନ୍ତର୍ଭାଵ <onenand_uboot.h>
#ଅନ୍ତର୍ଭାଵ <pvblock.h>
#ଅନ୍ତର୍ଭାଵ <scsi.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <stdio_dev.h>
#ଅନ୍ତର୍ଭାଵ <timer.h>
#ଅନ୍ତର୍ଭାଵ <trace.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <xen.h>
#ଅନ୍ତର୍ଭାଵ <asm/sections.h>
#ଅନ୍ତର୍ଭାଵ <dm/root.h>
#ଅନ୍ତର୍ଭାଵ <dm/ofnode.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>
#ଅନ୍ତର୍ଭାଵ <linux/err.h>
#ଅନ୍ତର୍ଭାଵ <wdt.h>
#ଅନ୍ତର୍ଭାଵ <asm-generic/gpio.h>
#ଅନ୍ତର୍ଭାଵ <relocate.h>

DECLARE_GLOBAL_DATA_PTR;

ulong monitor_flash_len;

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା board_flash_wp_on(ଶୂନ୍ୟ)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା cpu_secondary_init_r(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_trace(ଶୂନ୍ୟ)
{
#ifdef CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_BUFFER_SIZE);
#endif

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_reloc(ଶୂନ୍ୟ)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_FULL_MALLOC_INIT;

	ଫେରନ୍ତୁ 0;
}

#ଯଦି defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_caches(ଶୂନ୍ୟ)
{
	/* Enable caches */
	enable_caches();
	ଫେରନ୍ତୁ 0;
}
#endif

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା fixup_cpu(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_reloc_global_data(ଶୂନ୍ୟ)
{
#ifdef __ARM__
	monitor_flash_len = _end - __image_copy_start;
#ନଚେତଯଦି defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)_end - (ulong)_start;
#ନଚେତଯଦି !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#endif
#ଯଦି defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
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
	ଯଦି (IS_ENABLED(CONFIG_OF_EMBED))
		fdtdec_setup_embed();

#ifdef CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	ଯଦି (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, ଖାଲି);

#endif
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	ଯଦି (IS_ENABLED(CONFIG_MMU_PGPROT)) {
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

	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା arch_initr_trap(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

#ଯଦି defined(CONFIG_SYS_INIT_RAM_LOCK) && defined(CONFIG_E500)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_unlock_ram_in_cache(ଶୂନ୍ୟ)
{
	unlock_ram_in_cache();	/* it's time to unlock D-cache in e500 */
	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_barrier(ଶୂନ୍ୟ)
{
#ifdef CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#endif
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_malloc(ଶୂନ୍ୟ)
{
	ulong start;

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
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
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_of_live(ଶୂନ୍ୟ)
{
	ଯଦି (CONFIG_IS_ENABLED(OF_LIVE)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (ସଂରଚନା device_ଗଣ୍ଠି **)gd_of_ମୂଳ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_DM
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_dm(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_ମୂଳ = ଖାଲି;
#ifdef CONFIG_TIMER
	gd->timer = ଖାଲି;
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_scan(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଫେରନ୍ତୁ dm_autoprobe();
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_dm_devices(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ଯଦି (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_bootstage(ଶୂନ୍ୟ)
{
	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_R, "board_init_r");

	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା power_init_board(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_announce(ଶୂନ୍ୟ)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused initr_binman(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = binman_init();
	ଯଦି (ret)
		printf("binman_init failed:%d\n", ret);

	ଫେରନ୍ତୁ ret;
}

#ଯଦି defined(CONFIG_MTD_NOR_FLASH)
__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_flash_available(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_flash(ଶୂନ୍ୟ)
{
	ulong flash_size = 0;
	ସଂରଚନା bd_info *bd = gd->bd;

	ଯଦି (!is_flash_available())
		ଫେରନ୍ତୁ 0;

	puts("Flash: ");

	ଯଦି (board_flash_wp_on())
		printf("Uninitialized - Write Protect On\n");
	ଅନ୍ୟଥା
		flash_size = flash_init();

	print_size(flash_size, "");
#ifdef CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	ଯଦି (env_get_yesno("flashchecksum") == 1) {
		ଅଚଳ uchar *flash_base = (ଅଚଳ uchar *)CFG_SYS_FLASH_BASE;

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

#ଯଦି defined(CONFIG_SYS_UPDATE_FLASH_SIZE)
	/* Make a update of the Memctrl. */
	update_flash_size(flash_size);
#endif

#ଯଦି defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_TEXT_BASE + flash_size;
#ନଚେତଯଦି CONFIG_SYS_MONITOR_BASE == CFG_SYS_FLASH_BASE
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#endif
	ଫେରନ୍ତୁ 0;
}
#endif

#ifdef CONFIG_CMD_NAND
/* go init the NAND */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_nand(ଶୂନ୍ୟ)
{
	puts("NAND:  ");
	nand_init();
	printf("%lu MiB\n", nand_size() / 1024);
	ଫେରନ୍ତୁ 0;
}
#endif

#ଯଦି defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_onenand(ଶୂନ୍ୟ)
{
	puts("NAND:  ");
	onenand_init();
	ଫେରନ୍ତୁ 0;
}
#endif

#ifdef CONFIG_MMC
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_mmc(ଶୂନ୍ୟ)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	ଫେରନ୍ତୁ 0;
}
#endif

#ifdef CONFIG_PVBLOCK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_pvblock(ଶୂନ୍ୟ)
{
	puts("PVBLOCK: ");
	pvblock_init();
	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା should_load_env(ଶୂନ୍ୟ)
{
	ଯଦି (IS_ENABLED(CONFIG_OF_CONTROL))
		ଫେରନ୍ତୁ ofnode_conf_read_int("load-environment", 1);

	ଫେରନ୍ତୁ 1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_env(ଶୂନ୍ୟ)
{
	/* initialize environment */
	ଯଦି (should_load_env())
		env_relocate();
	ଅନ୍ୟଥା
		env_set_default(ଖାଲି, 0);

	env_import_fdt();

	ଯଦି (IS_ENABLED(CONFIG_OF_CONTROL))
		env_set_hex("fdtcontroladdr",
			    (unsigned long)map_to_sysmem(gd->fdt_blob));

	#ଯଦି (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_START_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_data();
	#endif

	/* Initialize from environment */
	image_load_addr = env_get_ulong("loadaddr", 16, image_load_addr);

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_SYS_MALLOC_BOOTPARAMS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_malloc_bootparams(ଶୂନ୍ୟ)
{
	gd->bd->bi_boot_params = (ulong)malloc(CONFIG_SYS_BOOTPARAMS_LEN);
	ଯଦି (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		ଫେରନ୍ତୁ -ENOMEM;
	}
	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_boot_led_blink(ଶୂନ୍ୟ)
{
	led_boot_blink();

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_boot_led_on(ଶୂନ୍ୟ)
{
	led_boot_on();

	ଫେରନ୍ତୁ 0;
}

#ଯଦି CONFIG_IS_ENABLED(NET)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_net(ଶୂନ୍ୟ)
{
	puts("Net:   ");
	eth_initialize();
#ଯଦି defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#endif
	ଫେରନ୍ତୁ 0;
}
#endif

#ifdef CONFIG_POST
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_post(ଶୂନ୍ୟ)
{
	post_run(ଖାଲି, POST_RAM | post_bootmode_get(0));
	ଫେରନ୍ତୁ 0;
}
#endif

#ଯଦି defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_mem(ଶୂନ୍ୟ)
{
	ulong pram = 0;
	ଅକ୍ଷର memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (long ପୂର୍ଣ୍ଣସଂଖ୍ୟା)((gd->ram_size / 1024) - pram));
	env_set("mem", memsz);

	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initr_lmb(ଶୂନ୍ୟ)
{
	ଯଦି (CONFIG_IS_ENABLED(LMB))
		ଫେରନ୍ତୁ lmb_init();
	ଅନ୍ୟଥା
		ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dm_announce(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା device_ଗଣନା;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା uclass_ଗଣନା;

	ଯଦି (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_ଗଣନା, &uclass_ଗଣନା);
		printf("Core:  %d devices, %d uclasses", device_ଗଣନା,
		       uclass_ଗଣନା);
		ଯଦି (CONFIG_IS_ENABLED(OF_REAL))
			printf(", devicetree: %s", fdtdec_get_srcname());
		ଯଦି (CONFIG_IS_ENABLED(UPL))
			printf(", universal payload active");
		printf("\n");
		ଯଦି (IS_ENABLED(CONFIG_OF_HAS_PRIOR_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			printf("Warning: Unexpected devicetree source (not from a prior stage)");
			printf("Warning: U-Boot may not function properly\n");
		}
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_main_loop(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

#ifdef CONFIG_SANDBOX
	sandbox_main_loop_init();
#endif

	ret = event_notify_null(EVT_MAIN_LOOP);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	ପାଇଁ (;;)
		main_loop();
	ଫେରନ୍ତୁ 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ସ୍ଥିର ଶୂନ୍ୟ initcall_run_r(ଶୂନ୍ୟ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#ଯଦି CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#endif
	INITCALL(initr_reloc_global_data);
#ଯଦି CONFIG_IS_ENABLED(SYS_INIT_RAM_LOCK) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_in_cache);
#endif
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#ଯଦି CONFIG_IS_ENABLED(CONSOLE_RECORD)
	INITCALL(console_record_init);
#endif
#ଯଦି CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_MEMORY)
	INITCALL(noncached_init);
#endif
	INITCALL(initr_of_live);
#ଯଦି CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#endif
#ଯଦି CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#endif
#ଯଦି CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#endif
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ଯଦି CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(set_cpu_clk_info);
#endif
	INITCALL(initr_lmb);
#ଯଦି CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_memory_init);
#endif
#ଯଦି CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#endif
#ଯଦି CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#endif
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#ଯଦି CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#endif
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#ଯଦି CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#endif
	WATCHDOG_RESET();
#ଯଦି CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#endif
	WATCHDOG_RESET();
#ଯଦି CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#endif
#ଯଦି CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#endif
	INITCALL(power_init_board);
#ଯଦି CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#endif
	WATCHDOG_RESET();
#ଯଦି IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#endif
#ଯଦି CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#endif
#ଯଦି CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#endif
#ଯଦି CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#endif
#ଯଦି CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#endif
#ଯଦି CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#endif
#ଯଦି CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#endif
	INITCALL(initr_env);
#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#endif
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#ଯଦି CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#endif
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#ଯଦି CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#endif
	INITCALL(stdio_add_devices);
	INITCALL(jumptable_init);
#ଯଦି CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#endif
	INITCALL(console_init_r);	/* fully init console as a device */
#ଯଦି CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#endif
	/* miscellaneous arch-dependent init */
#ଯଦି CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#endif
	/* miscellaneous platform-dependent init */
#ଯଦି CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#endif
	WATCHDOG_RESET();
#ଯଦି CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#endif
	INITCALL(interrupt_init);
#ଯଦି defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#endif
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#ଯଦି CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#endif
#ଯଦି CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#endif
#ଯଦି CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#endif
#ଯଦି CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#endif
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_LAST_STAGE_INIT);
#ଯଦି defined(CFG_PRAM)
	INITCALL(initr_mem);
#endif
	INITCALL(initr_boot_led_on);
	INITCALL(run_main_loop);
}

ଶୂନ୍ୟ board_init_r(gd_t *new_gd, ulong dest_addr)
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
	ଯଦି (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_setup_gd(new_gd);

#ଯଦି defined(CONFIG_RISCV)
	set_gd(new_gd);
#ନଚେତଯଦି !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#endif
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_run_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
