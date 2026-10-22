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

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <bloblist.h>
#അന്തര്ഭാവ <bootstage.h>
#അന്തര്ഭാവ <clock_legacy.h>
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <cpu.h>
#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <cyclic.h>
#അന്തര്ഭാവ <display_options.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <env_internal.h>
#അന്തര്ഭാവ <event.h>
#അന്തര്ഭാവ <fdtdec.h>
#അന്തര്ഭാവ <fs.h>
#അന്തര്ഭാവ <hang.h>
#അന്തര്ഭാവ <i2c.h>
#അന്തര്ഭാവ <init.h>
#അന്തര്ഭാവ <initcall.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <memtop.h>
#അന്തര്ഭാവ <os.h>
#അന്തര്ഭാവ <post.h>
#അന്തര്ഭാവ <relocate.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <sysreset.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <timer.h>
#അന്തര്ഭാവ <trace.h>
#അന്തര്ഭാവ <upl.h>
#അന്തര്ഭാവ <video.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <asm/cache.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <asm/sections.h>
#അന്തര്ഭാവ <dm/root.h>
#അന്തര്ഭാവ <linux/errno.h>
#അന്തര്ഭാവ <linux/log2.h>
#അന്തര്ഭാവ <linux/sizes.h>

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

#എങ്കിൽ defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
സ്ഥിതം പൂർണ്ണസംഖ്യ init_func_watchdog_init(ശൂന്യം)
{
# എങ്കിൽ defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ init_func_watchdog_reset(ശൂന്യം)
{
	schedule();

	തിരികെ 0;
}
#endif /* CONFIG_WATCHDOG */

__weak ശൂന്യം board_add_ram_info(പൂർണ്ണസംഖ്യ use_default)
{
	/* please define platform specific board_add_ram_info() */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ init_baud_rate(ശൂന്യം)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ display_text_info(ശൂന്യം)
{
#എങ്കിൽ !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#അല്ലെങ്കിൽ
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	തിരികെ 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
സ്ഥിതം പൂർണ്ണസംഖ്യ print_resetinfo(ശൂന്യം)
{
	ഘടന udevice *dev;
	അക്ഷരം status[256];
	bool status_printed = false;
	പൂർണ്ണസംഖ്യ ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	വേണ്ടി (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		എങ്കിൽ (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			തുടരുക;
		}

		എങ്കിൽ (!sysreset_get_status(dev, status, വലുപ്പം(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	എങ്കിൽ (status_printed)
		printf("\n");

	തിരികെ 0;
}
#endif

#എങ്കിൽ defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
സ്ഥിതം പൂർണ്ണസംഖ്യ print_cpuinfo(ശൂന്യം)
{
	ഘടന udevice *dev;
	അക്ഷരം desc[512];
	പൂർണ്ണസംഖ്യ ret;

	dev = cpu_get_current_dev();
	എങ്കിൽ (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		തിരികെ -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, വലുപ്പം(desc));
	എങ്കിൽ (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		തിരികെ ret;
	}

	printf("CPU:   %s\n", desc);

	തിരികെ 0;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ announce_dram_init(ശൂന്യം)
{
	puts("DRAM:  ");
	തിരികെ 0;
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
#define compute_size_scale_val(size, scale, val) ചെയ്യുക { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	എങ്കിൽ (val == 10240) { val = 10; scale += 10; } \
} ആകുമ്പോൾ (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned പൂർണ്ണസംഖ്യ size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	തിരികെ size1_scale == size2_scale && size1_val == size2_val;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ show_dram_config(ശൂന്യം)
{
	unsigned long long size;
	പൂർണ്ണസംഖ്യ i;

	debug("\nRAM Configuration:\n");
	വേണ്ടി (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	എങ്കിൽ (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ dram_init_banksize(ശൂന്യം)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	തിരികെ 0;
}

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
സ്ഥിതം പൂർണ്ണസംഖ്യ init_func_i2c(ശൂന്യം)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	തിരികെ 0;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ setup_mon_len(ശൂന്യം)
{
#എങ്കിൽ defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#അല്ലെങ്കിലെങ്കിൽ defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ setup_spl_handoff(ശൂന്യം)
{
#എങ്കിൽ CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					വലുപ്പം(ഘടന spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ arch_cpu_init(ശൂന്യം)
{
	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ mach_cpu_init(ശൂന്യം)
{
	തിരികെ 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_മുകൾ(phys_size_t total_size)
{
	എങ്കിൽ (CONFIG_IS_ENABLED(RELOC_ADDR_മുകൾ))
		തിരികെ gd->ram_മുകൾ;

#എങ്കിൽ defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	എങ്കിൽ (gd->ram_മുകൾ < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		തിരികെ 0;
#endif
	തിരികെ gd->ram_മുകൾ;
}

__weak പൂർണ്ണസംഖ്യ arch_setup_dest_addr(ശൂന്യം)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ setup_ram_base(ശൂന്യം)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ setup_ram_config(ശൂന്യം)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	എങ്കിൽ (CONFIG_IS_ENABLED(RELOC_ADDR_മുകൾ)) {
		പൂർണ്ണസംഖ്യ i;
		phys_addr_t മുകൾ;

		gd->ram_size = 0;
		വേണ്ടി (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			മുകൾ = get_mem_മുകൾ(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (ശൂന്യം *)gd->fdt_blob);
			gd->ram_മുകൾ = max(മുകൾ, gd->ram_മുകൾ);
			gd->ram_size += gd->dram[i].size;
		}
	} അല്ലെങ്കിൽ {
		gd->ram_മുകൾ = gd->ram_base + get_effective_memsize();
	}
	gd->ram_മുകൾ = board_get_usable_ram_മുകൾ(gd->mon_len);
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
#എങ്കിൽ CONFIG_VAL(SYS_MEM_മുകൾ_HIDE)
	gd->ram_മുകൾ -= CONFIG_SYS_MEM_മുകൾ_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_മുകൾ_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_മുകൾ);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ setup_dest_addr(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;

	gd->relocaddr = gd->ram_മുകൾ;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	എങ്കിൽ (ret)
		തിരികെ ret;

	gd->initial_relocaddr = gd->relocaddr;
	തിരികെ 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_pram(ശൂന്യം)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	തിരികെ 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_round_4k(ശൂന്യം)
{
	gd->relocaddr &= ~(4096 - 1);
	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ arch_reserve_mmu(ശൂന്യം)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_video_from_videoblob(ശൂന്യം)
{
	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ഘടന video_handoff *ho;
		പൂർണ്ണസംഖ്യ ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, വലുപ്പം(*ho));
		എങ്കിൽ (!ho)
			തിരികെ log_സന്ദേശം_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		എങ്കിൽ (ret)
			തിരികെ log_സന്ദേശം_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		എങ്കിൽ (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_relocaddr_from_bloblist(ശൂന്യം)
{
	reserve_video_from_videoblob();

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_video(ശൂന്യം)
{
	എങ്കിൽ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		പൂർണ്ണസംഖ്യ ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		എങ്കിൽ (ret)
			തിരികെ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_trace(ശൂന്യം)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_uboot(ശൂന്യം)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	എങ്കിൽ (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	എങ്കിൽ (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#എങ്കിൽ defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	തിരികെ 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
സ്ഥിതം unsigned long reserve_അട്ടി_aligned(size_t size)
{
	തിരികെ ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_noncached(ശൂന്യം)
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

	തിരികെ 0;
}
#endif

/* reserve memory for malloc() area */
സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_malloc(ശൂന്യം)
{
	gd->start_addr_sp = reserve_അട്ടി_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	തിരികെ 0;
}

/* (permanently) allocate a Board Info struct */
സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_board(ശൂന്യം)
{
	എങ്കിൽ (!gd->bd) {
		gd->start_addr_sp = reserve_അട്ടി_aligned(വലുപ്പം(ഘടന bd_info));
		gd->bd = (ഘടന bd_info *)map_sysmem(gd->start_addr_sp,
						      വലുപ്പം(ഘടന bd_info));
		memset(gd->bd, '\0', വലുപ്പം(ഘടന bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      വലുപ്പം(ഘടന bd_info), gd->start_addr_sp);
	}
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_global_data(ശൂന്യം)
{
	gd->start_addr_sp = reserve_അട്ടി_aligned(വലുപ്പം(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, വലുപ്പം(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      വലുപ്പം(gd_t), gd->start_addr_sp);
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_fdt(ശൂന്യം)
{
	എങ്കിൽ (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		എങ്കിൽ (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_അട്ടി_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_bootstage(ശൂന്യം)
{
#ifdef CONFIG_BOOTSTAGE
	പൂർണ്ണസംഖ്യ size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_അട്ടി_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ arch_reserve_stacks(ശൂന്യം)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_stacks(ശൂന്യം)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_അട്ടി_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	തിരികെ arch_reserve_stacks();
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reserve_bloblist(ശൂന്യം)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	എങ്കിൽ (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ display_new_sp(ശൂന്യം)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ arch_setup_bdinfo(ശൂന്യം)
{
	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ setup_bdinfo(ശൂന്യം)
{
	തിരികെ arch_setup_bdinfo();
}

#ifdef CONFIG_POST
സ്ഥിതം പൂർണ്ണസംഖ്യ init_post(ശൂന്യം)
{
	post_bootmode_init();
	post_run(ശൂന്യ, POST_ROM | post_bootmode_get(0));

	തിരികെ 0;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ reloc_fdt(ശൂന്യം)
{
	എങ്കിൽ (!IS_ENABLED(CONFIG_OF_EMBED)) {
		എങ്കിൽ (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reloc_bootstage(ശൂന്യം)
{
#ifdef CONFIG_BOOTSTAGE
	എങ്കിൽ (gd->flags & GD_FLG_SKIP_RELOC)
		തിരികെ 0;
	എങ്കിൽ (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ reloc_bloblist(ശൂന്യം)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	എങ്കിൽ ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		തിരികെ 0;
	}
	എങ്കിൽ (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		എങ്കിൽ (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		തിരികെ bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	തിരികെ 0;
}

ശൂന്യം mcheck_on_ramrelocation(size_t offset);
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_reloc(ശൂന്യം)
{
	എങ്കിൽ (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#അല്ലെങ്കിലെങ്കിൽ defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#അല്ലെങ്കിലെങ്കിൽ !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (അക്ഷരം *)gd, വലുപ്പം(gd_t));

	എങ്കിൽ (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} അല്ലെങ്കിൽ {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	തിരികെ 0;
}

#എങ്കിൽ CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
സ്ഥിതം പൂർണ്ണസംഖ്യ fix_fdt(ശൂന്യം)
{
	തിരികെ board_fix_fdt((ശൂന്യം *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#എങ്കിൽ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

സ്ഥിതം പൂർണ്ണസംഖ്യ jump_to_copy(ശൂന്യം)
{
	എങ്കിൽ (gd->flags & GD_FLG_SKIP_RELOC)
		തിരികെ 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#എങ്കിൽ defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# എങ്കിൽ CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# അല്ലെങ്കിൽ
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#അല്ലെങ്കിൽ
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	തിരികെ 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
സ്ഥിതം പൂർണ്ണസംഖ്യ initf_bootstage(ശൂന്യം)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	പൂർണ്ണസംഖ്യ ret;

	ret = bootstage_init(!from_spl);
	എങ്കിൽ (ret)
		തിരികെ ret;
	എങ്കിൽ (from_spl) {
		ret = bootstage_unstash_default();
		എങ്കിൽ (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			തിരികെ ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ initf_dm(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		തിരികെ 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	എങ്കിൽ (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	അല്ലെങ്കിൽ
		ret = dm_init(false);
	എങ്കിൽ (ret)
		തിരികെ ret;

	ret = dm_autoprobe();
	എങ്കിൽ (ret)
		തിരികെ ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	എങ്കിൽ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		എങ്കിൽ (ret)
			തിരികെ ret;
	}

	തിരികെ 0;
}

/* Architecture-specific memory reservation */
__weak പൂർണ്ണസംഖ്യ reserve_arch(ശൂന്യം)
{
	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ checkcpu(ശൂന്യം)
{
	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ clear_bss(ശൂന്യം)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ initf_upl(ശൂന്യം)
{
	ഘടന upl *upl;
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		തിരികെ 0;

	upl = malloc(വലുപ്പം(ഘടന upl));
	എങ്കിൽ (!upl)
		തിരികെ -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	എങ്കിൽ (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		തിരികെ ret;
	}
	gd_set_upl(upl);

	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം initcall_run_f(ശൂന്യം)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#എങ്കിൽ CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#എങ്കിൽ CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#എങ്കിൽ CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#എങ്കിൽ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#എങ്കിൽ defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#എങ്കിൽ !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#എങ്കിൽ CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#എങ്കിൽ CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#എങ്കിൽ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#എങ്കിൽ CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#എങ്കിൽ defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#എങ്കിൽ CONFIG_IS_ENABLED(POST)
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
#എങ്കിൽ CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#എങ്കിൽ CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#എങ്കിൽ !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#എങ്കിൽ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#എങ്കിൽ !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

ശൂന്യം board_init_f(ulong boot_flags)
{
	ഘടന board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#എങ്കിൽ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#എങ്കിൽ defined(CONFIG_X86) || defined(CONFIG_ARC)
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
സ്ഥിതം ശൂന്യം initcall_run_f_r(ശൂന്യം)
{
#എങ്കിൽ !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

ശൂന്യം board_init_f_r(ശൂന്യം)
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
	gd->timer = ശൂന്യ;
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
