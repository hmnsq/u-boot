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

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <bloblist.h>
#અન્તર્ભાવ <bootstage.h>
#અન્તર્ભાવ <clock_legacy.h>
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <cpu.h>
#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <cyclic.h>
#અન્તર્ભાવ <display_options.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <env_internal.h>
#અન્તર્ભાવ <event.h>
#અન્તર્ભાવ <fdtdec.h>
#અન્તર્ભાવ <fs.h>
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <i2c.h>
#અન્તર્ભાવ <init.h>
#અન્તર્ભાવ <initcall.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <memtop.h>
#અન્તર્ભાવ <os.h>
#અન્તર્ભાવ <post.h>
#અન્તર્ભાવ <relocate.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <sysreset.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <timer.h>
#અન્તર્ભાવ <trace.h>
#અન્તર્ભાવ <upl.h>
#અન્તર્ભાવ <video.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <asm/cache.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <asm/sections.h>
#અન્તર્ભાવ <dm/root.h>
#અન્તર્ભાવ <linux/errno.h>
#અન્તર્ભાવ <linux/log2.h>
#અન્તર્ભાવ <linux/sizes.h>

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

#જો defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
સ્થિર પૂર્ણાંક init_func_watchdog_init(શૂન્ય)
{
# જો defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	પાછા 0;
}

પૂર્ણાંક init_func_watchdog_reset(શૂન્ય)
{
	schedule();

	પાછા 0;
}
#endif /* CONFIG_WATCHDOG */

__weak શૂન્ય board_add_ram_info(પૂર્ણાંક use_default)
{
	/* please define platform specific board_add_ram_info() */
}

સ્થિર પૂર્ણાંક init_baud_rate(શૂન્ય)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	પાછા 0;
}

સ્થિર પૂર્ણાંક display_text_info(શૂન્ય)
{
#જો !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#નહીંતર
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	પાછા 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
સ્થિર પૂર્ણાંક print_resetinfo(શૂન્ય)
{
	સંરચના udevice *dev;
	અક્ષર status[256];
	bool status_printed = false;
	પૂર્ણાંક ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	માટે (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		જો (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			ચાલુ;
		}

		જો (!sysreset_get_status(dev, status, કદ(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	જો (status_printed)
		printf("\n");

	પાછા 0;
}
#endif

#જો defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
સ્થિર પૂર્ણાંક print_cpuinfo(શૂન્ય)
{
	સંરચના udevice *dev;
	અક્ષર desc[512];
	પૂર્ણાંક ret;

	dev = cpu_get_current_dev();
	જો (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		પાછા -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, કદ(desc));
	જો (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		પાછા ret;
	}

	printf("CPU:   %s\n", desc);

	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક announce_dram_init(શૂન્ય)
{
	puts("DRAM:  ");
	પાછા 0;
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
#define compute_size_scale_val(size, scale, val) કરો { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	જો (val == 10240) { val = 10; scale += 10; } \
} જ્યારે (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
સ્થિર પૂર્ણાંક sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned પૂર્ણાંક size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	પાછા size1_scale == size2_scale && size1_val == size2_val;
}

સ્થિર પૂર્ણાંક show_dram_config(શૂન્ય)
{
	unsigned long long size;
	પૂર્ણાંક i;

	debug("\nRAM Configuration:\n");
	માટે (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	જો (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	પાછા 0;
}

__weak પૂર્ણાંક dram_init_banksize(શૂન્ય)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	પાછા 0;
}

#જો CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
સ્થિર પૂર્ણાંક init_func_i2c(શૂન્ય)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક setup_mon_len(શૂન્ય)
{
#જો defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#અથવાજો defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#અથવાજો defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#અથવાજો defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#અથવાજો defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#અથવાજો defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#અથવાજો defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#અથવાજો defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	પાછા 0;
}

સ્થિર પૂર્ણાંક setup_spl_handoff(શૂન્ય)
{
#જો CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					કદ(સંરચના spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	પાછા 0;
}

__weak પૂર્ણાંક arch_cpu_init(શૂન્ય)
{
	પાછા 0;
}

__weak પૂર્ણાંક mach_cpu_init(શૂન્ય)
{
	પાછા 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_ટોચ(phys_size_t total_size)
{
	જો (CONFIG_IS_ENABLED(RELOC_ADDR_ટોચ))
		પાછા gd->ram_ટોચ;

#જો defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	જો (gd->ram_ટોચ < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		પાછા 0;
#endif
	પાછા gd->ram_ટોચ;
}

__weak પૂર્ણાંક arch_setup_dest_addr(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક setup_ram_base(શૂન્ય)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	પાછા 0;
}

સ્થિર પૂર્ણાંક setup_ram_config(શૂન્ય)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	જો (CONFIG_IS_ENABLED(RELOC_ADDR_ટોચ)) {
		પૂર્ણાંક i;
		phys_addr_t ટોચ;

		gd->ram_size = 0;
		માટે (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			ટોચ = get_mem_ટોચ(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (શૂન્ય *)gd->fdt_blob);
			gd->ram_ટોચ = max(ટોચ, gd->ram_ટોચ);
			gd->ram_size += gd->dram[i].size;
		}
	} નહીંતર {
		gd->ram_ટોચ = gd->ram_base + get_effective_memsize();
	}
	gd->ram_ટોચ = board_get_usable_ram_ટોચ(gd->mon_len);
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
#જો CONFIG_VAL(SYS_MEM_ટોચ_HIDE)
	gd->ram_ટોચ -= CONFIG_SYS_MEM_ટોચ_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_ટોચ_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_ટોચ);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	પાછા 0;
}

સ્થિર પૂર્ણાંક setup_dest_addr(શૂન્ય)
{
	પૂર્ણાંક ret;

	gd->relocaddr = gd->ram_ટોચ;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	જો (ret)
		પાછા ret;

	gd->initial_relocaddr = gd->relocaddr;
	પાછા 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
સ્થિર પૂર્ણાંક reserve_pram(શૂન્ય)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	પાછા 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
સ્થિર પૂર્ણાંક reserve_round_4k(શૂન્ય)
{
	gd->relocaddr &= ~(4096 - 1);
	પાછા 0;
}

__weak પૂર્ણાંક arch_reserve_mmu(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_video_from_videoblob(શૂન્ય)
{
	જો (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		સંરચના video_handoff *ho;
		પૂર્ણાંક ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, કદ(*ho));
		જો (!ho)
			પાછા log_સંદેશ_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		જો (ret)
			પાછા log_સંદેશ_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		જો (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	પાછા 0;
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
સ્થિર પૂર્ણાંક setup_relocaddr_from_bloblist(શૂન્ય)
{
	reserve_video_from_videoblob();

	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_video(શૂન્ય)
{
	જો (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		પૂર્ણાંક ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		જો (ret)
			પાછા ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_trace(શૂન્ય)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_uboot(શૂન્ય)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	જો (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	જો (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#જો defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	પાછા 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
સ્થિર unsigned long reserve_ઢગલો_aligned(size_t size)
{
	પાછા ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
સ્થિર પૂર્ણાંક reserve_noncached(શૂન્ય)
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

	પાછા 0;
}
#endif

/* reserve memory for malloc() area */
સ્થિર પૂર્ણાંક reserve_malloc(શૂન્ય)
{
	gd->start_addr_sp = reserve_ઢગલો_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	પાછા 0;
}

/* (permanently) allocate a Board Info struct */
સ્થિર પૂર્ણાંક reserve_board(શૂન્ય)
{
	જો (!gd->bd) {
		gd->start_addr_sp = reserve_ઢગલો_aligned(કદ(સંરચના bd_info));
		gd->bd = (સંરચના bd_info *)map_sysmem(gd->start_addr_sp,
						      કદ(સંરચના bd_info));
		memset(gd->bd, '\0', કદ(સંરચના bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      કદ(સંરચના bd_info), gd->start_addr_sp);
	}
	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_global_data(શૂન્ય)
{
	gd->start_addr_sp = reserve_ઢગલો_aligned(કદ(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, કદ(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      કદ(gd_t), gd->start_addr_sp);
	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_fdt(શૂન્ય)
{
	જો (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		જો (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_ઢગલો_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_bootstage(શૂન્ય)
{
#ifdef CONFIG_BOOTSTAGE
	પૂર્ણાંક size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_ઢગલો_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	પાછા 0;
}

__weak પૂર્ણાંક arch_reserve_stacks(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક reserve_stacks(શૂન્ય)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_ઢગલો_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	પાછા arch_reserve_stacks();
}

સ્થિર પૂર્ણાંક reserve_bloblist(શૂન્ય)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	જો (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	પાછા 0;
}

સ્થિર પૂર્ણાંક display_new_sp(શૂન્ય)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	પાછા 0;
}

__weak પૂર્ણાંક arch_setup_bdinfo(શૂન્ય)
{
	પાછા 0;
}

પૂર્ણાંક setup_bdinfo(શૂન્ય)
{
	પાછા arch_setup_bdinfo();
}

#ifdef CONFIG_POST
સ્થિર પૂર્ણાંક init_post(શૂન્ય)
{
	post_bootmode_init();
	post_run(ખાલી, POST_ROM | post_bootmode_get(0));

	પાછા 0;
}
#endif

સ્થિર પૂર્ણાંક reloc_fdt(શૂન્ય)
{
	જો (!IS_ENABLED(CONFIG_OF_EMBED)) {
		જો (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	પાછા 0;
}

સ્થિર પૂર્ણાંક reloc_bootstage(શૂન્ય)
{
#ifdef CONFIG_BOOTSTAGE
	જો (gd->flags & GD_FLG_SKIP_RELOC)
		પાછા 0;
	જો (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	પાછા 0;
}

સ્થિર પૂર્ણાંક reloc_bloblist(શૂન્ય)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	જો ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		પાછા 0;
	}
	જો (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		જો (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		પાછા bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	પાછા 0;
}

શૂન્ય mcheck_on_ramrelocation(size_t offset);
સ્થિર પૂર્ણાંક setup_reloc(શૂન્ય)
{
	જો (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#અથવાજો defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#અથવાજો defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#અથવાજો !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (અક્ષર *)gd, કદ(gd_t));

	જો (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} નહીંતર {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	પાછા 0;
}

#જો CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
સ્થિર પૂર્ણાંક fix_fdt(શૂન્ય)
{
	પાછા board_fix_fdt((શૂન્ય *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#જો !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

સ્થિર પૂર્ણાંક jump_to_copy(શૂન્ય)
{
	જો (gd->flags & GD_FLG_SKIP_RELOC)
		પાછા 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#જો defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# જો CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# નહીંતર
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#નહીંતર
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	પાછા 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
સ્થિર પૂર્ણાંક initf_bootstage(શૂન્ય)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	પૂર્ણાંક ret;

	ret = bootstage_init(!from_spl);
	જો (ret)
		પાછા ret;
	જો (from_spl) {
		ret = bootstage_unstash_default();
		જો (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			પાછા ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	પાછા 0;
}

સ્થિર પૂર્ણાંક initf_dm(શૂન્ય)
{
	પૂર્ણાંક ret;

	જો (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		પાછા 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	જો (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	નહીંતર
		ret = dm_init(false);
	જો (ret)
		પાછા ret;

	ret = dm_autoprobe();
	જો (ret)
		પાછા ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	જો (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		જો (ret)
			પાછા ret;
	}

	પાછા 0;
}

/* Architecture-specific memory reservation */
__weak પૂર્ણાંક reserve_arch(શૂન્ય)
{
	પાછા 0;
}

__weak પૂર્ણાંક checkcpu(શૂન્ય)
{
	પાછા 0;
}

__weak પૂર્ણાંક clear_bss(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક initf_upl(શૂન્ય)
{
	સંરચના upl *upl;
	પૂર્ણાંક ret;

	જો (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		પાછા 0;

	upl = malloc(કદ(સંરચના upl));
	જો (!upl)
		પાછા -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	જો (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		પાછા ret;
	}
	gd_set_upl(upl);

	પાછા 0;
}

સ્થિર શૂન્ય initcall_run_f(શૂન્ય)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#જો CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#જો CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#જો CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#જો CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#જો CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#જો defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#જો !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#જો CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#જો CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#જો CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#જો CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#જો CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#જો CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#જો CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#જો defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#જો CONFIG_IS_ENABLED(POST)
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
#જો CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#જો CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#જો !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#જો CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#જો !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

શૂન્ય board_init_f(ulong boot_flags)
{
	સંરચના board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#જો !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#જો defined(CONFIG_X86) || defined(CONFIG_ARC)
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
સ્થિર શૂન્ય initcall_run_f_r(શૂન્ય)
{
#જો !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

શૂન્ય board_init_f_r(શૂન્ય)
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
	gd->timer = ખાલી;
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
