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

#అన్తర్భావ <config.h>
#అన్తర్భావ <bloblist.h>
#అన్తర్భావ <bootstage.h>
#అన్తర్భావ <clock_legacy.h>
#అన్తర్భావ <console.h>
#అన్తర్భావ <cpu.h>
#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <cyclic.h>
#అన్తర్భావ <display_options.h>
#అన్తర్భావ <dm.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <env_internal.h>
#అన్తర్భావ <event.h>
#అన్తర్భావ <fdtdec.h>
#అన్తర్భావ <fs.h>
#అన్తర్భావ <hang.h>
#అన్తర్భావ <i2c.h>
#అన్తర్భావ <init.h>
#అన్తర్భావ <initcall.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <memtop.h>
#అన్తర్భావ <os.h>
#అన్తర్భావ <post.h>
#అన్తర్భావ <relocate.h>
#అన్తర్భావ <serial.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <sysreset.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <timer.h>
#అన్తర్భావ <trace.h>
#అన్తర్భావ <upl.h>
#అన్తర్భావ <video.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <asm/cache.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <asm/sections.h>
#అన్తర్భావ <dm/root.h>
#అన్తర్భావ <linux/errno.h>
#అన్తర్భావ <linux/log2.h>
#అన్తర్భావ <linux/sizes.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

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

#ఒకవేళ defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
స్థిర పూర్ణాంకం init_func_watchdog_init(శూన్య)
{
# ఒకవేళ defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	కాలపట్టిక();

	తిరిగి 0;
}

పూర్ణాంకం init_func_watchdog_reset(శూన్య)
{
	కాలపట్టిక();

	తిరిగి 0;
}
#అయితే_ముగింపు /* CONFIG_WATCHDOG */

__weak శూన్య board_కూడు_ram_info(పూర్ణాంకం use_default)
{
	/* please define platform specific board_add_ram_info() */
}

స్థిర పూర్ణాంకం init_baud_రేటు(శూన్య)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం display_పాఠ్యం_info(శూన్య)
{
#ఒకవేళ !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_ప్రారంభం, bss_చివర, పాఠ్యం_పునాది;

	bss_ప్రారంభం = (ulong)__bss_start;
	bss_చివర = (ulong)__bss_end;

#నిర్వచితమైతే CONFIG_పాఠ్యం_పునాది
	పాఠ్యం_పునాది = CONFIG_పాఠ్యం_పునాది;
#లేకపోతే
	పాఠ్యం_పునాది = CONFIG_SYS_MONITOR_పునాది;
#అయితే_ముగింపు

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      పాఠ్యం_పునాది, bss_ప్రారంభం, bss_చివర);
#అయితే_ముగింపు

	తిరిగి 0;
}

#నిర్వచితమైతే CONFIG_SYSRESET_PRINT_RESETINFO
స్థిర పూర్ణాంకం print_resetinfo(శూన్య)
{
	నిర్మాణం udevice *dev;
	అక్షరం స్థితి_2[256];
	bool స్థితి_printed = false;
	పూర్ణాంకం ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	కొరకు (ret = uclass_మొదటి_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_తదుపరి_device_check(&dev)) {
		ఒకవేళ (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			కొనసాగు;
		}

		ఒకవేళ (!sysreset_get_స్థితి(dev, స్థితి_2, పరిమాణం(స్థితి_2))) {
			రాయి("%s%s", స్థితి_printed ? " " : "", స్థితి_2);
			స్థితి_printed = true;
		}
	}
	ఒకవేళ (స్థితి_printed)
		రాయి("\n");

	తిరిగి 0;
}
#అయితే_ముగింపు

#ఒకవేళ defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
స్థిర పూర్ణాంకం print_cpuinfo(శూన్య)
{
	నిర్మాణం udevice *dev;
	అక్షరం desc[512];
	పూర్ణాంకం ret;

	dev = cpu_get_current_dev();
	ఒకవేళ (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		తిరిగి -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, పరిమాణం(desc));
	ఒకవేళ (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		తిరిగి ret;
	}

	రాయి("CPU:   %s\n", desc);

	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం announce_dram_init(శూన్య)
{
	puts("DRAM:  ");
	తిరిగి 0;
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
#నిర్వచించు compute_కొలత_scale_val(కొలత, scale, val) చేయి { \
	scale = ilog2(కొలత) / 10 * 10; \
	val = (10 * కొలత + ((1ULL << scale) >> 1)) >> scale; \
	ఒకవేళ (val == 10240) { val = 10; scale += 10; } \
} అయితే (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
స్థిర పూర్ణాంకం sizes_near(గుర్తులేని దీర్ఘ దీర్ఘ size1, గుర్తులేని దీర్ఘ దీర్ఘ size2)
{
	గుర్తులేని పూర్ణాంకం size1_scale, size1_val, size2_scale, size2_val;

	compute_కొలత_scale_val(size1, size1_scale, size1_val);
	compute_కొలత_scale_val(size2, size2_scale, size2_val);

	తిరిగి size1_scale == size2_scale && size1_val == size2_val;
}

స్థిర పూర్ణాంకం show_dram_config(శూన్య)
{
	గుర్తులేని దీర్ఘ దీర్ఘ కొలత;
	పూర్ణాంకం i;

	debug("\nRAM Configuration:\n");
	కొరకు (i = కొలత = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		కొలత += gd->dram[i].కొలత;
		debug("Bank #%d: %llx ", i,
		      (గుర్తులేని దీర్ఘ దీర్ఘ)(gd->dram[i].ప్రారంభం));
#నిర్వచితమైతే DEBUG
		print_కొలత(gd->dram[i].కొలత, "\n");
#అయితే_ముగింపు
	}
	debug("\nDRAM:  ");

	print_కొలత(gd->ram_కొలత, "");
	ఒకవేళ (!sizes_near(gd->ram_కొలత, కొలత)) {
		రాయి(" (total ");
		print_కొలత(కొలత, ")");
	}
	board_కూడు_ram_info(0);
	putc('\n');

	తిరిగి 0;
}

__weak పూర్ణాంకం dram_init_banksize(శూన్య)
{
	gd->dram[0].ప్రారంభం = gd->ram_పునాది;
	gd->dram[0].కొలత = get_effective_memsize();

	తిరిగి 0;
}

#ఒకవేళ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
స్థిర పూర్ణాంకం init_func_i2c(శూన్య)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం అమరిక_mon_len(శూన్య)
{
#ఒకవేళ defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#లేదావేళ defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)ప్రారంభం_2;
#లేదావేళ defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)చివర - (ulong)_init;
#లేదావేళ defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#లేదావేళ defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)చివర - (ulong)_init;
#లేదావేళ defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#లేదావేళ defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(ప్రారంభం_2);
#లేదావేళ defined(CONFIG_SYS_MONITOR_పునాది)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_పునాది;
#అయితే_ముగింపు
	తిరిగి 0;
}

స్థిర పూర్ణాంకం అమరిక_spl_handoff(శూన్య)
{
#ఒకవేళ CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_కనుగొను(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					పరిమాణం(నిర్మాణం spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#అయితే_ముగింపు

	తిరిగి 0;
}

__weak పూర్ణాంకం arch_cpu_init(శూన్య)
{
	తిరిగి 0;
}

__weak పూర్ణాంకం mach_cpu_init(శూన్య)
{
	తిరిగి 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_శిఖరం(phys_కొలత_t మొత్తం_కొలత)
{
	ఒకవేళ (CONFIG_IS_ENABLED(RELOC_ADDR_శిఖరం))
		తిరిగి gd->ram_శిఖరం;

#ఒకవేళ defined(CFG_SYS_SDRAM_పునాది) && CFG_SYS_SDRAM_పునాది > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	ఒకవేళ (gd->ram_శిఖరం < CFG_SYS_SDRAM_పునాది)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		తిరిగి 0;
#అయితే_ముగింపు
	తిరిగి gd->ram_శిఖరం;
}

__weak పూర్ణాంకం arch_అమరిక_dest_addr(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం అమరిక_ram_పునాది(శూన్య)
{
#నిర్వచితమైతే CFG_SYS_SDRAM_పునాది
	gd->ram_పునాది = CFG_SYS_SDRAM_పునాది;
#అయితే_ముగింపు
	తిరిగి 0;
}

స్థిర పూర్ణాంకం అమరిక_ram_config(శూన్య)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	ఒకవేళ (CONFIG_IS_ENABLED(RELOC_ADDR_శిఖరం)) {
		పూర్ణాంకం i;
		phys_addr_t శిఖరం;

		gd->ram_కొలత = 0;
		కొరకు (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			శిఖరం = get_mem_శిఖరం(gd->dram[i].ప్రారంభం, gd->dram[i].కొలత,
					  ALIGN(gd->mon_len, SZ_1M),
					  (శూన్య *)gd->fdt_blob);
			gd->ram_శిఖరం = max(శిఖరం, gd->ram_శిఖరం);
			gd->ram_కొలత += gd->dram[i].కొలత;
		}
	} లేకపోతే {
		gd->ram_శిఖరం = gd->ram_పునాది + get_effective_memsize();
	}
	gd->ram_శిఖరం = board_get_usable_ram_శిఖరం(gd->mon_len);
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
#ఒకవేళ CONFIG_VAL(SYS_MEM_శిఖరం_HIDE)
	gd->ram_శిఖరం -= CONFIG_SYS_MEM_శిఖరం_HIDE;
	gd->ram_కొలత -= CONFIG_SYS_MEM_శిఖరం_HIDE;
#అయితే_ముగింపు

	debug("Ram top: %08llx\n", (గుర్తులేని దీర్ఘ దీర్ఘ)gd->ram_శిఖరం);
	debug("Ram size: %08llx\n", (గుర్తులేని దీర్ఘ దీర్ఘ)gd->ram_కొలత);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం అమరిక_dest_addr(శూన్య)
{
	పూర్ణాంకం ret;

	gd->relocaddr = gd->ram_శిఖరం;
	debug("Reloc addr: %08llX\n", (గుర్తులేని దీర్ఘ దీర్ఘ)gd->relocaddr);

	ret = arch_అమరిక_dest_addr();
	ఒకవేళ (ret)
		తిరిగి ret;

	gd->initial_relocaddr = gd->relocaddr;
	తిరిగి 0;
}

#నిర్వచితమైతే CFG_PRAM
/* reserve protected RAM */
స్థిర పూర్ణాంకం reserve_pram(శూన్య)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	తిరిగి 0;
}
#అయితే_ముగింపు /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
స్థిర పూర్ణాంకం reserve_విడత_4k(శూన్య)
{
	gd->relocaddr &= ~(4096 - 1);
	తిరిగి 0;
}

__weak పూర్ణాంకం arch_reserve_mmu(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_video_from_videoblob(శూన్య)
{
	ఒకవేళ (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		నిర్మాణం video_handoff *ho;
		పూర్ణాంకం ret = 0;

		ho = bloblist_కనుగొను(BLOBLISTT_U_BOOT_VIDEO, పరిమాణం(*ho));
		ఒకవేళ (!ho)
			తిరిగి log_సందేశం_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		ఒకవేళ (ret)
			తిరిగి log_సందేశం_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		ఒకవేళ (likely(gd->relocaddr > (గుర్తులేని దీర్ఘ)ho->fb))
			gd->relocaddr = ho->fb;
	}

	తిరిగి 0;
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
స్థిర పూర్ణాంకం అమరిక_relocaddr_from_bloblist(శూన్య)
{
	reserve_video_from_videoblob();

	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_video(శూన్య)
{
	ఒకవేళ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		పూర్ణాంకం ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		ఒకవేళ (ret)
			తిరిగి ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((గుర్తులేని దీర్ఘ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_trace(శూన్య)
{
#నిర్వచితమైతే CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_బఫరు_కొలత;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_బఫరు_కొలత);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (గుర్తులేని దీర్ఘ)CONFIG_TRACE_బఫరు_కొలత >> 10, gd->relocaddr);
#అయితే_ముగింపు

	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_uboot(శూన్య)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	ఒకవేళ (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	ఒకవేళ (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_సంకేతం)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#ఒకవేళ defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#అయితే_ముగింపు

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->ప్రారంభం_addr_sp = gd->relocaddr;

	తిరిగి 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
స్థిర గుర్తులేని దీర్ఘ reserve_దొంతర_aligned(size_t కొలత)
{
	తిరిగి ALIGN_కిందికి(gd->ప్రారంభం_addr_sp - కొలత, 16);
}

#నిర్వచితమైతే CONFIG_SYS_NONCACHED_జ్ఞాపకం
స్థిర పూర్ణాంకం reserve_noncached(శూన్య)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->ప్రారంభం_addr_sp = ALIGN(gd->ప్రారంభం_addr_sp, MMU_SECTION_కొలత) -
		MMU_SECTION_కొలత;
	gd->ప్రారంభం_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_జ్ఞాపకం,
				   MMU_SECTION_కొలత);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_జ్ఞాపకం >> 20, gd->ప్రారంభం_addr_sp);

	తిరిగి 0;
}
#అయితే_ముగింపు

/* reserve memory for malloc() area */
స్థిర పూర్ణాంకం reserve_malloc(శూన్య)
{
	gd->ప్రారంభం_addr_sp = reserve_దొంతర_aligned(మొత్తం_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      మొత్తం_MALLOC_LEN >> 10, gd->ప్రారంభం_addr_sp);
#నిర్వచితమైతే CONFIG_SYS_NONCACHED_జ్ఞాపకం
	reserve_noncached();
#అయితే_ముగింపు

	తిరిగి 0;
}

/* (permanently) allocate a Board Info struct */
స్థిర పూర్ణాంకం reserve_board(శూన్య)
{
	ఒకవేళ (!gd->bd) {
		gd->ప్రారంభం_addr_sp = reserve_దొంతర_aligned(పరిమాణం(నిర్మాణం bd_info));
		gd->bd = (నిర్మాణం bd_info *)map_sysmem(gd->ప్రారంభం_addr_sp,
						      పరిమాణం(నిర్మాణం bd_info));
		memset(gd->bd, '\0', పరిమాణం(నిర్మాణం bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      పరిమాణం(నిర్మాణం bd_info), gd->ప్రారంభం_addr_sp);
	}
	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_global_దత్తాంశం(శూన్య)
{
	gd->ప్రారంభం_addr_sp = reserve_దొంతర_aligned(పరిమాణం(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->ప్రారంభం_addr_sp, పరిమాణం(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      పరిమాణం(gd_t), gd->ప్రారంభం_addr_sp);
	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_fdt(శూన్య)
{
	ఒకవేళ (!IS_ENABLED(CONFIG_యొక్క_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		ఒకవేళ (gd->fdt_blob) {
			gd->boardf->fdt_కొలత =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->ప్రారంభం_addr_sp = reserve_దొంతర_aligned(
				gd->boardf->fdt_కొలత);
			gd->boardf->new_fdt = map_sysmem(gd->ప్రారంభం_addr_sp,
							 gd->boardf->fdt_కొలత);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_కొలత, gd->ప్రారంభం_addr_sp);
		}
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_bootstage(శూన్య)
{
#నిర్వచితమైతే CONFIG_BOOTSTAGE
	పూర్ణాంకం కొలత = bootstage_get_కొలత(true);

	gd->ప్రారంభం_addr_sp = reserve_దొంతర_aligned(కొలత);
	gd->boardf->new_bootstage = map_sysmem(gd->ప్రారంభం_addr_sp, కొలత);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", కొలత,
	      gd->ప్రారంభం_addr_sp);
#అయితే_ముగింపు

	తిరిగి 0;
}

__weak పూర్ణాంకం arch_reserve_stacks(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం reserve_stacks(శూన్య)
{
	/* make stack pointer 16-byte aligned */
	gd->ప్రారంభం_addr_sp = reserve_దొంతర_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	తిరిగి arch_reserve_stacks();
}

స్థిర పూర్ణాంకం reserve_bloblist(శూన్య)
{
#నిర్వచితమైతే CONFIG_BLOBLIST
	ulong కొలత = bloblist_get_మొత్తం_కొలత();

	ఒకవేళ (కొలత < CONFIG_BLOBLIST_కొలత_RELOC)
		కొలత = CONFIG_BLOBLIST_కొలత_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->ప్రారంభం_addr_sp = ALIGN_కిందికి(gd->ప్రారంభం_addr_sp - కొలత, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->ప్రారంభం_addr_sp, కొలత);
#అయితే_ముగింపు

	తిరిగి 0;
}

స్థిర పూర్ణాంకం display_new_sp(శూన్య)
{
	debug("New Stack Pointer is: %08lx\n", gd->ప్రారంభం_addr_sp);

	తిరిగి 0;
}

__weak పూర్ణాంకం arch_అమరిక_bdinfo(శూన్య)
{
	తిరిగి 0;
}

పూర్ణాంకం అమరిక_bdinfo(శూన్య)
{
	తిరిగి arch_అమరిక_bdinfo();
}

#నిర్వచితమైతే CONFIG_POST
స్థిర పూర్ణాంకం init_post(శూన్య)
{
	post_bootmode_init();
	post_నిరంతరం(ఖాళీ, POST_ROM | post_bootmode_get(0));

	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం reloc_fdt(శూన్య)
{
	ఒకవేళ (!IS_ENABLED(CONFIG_యొక్క_EMBED)) {
		ఒకవేళ (gd->boardf->new_fdt) {
			జ్ఞాపకం_నకలు(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	తిరిగి 0;
}

స్థిర పూర్ణాంకం reloc_bootstage(శూన్య)
{
#నిర్వచితమైతే CONFIG_BOOTSTAGE
	ఒకవేళ (gd->flags & GD_FLG_SKIP_RELOC)
		తిరిగి 0;
	ఒకవేళ (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#అయితే_ముగింపు

	తిరిగి 0;
}

స్థిర పూర్ణాంకం reloc_bloblist(శూన్య)
{
#నిర్వచితమైతే CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	ఒకవేళ ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_కొలత == CONFIG_BLOBLIST_కొలత_RELOC) {
		debug("Not relocating bloblist\n");
		తిరిగి 0;
	}
	ఒకవేళ (gd->boardf->new_bloblist) {
		ulong కొలత = bloblist_get_మొత్తం_కొలత();

		ఒకవేళ (కొలత < CONFIG_BLOBLIST_కొలత_RELOC)
			కొలత = CONFIG_BLOBLIST_కొలత_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, కొలత);
		తిరిగి bloblist_reloc(gd->boardf->new_bloblist, కొలత);
	}
#అయితే_ముగింపు

	తిరిగి 0;
}

శూన్య mcheck_on_ramrelocation(size_t స్థానభ్రంశం);
స్థిర పూర్ణాంకం అమరిక_reloc(శూన్య)
{
	ఒకవేళ (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#నిర్వచితమైతే CONFIG_పాఠ్యం_పునాది
#నిర్వచితమైతే ARM
		gd->reloc_off = gd->relocaddr - (గుర్తులేని దీర్ఘ)__image_copy_start;
#లేదావేళ defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)ప్రారంభం_2;
#లేదావేళ defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_పాఠ్యం_పునాది + 0x400);
#లేదావేళ !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_పాఠ్యం_పునాది;
#అయితే_ముగింపు
#అయితే_ముగింపు
	}

	జ్ఞాపకం_నకలు(gd->new_gd, (అక్షరం *)gd, పరిమాణం(gd_t));

	ఒకవేళ (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} లేకపోతే {
#నిర్వచితమైతే MCHECK_కుప్ప_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#అయితే_ముగింపు
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_గమ్యం_sysmem(gd->new_gd),
		      gd->ప్రారంభం_addr_sp);
	}

	తిరిగి 0;
}

#ఒకవేళ CONFIG_IS_ENABLED(యొక్క_BOARD_FIXUP)
స్థిర పూర్ణాంకం fix_fdt(శూన్య)
{
	తిరిగి board_fix_fdt((శూన్య *)gd->fdt_blob);
}
#అయితే_ముగింపు

/* ARM calls relocate_code from its crt0.S */
#ఒకవేళ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

స్థిర పూర్ణాంకం jump_గమ్యం_copy(శూన్య)
{
	ఒకవేళ (gd->flags & GD_FLG_SKIP_RELOC)
		తిరిగి 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#ఒకవేళ defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_అమరిక_gd(gd->new_gd);
# ఒకవేళ CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->ప్రారంభం_addr_sp);
# లేకపోతే
		board_init_f_r_trampoline(gd->ప్రారంభం_addr_sp);
# endif
#లేకపోతే
	relocate_సంకేతం(gd->ప్రారంభం_addr_sp, gd->new_gd, gd->relocaddr);
#అయితే_ముగింపు

	తిరిగి 0;
}
#అయితే_ముగింపు

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
స్థిర పూర్ణాంకం initf_bootstage(శూన్య)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	పూర్ణాంకం ret;

	ret = bootstage_init(!from_spl);
	ఒకవేళ (ret)
		తిరిగి ret;
	ఒకవేళ (from_spl) {
		ret = bootstage_unstash_default();
		ఒకవేళ (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			తిరిగి ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_ప్రారంభం_UBOOT_F, "board_init_f");

	తిరిగి 0;
}

స్థిర పూర్ణాంకం initf_dm(శూన్య)
{
	పూర్ణాంకం ret;

	ఒకవేళ (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		తిరిగి 0;

	bootstage_ప్రారంభం(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	ఒకవేళ (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_పరిశీలన(true);
	లేకపోతే
		ret = dm_init(false);
	ఒకవేళ (ret)
		తిరిగి ret;

	ret = dm_autoprobe();
	ఒకవేళ (ret)
		తిరిగి ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	ఒకవేళ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	తిరిగి 0;
}

/* Architecture-specific memory reservation */
__weak పూర్ణాంకం reserve_arch(శూన్య)
{
	తిరిగి 0;
}

__weak పూర్ణాంకం checkcpu(శూన్య)
{
	తిరిగి 0;
}

__weak పూర్ణాంకం clear_bss(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం initf_upl(శూన్య)
{
	నిర్మాణం upl *upl;
	పూర్ణాంకం ret;

	ఒకవేళ (!IS_ENABLED(CONFIG_UPL_లోపల) || !(gd->flags & GD_FLG_UPL))
		తిరిగి 0;

	upl = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం upl));
	ఒకవేళ (!upl)
		తిరిగి -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	ఒకవేళ (ret) {
		రాయి("UPL handoff: read failure (err=%dE)\n", ret);
		తిరిగి ret;
	}
	gd_సమితి_upl(upl);

	తిరిగి 0;
}

స్థిర శూన్య initcall_నిరంతరం_f(శూన్య)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(అమరిక_mon_len);
	INITCALL(initf_malloc);
#ఒకవేళ CONFIG_IS_ENABLED(యొక్క_CONTROL)
	INITCALL(fdtdec_అమరిక);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#అయితే_ముగింపు
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#ఒకవేళ CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#అయితే_ముగింపు
	INITCALL(అమరిక_spl_handoff);
#ఒకవేళ CONFIG_IS_ENABLED(CONSOLE_రికార్డు_INIT_F)
	INITCALL(console_రికార్డు_init);
#అయితే_ముగింపు
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#ఒకవేళ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#అయితే_ముగింపు
#ఒకవేళ defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#అయితే_ముగింపు
#ఒకవేళ !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#అయితే_ముగింపు
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_రేటు);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_పాఠ్యం_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#ఒకవేళ CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#అయితే_ముగింపు
	/* display cpu info (and speed) */
#ఒకవేళ CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#అయితే_ముగింపు
#ఒకవేళ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#అయితే_ముగింపు
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#ఒకవేళ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#అయితే_ముగింపు
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#ఒకవేళ CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#అయితే_ముగింపు
	WATCHDOG_RESET();
#ఒకవేళ defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#అయితే_ముగింపు /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#ఒకవేళ CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#అయితే_ముగింపు
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
	INITCALL(అమరిక_ram_పునాది);
	INITCALL(dram_init_banksize);
	INITCALL(అమరిక_ram_config);
	INITCALL(అమరిక_dest_addr);
#ఒకవేళ CONFIG_IS_ENABLED(యొక్క_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(యొక్క_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#అయితే_ముగింపు
#నిర్వచితమైతే CFG_PRAM
	INITCALL(reserve_pram);
#అయితే_ముగింపు
	INITCALL(reserve_విడత_4k);
	INITCALL(అమరిక_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_దత్తాంశం);
	INITCALL(reserve_fdt);
#ఒకవేళ CONFIG_IS_ENABLED(యొక్క_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(యొక్క_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#అయితే_ముగింపు
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(అమరిక_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#ఒకవేళ !CONFIG_IS_ENABLED(యొక్క_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#అయితే_ముగింపు
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(అమరిక_reloc);
#ఒకవేళ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_గమ్యం_ram);
	INITCALL(do_elf_reloc_fixups);
#అయితే_ముగింపు
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
#ఒకవేళ !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_గమ్యం_copy);
#అయితే_ముగింపు
}

శూన్య board_init_f(ulong boot_flags)
{
	నిర్మాణం board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_నిరంతరం_f();

#ఒకవేళ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#అయితే_ముగింపు
}

#ఒకవేళ defined(CONFIG_X86) || defined(CONFIG_ARC)
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
స్థిర శూన్య initcall_నిరంతరం_f_r(శూన్య)
{
#ఒకవేళ !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#అయితే_ముగింపు
}

శూన్య board_init_f_r(శూన్య)
{
	initcall_నిరంతరం_f_r();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#నిర్వచితమైతే CONFIG_TIMER
	gd->timer = ఖాళీ;
#అయితే_ముగింపు

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#అయితే_ముగింపు /* CONFIG_X86 */
