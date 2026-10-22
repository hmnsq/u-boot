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
#ಅನ್ತರ್ಭಾವ <bloblist.h>
#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <clock_legacy.h>
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <cpu.h>
#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <cyclic.h>
#ಅನ್ತರ್ಭಾವ <display_options.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <env_internal.h>
#ಅನ್ತರ್ಭಾವ <event.h>
#ಅನ್ತರ್ಭಾವ <fdtdec.h>
#ಅನ್ತರ್ಭಾವ <fs.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <i2c.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <initcall.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <memtop.h>
#ಅನ್ತರ್ಭಾವ <os.h>
#ಅನ್ತರ್ಭಾವ <post.h>
#ಅನ್ತರ್ಭಾವ <relocate.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <sysreset.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <timer.h>
#ಅನ್ತರ್ಭಾವ <trace.h>
#ಅನ್ತರ್ಭಾವ <upl.h>
#ಅನ್ತರ್ಭಾವ <video.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <asm/sections.h>
#ಅನ್ತರ್ಭಾವ <dm/root.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <linux/log2.h>
#ಅನ್ತರ್ಭಾವ <linux/sizes.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

/*
 * Why is gd allocated a register? Prior to reloc it might be better to
 * just pass it around to each function in this file?
 *
 * After reloc one could argue that it is hardly used and doesn't need
 * to be in a register. Or if it is it should perhaps hold pointers to all
 * global data for all modules, so that post-reloc we can avoid the massive
 * literal pool we get on ARM. Or perhaps just encourage each module to use
 * a structure...
 */

#ಆದರೆ defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ init_func_watchdog_init(ಶೂನ್ಯ)
{
# ಆದರೆ defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	ವೇಳಾಪಟ್ಟಿ();

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ init_func_watchdog_reset(ಶೂನ್ಯ)
{
	ವೇಳಾಪಟ್ಟಿ();

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_WATCHDOG */

__weak ಶೂನ್ಯ board_ಕೂಡಿಸು_ram_info(ಪೂರ್ಣಾಂಕ use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ init_baud_ದರ(ಶೂನ್ಯ)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ display_ಪಠ್ಯ_info(ಶೂನ್ಯ)
{
#ಆದರೆ !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_ಆರಂಭ, bss_ಕೊನೆ, ಪಠ್ಯ_ತಳ;

	bss_ಆರಂಭ = (ulong)__bss_start;
	bss_ಕೊನೆ = (ulong)__bss_end;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ಪಠ್ಯ_ತಳ
	ಪಠ್ಯ_ತಳ = CONFIG_ಪಠ್ಯ_ತಳ;
#ಇಲ್ಲದಿದ್ದರೆ
	ಪಠ್ಯ_ತಳ = CONFIG_SYS_MONITOR_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      ಪಠ್ಯ_ತಳ, bss_ಆರಂಭ, bss_ಕೊನೆ);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYSRESET_PRINT_RESETINFO
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_resetinfo(ಶೂನ್ಯ)
{
	ರಚನೆ udevice *dev;
	ಅಕ್ಷರ ಸ್ಥಿತಿ_2[256];
	bool ಸ್ಥಿತಿ_printed = false;
	ಪೂರ್ಣಾಂಕ ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	ಗಾಗಿ (ret = uclass_ಮೊದಲ_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_ಮುಂದಿನ_device_check(&dev)) {
		ಆದರೆ (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (!sysreset_get_ಸ್ಥಿತಿ(dev, ಸ್ಥಿತಿ_2, ಗಾತ್ರ(ಸ್ಥಿತಿ_2))) {
			ಬರೆ("%s%s", ಸ್ಥಿತಿ_printed ? " " : "", ಸ್ಥಿತಿ_2);
			ಸ್ಥಿತಿ_printed = true;
		}
	}
	ಆದರೆ (ಸ್ಥಿತಿ_printed)
		ಬರೆ("\n");

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ print_cpuinfo(ಶೂನ್ಯ)
{
	ರಚನೆ udevice *dev;
	ಅಕ್ಷರ desc[512];
	ಪೂರ್ಣಾಂಕ ret;

	dev = cpu_get_current_dev();
	ಆದರೆ (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		ಹಿಂತಿರುಗಿ -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, ಗಾತ್ರ(desc));
	ಆದರೆ (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಬರೆ("CPU:   %s\n", desc);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ announce_dram_init(ಶೂನ್ಯ)
{
	puts("DRAM:  ");
	ಹಿಂತಿರುಗಿ 0;
}

/*
 * From input size calculate its nearest rounded unit scale (multiply of 2^10)
 * and value in calculated unit scale multiplied by 10 (as fractional fixed
 * point number with one decimal digit), which is human natural format,
 * same what uses print_size() function for displaying. Mathematically it is:
 * round_nearest(val * 2^scale) = size * 10; where: 10 <= val < 10240.
 *
 * For example for size=87654321 we calculate scale=20 and val=836 which means
 * that input has natural human format 83.6 M (mega = 2^20).
 */
#ವ್ಯಾಖ್ಯಾನಿಸು compute_ಗಾತ್ರ_scale_val(ಗಾತ್ರ_2, scale, val) ಮಾಡು { \
	scale = ilog2(ಗಾತ್ರ_2) / 10 * 10; \
	val = (10 * ಗಾತ್ರ_2 + ((1ULL << scale) >> 1)) >> scale; \
	ಆದರೆ (val == 10240) { val = 10; scale += 10; } \
} ಆಗ (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ sizes_near(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ size1, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ size2)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ size1_scale, size1_val, size2_scale, size2_val;

	compute_ಗಾತ್ರ_scale_val(size1, size1_scale, size1_val);
	compute_ಗಾತ್ರ_scale_val(size2, size2_scale, size2_val);

	ಹಿಂತಿರುಗಿ size1_scale == size2_scale && size1_val == size2_val;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ show_dram_config(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಗಾತ್ರ_2;
	ಪೂರ್ಣಾಂಕ i;

	debug("\nRAM Configuration:\n");
	ಗಾಗಿ (i = ಗಾತ್ರ_2 = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ಗಾತ್ರ_2 += gd->dram[i].ಗಾತ್ರ_2;
		debug("Bank #%d: %llx ", i,
		      (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)(gd->dram[i].ಆರಂಭ));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG
		print_ಗಾತ್ರ(gd->dram[i].ಗಾತ್ರ_2, "\n");
#ಆದರೆ_ಅಂತ್ಯ
	}
	debug("\nDRAM:  ");

	print_ಗಾತ್ರ(gd->ram_ಗಾತ್ರ, "");
	ಆದರೆ (!sizes_near(gd->ram_ಗಾತ್ರ, ಗಾತ್ರ_2)) {
		ಬರೆ(" (total ");
		print_ಗಾತ್ರ(ಗಾತ್ರ_2, ")");
	}
	board_ಕೂಡಿಸು_ram_info(0);
	putc('\n');

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ dram_init_banksize(ಶೂನ್ಯ)
{
	gd->dram[0].ಆರಂಭ = gd->ram_ತಳ;
	gd->dram[0].ಗಾತ್ರ_2 = get_effective_memsize();

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ init_func_i2c(ಶೂನ್ಯ)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_mon_len(ಶೂನ್ಯ)
{
#ಆದರೆ defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#ಅಥವಾಆದರೆ defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)ಆರಂಭ_2;
#ಅಥವಾಆದರೆ defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)ಕೊನೆ - (ulong)_init;
#ಅಥವಾಆದರೆ defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#ಅಥವಾಆದರೆ defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)ಕೊನೆ - (ulong)_init;
#ಅಥವಾಆದರೆ defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#ಅಥವಾಆದರೆ defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(ಆರಂಭ_2);
#ಅಥವಾಆದರೆ defined(CONFIG_SYS_MONITOR_ತಳ)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_spl_handoff(ಶೂನ್ಯ)
{
#ಆದರೆ CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_ಕಂಡುಹಿಡಿ(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					ಗಾತ್ರ(ರಚನೆ spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ arch_cpu_init(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ mach_cpu_init(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_ತುದಿ(phys_ಗಾತ್ರ_t ಒಟ್ಟು_ಗಾತ್ರ)
{
	ಆದರೆ (CONFIG_IS_ENABLED(RELOC_ADDR_ತುದಿ))
		ಹಿಂತಿರುಗಿ gd->ram_ತುದಿ;

#ಆದರೆ defined(CFG_SYS_SDRAM_ತಳ) && CFG_SYS_SDRAM_ತಳ > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	ಆದರೆ (gd->ram_ತುದಿ < CFG_SYS_SDRAM_ತಳ)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ gd->ram_ತುದಿ;
}

__weak ಪೂರ್ಣಾಂಕ arch_ಸಿದ್ಧತೆ_dest_addr(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_ram_ತಳ(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CFG_SYS_SDRAM_ತಳ
	gd->ram_ತಳ = CFG_SYS_SDRAM_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_ram_config(ಶೂನ್ಯ)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	ಆದರೆ (CONFIG_IS_ENABLED(RELOC_ADDR_ತುದಿ)) {
		ಪೂರ್ಣಾಂಕ i;
		phys_addr_t ತುದಿ;

		gd->ram_ಗಾತ್ರ = 0;
		ಗಾಗಿ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			ತುದಿ = get_mem_ತುದಿ(gd->dram[i].ಆರಂಭ, gd->dram[i].ಗಾತ್ರ_2,
					  ALIGN(gd->mon_len, SZ_1M),
					  (ಶೂನ್ಯ *)gd->fdt_blob);
			gd->ram_ತುದಿ = max(ತುದಿ, gd->ram_ತುದಿ);
			gd->ram_ಗಾತ್ರ += gd->dram[i].ಗಾತ್ರ_2;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		gd->ram_ತುದಿ = gd->ram_ತಳ + get_effective_memsize();
	}
	gd->ram_ತುದಿ = board_get_usable_ram_ತುದಿ(gd->mon_len);
	/*
	 * Subtract specified amount of memory to hide so that it won't
	 * get "touched" at all by U-Boot. By fixing up gd->ram_size
	 * the Linux kernel should now get passed the now "corrected"
	 * memory size and won't touch it either. This should work
	 * for arch/ppc and arch/powerpc. Only Linux board ports in
	 * arch/powerpc with bootwrapper support, that recalculate the
	 * memory size from the SDRAM controller setup will have to
	 * get fixed.
	 */
#ಆದರೆ CONFIG_VAL(SYS_MEM_ತುದಿ_HIDE)
	gd->ram_ತುದಿ -= CONFIG_SYS_MEM_ತುದಿ_HIDE;
	gd->ram_ಗಾತ್ರ -= CONFIG_SYS_MEM_ತುದಿ_HIDE;
#ಆದರೆ_ಅಂತ್ಯ

	debug("Ram top: %08llx\n", (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)gd->ram_ತುದಿ);
	debug("Ram size: %08llx\n", (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)gd->ram_ಗಾತ್ರ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_dest_addr(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	gd->relocaddr = gd->ram_ತುದಿ;
	debug("Reloc addr: %08llX\n", (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)gd->relocaddr);

	ret = arch_ಸಿದ್ಧತೆ_dest_addr();
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	gd->initial_relocaddr = gd->relocaddr;
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CFG_PRAM
/* reserve protected RAM */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_pram(ಶೂನ್ಯ)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_ಸುತ್ತು_4k(ಶೂನ್ಯ)
{
	gd->relocaddr &= ~(4096 - 1);
	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ arch_reserve_mmu(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_video_from_videoblob(ಶೂನ್ಯ)
{
	ಆದರೆ (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ರಚನೆ video_handoff *ho;
		ಪೂರ್ಣಾಂಕ ret = 0;

		ho = bloblist_ಕಂಡುಹಿಡಿ(BLOBLISTT_U_BOOT_VIDEO, ಗಾತ್ರ(*ho));
		ಆದರೆ (!ho)
			ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ log_ಸಂದೇಶ_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		ಆದರೆ (likely(gd->relocaddr > (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ho->fb))
			gd->relocaddr = ho->fb;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * Check if any bloblist received specifying reserved areas from previous stage and adjust
 * gd->relocaddr accordingly, so that we start reserving after pre-reserved areas
 * from previous stage.
 *
 * NOTE:
 * IT is recommended that all bloblists from previous stage are reserved from ram_top
 * as next stage will simply start reserving further regions after them.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_relocaddr_from_bloblist(ಶೂನ್ಯ)
{
	reserve_video_from_videoblob();

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_video(ಶೂನ್ಯ)
{
	ಆದರೆ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ಪೂರ್ಣಾಂಕ ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_trace(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_ಬಫರ್_ಗಾತ್ರ;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_ಬಫರ್_ಗಾತ್ರ);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)CONFIG_TRACE_ಬಫರ್_ಗಾತ್ರ >> 10, gd->relocaddr);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_uboot(ಶೂನ್ಯ)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	ಆದರೆ (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	ಆದರೆ (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_ಸಂಕೇತ)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#ಆದರೆ defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#ಆದರೆ_ಅಂತ್ಯ

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->ಆರಂಭ_addr_sp = gd->relocaddr;

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ reserve_ರಾಶಿ_aligned(size_t ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ ALIGN_ಕೆಳಕ್ಕೆ(gd->ಆರಂಭ_addr_sp - ಗಾತ್ರ_2, 16);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_NONCACHED_ಸ್ಮರಣೆ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_noncached(ಶೂನ್ಯ)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->ಆರಂಭ_addr_sp = ALIGN(gd->ಆರಂಭ_addr_sp, MMU_SECTION_ಗಾತ್ರ) -
		MMU_SECTION_ಗಾತ್ರ;
	gd->ಆರಂಭ_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_ಸ್ಮರಣೆ,
				   MMU_SECTION_ಗಾತ್ರ);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_ಸ್ಮರಣೆ >> 20, gd->ಆರಂಭ_addr_sp);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* reserve memory for malloc() area */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_malloc(ಶೂನ್ಯ)
{
	gd->ಆರಂಭ_addr_sp = reserve_ರಾಶಿ_aligned(ಒಟ್ಟು_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      ಒಟ್ಟು_MALLOC_LEN >> 10, gd->ಆರಂಭ_addr_sp);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_NONCACHED_ಸ್ಮರಣೆ
	reserve_noncached();
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

/* (permanently) allocate a Board Info struct */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_board(ಶೂನ್ಯ)
{
	ಆದರೆ (!gd->bd) {
		gd->ಆರಂಭ_addr_sp = reserve_ರಾಶಿ_aligned(ಗಾತ್ರ(ರಚನೆ bd_info));
		gd->bd = (ರಚನೆ bd_info *)map_sysmem(gd->ಆರಂಭ_addr_sp,
						      ಗಾತ್ರ(ರಚನೆ bd_info));
		memset(gd->bd, '\0', ಗಾತ್ರ(ರಚನೆ bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      ಗಾತ್ರ(ರಚನೆ bd_info), gd->ಆರಂಭ_addr_sp);
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_global_ದತ್ತಾಂಶ(ಶೂನ್ಯ)
{
	gd->ಆರಂಭ_addr_sp = reserve_ರಾಶಿ_aligned(ಗಾತ್ರ(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->ಆರಂಭ_addr_sp, ಗಾತ್ರ(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      ಗಾತ್ರ(gd_t), gd->ಆರಂಭ_addr_sp);
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_fdt(ಶೂನ್ಯ)
{
	ಆದರೆ (!IS_ENABLED(CONFIG_ದ_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		ಆದರೆ (gd->fdt_blob) {
			gd->boardf->fdt_ಗಾತ್ರ =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->ಆರಂಭ_addr_sp = reserve_ರಾಶಿ_aligned(
				gd->boardf->fdt_ಗಾತ್ರ);
			gd->boardf->new_fdt = map_sysmem(gd->ಆರಂಭ_addr_sp,
							 gd->boardf->fdt_ಗಾತ್ರ);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_ಗಾತ್ರ, gd->ಆರಂಭ_addr_sp);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_bootstage(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BOOTSTAGE
	ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2 = bootstage_get_ಗಾತ್ರ(true);

	gd->ಆರಂಭ_addr_sp = reserve_ರಾಶಿ_aligned(ಗಾತ್ರ_2);
	gd->boardf->new_bootstage = map_sysmem(gd->ಆರಂಭ_addr_sp, ಗಾತ್ರ_2);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", ಗಾತ್ರ_2,
	      gd->ಆರಂಭ_addr_sp);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ arch_reserve_stacks(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_stacks(ಶೂನ್ಯ)
{
	/* make stack pointer 16-byte aligned */
	gd->ಆರಂಭ_addr_sp = reserve_ರಾಶಿ_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	ಹಿಂತಿರುಗಿ arch_reserve_stacks();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserve_bloblist(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BLOBLIST
	ulong ಗಾತ್ರ_2 = bloblist_get_ಒಟ್ಟು_ಗಾತ್ರ();

	ಆದರೆ (ಗಾತ್ರ_2 < CONFIG_BLOBLIST_ಗಾತ್ರ_RELOC)
		ಗಾತ್ರ_2 = CONFIG_BLOBLIST_ಗಾತ್ರ_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->ಆರಂಭ_addr_sp = ALIGN_ಕೆಳಕ್ಕೆ(gd->ಆರಂಭ_addr_sp - ಗಾತ್ರ_2, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->ಆರಂಭ_addr_sp, ಗಾತ್ರ_2);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ display_new_sp(ಶೂನ್ಯ)
{
	debug("New Stack Pointer is: %08lx\n", gd->ಆರಂಭ_addr_sp);

	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ arch_ಸಿದ್ಧತೆ_bdinfo(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_bdinfo(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ arch_ಸಿದ್ಧತೆ_bdinfo();
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_POST
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ init_post(ಶೂನ್ಯ)
{
	post_bootmode_init();
	post_ಸತತ(ಖಾಲಿ, POST_ROM | post_bootmode_get(0));

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reloc_fdt(ಶೂನ್ಯ)
{
	ಆದರೆ (!IS_ENABLED(CONFIG_ದ_EMBED)) {
		ಆದರೆ (gd->boardf->new_fdt) {
			ಸ್ಮರಣೆ_ನಕಲು(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reloc_bootstage(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BOOTSTAGE
	ಆದರೆ (gd->flags & GD_FLG_SKIP_RELOC)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reloc_bloblist(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	ಆದರೆ ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_ಗಾತ್ರ == CONFIG_BLOBLIST_ಗಾತ್ರ_RELOC) {
		debug("Not relocating bloblist\n");
		ಹಿಂತಿರುಗಿ 0;
	}
	ಆದರೆ (gd->boardf->new_bloblist) {
		ulong ಗಾತ್ರ_2 = bloblist_get_ಒಟ್ಟು_ಗಾತ್ರ();

		ಆದರೆ (ಗಾತ್ರ_2 < CONFIG_BLOBLIST_ಗಾತ್ರ_RELOC)
			ಗಾತ್ರ_2 = CONFIG_BLOBLIST_ಗಾತ್ರ_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, ಗಾತ್ರ_2);
		ಹಿಂತಿರುಗಿ bloblist_reloc(gd->boardf->new_bloblist, ಗಾತ್ರ_2);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}

ಶೂನ್ಯ mcheck_on_ramrelocation(size_t ಸ್ಥಾನಾಂತರ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_reloc(ಶೂನ್ಯ)
{
	ಆದರೆ (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_ಪಠ್ಯ_ತಳ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ ARM
		gd->reloc_off = gd->relocaddr - (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)__image_copy_start;
#ಅಥವಾಆದರೆ defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)ಆರಂಭ_2;
#ಅಥವಾಆದರೆ defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_ಪಠ್ಯ_ತಳ + 0x400);
#ಅಥವಾಆದರೆ !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_ಪಠ್ಯ_ತಳ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಸ್ಮರಣೆ_ನಕಲು(gd->new_gd, (ಅಕ್ಷರ *)gd, ಗಾತ್ರ(gd_t));

	ಆದರೆ (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ MCHECK_ರಾಶಿ_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#ಆದರೆ_ಅಂತ್ಯ
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_ಗಮ್ಯ_sysmem(gd->new_gd),
		      gd->ಆರಂಭ_addr_sp);
	}

	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ CONFIG_IS_ENABLED(ದ_BOARD_FIXUP)
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fix_fdt(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ board_fix_fdt((ಶೂನ್ಯ *)gd->fdt_blob);
}
#ಆದರೆ_ಅಂತ್ಯ

/* ARM calls relocate_code from its crt0.S */
#ಆದರೆ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ jump_ಗಮ್ಯ_copy(ಶೂನ್ಯ)
{
	ಆದರೆ (gd->flags & GD_FLG_SKIP_RELOC)
		ಹಿಂತಿರುಗಿ 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#ಆದರೆ defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_ಸಿದ್ಧತೆ_gd(gd->new_gd);
# ಆದರೆ CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->ಆರಂಭ_addr_sp);
# ಇಲ್ಲದಿದ್ದರೆ
		board_init_f_r_trampoline(gd->ಆರಂಭ_addr_sp);
# endif
#ಇಲ್ಲದಿದ್ದರೆ
	relocate_ಸಂಕೇತ(gd->ಆರಂಭ_addr_sp, gd->new_gd, gd->relocaddr);
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initf_bootstage(ಶೂನ್ಯ)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	ಪೂರ್ಣಾಂಕ ret;

	ret = bootstage_init(!from_spl);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	ಆದರೆ (from_spl) {
		ret = bootstage_unstash_default();
		ಆದರೆ (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			ಹಿಂತಿರುಗಿ ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_ಆರಂಭ_UBOOT_F, "board_init_f");

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initf_dm(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		ಹಿಂತಿರುಗಿ 0;

	bootstage_ಆರಂಭ(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	ಆದರೆ (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_ಪರಿಶೀಲನೆ(true);
	ಇಲ್ಲದಿದ್ದರೆ
		ret = dm_init(false);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ret = dm_autoprobe();
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	ಆದರೆ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ 0;
}

/* Architecture-specific memory reservation */
__weak ಪೂರ್ಣಾಂಕ reserve_arch(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ checkcpu(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

__weak ಪೂರ್ಣಾಂಕ clear_bss(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initf_upl(ಶೂನ್ಯ)
{
	ರಚನೆ upl *upl;
	ಪೂರ್ಣಾಂಕ ret;

	ಆದರೆ (!IS_ENABLED(CONFIG_UPL_ಒಳ) || !(gd->flags & GD_FLG_UPL))
		ಹಿಂತಿರುಗಿ 0;

	upl = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ upl));
	ಆದರೆ (!upl)
		ಹಿಂತಿರುಗಿ -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	ಆದರೆ (ret) {
		ಬರೆ("UPL handoff: read failure (err=%dE)\n", ret);
		ಹಿಂತಿರುಗಿ ret;
	}
	gd_ಗಣ_upl(upl);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ initcall_ಸತತ_f(ಶೂನ್ಯ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(ಸಿದ್ಧತೆ_mon_len);
	INITCALL(initf_malloc);
#ಆದರೆ CONFIG_IS_ENABLED(ದ_CONTROL)
	INITCALL(fdtdec_ಸಿದ್ಧತೆ);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#ಆದರೆ CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(ಸಿದ್ಧತೆ_spl_handoff);
#ಆದರೆ CONFIG_IS_ENABLED(CONSOLE_ದಾಖಲೆ_INIT_F)
	INITCALL(console_ದಾಖಲೆ_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#ಆದರೆ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_ದರ);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_ಪಠ್ಯ_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#ಆದರೆ CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#ಆದರೆ_ಅಂತ್ಯ
	/* display cpu info (and speed) */
#ಆದರೆ CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#ಆದರೆ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#ಆದರೆ CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
#ಆದರೆ defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#ಆದರೆ_ಅಂತ್ಯ /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#ಆದರೆ CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#ಆದರೆ_ಅಂತ್ಯ
	WATCHDOG_RESET();
	/*
	 * Now that we have DRAM mapped and working, we can
	 * relocate the code and continue running from DRAM.
	 *
	 * Reserve memory at end of RAM for (top down in that order):
	 *  - area that won't get touched by U-Boot and Linux (optional)
	 *  - kernel log buffer
	 *  - protected RAM
	 *  - LCD framebuffer
	 *  - monitor code
	 *  - board info struct
	 */
	INITCALL(ಸಿದ್ಧತೆ_ram_ತಳ);
	INITCALL(dram_init_banksize);
	INITCALL(ಸಿದ್ಧತೆ_ram_config);
	INITCALL(ಸಿದ್ಧತೆ_dest_addr);
#ಆದರೆ CONFIG_IS_ENABLED(ದ_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(ದ_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CFG_PRAM
	INITCALL(reserve_pram);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(reserve_ಸುತ್ತು_4k);
	INITCALL(ಸಿದ್ಧತೆ_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_ದತ್ತಾಂಶ);
	INITCALL(reserve_fdt);
#ಆದರೆ CONFIG_IS_ENABLED(ದ_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(ದ_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(ಸಿದ್ಧತೆ_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#ಆದರೆ !CONFIG_IS_ENABLED(ದ_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(ಸಿದ್ಧತೆ_reloc);
#ಆದರೆ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_ಗಮ್ಯ_ram);
	INITCALL(do_elf_reloc_fixups);
#ಆದರೆ_ಅಂತ್ಯ
	INITCALL(clear_bss);
	/*
	 * Deregister all cyclic functions before relocation, so that
	 * gd->cyclic_list does not contain any references to pre-relocation
	 * devices. Drivers will register their cyclic functions anew when the
	 * devices are probed again.
	 *
	 * This should happen as late as possible so that the window where a
	 * watchdog device is not serviced is as small as possible.
	 */
	INITCALL(cyclic_unregister_all);
#ಆದರೆ !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_ಗಮ್ಯ_copy);
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ board_init_f(ulong boot_flags)
{
	ರಚನೆ board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_ಸತತ_f();

#ಆದರೆ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#ಆದರೆ_ಅಂತ್ಯ
}

#ಆದರೆ defined(CONFIG_X86) || defined(CONFIG_ARC)
/*
 * For now this code is only used on x86.
 *
 * Run init functions which are run when U-Boot is executing from Flash with a
 * semi-limited 'C' environment.
 * The following limitations must be considered when implementing an
 * '_f_r' function:
 *  - 'static' variables are read-only
 *  - Global Data (gd->xxx) is read/write
 *
 * The '_f_r' sequence must, as a minimum, copy U-Boot to RAM (if
 * supported).  It _should_, if possible, copy global data to RAM and
 * initialise the CPU caches (to speed up the relocation process)
 *
 * NOTE: At present only x86 uses this route, but it is intended that
 * all archs will move to this when generic relocation is implemented.
 */
ಸ್ಥಿರ ಶೂನ್ಯ initcall_ಸತತ_f_r(ಶೂನ್ಯ)
{
#ಆದರೆ !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#ಆದರೆ_ಅಂತ್ಯ
}

ಶೂನ್ಯ board_init_f_r(ಶೂನ್ಯ)
{
	initcall_ಸತತ_f_r();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_TIMER
	gd->timer = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_X86 */
