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

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <bloblist.h>
#அந்தர்பாவ <bootstage.h>
#அந்தர்பாவ <clock_legacy.h>
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <cpu.h>
#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <cyclic.h>
#அந்தர்பாவ <display_options.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <env_internal.h>
#அந்தர்பாவ <event.h>
#அந்தர்பாவ <fdtdec.h>
#அந்தர்பாவ <fs.h>
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <i2c.h>
#அந்தர்பாவ <init.h>
#அந்தர்பாவ <initcall.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <memtop.h>
#அந்தர்பாவ <os.h>
#அந்தர்பாவ <post.h>
#அந்தர்பாவ <relocate.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <sysreset.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <timer.h>
#அந்தர்பாவ <trace.h>
#அந்தர்பாவ <upl.h>
#அந்தர்பாவ <video.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <asm/cache.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <asm/sections.h>
#அந்தர்பாவ <dm/root.h>
#அந்தர்பாவ <linux/errno.h>
#அந்தர்பாவ <linux/log2.h>
#அந்தர்பாவ <linux/sizes.h>

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

#எனில் defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
நிலையான முழு init_func_watchdog_init(வெறுமை)
{
# எனில் defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	திரும்பு 0;
}

முழு init_func_watchdog_reset(வெறுமை)
{
	schedule();

	திரும்பு 0;
}
#endif /* CONFIG_WATCHDOG */

__weak வெறுமை board_add_ram_info(முழு use_default)
{
	/* please define platform specific board_add_ram_info() */
}

நிலையான முழு init_baud_rate(வெறுமை)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	திரும்பு 0;
}

நிலையான முழு display_text_info(வெறுமை)
{
#எனில் !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#இல்லையெனில்
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	திரும்பு 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
நிலையான முழு print_resetinfo(வெறுமை)
{
	கட்டமைப்பு udevice *dev;
	எழுத்து status[256];
	bool status_printed = false;
	முழு ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	க்கு (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		எனில் (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			தொடர்;
		}

		எனில் (!sysreset_get_status(dev, status, அளவு(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	எனில் (status_printed)
		printf("\n");

	திரும்பு 0;
}
#endif

#எனில் defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
நிலையான முழு print_cpuinfo(வெறுமை)
{
	கட்டமைப்பு udevice *dev;
	எழுத்து desc[512];
	முழு ret;

	dev = cpu_get_current_dev();
	எனில் (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		திரும்பு -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, அளவு(desc));
	எனில் (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		திரும்பு ret;
	}

	printf("CPU:   %s\n", desc);

	திரும்பு 0;
}
#endif

நிலையான முழு announce_dram_init(வெறுமை)
{
	puts("DRAM:  ");
	திரும்பு 0;
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
#define compute_size_scale_val(size, scale, val) செய் { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	எனில் (val == 10240) { val = 10; scale += 10; } \
} போது (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
நிலையான முழு sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned முழு size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	திரும்பு size1_scale == size2_scale && size1_val == size2_val;
}

நிலையான முழு show_dram_config(வெறுமை)
{
	unsigned long long size;
	முழு i;

	debug("\nRAM Configuration:\n");
	க்கு (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	எனில் (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	திரும்பு 0;
}

__weak முழு dram_init_banksize(வெறுமை)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	திரும்பு 0;
}

#எனில் CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
நிலையான முழு init_func_i2c(வெறுமை)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	திரும்பு 0;
}
#endif

நிலையான முழு setup_mon_len(வெறுமை)
{
#எனில் defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#அல்லதெனில் defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#அல்லதெனில் defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#அல்லதெனில் defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#அல்லதெனில் defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#அல்லதெனில் defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#அல்லதெனில் defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#அல்லதெனில் defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	திரும்பு 0;
}

நிலையான முழு setup_spl_handoff(வெறுமை)
{
#எனில் CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					அளவு(கட்டமைப்பு spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	திரும்பு 0;
}

__weak முழு arch_cpu_init(வெறுமை)
{
	திரும்பு 0;
}

__weak முழு mach_cpu_init(வெறுமை)
{
	திரும்பு 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_உச்சி(phys_size_t total_size)
{
	எனில் (CONFIG_IS_ENABLED(RELOC_ADDR_உச்சி))
		திரும்பு gd->ram_உச்சி;

#எனில் defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	எனில் (gd->ram_உச்சி < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		திரும்பு 0;
#endif
	திரும்பு gd->ram_உச்சி;
}

__weak முழு arch_setup_dest_addr(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு setup_ram_base(வெறுமை)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	திரும்பு 0;
}

நிலையான முழு setup_ram_config(வெறுமை)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	எனில் (CONFIG_IS_ENABLED(RELOC_ADDR_உச்சி)) {
		முழு i;
		phys_addr_t உச்சி;

		gd->ram_size = 0;
		க்கு (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			உச்சி = get_mem_உச்சி(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (வெறுமை *)gd->fdt_blob);
			gd->ram_உச்சி = max(உச்சி, gd->ram_உச்சி);
			gd->ram_size += gd->dram[i].size;
		}
	} இல்லையெனில் {
		gd->ram_உச்சி = gd->ram_base + get_effective_memsize();
	}
	gd->ram_உச்சி = board_get_usable_ram_உச்சி(gd->mon_len);
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
#எனில் CONFIG_VAL(SYS_MEM_உச்சி_HIDE)
	gd->ram_உச்சி -= CONFIG_SYS_MEM_உச்சி_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_உச்சி_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_உச்சி);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	திரும்பு 0;
}

நிலையான முழு setup_dest_addr(வெறுமை)
{
	முழு ret;

	gd->relocaddr = gd->ram_உச்சி;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	எனில் (ret)
		திரும்பு ret;

	gd->initial_relocaddr = gd->relocaddr;
	திரும்பு 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
நிலையான முழு reserve_pram(வெறுமை)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	திரும்பு 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
நிலையான முழு reserve_round_4k(வெறுமை)
{
	gd->relocaddr &= ~(4096 - 1);
	திரும்பு 0;
}

__weak முழு arch_reserve_mmu(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு reserve_video_from_videoblob(வெறுமை)
{
	எனில் (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		கட்டமைப்பு video_handoff *ho;
		முழு ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, அளவு(*ho));
		எனில் (!ho)
			திரும்பு log_செய்தி_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		எனில் (ret)
			திரும்பு log_செய்தி_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		எனில் (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	திரும்பு 0;
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
நிலையான முழு setup_relocaddr_from_bloblist(வெறுமை)
{
	reserve_video_from_videoblob();

	திரும்பு 0;
}

நிலையான முழு reserve_video(வெறுமை)
{
	எனில் (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		முழு ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		எனில் (ret)
			திரும்பு ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	திரும்பு 0;
}

நிலையான முழு reserve_trace(வெறுமை)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	திரும்பு 0;
}

நிலையான முழு reserve_uboot(வெறுமை)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	எனில் (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	எனில் (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#எனில் defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	திரும்பு 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
நிலையான unsigned long reserve_அடுக்கு_aligned(size_t size)
{
	திரும்பு ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
நிலையான முழு reserve_noncached(வெறுமை)
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

	திரும்பு 0;
}
#endif

/* reserve memory for malloc() area */
நிலையான முழு reserve_malloc(வெறுமை)
{
	gd->start_addr_sp = reserve_அடுக்கு_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	திரும்பு 0;
}

/* (permanently) allocate a Board Info struct */
நிலையான முழு reserve_board(வெறுமை)
{
	எனில் (!gd->bd) {
		gd->start_addr_sp = reserve_அடுக்கு_aligned(அளவு(கட்டமைப்பு bd_info));
		gd->bd = (கட்டமைப்பு bd_info *)map_sysmem(gd->start_addr_sp,
						      அளவு(கட்டமைப்பு bd_info));
		memset(gd->bd, '\0', அளவு(கட்டமைப்பு bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      அளவு(கட்டமைப்பு bd_info), gd->start_addr_sp);
	}
	திரும்பு 0;
}

நிலையான முழு reserve_global_data(வெறுமை)
{
	gd->start_addr_sp = reserve_அடுக்கு_aligned(அளவு(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, அளவு(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      அளவு(gd_t), gd->start_addr_sp);
	திரும்பு 0;
}

நிலையான முழு reserve_fdt(வெறுமை)
{
	எனில் (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		எனில் (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_அடுக்கு_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	திரும்பு 0;
}

நிலையான முழு reserve_bootstage(வெறுமை)
{
#ifdef CONFIG_BOOTSTAGE
	முழு size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_அடுக்கு_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	திரும்பு 0;
}

__weak முழு arch_reserve_stacks(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு reserve_stacks(வெறுமை)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_அடுக்கு_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	திரும்பு arch_reserve_stacks();
}

நிலையான முழு reserve_bloblist(வெறுமை)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	எனில் (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	திரும்பு 0;
}

நிலையான முழு display_new_sp(வெறுமை)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	திரும்பு 0;
}

__weak முழு arch_setup_bdinfo(வெறுமை)
{
	திரும்பு 0;
}

முழு setup_bdinfo(வெறுமை)
{
	திரும்பு arch_setup_bdinfo();
}

#ifdef CONFIG_POST
நிலையான முழு init_post(வெறுமை)
{
	post_bootmode_init();
	post_run(வெற்று, POST_ROM | post_bootmode_get(0));

	திரும்பு 0;
}
#endif

நிலையான முழு reloc_fdt(வெறுமை)
{
	எனில் (!IS_ENABLED(CONFIG_OF_EMBED)) {
		எனில் (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	திரும்பு 0;
}

நிலையான முழு reloc_bootstage(வெறுமை)
{
#ifdef CONFIG_BOOTSTAGE
	எனில் (gd->flags & GD_FLG_SKIP_RELOC)
		திரும்பு 0;
	எனில் (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	திரும்பு 0;
}

நிலையான முழு reloc_bloblist(வெறுமை)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	எனில் ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		திரும்பு 0;
	}
	எனில் (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		எனில் (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		திரும்பு bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	திரும்பு 0;
}

வெறுமை mcheck_on_ramrelocation(size_t offset);
நிலையான முழு setup_reloc(வெறுமை)
{
	எனில் (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#அல்லதெனில் defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#அல்லதெனில் defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#அல்லதெனில் !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (எழுத்து *)gd, அளவு(gd_t));

	எனில் (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} இல்லையெனில் {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	திரும்பு 0;
}

#எனில் CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
நிலையான முழு fix_fdt(வெறுமை)
{
	திரும்பு board_fix_fdt((வெறுமை *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#எனில் !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

நிலையான முழு jump_to_copy(வெறுமை)
{
	எனில் (gd->flags & GD_FLG_SKIP_RELOC)
		திரும்பு 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#எனில் defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# எனில் CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# இல்லையெனில்
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#இல்லையெனில்
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	திரும்பு 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
நிலையான முழு initf_bootstage(வெறுமை)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	முழு ret;

	ret = bootstage_init(!from_spl);
	எனில் (ret)
		திரும்பு ret;
	எனில் (from_spl) {
		ret = bootstage_unstash_default();
		எனில் (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			திரும்பு ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	திரும்பு 0;
}

நிலையான முழு initf_dm(வெறுமை)
{
	முழு ret;

	எனில் (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		திரும்பு 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	எனில் (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	இல்லையெனில்
		ret = dm_init(false);
	எனில் (ret)
		திரும்பு ret;

	ret = dm_autoprobe();
	எனில் (ret)
		திரும்பு ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	எனில் (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		எனில் (ret)
			திரும்பு ret;
	}

	திரும்பு 0;
}

/* Architecture-specific memory reservation */
__weak முழு reserve_arch(வெறுமை)
{
	திரும்பு 0;
}

__weak முழு checkcpu(வெறுமை)
{
	திரும்பு 0;
}

__weak முழு clear_bss(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு initf_upl(வெறுமை)
{
	கட்டமைப்பு upl *upl;
	முழு ret;

	எனில் (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		திரும்பு 0;

	upl = malloc(அளவு(கட்டமைப்பு upl));
	எனில் (!upl)
		திரும்பு -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	எனில் (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		திரும்பு ret;
	}
	gd_set_upl(upl);

	திரும்பு 0;
}

நிலையான வெறுமை initcall_run_f(வெறுமை)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#எனில் CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#எனில் CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#எனில் CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#எனில் CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#எனில் CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#எனில் defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#எனில் !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#எனில் CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#எனில் CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#எனில் CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#எனில் CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#எனில் CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#எனில் CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#எனில் CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#எனில் defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#எனில் CONFIG_IS_ENABLED(POST)
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
#எனில் CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#எனில் CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#எனில் !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#எனில் CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#எனில் !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

வெறுமை board_init_f(ulong boot_flags)
{
	கட்டமைப்பு board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#எனில் !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#எனில் defined(CONFIG_X86) || defined(CONFIG_ARC)
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
நிலையான வெறுமை initcall_run_f_r(வெறுமை)
{
#எனில் !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

வெறுமை board_init_f_r(வெறுமை)
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
	gd->timer = வெற்று;
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
