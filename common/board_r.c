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

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <api.h>
#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <cyclic.h>
#ಅನ್ತರ್ಭಾವ <display_options.h>
#ಅನ್ತರ್ಭಾವ <exports.h>
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MTD_NOR_FLASH
#ಅನ್ತರ್ಭಾವ <flash.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <irq_func.h>
#ಅನ್ತರ್ಭಾವ <lmb.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <net.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಅನ್ತರ್ಭಾವ <binman.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <efi_loader.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <env_internal.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <initcall.h>
#ಅನ್ತರ್ಭಾವ <kgdb.h>
#ಅನ್ತರ್ಭಾವ <irq_func.h>
#ಅನ್ತರ್ಭಾವ <led.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <miiphy.h>
#ಅನ್ತರ್ಭಾವ <mmc.h>
#ಅನ್ತರ್ಭಾವ <mux.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <of_live.h>
#ಅನ್ತರ್ಭಾವ <onenand_uboot.h>
#ಅನ್ತರ್ಭಾವ <pvblock.h>
#ಅನ್ತರ್ಭಾವ <scsi.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <stdio_dev.h>
#ಅನ್ತರ್ಭಾವ <timer.h>
#ಅನ್ತರ್ಭಾವ <trace.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <xen.h>
#ಅನ್ತರ್ಭಾವ <asm/sections.h>
#ಅನ್ತರ್ಭಾವ <dm/root.h>
#ಅನ್ತರ್ಭಾವ <dm/ofnode.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <linux/err.h>
#ಅನ್ತರ್ಭಾವ <wdt.h>
#ಅನ್ತರ್ಭಾವ <asm-generic/gpio.h>
#ಅನ್ತರ್ಭಾವ <relocate.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ulong monitor_flash_len;

__weak ಪೂರ್ಣಾಂಕ board_flash_wp_on(ಶೂನ್ಯ)
{
	/*
	 * Most flashes can't be detected when write protection is enabled,
	 * so provide a way to let U-Boot gracefully ignore write protected
	 * devices.
	 */
	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ cpu_secondary_init_r(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_trace(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TRACE
	trace_init(gd->trace_buff, CONFIG_TRACE_ಬಫರ್_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_reloc(ಶೂನ್ಯ)
{
	/* tell others: relocation done */
	gd->flags |= GD_FLG_RELOC | GD_FLG_ತುಂಬಿದ_MALLOC_INIT;

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ defined(CONFIG_ARM) || defined(CONFIG_RISCV)
/*
 * Some of these functions are needed purely because the functions they
 * call return void. If we change them to return 0, these stubs can go away.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_caches(ಶೂನ್ಯ)
{
	/* Enable caches */
	enable_caches();
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

__weak ಪೂರ್ಣಾಂಕ fixup_cpu(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_reloc_global_ದತ್ತಾಂಶ(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __ARM__
	monitor_flash_len = ಕೊನೆ - __image_copy_start;
#ಅಥವಾಆದರೆ defined(CONFIG_RISCV)
	monitor_flash_len = (ulong)ಕೊನೆ - (ulong)ಆರಂಭ_2;
#ಅಥವಾಆದರೆ !defined(CONFIG_SANDBOX) && !defined(CONFIG_NIOS2)
	monitor_flash_len = (ulong)__init_end - gd->relocaddr;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CONFIG_MPC85xx) || defined(CONFIG_MPC86xx)
	/*
	 * The gd->cpu pointer is set to an address in flash before relocation.
	 * We need to update it to point to the same CPU entry in RAM.
	 * TODO: why not just add gd->reloc_ofs?
	 */
	gd->arch.cpu += gd->relocaddr - CONFIG_SYS_MONITOR_ತಳ;

	/*
	 * If we didn't know the cpu mask & # cores, we can save them of
	 * now rather than 'computing' them constantly
	 */
	fixup_cpu();
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ENV_RELOC_GD_ENV_ADDR
	/*
	 * Relocate the early env_addr pointer unless we know it is not inside
	 * the binary. Some systems need this and for the rest, it doesn't hurt.
	 */
	gd->env_addr += gd->reloc_off;
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * For CONFIG_OF_EMBED case the FDT is embedded into ELF, available by
	 * __dtb_dt_begin. After U-Boot ELF self-relocation to RAM top address
	 * it is worth to update fdt_blob in global_data
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_ದ_EMBED))
		fdtdec_ಸಿದ್ಧತೆ_embed();

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_EFI_LOADER
	/*
	 * On the ARM architecture gd is mapped to a fixed register (r9 or x18).
	 * As this register may be overwritten by an EFI payload we save it here
	 * and restore it on every callback entered.
	 */
	efi_save_gd();

	ಆದರೆ (!(gd->flags & GD_FLG_SKIP_RELOC))
		efi_runtime_relocate(gd->relocaddr, ಖಾಲಿ);

#ಆದರೆ_ಅಂತ್ಯ
	/*
	 * We are done with all relocations change the permissions of the binary
	 * NOTE: __start_rodata etc are defined in arm64 linker scripts and
	 * sections.h. If you want to add support for your platform you need to
	 * add the symbols on your linker script, otherwise they will point to
	 * random addresses.
	 *
	 */
	ಆದರೆ (IS_ENABLED(CONFIG_MMU_PGPROT)) {
		pgprot_ಗಣ_attrs((phys_addr_t)(uintptr_t)(__start_rodata),
				 (size_t)(uintptr_t)(__end_rodata - __start_rodata),
				 MMU_ATTR_RO);
		pgprot_ಗಣ_attrs((phys_addr_t)(uintptr_t)(__start_data),
				 (size_t)(uintptr_t)(__end_data - __start_data),
				 MMU_ATTR_RW);
		pgprot_ಗಣ_attrs((phys_addr_t)(uintptr_t)(__text_start),
				 (size_t)(uintptr_t)(__text_end - __text_start),
				 MMU_ATTR_RX);
	}

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ arch_initr_trap(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ defined(CONFIG_SYS_INIT_RAM_ಬೀಗ) && defined(CONFIG_E500)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_unlock_ram_ಒಳ_cache(ಶೂನ್ಯ)
{
	unlock_ram_ಒಳ_cache();	/* it's time to unlock D-cache in e500 */
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_barrier(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PPC
	/* TODO: Can we not use dmb() macros for this? */
	asm("sync ; isync");
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_malloc(ಶೂನ್ಯ)
{
	ulong ಆರಂಭ;

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	debug("Pre-reloc malloc() used %#x bytes (%d KB)\n", gd->malloc_ptr,
	      gd->malloc_ptr / 1024);
#ಆದರೆ_ಅಂತ್ಯ
	/* The malloc area is immediately below the monitor copy in DRAM */
	/*
	 * This value MUST match the value of gd->start_addr_sp in board_f.c:
	 * reserve_noncached().
	 */
	ಆರಂಭ = gd->relocaddr - ಒಟ್ಟು_MALLOC_LEN;
	gd_ಗಣ_malloc_ಆರಂಭ(ಆರಂಭ);
	mem_malloc_init(ಆರಂಭ, ಒಟ್ಟು_MALLOC_LEN);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_ದ_ಜೀವಂತ(ಶೂನ್ಯ)
{
	ಆದರೆ (CONFIG_IS_ENABLED(ದ_ಜೀವಂತ)) {
		ಪೂರ್ಣಾಂಕ ret;

		bootstage_ಆರಂಭ(BOOTSTAGE_ID_ACCUM_ದ_ಜೀವಂತ, "of_live");
		ret = ದ_ಜೀವಂತ_build(gd->fdt_blob,
				    (ರಚನೆ device_ಗಂಟು **)gd_ದ_ಬೇರು_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_ದ_ಜೀವಂತ);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_DM
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_dm(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	oftree_reset();

	/* Drop the pre-reloc driver model and start a new one */
	gd->dm_ಬೇರು = ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMER
	gd->timer = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	bootstage_ಆರಂಭ(BOOTSTAGE_ID_ACCUM_DM_R, "dm_r");
	ret = dm_init_and_ಪರಿಶೀಲನೆ(false);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_R);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಹಿಂತಿರುಗಿ dm_autoprobe();
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_dm_devices(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಆದರೆ (IS_ENABLED(CONFIG_MULTIPLEXER)) {
		/*
		 * Initialize the multiplexer controls to their default state.
		 * This must be done early as other drivers may unknowingly
		 * rely on it.
		 */
		ret = dm_mux_init();
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_bootstage(ಶೂನ್ಯ)
{
	bootstage_mark_name(BOOTSTAGE_ID_ಆರಂಭ_UBOOT_R, "board_init_r");

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ power_init_board(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_announce(ಶೂನ್ಯ)
{
	debug("Now running in RAM - U-Boot at: %08lx\n", gd->relocaddr);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused initr_binman(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = binman_init();
	ಆದರೆ (ret)
		ಬರೆ("binman_init failed:%d\n", ret);

	ಹಿಂತಿರುಗಿ ret;
}

#ಆದರೆ defined(CONFIG_MTD_NOR_FLASH)
__weak ಪೂರ್ಣಾಂಕ is_flash_ಲಭ್ಯ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_flash(ಶೂನ್ಯ)
{
	ulong flash_ಗಾತ್ರ = 0;
	ರಚನೆ bd_info *bd = gd->bd;

	ಆದರೆ (!is_flash_ಲಭ್ಯ())
		ಹಿಂತಿರುಗಿ 0;

	puts("Flash: ");

	ಆದರೆ (board_flash_wp_on())
		ಬರೆ("Uninitialized - Write Protect On\n");
	ಇಲ್ಲದಿದ್ದರೆ
		flash_ಗಾತ್ರ = flash_init();

	print_ಗಾತ್ರ(flash_ಗಾತ್ರ, "");
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_FLASH_CHECKSUM
	/*
	 * Compute and print flash CRC if flashchecksum is set to 'y'
	 *
	 * NOTE: Maybe we should add some schedule()? XXX
	 */
	ಆದರೆ (env_get_yesno("flashchecksum") == 1) {
		ಸ್ಥಿರಾಂಕ uchar *flash_ತಳ = (ಸ್ಥಿರಾಂಕ uchar *)CFG_SYS_FLASH_ತಳ;

		ಬರೆ("  CRC: %08X", ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(0,
					    flash_ತಳ,
					    flash_ಗಾತ್ರ));
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_FLASH_CHECKSUM */
	putc('\n');

	/* update start of FLASH memory    */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CFG_SYS_FLASH_ತಳ
	bd->bi_flashstart = CFG_SYS_FLASH_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ
	/* size of FLASH memory (final value) */
	bd->bi_flashsize = flash_ಗಾತ್ರ;

#ಆದರೆ defined(CONFIG_SYS_UPDATE_FLASH_ಗಾತ್ರ)
	/* Make a update of the Memctrl. */
	update_flash_ಗಾತ್ರ(flash_ಗಾತ್ರ);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_OXC) || defined(CONFIG_RMU)
	/* flash mapped at end of memory map */
	bd->bi_flashoffset = CONFIG_ಪಠ್ಯ_ತಳ + flash_ಗಾತ್ರ;
#ಅಥವಾಆದರೆ CONFIG_SYS_MONITOR_ತಳ == CFG_SYS_FLASH_ತಳ
	bd->bi_flashoffset = monitor_flash_len;	/* reserved area for monitor */
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMD_NAND
/* go init the NAND */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_nand(ಶೂನ್ಯ)
{
	puts("NAND:  ");
	nand_init();
	ಬರೆ("%lu MiB\n", nand_ಗಾತ್ರ() / 1024);
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_CMD_ONENAND)
/* go init the NAND */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_onenand(ಶೂನ್ಯ)
{
	puts("NAND:  ");
	onenand_init();
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_MMC
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_mmc(ಶೂನ್ಯ)
{
	puts("MMC:   ");
	mmc_initialize(gd->bd);
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_PVBLOCK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_pvblock(ಶೂನ್ಯ)
{
	puts("PVBLOCK: ");
	pvblock_init();
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ should_ತುಂಬು_env(ಶೂನ್ಯ)
{
	ಆದರೆ (IS_ENABLED(CONFIG_ದ_CONTROL))
		ಹಿಂತಿರುಗಿ ofnode_conf_read_int("load-environment", 1);

	ಹಿಂತಿರುಗಿ 1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_env(ಶೂನ್ಯ)
{
	/* initialize environment */
	ಆದರೆ (should_ತುಂಬು_env())
		env_relocate();
	ಇಲ್ಲದಿದ್ದರೆ
		env_ಗಣ_default(ಖಾಲಿ, 0);

	env_import_fdt();

	ಆದರೆ (IS_ENABLED(CONFIG_ದ_CONTROL))
		env_ಗಣ_hex("fdtcontroladdr",
			    (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)map_ಗಮ್ಯ_sysmem(gd->fdt_blob));

	#ಆದರೆ (IS_ENABLED(CONFIG_SAVE_PREV_BL_INITRAMFS_ಆರಂಭ_ADDR) || \
						IS_ENABLED(CONFIG_SAVE_PREV_BL_FDT_ADDR))
		save_prev_bl_ದತ್ತಾಂಶ();
	#ಆದರೆ_ಅಂತ್ಯ

	/* Initialize from environment */
	image_ತುಂಬು_addr = env_get_ulong("loadaddr", 16, image_ತುಂಬು_addr);

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_MALLOC_BOOTPARAMS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_malloc_bootparams(ಶೂನ್ಯ)
{
	gd->bd->bi_boot_params = (ulong)ಸ್ಮರಣೆ_ಕೊಡು(CONFIG_SYS_BOOTPARAMS_LEN);
	ಆದರೆ (!gd->bd->bi_boot_params) {
		puts("WARNING: Cannot allocate space for boot parameters\n");
		ಹಿಂತಿರುಗಿ -ENOMEM;
	}
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_boot_led_blink(ಶೂನ್ಯ)
{
	led_boot_blink();

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_boot_led_on(ಶೂನ್ಯ)
{
	led_boot_on();

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ CONFIG_IS_ENABLED(NET)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_net(ಶೂನ್ಯ)
{
	puts("Net:   ");
	eth_initialize();
#ಆದರೆ defined(CONFIG_RESET_PHY_R)
	debug("Reset Ethernet PHY\n");
	reset_phy();
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POST
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_post(ಶೂನ್ಯ)
{
	post_ಸತತ(ಖಾಲಿ, POST_RAM | post_bootmode_get(0));
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CFG_PRAM)
/*
 * Export available size of memory for Linux, taking into account the
 * protected RAM at top of memory
 */
ಪೂರ್ಣಾಂಕ initr_mem(ಶೂನ್ಯ)
{
	ulong pram = 0;
	ಅಕ್ಷರ memsz[32];

	pram = env_get_ulong("pram", 10, CFG_PRAM);
	sprintf(memsz, "%ldk", (ದೀರ್ಘ ಪೂರ್ಣಾಂಕ)((gd->ram_ಗಾತ್ರ / 1024) - pram));
	env_ಗಣ("mem", memsz);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initr_lmb(ಶೂನ್ಯ)
{
	ಆದರೆ (CONFIG_IS_ENABLED(LMB))
		ಹಿಂತಿರುಗಿ lmb_init();
	ಇಲ್ಲದಿದ್ದರೆ
		ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dm_announce(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ device_ಎಣಿಕೆ;
	ಪೂರ್ಣಾಂಕ uclass_ಎಣಿಕೆ;

	ಆದರೆ (IS_ENABLED(CONFIG_DM)) {
		dm_get_stats(&device_ಎಣಿಕೆ, &uclass_ಎಣಿಕೆ);
		ಬರೆ("Core:  %d devices, %d uclasses", device_ಎಣಿಕೆ,
		       uclass_ಎಣಿಕೆ);
		ಆದರೆ (CONFIG_IS_ENABLED(ದ_REAL))
			ಬರೆ(", devicetree: %s", fdtdec_get_srcname());
		ಆದರೆ (CONFIG_IS_ENABLED(UPL))
			ಬರೆ(", universal payload active");
		ಬರೆ("\n");
		ಆದರೆ (IS_ENABLED(CONFIG_ದ_HAS_ಪೂರ್ವ_STAGE) &&
		    (gd->fdt_src == FDTSRC_SEPARATE ||
		     gd->fdt_src == FDTSRC_EMBED)) {
			ಬರೆ("Warning: Unexpected devicetree source (not from a prior stage)");
			ಬರೆ("Warning: U-Boot may not function properly\n");
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_main_loop(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SANDBOX
	sandbox_main_loop_init();
#ಆದರೆ_ಅಂತ್ಯ

	ret = event_notify_null(EVT_MAIN_LOOP);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	/* main_loop() can return to retry autoboot, if so just run it again */
	ಗಾಗಿ (;;)
		main_loop();
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Over time we hope to remove most of the driver-related init and do it
 * if/when the driver is later used.
 *
 * TODO: perhaps reset the watchdog in the initcall function after each call?
 */

ಸ್ಥಿರ ಶೂನ್ಯ initcall_ಸತತ_r(ಶೂನ್ಯ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(initr_trace);
	INITCALL(initr_reloc);
	INITCALL(event_init);
	/* TODO: could x86/PPC have this also perhaps? */
#ಆದರೆ CONFIG_IS_ENABLED(ARM) || CONFIG_IS_ENABLED(RISCV)
	INITCALL(initr_caches);
	/* Note: For Freescale LS2 SoCs, new MMU table is created in DDR.
	 *	 A temporary mapping of IFC high region is since removed,
	 *	 so environmental variables in NOR flash is not available
	 *	 until board_init() is called below to remap IFC to high
	 *	 region.
	 */
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_reloc_global_ದತ್ತಾಂಶ);
#ಆದರೆ CONFIG_IS_ENABLED(SYS_INIT_RAM_ಬೀಗ) && CONFIG_IS_ENABLED(E500)
	INITCALL(initr_unlock_ram_ಒಳ_cache);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_barrier);
	INITCALL(initr_malloc);
	INITCALL(log_init);
	INITCALL(initr_bootstage); /* Needs malloc() but has its own timer */
#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_ದಾಖಲೆ)
	INITCALL(console_ದಾಖಲೆ_init);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(SYS_HAS_NONCACHED_ಸ್ಮರಣೆ)
	INITCALL(noncached_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_ದ_ಜೀವಂತ);
#ಆದರೆ CONFIG_IS_ENABLED(DM)
	INITCALL(initr_dm);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(ADDR_MAP)
	INITCALL(init_addr_map);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(BOARD_INIT)
	INITCALL(board_init);	/* Setup chipselects */
#ಆದರೆ_ಅಂತ್ಯ
	/*
	 * TODO: printing of the clock inforamtion of the board is now
	 * implemented as part of bdinfo command. Currently only support for
	 * davinci SOC's is added. Remove this check once all the board
	 * implement this.
	 */
#ಆದರೆ CONFIG_IS_ENABLED(CLOCKS)
	INITCALL(ಗಣ_cpu_clk_info);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_lmb);
#ಆದರೆ CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_ಸ್ಮರಣೆ_init);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(BINMAN_FDT)
	INITCALL(initr_binman);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(FSP_VERSION2)
	INITCALL(arch_fsp_init_r);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_dm_devices);
	INITCALL(stdio_init_tables);
	INITCALL(serial_initialize);
	INITCALL(initr_announce);
	INITCALL(dm_announce);
#ಆದರೆ CONFIG_IS_ENABLED(WDT)
	INITCALL(initr_watchdog);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
	INITCALL(arch_initr_trap);
#ಆದರೆ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_R)
	INITCALL(board_early_init_r);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
#ಆದರೆ CONFIG_IS_ENABLED(POST)
	INITCALL(post_output_backlog);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
#ಆದರೆ CONFIG_IS_ENABLED(PCI_INIT_R) && CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do early PCI configuration _before_ the flash gets initialised,
	 * because PCU resources are crucial for flash access on some boards.
	 */
	INITCALL(pci_init);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(ARCH_EARLY_INIT_R)
	INITCALL(arch_early_init_r);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(power_init_board);
#ಆದರೆ CONFIG_IS_ENABLED(MTD_NOR_FLASH)
	INITCALL(initr_flash);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
#ಆದರೆ IS_ENABLED(CONFIG_PPC) || CONFIG_IS_ENABLED(M68K) || CONFIG_IS_ENABLED(X86)
	/* initialize higher level parts of CPU like time base and timers */
	INITCALL(cpu_init_r);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(EFI_LOADER)
	INITCALL(efi_init_early);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(CMD_NAND)
	INITCALL(initr_nand);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(CMD_ONENAND)
	INITCALL(initr_onenand);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(MMC)
	INITCALL(initr_mmc);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(XEN)
	INITCALL(xen_init);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(PVBLOCK)
	INITCALL(initr_pvblock);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_env);
#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_BOOTPARAMS)
	INITCALL(initr_malloc_bootparams);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
	INITCALL(cpu_secondary_init_r);
#ಆದರೆ CONFIG_IS_ENABLED(ID_EEPROM)
	INITCALL(mac_read_from_eeprom);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL_EVT(EVT_SETTINGS_R);
	WATCHDOG_RESET();
#ಆದರೆ CONFIG_IS_ENABLED(PCI_INIT_R) && !CONFIG_IS_ENABLED(SYS_EARLY_PCI_INIT)
	/*
	 * Do pci configuration
	 */
	INITCALL(pci_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(stdio_ಕೂಡಿಸು_devices);
	INITCALL(jumptable_init);
#ಆದರೆ CONFIG_IS_ENABLED(API)
	INITCALL(api_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(console_init_r);	/* fully init console as a device */
#ಆದರೆ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO_LATE)
	INITCALL(console_announce_r);
	INITCALL(show_board_info);
#ಆದರೆ_ಅಂತ್ಯ
	/* miscellaneous arch-dependent init */
#ಆದರೆ CONFIG_IS_ENABLED(ARCH_MISC_INIT)
	INITCALL(arch_misc_init);
#ಆದರೆ_ಅಂತ್ಯ
	/* miscellaneous platform-dependent init */
#ಆದರೆ CONFIG_IS_ENABLED(MISC_INIT_R)
	INITCALL(misc_init_r);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
#ಆದರೆ CONFIG_IS_ENABLED(CMD_KGDB)
	INITCALL(kgdb_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(ಅಡಚಣೆ_init);
#ಆದರೆ defined(CONFIG_MICROBLAZE) || defined(CONFIG_M68K)
	INITCALL(timer_init);		/* initialize timer */
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_boot_led_blink);
	/* PPC has a udelay(20) here dating from 2002. Why? */
#ಆದರೆ CONFIG_IS_ENABLED(BOARD_LATE_INIT)
	INITCALL(board_late_init);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(PCI_ENDPOINT)
	INITCALL(pci_ep_init);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(NET)
	WATCHDOG_RESET();
	INITCALL(initr_net);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(POST)
	INITCALL(initr_post);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
	INITCALL_EVT(EVT_ಕೊನೆಯ_STAGE_INIT);
#ಆದರೆ defined(CFG_PRAM)
	INITCALL(initr_mem);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initr_boot_led_on);
	INITCALL(ಸತತ_main_loop);
}

ಶೂನ್ಯ board_init_r(gd_t *new_gd, ulong dest_addr)
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
	ಆದರೆ (CONFIG_IS_ENABLED(X86_64) && !IS_ENABLED(CONFIG_EFI_APP))
		arch_ಸಿದ್ಧತೆ_gd(new_gd);

#ಆದರೆ defined(CONFIG_RISCV)
	ಗಣ_gd(new_gd);
#ಅಥವಾಆದರೆ !defined(CONFIG_X86) && !defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	gd = new_gd;
#ಆದರೆ_ಅಂತ್ಯ
	gd->flags &= ~GD_FLG_LOG_READY;

	initcall_ಸತತ_r();

	/* NOTREACHED - run_main_loop() does not return */
	hang();
}
