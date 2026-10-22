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

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <api.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootstage.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cyclic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <display_options.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <exports.h>
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MTD_NOR_FLASH
#ਅਨ੍ਤਰ੍ਭਾਵ <flash.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <irq_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <lmb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <net.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <binman.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <efi_loader.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <initcall.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <kgdb.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <irq_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <led.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <miiphy.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mmc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mux.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <nand.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <of_live.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <onenand_uboot.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <pvblock.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <scsi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio_dev.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <trace.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <xen.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/root.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/ofnode.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/err.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <wdt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm-generic/gpio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <relocate.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ulong monitor_flash_len;

__weak ਪੂਰਨ board_flash_wp_on(ਖਾਲੀ)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ cpu_secondary_init_r(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_trace(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_ਬਫਰ_ਮਾਪ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_reloc(ਖਾਲੀ)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_ਭਰਿਆ_MALLOC_INIT;

	ਵਾਪਸ 0;
}

#ਜੇ defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ਸਥਿਰ ਪੂਰਨ initr_caches(ਖਾਲੀ)
{
	/* Enable caches */
	enable_caches();
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

__weak ਪੂਰਨ fixup_cpu(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_reloc_global_ਅੰਕੜੇ(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __ARM__
	monitor_flash_len = ਸਿਰਾ - __image_copy_start;
#ਜਾਂਜੇ defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)ਸਿਰਾ - (ulong)ਸ਼ੁਰੂ_2;
#ਜਾਂਜੇ !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#ਜੇ_ਅੰਤ
#ਜੇ defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_ਅਧਾਰ;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#ਜੇ_ਅੰਤ

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	ਜੇ (IS_ENABLED(CONFIG_ਦਾ_EMBED))
		fdtdec_ਤਿਆਰੀ_embed();

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	ਜੇ (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, ਨਿਰਾ);

#ਜੇ_ਅੰਤ
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	ਜੇ (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_ਸੈੱਟ_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_ਸੈੱਟ_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_ਸੈੱਟ_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ arch_initr_trap(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

#ਜੇ defined(CONFIG_SYS_INIT_RAM_ਤਾਲਾ) && defined(CONFIG_E500)
ਸਥਿਰ ਪੂਰਨ initr_unlock_ram_ਵਿੱਚ_cache(ਖਾਲੀ)
{
	unlock_ram_ਵਿੱਚ_cache();	/* it's time to unlock D-cache in e500 */
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ initr_barrier(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_malloc(ਖਾਲੀ)
{
	ulong ਸ਼ੁਰੂ;

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#ਜੇ_ਅੰਤ
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	ਸ਼ੁਰੂ = gd->relocaddr - ਕੁੱਲ_MALLOC_LEN;
	gd_ਸੈੱਟ_malloc_ਸ਼ੁਰੂ(ਸ਼ੁਰੂ);
	mem_malloc_init(ਸ਼ੁਰੂ, ਕੁੱਲ_MALLOC_LEN);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_ਦਾ_ਜੀਵਤ(ਖਾਲੀ)
{
	ਜੇ (CONFIG_IS_ENABLED(ਦਾ_ਜੀਵਤ)) {
		ਪੂਰਨ ret;

		bootstage_ਸ਼ੁਰੂ(BOOTSTAGE_ID_ACCUM_ਦਾ_ਜੀਵਤ, "of_live");
		ret = ਦਾ_ਜੀਵਤ_build(gd->fdt_blob,
				    (ਬਣਤਰ device_ਗੰਢ **)gd_ਦਾ_ਜੜ੍ਹ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_ਦਾ_ਜੀਵਤ);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_DM
ਸਥਿਰ ਪੂਰਨ initr_dm(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_ਜੜ੍ਹ = ਨਿਰਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMER
	gd->timer = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	bootstage_ਸ਼ੁਰੂ(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_ਜਾਂਚ(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਵਾਪਸ dm_autoprobe();
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ initr_dm_devices(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	ਜੇ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਜੇ (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_bootstage(ਖਾਲੀ)
{
	bootstage_mark_name(BOOTSTAGE_ID_ਸ਼ੁਰੂ_UBOOT_R, "board_init_r");

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ power_init_board(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_announce(ਖਾਲੀ)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ __maybe_unused initr_binman(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	ret = binman_init();
	ਜੇ (ret)
		ਲਿਖੋ("binman_init failed:%d\n", ret);

	ਵਾਪਸ ret;
}

#ਜੇ defined(CONFIG_MTD_NOR_FLASH)
__weak ਪੂਰਨ is_flash_ਉਪਲਬਧ(ਖਾਲੀ)
{
	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ initr_flash(ਖਾਲੀ)
{
	ulong flash_ਮਾਪ = 0;
	ਬਣਤਰ bd_info *bd = gd->bd;

	ਜੇ (!is_flash_ਉਪਲਬਧ())
		ਵਾਪਸ 0;

	puts("Flash: ");

	ਜੇ (board_flash_wp_on())
		ਲਿਖੋ("Uninitialized - Write Protect On\n");
	ਨਹੀਂਤਾਂ
		flash_ਮਾਪ = flash_init();

	print_ਮਾਪ(flash_ਮਾਪ, "");
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	ਜੇ (env_get_yesno("flashchecksum") == 1) {
		ਅਚਲ uchar *flash_ਅਧਾਰ = (ਅਚਲ uchar *)CFG_SYS_FLASH_ਅਧਾਰ;

		ਲਿਖੋ("  CRC: %08X", ਚੱਕਰੀ_ਜਾਂਚ_32(0,
					    flash_ਅਧਾਰ,
					    flash_ਮਾਪ));
	}
#ਜੇ_ਅੰਤ /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CFG_SYS_FLASH_ਅਧਾਰ
	bd->bi_flashstart = CFG_SYS_FLASH_ਅਧਾਰ;
#ਜੇ_ਅੰਤ
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_ਮਾਪ;

#ਜੇ defined(CONFIG_SYS_UPDATE_FLASH_ਮਾਪ)
	/* Make a update of the Memctrl. */
	update_flash_ਮਾਪ(flash_ਮਾਪ);
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_ਪਾਠ_ਅਧਾਰ + flash_ਮਾਪ;
#ਜਾਂਜੇ CONFIG_SYS_MONITOR_ਅਧਾਰ == CFG_SYS_FLASH_ਅਧਾਰ
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMD_NAND
/* go init the NAND */
ਸਥਿਰ ਪੂਰਨ initr_nand(ਖਾਲੀ)
{
	puts("NAND:  ");
	nand_init();
	ਲਿਖੋ("%lu MiB\n", nand_ਮਾਪ() / 1024);
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ਸਥਿਰ ਪੂਰਨ initr_onenand(ਖਾਲੀ)
{
	puts("NAND:  ");
	onenand_init();
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_MMC
ਸਥਿਰ ਪੂਰਨ initr_mmc(ਖਾਲੀ)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_PVBLOCK
ਸਥਿਰ ਪੂਰਨ initr_pvblock(ਖਾਲੀ)
{
	puts("PVBLOCK: ");
	pvblock_init();
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ should_ਲੱਦੋ_env(ਖਾਲੀ)
{
	ਜੇ (IS_ENABLED(CONFIG_ਦਾ_CONTROL))
		ਵਾਪਸ ofnode_conf_read_int("load-environment", 1);

	ਵਾਪਸ 1;
}

ਸਥਿਰ ਪੂਰਨ initr_env(ਖਾਲੀ)
{
	/* initialize environment */
	ਜੇ (should_ਲੱਦੋ_env())
		env_relocate();
	ਨਹੀਂਤਾਂ
		env_ਸੈੱਟ_default(ਨਿਰਾ, 0);

	env_import_fdt();

	ਜੇ (IS_ENABLED(CONFIG_ਦਾ_CONTROL))
		env_ਸੈੱਟ_hex("fdtcontroladdr",
			    (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)map_ਮੰਜ਼ਿਲ_sysmem(gd->fdt_blob));

	#ਜੇ (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_ਸ਼ੁਰੂ_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_ਅੰਕੜੇ();
	#ਜੇ_ਅੰਤ

	/* Initialize from environment */
	image_ਲੱਦੋ_addr = env_get_ulong("loadaddr", 16, image_ਲੱਦੋ_addr);

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MALLOC_BOOTPARAMS
ਸਥਿਰ ਪੂਰਨ initr_malloc_bootparams(ਖਾਲੀ)
{
	gd->bd->bi_boot_params = (ulong)ਸਮ੍ਰਿਤੀ_ਦਿਓ(CONFIG_SYS_BOOTPARAMS_LEN);
	ਜੇ (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		ਵਾਪਸ -ENOMEM;
	}
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ initr_boot_led_blink(ਖਾਲੀ)
{
	led_boot_blink();

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initr_boot_led_on(ਖਾਲੀ)
{
	led_boot_on();

	ਵਾਪਸ 0;
}

#ਜੇ CONFIG_IS_ENABLED(NET)
ਸਥਿਰ ਪੂਰਨ initr_net(ਖਾਲੀ)
{
	puts("Net:   ");
	eth_initialize();
#ਜੇ defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POST
ਸਥਿਰ ਪੂਰਨ initr_post(ਖਾਲੀ)
{
	post_ਲੜੀਵਾਰ(ਨਿਰਾ, POST_RAM | post_bootmode_get(0));
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
ਪੂਰਨ initr_mem(ਖਾਲੀ)
{
	ulong pram = 0;
	ਅੱਖਰ memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (ਲੰਮਾ ਪੂਰਨ)((gd->ram_ਮਾਪ / 1024) - pram));
	env_ਸੈੱਟ("mem", memsz);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ initr_lmb(ਖਾਲੀ)
{
	ਜੇ (CONFIG_IS_ENABLED(LMB))
		ਵਾਪਸ lmb_init();
	ਨਹੀਂਤਾਂ
		ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ dm_announce(ਖਾਲੀ)
{
	ਪੂਰਨ device_ਗਿਣਤੀ;
	ਪੂਰਨ uclass_ਗਿਣਤੀ;

	ਜੇ (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_ਗਿਣਤੀ, &uclass_ਗਿਣਤੀ);
		ਲਿਖੋ("Core:  %d devices, %d uclasses", device_ਗਿਣਤੀ,
		       uclass_ਗਿਣਤੀ);
		ਜੇ (CONFIG_IS_ENABLED(ਦਾ_REAL))
			ਲਿਖੋ(", devicetree: %s", fdtdec_get_srcname());
		ਜੇ (CONFIG_IS_ENABLED(UPL))
			ਲਿਖੋ(", universal payload active");
		ਲਿਖੋ("\n");
		ਜੇ (IS_ENABLED(CONFIG_ਦਾ_HAS_ਪੂਰਵ_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			ਲਿਖੋ("Warning: Unexpected devicetree source (not from a prior stage)");
			ਲਿਖੋ("Warning: U-Boot may not function properly\n");
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_main_loop(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SANDBOX
	sandbox_main_loop_init();
#ਜੇ_ਅੰਤ

	ret = event_notify_null(EVT_MAIN_LOOP);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	ਲਈ (;;)
		main_loop();
	ਵਾਪਸ 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ਸਥਿਰ ਖਾਲੀ initcall_ਲੜੀਵਾਰ_r(ਖਾਲੀ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#ਜੇ CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#ਜੇ_ਅੰਤ
	INITCALL(initr_reloc_global_ਅੰਕੜੇ);
#ਜੇ CONFIG_IS_ENABLED(SYS_INIT_RAM_ਤਾਲਾ) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_ਵਿੱਚ_cache);
#ਜੇ_ਅੰਤ
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#ਜੇ CONFIG_IS_ENABLED(CONSOLE_ਰਿਕਾਰਡ)
	INITCALL(console_ਰਿਕਾਰਡ_init);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_ਸਮ੍ਰਿਤੀ)
	INITCALL(noncached_init);
#ਜੇ_ਅੰਤ
	INITCALL(initr_ਦਾ_ਜੀਵਤ);
#ਜੇ CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#ਜੇ_ਅੰਤ
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ਜੇ CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(ਸੈੱਟ_cpu_clk_info);
#ਜੇ_ਅੰਤ
	INITCALL(initr_lmb);
#ਜੇ CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_ਸਮ੍ਰਿਤੀ_init);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#ਜੇ_ਅੰਤ
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#ਜੇ CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#ਜੇ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
#ਜੇ CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
#ਜੇ CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#ਜੇ_ਅੰਤ
	INITCALL(power_init_board);
#ਜੇ CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
#ਜੇ IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#ਜੇ_ਅੰਤ
	INITCALL(initr_env);
#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#ਜੇ CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#ਜੇ_ਅੰਤ
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#ਜੇ CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#ਜੇ_ਅੰਤ
	INITCALL(stdio_ਜੋੜੋ_devices);
	INITCALL(jumptable_init);
#ਜੇ CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#ਜੇ_ਅੰਤ
	INITCALL(console_init_r);	/* fully init console as a device */
#ਜੇ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#ਜੇ_ਅੰਤ
	/* miscellaneous arch-dependent init */
#ਜੇ CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#ਜੇ_ਅੰਤ
	/* miscellaneous platform-dependent init */
#ਜੇ CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
#ਜੇ CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#ਜੇ_ਅੰਤ
	INITCALL(ਰੁਕਾਵਟ_init);
#ਜੇ defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#ਜੇ_ਅੰਤ
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#ਜੇ CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_ਆਖਰੀ_STAGE_INIT);
#ਜੇ defined(CFG_PRAM)
	INITCALL(initr_mem);
#ਜੇ_ਅੰਤ
	INITCALL(initr_boot_led_on);
	INITCALL(ਲੜੀਵਾਰ_main_loop);
}

ਖਾਲੀ board_init_r(gd_t *new_gd, ulong dest_addr)
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
	ਜੇ (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_ਤਿਆਰੀ_gd(new_gd);

#ਜੇ defined(CONFIG_RISCV)
	ਸੈੱਟ_gd(new_gd);
#ਜਾਂਜੇ !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#ਜੇ_ਅੰਤ
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_ਲੜੀਵਾਰ_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
