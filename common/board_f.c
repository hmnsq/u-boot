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

DECLARE_GLOBAL_DATA_PTR;

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

#ಯದಿ defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ init_func_watchdog_init(ಶೂನ್ಯ)
{
# ಯದಿ defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ init_func_watchdog_reset(ಶೂನ್ಯ)
{
	schedule();

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CONFIG_WATCHDOG */

__weak ಶೂನ್ಯ board_add_ram_info(ಪೂರ್ಣಾಙ್ಕ use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ init_baud_rate(ಶೂನ್ಯ)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ display_text_info(ಶೂನ್ಯ)
{
#ಯದಿ !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#ಅನ್ಯಥಾ
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ print_resetinfo(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ udevice *dev;
	ವರ್ಣ status[256];
	bool status_printed = false;
	ಪೂರ್ಣಾಙ್ಕ ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	ಪ್ರತಿ (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		ಯದಿ (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			ಅನುವರ್ತ;
		}

		ಯದಿ (!sysreset_get_status(dev, status, ಪರಿಮಾಣ(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	ಯದಿ (status_printed)
		printf("\n");

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

#ಯದಿ defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ print_cpuinfo(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ udevice *dev;
	ವರ್ಣ desc[512];
	ಪೂರ್ಣಾಙ್ಕ ret;

	dev = cpu_get_current_dev();
	ಯದಿ (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		ಪ್ರತ್ಯಾಗಮ -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, ಪರಿಮಾಣ(desc));
	ಯದಿ (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}

	printf("CPU:   %s\n", desc);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ announce_dram_init(ಶೂನ್ಯ)
{
	puts("DRAM:  ");
	ಪ್ರತ್ಯಾಗಮ 0;
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
#define compute_size_scale_val(size, scale, val) ಕುರು { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	ಯದಿ (val == 10240) { val = 10; scale += 10; } \
} ಯಾವತ್ (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	ಪ್ರತ್ಯಾಗಮ size1_scale == size2_scale && size1_val == size2_val;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ show_dram_config(ಶೂನ್ಯ)
{
	unsigned long long size;
	ಪೂರ್ಣಾಙ್ಕ i;

	debug("\nRAM Configuration:\n");
	ಪ್ರತಿ (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	ಯದಿ (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ dram_init_banksize(ಶೂನ್ಯ)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ init_func_i2c(ಶೂನ್ಯ)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_mon_len(ಶೂನ್ಯ)
{
#ಯದಿ defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#ನೋಚೇದ್ಯದಿ defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#ನೋಚೇದ್ಯದಿ defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_spl_handoff(ಶೂನ್ಯ)
{
#ಯದಿ CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					ಪರಿಮಾಣ(ಸಂರಚನಾ spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ arch_cpu_init(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ mach_cpu_init(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_ತುದಿ(phys_size_t total_size)
{
	ಯದಿ (CONFIG_IS_ENABLED(RELOC_ADDR_ತುದಿ))
		ಪ್ರತ್ಯಾಗಮ gd->ram_ತುದಿ;

#ಯದಿ defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	ಯದಿ (gd->ram_ತುದಿ < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		ಪ್ರತ್ಯಾಗಮ 0;
#endif
	ಪ್ರತ್ಯಾಗಮ gd->ram_ತುದಿ;
}

__weak ಪೂರ್ಣಾಙ್ಕ arch_setup_dest_addr(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_ram_base(ಶೂನ್ಯ)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_ram_config(ಶೂನ್ಯ)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	ಯದಿ (CONFIG_IS_ENABLED(RELOC_ADDR_ತುದಿ)) {
		ಪೂರ್ಣಾಙ್ಕ i;
		phys_addr_t ತುದಿ;

		gd->ram_size = 0;
		ಪ್ರತಿ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			ತುದಿ = get_mem_ತುದಿ(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (ಶೂನ್ಯ *)gd->fdt_blob);
			gd->ram_ತುದಿ = max(ತುದಿ, gd->ram_ತುದಿ);
			gd->ram_size += gd->dram[i].size;
		}
	} ಅನ್ಯಥಾ {
		gd->ram_ತುದಿ = gd->ram_base + get_effective_memsize();
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
#ಯದಿ CONFIG_VAL(SYS_MEM_ತುದಿ_HIDE)
	gd->ram_ತುದಿ -= CONFIG_SYS_MEM_ತುದಿ_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_ತುದಿ_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_ತುದಿ);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_dest_addr(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	gd->relocaddr = gd->ram_ತುದಿ;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	gd->initial_relocaddr = gd->relocaddr;
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_pram(ಶೂನ್ಯ)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_round_4k(ಶೂನ್ಯ)
{
	gd->relocaddr &= ~(4096 - 1);
	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ arch_reserve_mmu(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_video_from_videoblob(ಶೂನ್ಯ)
{
	ಯದಿ (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ಸಂರಚನಾ video_handoff *ho;
		ಪೂರ್ಣಾಙ್ಕ ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, ಪರಿಮಾಣ(*ho));
		ಯದಿ (!ho)
			ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ log_ಸಂದೇಶ_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		ಯದಿ (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_relocaddr_from_bloblist(ಶೂನ್ಯ)
{
	reserve_video_from_videoblob();

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_video(ಶೂನ್ಯ)
{
	ಯದಿ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ಪೂರ್ಣಾಙ್ಕ ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_trace(ಶೂನ್ಯ)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_uboot(ಶೂನ್ಯ)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	ಯದಿ (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	ಯದಿ (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#ಯದಿ defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	ಪ್ರತ್ಯಾಗಮ 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ಸ್ಥಿರ unsigned long reserve_ರಾಶಿ_aligned(size_t size)
{
	ಪ್ರತ್ಯಾಗಮ ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_noncached(ಶೂನ್ಯ)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->start_addr_sp = ALIGN(gd->start_addr_sp, MMU_SECTION_SIZE) -
		MMU_SECTION_SIZE;
	gd->start_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_MEMORY,
				   MMU_SECTION_SIZE);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_MEMORY >> 20, gd->start_addr_sp);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

/* reserve memory for malloc() area */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_malloc(ಶೂನ್ಯ)
{
	gd->start_addr_sp = reserve_ರಾಶಿ_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* (permanently) allocate a Board Info struct */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_board(ಶೂನ್ಯ)
{
	ಯದಿ (!gd->bd) {
		gd->start_addr_sp = reserve_ರಾಶಿ_aligned(ಪರಿಮಾಣ(ಸಂರಚನಾ bd_info));
		gd->bd = (ಸಂರಚನಾ bd_info *)map_sysmem(gd->start_addr_sp,
						      ಪರಿಮಾಣ(ಸಂರಚನಾ bd_info));
		memset(gd->bd, '\0', ಪರಿಮಾಣ(ಸಂರಚನಾ bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      ಪರಿಮಾಣ(ಸಂರಚನಾ bd_info), gd->start_addr_sp);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_global_data(ಶೂನ್ಯ)
{
	gd->start_addr_sp = reserve_ರಾಶಿ_aligned(ಪರಿಮಾಣ(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, ಪರಿಮಾಣ(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      ಪರಿಮಾಣ(gd_t), gd->start_addr_sp);
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_fdt(ಶೂನ್ಯ)
{
	ಯದಿ (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		ಯದಿ (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_ರಾಶಿ_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_bootstage(ಶೂನ್ಯ)
{
#ifdef CONFIG_BOOTSTAGE
	ಪೂರ್ಣಾಙ್ಕ size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_ರಾಶಿ_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ arch_reserve_stacks(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_stacks(ಶೂನ್ಯ)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_ರಾಶಿ_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	ಪ್ರತ್ಯಾಗಮ arch_reserve_stacks();
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserve_bloblist(ಶೂನ್ಯ)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	ಯದಿ (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ display_new_sp(ಶೂನ್ಯ)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ arch_setup_bdinfo(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ setup_bdinfo(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ arch_setup_bdinfo();
}

#ifdef CONFIG_POST
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ init_post(ಶೂನ್ಯ)
{
	post_bootmode_init();
	post_run(ರಿಕ್ತ, POST_ROM | post_bootmode_get(0));

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reloc_fdt(ಶೂನ್ಯ)
{
	ಯದಿ (!IS_ENABLED(CONFIG_OF_EMBED)) {
		ಯದಿ (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reloc_bootstage(ಶೂನ್ಯ)
{
#ifdef CONFIG_BOOTSTAGE
	ಯದಿ (gd->flags & GD_FLG_SKIP_RELOC)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reloc_bloblist(ಶೂನ್ಯ)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	ಯದಿ ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಯದಿ (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		ಯದಿ (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		ಪ್ರತ್ಯಾಗಮ bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ mcheck_on_ramrelocation(size_t offset);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_reloc(ಶೂನ್ಯ)
{
	ಯದಿ (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#ನೋಚೇದ್ಯದಿ defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#ನೋಚೇದ್ಯದಿ !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (ವರ್ಣ *)gd, ಪರಿಮಾಣ(gd_t));

	ಯದಿ (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} ಅನ್ಯಥಾ {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fix_fdt(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ board_fix_fdt((ಶೂನ್ಯ *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#ಯದಿ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ jump_to_copy(ಶೂನ್ಯ)
{
	ಯದಿ (gd->flags & GD_FLG_SKIP_RELOC)
		ಪ್ರತ್ಯಾಗಮ 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#ಯದಿ defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# ಯದಿ CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# ಅನ್ಯಥಾ
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#ಅನ್ಯಥಾ
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ initf_bootstage(ಶೂನ್ಯ)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	ಪೂರ್ಣಾಙ್ಕ ret;

	ret = bootstage_init(!from_spl);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	ಯದಿ (from_spl) {
		ret = bootstage_unstash_default();
		ಯದಿ (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			ಪ್ರತ್ಯಾಗಮ ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ initf_dm(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		ಪ್ರತ್ಯಾಗಮ 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	ಯದಿ (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	ಅನ್ಯಥಾ
		ret = dm_init(false);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ret = dm_autoprobe();
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	ಯದಿ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

/* Architecture-specific memory reservation */
__weak ಪೂರ್ಣಾಙ್ಕ reserve_arch(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ checkcpu(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ clear_bss(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ initf_upl(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ upl *upl;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		ಪ್ರತ್ಯಾಗಮ 0;

	upl = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ upl));
	ಯದಿ (!upl)
		ಪ್ರತ್ಯಾಗಮ -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	ಯದಿ (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}
	gd_set_upl(upl);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ initcall_run_f(ಶೂನ್ಯ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#ಯದಿ CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#ಯದಿ CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#ಯದಿ CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#ಯದಿ CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#ಯದಿ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#ಯದಿ defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#ಯದಿ !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#ಯದಿ CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#ಯದಿ CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#ಯದಿ CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#ಯದಿ CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#ಯದಿ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#ಯದಿ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#ಯದಿ CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#ಯದಿ defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#ಯದಿ CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#endif
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
	INITCALL(setup_ram_base);
	INITCALL(dram_init_banksize);
	INITCALL(setup_ram_config);
	INITCALL(setup_dest_addr);
#ಯದಿ CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(OF_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#endif
#ifdef CFG_PRAM
	INITCALL(reserve_pram);
#endif
	INITCALL(reserve_round_4k);
	INITCALL(setup_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_data);
	INITCALL(reserve_fdt);
#ಯದಿ CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(OF_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#endif
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(setup_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#ಯದಿ !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#ಯದಿ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_to_ram);
	INITCALL(do_elf_reloc_fixups);
#endif
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
#ಯದಿ !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

ಶೂನ್ಯ board_init_f(ulong boot_flags)
{
	ಸಂರಚನಾ board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#ಯದಿ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#ಯದಿ defined(CONFIG_X86) || defined(CONFIG_ARC)
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
ಸ್ಥಿರ ಶೂನ್ಯ initcall_run_f_r(ಶೂನ್ಯ)
{
#ಯದಿ !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

ಶೂನ್ಯ board_init_f_r(ಶೂನ್ಯ)
{
	initcall_run_f_r();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#ifdef CONFIG_TIMER
	gd->timer = ರಿಕ್ತ;
#endif

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#endif /* CONFIG_X86 */
