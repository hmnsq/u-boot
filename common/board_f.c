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

#অন্তর্ভাৱ <config.h>
#অন্তর্ভাৱ <bloblist.h>
#অন্তর্ভাৱ <bootstage.h>
#অন্তর্ভাৱ <clock_legacy.h>
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <cpu.h>
#অন্তর্ভাৱ <cpu_func.h>
#অন্তর্ভাৱ <cyclic.h>
#অন্তর্ভাৱ <display_options.h>
#অন্তর্ভাৱ <dm.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <env_internal.h>
#অন্তর্ভাৱ <event.h>
#অন্তর্ভাৱ <fdtdec.h>
#অন্তর্ভাৱ <fs.h>
#অন্তর্ভাৱ <hang.h>
#অন্তর্ভাৱ <i2c.h>
#অন্তর্ভাৱ <init.h>
#অন্তর্ভাৱ <initcall.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <memtop.h>
#অন্তর্ভাৱ <os.h>
#অন্তর্ভাৱ <post.h>
#অন্তর্ভাৱ <relocate.h>
#অন্তর্ভাৱ <serial.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <sysreset.h>
#অন্তর্ভাৱ <time.h>
#অন্তর্ভাৱ <timer.h>
#অন্তর্ভাৱ <trace.h>
#অন্তর্ভাৱ <upl.h>
#অন্তর্ভাৱ <video.h>
#অন্তর্ভাৱ <watchdog.h>
#অন্তর্ভাৱ <asm/cache.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <asm/io.h>
#অন্তর্ভাৱ <asm/sections.h>
#অন্তর্ভাৱ <dm/root.h>
#অন্তর্ভাৱ <linux/errno.h>
#অন্তর্ভাৱ <linux/log2.h>
#অন্তর্ভাৱ <linux/sizes.h>

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

#যদি defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
স্থির পূর্ণাঙ্ক init_func_watchdog_init(শূন্য)
{
# যদি defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	ঘূৰাই 0;
}

পূর্ণাঙ্ক init_func_watchdog_reset(শূন্য)
{
	schedule();

	ঘূৰাই 0;
}
#endif /* CONFIG_WATCHDOG */

__weak শূন্য board_add_ram_info(পূর্ণাঙ্ক use_default)
{
	/* please define platform specific board_add_ram_info() */
}

স্থির পূর্ণাঙ্ক init_baud_rate(শূন্য)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক display_text_info(শূন্য)
{
#যদি !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#অন্যথা
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	ঘূৰাই 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
স্থির পূর্ণাঙ্ক print_resetinfo(শূন্য)
{
	সংরচনা udevice *dev;
	ৱর্ণ status[256];
	bool status_printed = false;
	পূর্ণাঙ্ক ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	বাবে (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		যদি (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			অনুৱর্ত;
		}

		যদি (!sysreset_get_status(dev, status, পরিমাণ(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	যদি (status_printed)
		printf("\n");

	ঘূৰাই 0;
}
#endif

#যদি defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
স্থির পূর্ণাঙ্ক print_cpuinfo(শূন্য)
{
	সংরচনা udevice *dev;
	ৱর্ণ desc[512];
	পূর্ণাঙ্ক ret;

	dev = cpu_get_current_dev();
	যদি (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		ঘূৰাই -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, পরিমাণ(desc));
	যদি (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		ঘূৰাই ret;
	}

	printf("CPU:   %s\n", desc);

	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক announce_dram_init(শূন্য)
{
	puts("DRAM:  ");
	ঘূৰাই 0;
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
#define compute_size_scale_val(size, scale, val) কৰা { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	যদি (val == 10240) { val = 10; scale += 10; } \
} যেতিয়ালৈ (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
স্থির পূর্ণাঙ্ক sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned পূর্ণাঙ্ক size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	ঘূৰাই size1_scale == size2_scale && size1_val == size2_val;
}

স্থির পূর্ণাঙ্ক show_dram_config(শূন্য)
{
	unsigned long long size;
	পূর্ণাঙ্ক i;

	debug("\nRAM Configuration:\n");
	বাবে (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	যদি (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক dram_init_banksize(শূন্য)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	ঘূৰাই 0;
}

#যদি CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
স্থির পূর্ণাঙ্ক init_func_i2c(শূন্য)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক setup_mon_len(শূন্য)
{
#যদি defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#নোচেদ্যদি defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#নোচেদ্যদি defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#নোচেদ্যদি defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#নোচেদ্যদি defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#নোচেদ্যদি defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#নোচেদ্যদি defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#নোচেদ্যদি defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক setup_spl_handoff(শূন্য)
{
#যদি CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					পরিমাণ(সংরচনা spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক arch_cpu_init(শূন্য)
{
	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক mach_cpu_init(শূন্য)
{
	ঘূৰাই 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_শীৰ্ষ(phys_size_t total_size)
{
	যদি (CONFIG_IS_ENABLED(RELOC_ADDR_শীৰ্ষ))
		ঘূৰাই gd->ram_শীৰ্ষ;

#যদি defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	যদি (gd->ram_শীৰ্ষ < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		ঘূৰাই 0;
#endif
	ঘূৰাই gd->ram_শীৰ্ষ;
}

__weak পূর্ণাঙ্ক arch_setup_dest_addr(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক setup_ram_base(শূন্য)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক setup_ram_config(শূন্য)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	যদি (CONFIG_IS_ENABLED(RELOC_ADDR_শীৰ্ষ)) {
		পূর্ণাঙ্ক i;
		phys_addr_t শীৰ্ষ;

		gd->ram_size = 0;
		বাবে (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			শীৰ্ষ = get_mem_শীৰ্ষ(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (শূন্য *)gd->fdt_blob);
			gd->ram_শীৰ্ষ = max(শীৰ্ষ, gd->ram_শীৰ্ষ);
			gd->ram_size += gd->dram[i].size;
		}
	} অন্যথা {
		gd->ram_শীৰ্ষ = gd->ram_base + get_effective_memsize();
	}
	gd->ram_শীৰ্ষ = board_get_usable_ram_শীৰ্ষ(gd->mon_len);
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
#যদি CONFIG_VAL(SYS_MEM_শীৰ্ষ_HIDE)
	gd->ram_শীৰ্ষ -= CONFIG_SYS_MEM_শীৰ্ষ_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_শীৰ্ষ_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_শীৰ্ষ);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক setup_dest_addr(শূন্য)
{
	পূর্ণাঙ্ক ret;

	gd->relocaddr = gd->ram_শীৰ্ষ;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	যদি (ret)
		ঘূৰাই ret;

	gd->initial_relocaddr = gd->relocaddr;
	ঘূৰাই 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
স্থির পূর্ণাঙ্ক reserve_pram(শূন্য)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	ঘূৰাই 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
স্থির পূর্ণাঙ্ক reserve_round_4k(শূন্য)
{
	gd->relocaddr &= ~(4096 - 1);
	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক arch_reserve_mmu(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_video_from_videoblob(শূন্য)
{
	যদি (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		সংরচনা video_handoff *ho;
		পূর্ণাঙ্ক ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, পরিমাণ(*ho));
		যদি (!ho)
			ঘূৰাই log_বাৰ্তা_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		যদি (ret)
			ঘূৰাই log_বাৰ্তা_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		যদি (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	ঘূৰাই 0;
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
স্থির পূর্ণাঙ্ক setup_relocaddr_from_bloblist(শূন্য)
{
	reserve_video_from_videoblob();

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_video(শূন্য)
{
	যদি (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		পূর্ণাঙ্ক ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		যদি (ret)
			ঘূৰাই ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_trace(শূন্য)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_uboot(শূন্য)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	যদি (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	যদি (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#যদি defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	ঘূৰাই 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
স্থির unsigned long reserve_স্তূপ_aligned(size_t size)
{
	ঘূৰাই ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
স্থির পূর্ণাঙ্ক reserve_noncached(শূন্য)
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

	ঘূৰাই 0;
}
#endif

/* reserve memory for malloc() area */
স্থির পূর্ণাঙ্ক reserve_malloc(শূন্য)
{
	gd->start_addr_sp = reserve_স্তূপ_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	ঘূৰাই 0;
}

/* (permanently) allocate a Board Info struct */
স্থির পূর্ণাঙ্ক reserve_board(শূন্য)
{
	যদি (!gd->bd) {
		gd->start_addr_sp = reserve_স্তূপ_aligned(পরিমাণ(সংরচনা bd_info));
		gd->bd = (সংরচনা bd_info *)map_sysmem(gd->start_addr_sp,
						      পরিমাণ(সংরচনা bd_info));
		memset(gd->bd, '\0', পরিমাণ(সংরচনা bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      পরিমাণ(সংরচনা bd_info), gd->start_addr_sp);
	}
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_global_data(শূন্য)
{
	gd->start_addr_sp = reserve_স্তূপ_aligned(পরিমাণ(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, পরিমাণ(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      পরিমাণ(gd_t), gd->start_addr_sp);
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_fdt(শূন্য)
{
	যদি (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		যদি (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_স্তূপ_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_bootstage(শূন্য)
{
#ifdef CONFIG_BOOTSTAGE
	পূর্ণাঙ্ক size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_স্তূপ_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক arch_reserve_stacks(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reserve_stacks(শূন্য)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_স্তূপ_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	ঘূৰাই arch_reserve_stacks();
}

স্থির পূর্ণাঙ্ক reserve_bloblist(শূন্য)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	যদি (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক display_new_sp(শূন্য)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক arch_setup_bdinfo(শূন্য)
{
	ঘূৰাই 0;
}

পূর্ণাঙ্ক setup_bdinfo(শূন্য)
{
	ঘূৰাই arch_setup_bdinfo();
}

#ifdef CONFIG_POST
স্থির পূর্ণাঙ্ক init_post(শূন্য)
{
	post_bootmode_init();
	post_run(রিক্ত, POST_ROM | post_bootmode_get(0));

	ঘূৰাই 0;
}
#endif

স্থির পূর্ণাঙ্ক reloc_fdt(শূন্য)
{
	যদি (!IS_ENABLED(CONFIG_OF_EMBED)) {
		যদি (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reloc_bootstage(শূন্য)
{
#ifdef CONFIG_BOOTSTAGE
	যদি (gd->flags & GD_FLG_SKIP_RELOC)
		ঘূৰাই 0;
	যদি (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক reloc_bloblist(শূন্য)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	যদি ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		ঘূৰাই 0;
	}
	যদি (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		যদি (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		ঘূৰাই bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	ঘূৰাই 0;
}

শূন্য mcheck_on_ramrelocation(size_t offset);
স্থির পূর্ণাঙ্ক setup_reloc(শূন্য)
{
	যদি (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#নোচেদ্যদি defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#নোচেদ্যদি defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#নোচেদ্যদি !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (ৱর্ণ *)gd, পরিমাণ(gd_t));

	যদি (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} অন্যথা {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	ঘূৰাই 0;
}

#যদি CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
স্থির পূর্ণাঙ্ক fix_fdt(শূন্য)
{
	ঘূৰাই board_fix_fdt((শূন্য *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#যদি !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

স্থির পূর্ণাঙ্ক jump_to_copy(শূন্য)
{
	যদি (gd->flags & GD_FLG_SKIP_RELOC)
		ঘূৰাই 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#যদি defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# যদি CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# অন্যথা
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#অন্যথা
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	ঘূৰাই 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
স্থির পূর্ণাঙ্ক initf_bootstage(শূন্য)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	পূর্ণাঙ্ক ret;

	ret = bootstage_init(!from_spl);
	যদি (ret)
		ঘূৰাই ret;
	যদি (from_spl) {
		ret = bootstage_unstash_default();
		যদি (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			ঘূৰাই ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initf_dm(শূন্য)
{
	পূর্ণাঙ্ক ret;

	যদি (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		ঘূৰাই 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	যদি (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	অন্যথা
		ret = dm_init(false);
	যদি (ret)
		ঘূৰাই ret;

	ret = dm_autoprobe();
	যদি (ret)
		ঘূৰাই ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	যদি (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		যদি (ret)
			ঘূৰাই ret;
	}

	ঘূৰাই 0;
}

/* Architecture-specific memory reservation */
__weak পূর্ণাঙ্ক reserve_arch(শূন্য)
{
	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক checkcpu(শূন্য)
{
	ঘূৰাই 0;
}

__weak পূর্ণাঙ্ক clear_bss(শূন্য)
{
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক initf_upl(শূন্য)
{
	সংরচনা upl *upl;
	পূর্ণাঙ্ক ret;

	যদি (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		ঘূৰাই 0;

	upl = malloc(পরিমাণ(সংরচনা upl));
	যদি (!upl)
		ঘূৰাই -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	যদি (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		ঘূৰাই ret;
	}
	gd_set_upl(upl);

	ঘূৰাই 0;
}

স্থির শূন্য initcall_run_f(শূন্য)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#যদি CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#যদি CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#যদি CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#যদি CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#যদি CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#যদি defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#যদি !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#যদি CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#যদি CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#যদি CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#যদি CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#যদি CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#যদি CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#যদি CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#যদি defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#যদি CONFIG_IS_ENABLED(POST)
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
#যদি CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#যদি CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#যদি !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#যদি CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#যদি !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

শূন্য board_init_f(ulong boot_flags)
{
	সংরচনা board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#যদি !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#যদি defined(CONFIG_X86) || defined(CONFIG_ARC)
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
স্থির শূন্য initcall_run_f_r(শূন্য)
{
#যদি !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

শূন্য board_init_f_r(শূন্য)
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
	gd->timer = রিক্ত;
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
