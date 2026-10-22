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

#شامل <config.h>
#شامل <bloblist.h>
#شامل <bootstage.h>
#شامل <clock_legacy.h>
#شامل <console.h>
#شامل <cpu.h>
#شامل <cpu_func.h>
#شامل <cyclic.h>
#شامل <display_options.h>
#شامل <dm.h>
#شامل <env.h>
#شامل <env_internal.h>
#شامل <event.h>
#شامل <fdtdec.h>
#شامل <fs.h>
#شامل <hang.h>
#شامل <i2c.h>
#شامل <init.h>
#شامل <initcall.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <memtop.h>
#شامل <os.h>
#شامل <post.h>
#شامل <relocate.h>
#شامل <serial.h>
#شامل <spl.h>
#شامل <sysreset.h>
#شامل <time.h>
#شامل <timer.h>
#شامل <trace.h>
#شامل <upl.h>
#شامل <video.h>
#شامل <watchdog.h>
#شامل <asm/cache.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <asm/sections.h>
#شامل <dm/root.h>
#شامل <linux/errno.h>
#شامل <linux/log2.h>
#شامل <linux/sizes.h>

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

#اگر defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
ایستا صحیح init_func_watchdog_init(تهی)
{
# اگر defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	بازگشت 0;
}

صحیح init_func_watchdog_reset(تهی)
{
	schedule();

	بازگشت 0;
}
#endif /* CONFIG_WATCHDOG */

__weak تهی board_add_ram_info(صحیح use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ایستا صحیح init_baud_rate(تهی)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	بازگشت 0;
}

ایستا صحیح display_text_info(تهی)
{
#اگر !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#وگرنه
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	بازگشت 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
ایستا صحیح print_resetinfo(تهی)
{
	ساختار udevice *dev;
	نویسه status[256];
	bool status_printed = false;
	صحیح ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	برای (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		اگر (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			ادامه;
		}

		اگر (!sysreset_get_status(dev, status, اندازه(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	اگر (status_printed)
		printf("\n");

	بازگشت 0;
}
#endif

#اگر defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ایستا صحیح print_cpuinfo(تهی)
{
	ساختار udevice *dev;
	نویسه desc[512];
	صحیح ret;

	dev = cpu_get_current_dev();
	اگر (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		بازگشت -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, اندازه(desc));
	اگر (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		بازگشت ret;
	}

	printf("CPU:   %s\n", desc);

	بازگشت 0;
}
#endif

ایستا صحیح announce_dram_init(تهی)
{
	puts("DRAM:  ");
	بازگشت 0;
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
#define compute_size_scale_val(size, scale, val) انجامبده { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	اگر (val == 10240) { val = 10; scale += 10; } \
} وقتی (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ایستا صحیح sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned صحیح size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	بازگشت size1_scale == size2_scale && size1_val == size2_val;
}

ایستا صحیح show_dram_config(تهی)
{
	unsigned long long size;
	صحیح i;

	debug("\nRAM Configuration:\n");
	برای (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	اگر (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	بازگشت 0;
}

__weak صحیح dram_init_banksize(تهی)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	بازگشت 0;
}

#اگر CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ایستا صحیح init_func_i2c(تهی)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	بازگشت 0;
}
#endif

ایستا صحیح setup_mon_len(تهی)
{
#اگر defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#وگرنهاگر defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#وگرنهاگر defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#وگرنهاگر defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#وگرنهاگر defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#وگرنهاگر defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#وگرنهاگر defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#وگرنهاگر defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	بازگشت 0;
}

ایستا صحیح setup_spl_handoff(تهی)
{
#اگر CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					اندازه(ساختار spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	بازگشت 0;
}

__weak صحیح arch_cpu_init(تهی)
{
	بازگشت 0;
}

__weak صحیح mach_cpu_init(تهی)
{
	بازگشت 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_بالاترین(phys_size_t total_size)
{
	اگر (CONFIG_IS_ENABLED(RELOC_ADDR_بالاترین))
		بازگشت gd->ram_بالاترین;

#اگر defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	اگر (gd->ram_بالاترین < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		بازگشت 0;
#endif
	بازگشت gd->ram_بالاترین;
}

__weak صحیح arch_setup_dest_addr(تهی)
{
	بازگشت 0;
}

ایستا صحیح setup_ram_base(تهی)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	بازگشت 0;
}

ایستا صحیح setup_ram_config(تهی)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	اگر (CONFIG_IS_ENABLED(RELOC_ADDR_بالاترین)) {
		صحیح i;
		phys_addr_t بالاترین;

		gd->ram_size = 0;
		برای (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			بالاترین = get_mem_بالاترین(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (تهی *)gd->fdt_blob);
			gd->ram_بالاترین = max(بالاترین, gd->ram_بالاترین);
			gd->ram_size += gd->dram[i].size;
		}
	} وگرنه {
		gd->ram_بالاترین = gd->ram_base + get_effective_memsize();
	}
	gd->ram_بالاترین = board_get_usable_ram_بالاترین(gd->mon_len);
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
#اگر CONFIG_VAL(SYS_MEM_بالاترین_HIDE)
	gd->ram_بالاترین -= CONFIG_SYS_MEM_بالاترین_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_بالاترین_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_بالاترین);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	بازگشت 0;
}

ایستا صحیح setup_dest_addr(تهی)
{
	صحیح ret;

	gd->relocaddr = gd->ram_بالاترین;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	اگر (ret)
		بازگشت ret;

	gd->initial_relocaddr = gd->relocaddr;
	بازگشت 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
ایستا صحیح reserve_pram(تهی)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	بازگشت 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ایستا صحیح reserve_round_4k(تهی)
{
	gd->relocaddr &= ~(4096 - 1);
	بازگشت 0;
}

__weak صحیح arch_reserve_mmu(تهی)
{
	بازگشت 0;
}

ایستا صحیح reserve_video_from_videoblob(تهی)
{
	اگر (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ساختار video_handoff *ho;
		صحیح ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, اندازه(*ho));
		اگر (!ho)
			بازگشت log_پیام_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		اگر (ret)
			بازگشت log_پیام_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		اگر (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	بازگشت 0;
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
ایستا صحیح setup_relocaddr_from_bloblist(تهی)
{
	reserve_video_from_videoblob();

	بازگشت 0;
}

ایستا صحیح reserve_video(تهی)
{
	اگر (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحیح ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		اگر (ret)
			بازگشت ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	بازگشت 0;
}

ایستا صحیح reserve_trace(تهی)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	بازگشت 0;
}

ایستا صحیح reserve_uboot(تهی)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	اگر (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	اگر (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#اگر defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	بازگشت 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ایستا unsigned long reserve_پشته_aligned(size_t size)
{
	بازگشت ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
ایستا صحیح reserve_noncached(تهی)
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

	بازگشت 0;
}
#endif

/* reserve memory for malloc() area */
ایستا صحیح reserve_malloc(تهی)
{
	gd->start_addr_sp = reserve_پشته_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	بازگشت 0;
}

/* (permanently) allocate a Board Info struct */
ایستا صحیح reserve_board(تهی)
{
	اگر (!gd->bd) {
		gd->start_addr_sp = reserve_پشته_aligned(اندازه(ساختار bd_info));
		gd->bd = (ساختار bd_info *)map_sysmem(gd->start_addr_sp,
						      اندازه(ساختار bd_info));
		memset(gd->bd, '\0', اندازه(ساختار bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      اندازه(ساختار bd_info), gd->start_addr_sp);
	}
	بازگشت 0;
}

ایستا صحیح reserve_global_data(تهی)
{
	gd->start_addr_sp = reserve_پشته_aligned(اندازه(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, اندازه(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      اندازه(gd_t), gd->start_addr_sp);
	بازگشت 0;
}

ایستا صحیح reserve_fdt(تهی)
{
	اگر (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		اگر (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_پشته_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	بازگشت 0;
}

ایستا صحیح reserve_bootstage(تهی)
{
#ifdef CONFIG_BOOTSTAGE
	صحیح size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_پشته_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	بازگشت 0;
}

__weak صحیح arch_reserve_stacks(تهی)
{
	بازگشت 0;
}

ایستا صحیح reserve_stacks(تهی)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_پشته_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	بازگشت arch_reserve_stacks();
}

ایستا صحیح reserve_bloblist(تهی)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	اگر (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	بازگشت 0;
}

ایستا صحیح display_new_sp(تهی)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	بازگشت 0;
}

__weak صحیح arch_setup_bdinfo(تهی)
{
	بازگشت 0;
}

صحیح setup_bdinfo(تهی)
{
	بازگشت arch_setup_bdinfo();
}

#ifdef CONFIG_POST
ایستا صحیح init_post(تهی)
{
	post_bootmode_init();
	post_run(پوچ, POST_ROM | post_bootmode_get(0));

	بازگشت 0;
}
#endif

ایستا صحیح reloc_fdt(تهی)
{
	اگر (!IS_ENABLED(CONFIG_OF_EMBED)) {
		اگر (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	بازگشت 0;
}

ایستا صحیح reloc_bootstage(تهی)
{
#ifdef CONFIG_BOOTSTAGE
	اگر (gd->flags & GD_FLG_SKIP_RELOC)
		بازگشت 0;
	اگر (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	بازگشت 0;
}

ایستا صحیح reloc_bloblist(تهی)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	اگر ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		بازگشت 0;
	}
	اگر (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		اگر (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		بازگشت bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	بازگشت 0;
}

تهی mcheck_on_ramrelocation(size_t offset);
ایستا صحیح setup_reloc(تهی)
{
	اگر (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#وگرنهاگر defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#وگرنهاگر defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#وگرنهاگر !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (نویسه *)gd, اندازه(gd_t));

	اگر (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} وگرنه {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	بازگشت 0;
}

#اگر CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
ایستا صحیح fix_fdt(تهی)
{
	بازگشت board_fix_fdt((تهی *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#اگر !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ایستا صحیح jump_to_copy(تهی)
{
	اگر (gd->flags & GD_FLG_SKIP_RELOC)
		بازگشت 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#اگر defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# اگر CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# وگرنه
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#وگرنه
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	بازگشت 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ایستا صحیح initf_bootstage(تهی)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	صحیح ret;

	ret = bootstage_init(!from_spl);
	اگر (ret)
		بازگشت ret;
	اگر (from_spl) {
		ret = bootstage_unstash_default();
		اگر (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			بازگشت ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	بازگشت 0;
}

ایستا صحیح initf_dm(تهی)
{
	صحیح ret;

	اگر (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		بازگشت 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	اگر (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	وگرنه
		ret = dm_init(false);
	اگر (ret)
		بازگشت ret;

	ret = dm_autoprobe();
	اگر (ret)
		بازگشت ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	اگر (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		اگر (ret)
			بازگشت ret;
	}

	بازگشت 0;
}

/* Architecture-specific memory reservation */
__weak صحیح reserve_arch(تهی)
{
	بازگشت 0;
}

__weak صحیح checkcpu(تهی)
{
	بازگشت 0;
}

__weak صحیح clear_bss(تهی)
{
	بازگشت 0;
}

ایستا صحیح initf_upl(تهی)
{
	ساختار upl *upl;
	صحیح ret;

	اگر (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		بازگشت 0;

	upl = malloc(اندازه(ساختار upl));
	اگر (!upl)
		بازگشت -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	اگر (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		بازگشت ret;
	}
	gd_set_upl(upl);

	بازگشت 0;
}

ایستا تهی initcall_run_f(تهی)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#اگر CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#اگر CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#اگر CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#اگر CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#اگر CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#اگر defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#اگر !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#اگر CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#اگر CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#اگر CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#اگر CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#اگر CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#اگر defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(POST)
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
#اگر CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#اگر CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#اگر !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#اگر CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#اگر !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

تهی board_init_f(ulong boot_flags)
{
	ساختار board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#اگر !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#اگر defined(CONFIG_X86) || defined(CONFIG_ARC)
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
ایستا تهی initcall_run_f_r(تهی)
{
#اگر !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

تهی board_init_f_r(تهی)
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
	gd->timer = پوچ;
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
