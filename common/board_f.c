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
#ଅନ୍ତର୍ଭାଵ <bloblist.h>
#ଅନ୍ତର୍ଭାଵ <bootstage.h>
#ଅନ୍ତର୍ଭାଵ <clock_legacy.h>
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <cpu.h>
#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <cyclic.h>
#ଅନ୍ତର୍ଭାଵ <display_options.h>
#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <env_internal.h>
#ଅନ୍ତର୍ଭାଵ <event.h>
#ଅନ୍ତର୍ଭାଵ <fdtdec.h>
#ଅନ୍ତର୍ଭାଵ <fs.h>
#ଅନ୍ତର୍ଭାଵ <hang.h>
#ଅନ୍ତର୍ଭାଵ <i2c.h>
#ଅନ୍ତର୍ଭାଵ <init.h>
#ଅନ୍ତର୍ଭାଵ <initcall.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <memtop.h>
#ଅନ୍ତର୍ଭାଵ <os.h>
#ଅନ୍ତର୍ଭାଵ <post.h>
#ଅନ୍ତର୍ଭାଵ <relocate.h>
#ଅନ୍ତର୍ଭାଵ <serial.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <sysreset.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <timer.h>
#ଅନ୍ତର୍ଭାଵ <trace.h>
#ଅନ୍ତର୍ଭାଵ <upl.h>
#ଅନ୍ତର୍ଭାଵ <video.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <asm/sections.h>
#ଅନ୍ତର୍ଭାଵ <dm/root.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>
#ଅନ୍ତର୍ଭାଵ <linux/log2.h>
#ଅନ୍ତର୍ଭାଵ <linux/sizes.h>

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

#ଯଦି defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_func_watchdog_init(ଶୂନ୍ୟ)
{
# ଯଦି defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_func_watchdog_reset(ଶୂନ୍ୟ)
{
	schedule();

	ଫେରନ୍ତୁ 0;
}
#endif /* CONFIG_WATCHDOG */

__weak ଶୂନ୍ୟ board_add_ram_info(ପୂର୍ଣ୍ଣସଂଖ୍ୟା use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_baud_rate(ଶୂନ୍ୟ)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା display_text_info(ଶୂନ୍ୟ)
{
#ଯଦି !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#ଅନ୍ୟଥା
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	ଫେରନ୍ତୁ 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା print_resetinfo(ଶୂନ୍ୟ)
{
	ସଂରଚନା udevice *dev;
	ଅକ୍ଷର status[256];
	bool status_printed = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	ପାଇଁ (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		ଯଦି (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			ଜାରି;
		}

		ଯଦି (!sysreset_get_status(dev, status, ଆକାର(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	ଯଦି (status_printed)
		printf("\n");

	ଫେରନ୍ତୁ 0;
}
#endif

#ଯଦି defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା print_cpuinfo(ଶୂନ୍ୟ)
{
	ସଂରଚନା udevice *dev;
	ଅକ୍ଷର desc[512];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	dev = cpu_get_current_dev();
	ଯଦି (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		ଫେରନ୍ତୁ -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, ଆକାର(desc));
	ଯଦି (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		ଫେରନ୍ତୁ ret;
	}

	printf("CPU:   %s\n", desc);

	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା announce_dram_init(ଶୂନ୍ୟ)
{
	puts("DRAM:  ");
	ଫେରନ୍ତୁ 0;
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
#define compute_size_scale_val(size, scale, val) କର { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	ଯଦି (val == 10240) { val = 10; scale += 10; } \
} ଯେତେବେଳେ (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	ଫେରନ୍ତୁ size1_scale == size2_scale && size1_val == size2_val;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା show_dram_config(ଶୂନ୍ୟ)
{
	unsigned long long size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	debug("\nRAM Configuration:\n");
	ପାଇଁ (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	ଯଦି (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା dram_init_banksize(ଶୂନ୍ୟ)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	ଫେରନ୍ତୁ 0;
}

#ଯଦି CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_func_i2c(ଶୂନ୍ୟ)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_mon_len(ଶୂନ୍ୟ)
{
#ଯଦି defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#ନଚେତଯଦି defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#ନଚେତଯଦି defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#ନଚେତଯଦି defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#ନଚେତଯଦି defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#ନଚେତଯଦି defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#ନଚେତଯଦି defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#ନଚେତଯଦି defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_spl_handoff(ଶୂନ୍ୟ)
{
#ଯଦି CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					ଆକାର(ସଂରଚନା spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା arch_cpu_init(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା mach_cpu_init(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_ଶୀର୍ଷ(phys_size_t total_size)
{
	ଯଦି (CONFIG_IS_ENABLED(RELOC_ADDR_ଶୀର୍ଷ))
		ଫେରନ୍ତୁ gd->ram_ଶୀର୍ଷ;

#ଯଦି defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	ଯଦି (gd->ram_ଶୀର୍ଷ < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		ଫେରନ୍ତୁ 0;
#endif
	ଫେରନ୍ତୁ gd->ram_ଶୀର୍ଷ;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା arch_setup_dest_addr(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_ram_base(ଶୂନ୍ୟ)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_ram_config(ଶୂନ୍ୟ)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	ଯଦି (CONFIG_IS_ENABLED(RELOC_ADDR_ଶୀର୍ଷ)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		phys_addr_t ଶୀର୍ଷ;

		gd->ram_size = 0;
		ପାଇଁ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			ଶୀର୍ଷ = get_mem_ଶୀର୍ଷ(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (ଶୂନ୍ୟ *)gd->fdt_blob);
			gd->ram_ଶୀର୍ଷ = max(ଶୀର୍ଷ, gd->ram_ଶୀର୍ଷ);
			gd->ram_size += gd->dram[i].size;
		}
	} ଅନ୍ୟଥା {
		gd->ram_ଶୀର୍ଷ = gd->ram_base + get_effective_memsize();
	}
	gd->ram_ଶୀର୍ଷ = board_get_usable_ram_ଶୀର୍ଷ(gd->mon_len);
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
#ଯଦି CONFIG_VAL(SYS_MEM_ଶୀର୍ଷ_HIDE)
	gd->ram_ଶୀର୍ଷ -= CONFIG_SYS_MEM_ଶୀର୍ଷ_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_ଶୀର୍ଷ_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_ଶୀର୍ଷ);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_dest_addr(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	gd->relocaddr = gd->ram_ଶୀର୍ଷ;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	gd->initial_relocaddr = gd->relocaddr;
	ଫେରନ୍ତୁ 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_pram(ଶୂନ୍ୟ)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	ଫେରନ୍ତୁ 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_round_4k(ଶୂନ୍ୟ)
{
	gd->relocaddr &= ~(4096 - 1);
	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା arch_reserve_mmu(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_video_from_videoblob(ଶୂନ୍ୟ)
{
	ଯଦି (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ସଂରଚନା video_handoff *ho;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, ଆକାର(*ho));
		ଯଦି (!ho)
			ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		ଯଦି (ret)
			ଫେରନ୍ତୁ log_ସନ୍ଦେଶ_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		ଯଦି (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_relocaddr_from_bloblist(ଶୂନ୍ୟ)
{
	reserve_video_from_videoblob();

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_video(ଶୂନ୍ୟ)
{
	ଯଦି (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_trace(ଶୂନ୍ୟ)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_uboot(ଶୂନ୍ୟ)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	ଯଦି (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	ଯଦି (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#ଯଦି defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	ଫେରନ୍ତୁ 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ସ୍ଥିର unsigned long reserve_ଗଦା_aligned(size_t size)
{
	ଫେରନ୍ତୁ ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_noncached(ଶୂନ୍ୟ)
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

	ଫେରନ୍ତୁ 0;
}
#endif

/* reserve memory for malloc() area */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_malloc(ଶୂନ୍ୟ)
{
	gd->start_addr_sp = reserve_ଗଦା_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	ଫେରନ୍ତୁ 0;
}

/* (permanently) allocate a Board Info struct */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_board(ଶୂନ୍ୟ)
{
	ଯଦି (!gd->bd) {
		gd->start_addr_sp = reserve_ଗଦା_aligned(ଆକାର(ସଂରଚନା bd_info));
		gd->bd = (ସଂରଚନା bd_info *)map_sysmem(gd->start_addr_sp,
						      ଆକାର(ସଂରଚନା bd_info));
		memset(gd->bd, '\0', ଆକାର(ସଂରଚନା bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      ଆକାର(ସଂରଚନା bd_info), gd->start_addr_sp);
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_global_data(ଶୂନ୍ୟ)
{
	gd->start_addr_sp = reserve_ଗଦା_aligned(ଆକାର(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, ଆକାର(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      ଆକାର(gd_t), gd->start_addr_sp);
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_fdt(ଶୂନ୍ୟ)
{
	ଯଦି (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		ଯଦି (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_ଗଦା_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_bootstage(ଶୂନ୍ୟ)
{
#ifdef CONFIG_BOOTSTAGE
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_ଗଦା_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା arch_reserve_stacks(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_stacks(ଶୂନ୍ୟ)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_ଗଦା_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	ଫେରନ୍ତୁ arch_reserve_stacks();
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_bloblist(ଶୂନ୍ୟ)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	ଯଦି (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା display_new_sp(ଶୂନ୍ୟ)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା arch_setup_bdinfo(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_bdinfo(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ arch_setup_bdinfo();
}

#ifdef CONFIG_POST
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_post(ଶୂନ୍ୟ)
{
	post_bootmode_init();
	post_run(ଖାଲି, POST_ROM | post_bootmode_get(0));

	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reloc_fdt(ଶୂନ୍ୟ)
{
	ଯଦି (!IS_ENABLED(CONFIG_OF_EMBED)) {
		ଯଦି (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reloc_bootstage(ଶୂନ୍ୟ)
{
#ifdef CONFIG_BOOTSTAGE
	ଯଦି (gd->flags & GD_FLG_SKIP_RELOC)
		ଫେରନ୍ତୁ 0;
	ଯଦି (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reloc_bloblist(ଶୂନ୍ୟ)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	ଯଦି ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		ଫେରନ୍ତୁ 0;
	}
	ଯଦି (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		ଯଦି (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		ଫେରନ୍ତୁ bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ mcheck_on_ramrelocation(size_t offset);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_reloc(ଶୂନ୍ୟ)
{
	ଯଦି (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#ନଚେତଯଦି defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#ନଚେତଯଦି defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#ନଚେତଯଦି !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (ଅକ୍ଷର *)gd, ଆକାର(gd_t));

	ଯଦି (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} ଅନ୍ୟଥା {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	ଫେରନ୍ତୁ 0;
}

#ଯଦି CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fix_fdt(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ board_fix_fdt((ଶୂନ୍ୟ *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#ଯଦି !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା jump_to_copy(ଶୂନ୍ୟ)
{
	ଯଦି (gd->flags & GD_FLG_SKIP_RELOC)
		ଫେରନ୍ତୁ 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#ଯଦି defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# ଯଦି CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# ଅନ୍ୟଥା
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#ଅନ୍ୟଥା
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	ଫେରନ୍ତୁ 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initf_bootstage(ଶୂନ୍ୟ)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = bootstage_init(!from_spl);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	ଯଦି (from_spl) {
		ret = bootstage_unstash_default();
		ଯଦି (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			ଫେରନ୍ତୁ ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initf_dm(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		ଫେରନ୍ତୁ 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	ଯଦି (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	ଅନ୍ୟଥା
		ret = dm_init(false);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ret = dm_autoprobe();
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	ଯଦି (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ 0;
}

/* Architecture-specific memory reservation */
__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserve_arch(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkcpu(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା clear_bss(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initf_upl(ଶୂନ୍ୟ)
{
	ସଂରଚନା upl *upl;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ଯଦି (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		ଫେରନ୍ତୁ 0;

	upl = malloc(ଆକାର(ସଂରଚନା upl));
	ଯଦି (!upl)
		ଫେରନ୍ତୁ -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	ଯଦି (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		ଫେରନ୍ତୁ ret;
	}
	gd_set_upl(upl);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ initcall_run_f(ଶୂନ୍ୟ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#ଯଦି CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#ଯଦି CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#ଯଦି CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#ଯଦି CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#ଯଦି CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#ଯଦି defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#ଯଦି !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#ଯଦି CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#ଯଦି CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#ଯଦି CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#ଯଦି CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#ଯଦି CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#ଯଦି CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#ଯଦି CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#ଯଦି defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#ଯଦି CONFIG_IS_ENABLED(POST)
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
#ଯଦି CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#ଯଦି CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#ଯଦି !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#ଯଦି CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#ଯଦି !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

ଶୂନ୍ୟ board_init_f(ulong boot_flags)
{
	ସଂରଚନା board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#ଯଦି !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#ଯଦି defined(CONFIG_X86) || defined(CONFIG_ARC)
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
ସ୍ଥିର ଶୂନ୍ୟ initcall_run_f_r(ଶୂନ୍ୟ)
{
#ଯଦି !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

ଶୂନ୍ୟ board_init_f_r(ଶୂନ୍ୟ)
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
	gd->timer = ଖାଲି;
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
