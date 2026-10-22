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

#ضمن <config.h>
#ضمن <bloblist.h>
#ضمن <bootstage.h>
#ضمن <clock_legacy.h>
#ضمن <console.h>
#ضمن <cpu.h>
#ضمن <cpu_func.h>
#ضمن <cyclic.h>
#ضمن <display_options.h>
#ضمن <dm.h>
#ضمن <env.h>
#ضمن <env_internal.h>
#ضمن <event.h>
#ضمن <fdtdec.h>
#ضمن <fs.h>
#ضمن <hang.h>
#ضمن <i2c.h>
#ضمن <init.h>
#ضمن <initcall.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <memtop.h>
#ضمن <os.h>
#ضمن <post.h>
#ضمن <relocate.h>
#ضمن <serial.h>
#ضمن <spl.h>
#ضمن <sysreset.h>
#ضمن <time.h>
#ضمن <timer.h>
#ضمن <trace.h>
#ضمن <upl.h>
#ضمن <video.h>
#ضمن <watchdog.h>
#ضمن <asm/cache.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <asm/sections.h>
#ضمن <dm/root.h>
#ضمن <linux/errno.h>
#ضمن <linux/log2.h>
#ضمن <linux/sizes.h>

DECLARE_GLOBAL_معلومات_PTR;

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

#إذا defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
ساكن صحيح init_func_watchdog_init(فراغ)
{
# إذا defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	وقت_جدول();

	أرجع 0;
}

صحيح init_func_watchdog_reset(فراغ)
{
	وقت_جدول();

	أرجع 0;
}
#جيڪڏهن_پورو /* CONFIG_WATCHDOG */

__weak فراغ board_جوڙيو_ram_info(صحيح use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ساكن صحيح init_baud_شرح(فراغ)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	أرجع 0;
}

ساكن صحيح display_متن_info(فراغ)
{
#إذا !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_شروعات, bss_پڇاڙي, متن_بنياد;

	bss_شروعات = (ulong)__bss_start;
	bss_پڇاڙي = (ulong)__bss_end;

#جيڪڏهن_وصف_ٿيل CONFIG_متن_بنياد
	متن_بنياد = CONFIG_متن_بنياد;
#وإلا
	متن_بنياد = CONFIG_SYS_MONITOR_بنياد;
#جيڪڏهن_پورو

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      متن_بنياد, bss_شروعات, bss_پڇاڙي);
#جيڪڏهن_پورو

	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CONFIG_SYSRESET_PRINT_RESETINFO
ساكن صحيح print_resetinfo(فراغ)
{
	بنية udevice *dev;
	حرف حالت[256];
	bool حالت_printed = false;
	صحيح ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	لكل (ret = uclass_پهريون_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_ايندڙ_device_check(&dev)) {
		إذا (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			استمر;
		}

		إذا (!sysreset_get_حالت(dev, حالت, حجم(حالت))) {
			لکو("%s%s", حالت_printed ? " " : "", حالت);
			حالت_printed = true;
		}
	}
	إذا (حالت_printed)
		لکو("\n");

	أرجع 0;
}
#جيڪڏهن_پورو

#إذا defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ساكن صحيح print_cpuinfo(فراغ)
{
	بنية udevice *dev;
	حرف desc[512];
	صحيح ret;

	dev = cpu_get_current_dev();
	إذا (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		أرجع -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, حجم(desc));
	إذا (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		أرجع ret;
	}

	لکو("CPU:   %s\n", desc);

	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح announce_dram_init(فراغ)
{
	puts("DRAM:  ");
	أرجع 0;
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
#وصف_ڪريو compute_سائيز_scale_val(سائيز, scale, val) افعل { \
	scale = ilog2(سائيز) / 10 * 10; \
	val = (10 * سائيز + ((1ULL << scale) >> 1)) >> scale; \
	إذا (val == 10240) { val = 10; scale += 10; } \
} طالما (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ساكن صحيح sizes_near(بنانشان ڊگهو ڊگهو size1, بنانشان ڊگهو ڊگهو size2)
{
	بنانشان صحيح size1_scale, size1_val, size2_scale, size2_val;

	compute_سائيز_scale_val(size1, size1_scale, size1_val);
	compute_سائيز_scale_val(size2, size2_scale, size2_val);

	أرجع size1_scale == size2_scale && size1_val == size2_val;
}

ساكن صحيح show_dram_config(فراغ)
{
	بنانشان ڊگهو ڊگهو سائيز;
	صحيح i;

	debug("\nRAM Configuration:\n");
	لكل (i = سائيز = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		سائيز += gd->dram[i].سائيز;
		debug("Bank #%d: %llx ", i,
		      (بنانشان ڊگهو ڊگهو)(gd->dram[i].شروعات));
#جيڪڏهن_وصف_ٿيل DEBUG
		print_سائيز(gd->dram[i].سائيز, "\n");
#جيڪڏهن_پورو
	}
	debug("\nDRAM:  ");

	print_سائيز(gd->ram_سائيز, "");
	إذا (!sizes_near(gd->ram_سائيز, سائيز)) {
		لکو(" (total ");
		print_سائيز(سائيز, ")");
	}
	board_جوڙيو_ram_info(0);
	putc('\n');

	أرجع 0;
}

__weak صحيح dram_init_banksize(فراغ)
{
	gd->dram[0].شروعات = gd->ram_بنياد;
	gd->dram[0].سائيز = get_effective_memsize();

	أرجع 0;
}

#إذا CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ساكن صحيح init_func_i2c(فراغ)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح تياري_mon_len(فراغ)
{
#إذا defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#وإلاإذا defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)شروعات_2;
#وإلاإذا defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)پڇاڙي - (ulong)_init;
#وإلاإذا defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#وإلاإذا defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)پڇاڙي - (ulong)_init;
#وإلاإذا defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#وإلاإذا defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(شروعات_2);
#وإلاإذا defined(CONFIG_SYS_MONITOR_بنياد)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_بنياد;
#جيڪڏهن_پورو
	أرجع 0;
}

ساكن صحيح تياري_spl_handoff(فراغ)
{
#إذا CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_ڳوليو(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					حجم(بنية spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#جيڪڏهن_پورو

	أرجع 0;
}

__weak صحيح arch_cpu_init(فراغ)
{
	أرجع 0;
}

__weak صحيح mach_cpu_init(فراغ)
{
	أرجع 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_چوٽي(phys_سائيز_t ڪل_سائيز)
{
	إذا (CONFIG_IS_ENABLED(RELOC_ADDR_چوٽي))
		أرجع gd->ram_چوٽي;

#إذا defined(CFG_SYS_SDRAM_بنياد) && CFG_SYS_SDRAM_بنياد > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	إذا (gd->ram_چوٽي < CFG_SYS_SDRAM_بنياد)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		أرجع 0;
#جيڪڏهن_پورو
	أرجع gd->ram_چوٽي;
}

__weak صحيح arch_تياري_dest_addr(فراغ)
{
	أرجع 0;
}

ساكن صحيح تياري_ram_بنياد(فراغ)
{
#جيڪڏهن_وصف_ٿيل CFG_SYS_SDRAM_بنياد
	gd->ram_بنياد = CFG_SYS_SDRAM_بنياد;
#جيڪڏهن_پورو
	أرجع 0;
}

ساكن صحيح تياري_ram_config(فراغ)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	إذا (CONFIG_IS_ENABLED(RELOC_ADDR_چوٽي)) {
		صحيح i;
		phys_addr_t چوٽي;

		gd->ram_سائيز = 0;
		لكل (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			چوٽي = get_mem_چوٽي(gd->dram[i].شروعات, gd->dram[i].سائيز,
					  ALIGN(gd->mon_len, SZ_1M),
					  (فراغ *)gd->fdt_blob);
			gd->ram_چوٽي = max(چوٽي, gd->ram_چوٽي);
			gd->ram_سائيز += gd->dram[i].سائيز;
		}
	} وإلا {
		gd->ram_چوٽي = gd->ram_بنياد + get_effective_memsize();
	}
	gd->ram_چوٽي = board_get_usable_ram_چوٽي(gd->mon_len);
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
#إذا CONFIG_VAL(SYS_MEM_چوٽي_HIDE)
	gd->ram_چوٽي -= CONFIG_SYS_MEM_چوٽي_HIDE;
	gd->ram_سائيز -= CONFIG_SYS_MEM_چوٽي_HIDE;
#جيڪڏهن_پورو

	debug("Ram top: %08llx\n", (بنانشان ڊگهو ڊگهو)gd->ram_چوٽي);
	debug("Ram size: %08llx\n", (بنانشان ڊگهو ڊگهو)gd->ram_سائيز);

	أرجع 0;
}

ساكن صحيح تياري_dest_addr(فراغ)
{
	صحيح ret;

	gd->relocaddr = gd->ram_چوٽي;
	debug("Reloc addr: %08llX\n", (بنانشان ڊگهو ڊگهو)gd->relocaddr);

	ret = arch_تياري_dest_addr();
	إذا (ret)
		أرجع ret;

	gd->initial_relocaddr = gd->relocaddr;
	أرجع 0;
}

#جيڪڏهن_وصف_ٿيل CFG_PRAM
/* reserve protected RAM */
ساكن صحيح reserve_pram(فراغ)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	أرجع 0;
}
#جيڪڏهن_پورو /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ساكن صحيح reserve_ڦيرو_4k(فراغ)
{
	gd->relocaddr &= ~(4096 - 1);
	أرجع 0;
}

__weak صحيح arch_reserve_mmu(فراغ)
{
	أرجع 0;
}

ساكن صحيح reserve_video_from_videoblob(فراغ)
{
	إذا (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		بنية video_handoff *ho;
		صحيح ret = 0;

		ho = bloblist_ڳوليو(BLOBLISTT_U_BOOT_VIDEO, حجم(*ho));
		إذا (!ho)
			أرجع log_نياپو_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		إذا (ret)
			أرجع log_نياپو_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		إذا (likely(gd->relocaddr > (بنانشان ڊگهو)ho->fb))
			gd->relocaddr = ho->fb;
	}

	أرجع 0;
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
ساكن صحيح تياري_relocaddr_from_bloblist(فراغ)
{
	reserve_video_from_videoblob();

	أرجع 0;
}

ساكن صحيح reserve_video(فراغ)
{
	إذا (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحيح ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		إذا (ret)
			أرجع ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((بنانشان ڊگهو)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	أرجع 0;
}

ساكن صحيح reserve_trace(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_بفر_سائيز;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_بفر_سائيز);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (بنانشان ڊگهو)CONFIG_TRACE_بفر_سائيز >> 10, gd->relocaddr);
#جيڪڏهن_پورو

	أرجع 0;
}

ساكن صحيح reserve_uboot(فراغ)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	إذا (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	إذا (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_ڪوڊ)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#إذا defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#جيڪڏهن_پورو

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->شروعات_addr_sp = gd->relocaddr;

	أرجع 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ساكن بنانشان ڊگهو reserve_ڍير_aligned(size_t سائيز)
{
	أرجع ALIGN_هيٺ_ڏانهن(gd->شروعات_addr_sp - سائيز, 16);
}

#جيڪڏهن_وصف_ٿيل CONFIG_SYS_NONCACHED_يادگيري
ساكن صحيح reserve_noncached(فراغ)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->شروعات_addr_sp = ALIGN(gd->شروعات_addr_sp, MMU_SECTION_سائيز) -
		MMU_SECTION_سائيز;
	gd->شروعات_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_يادگيري,
				   MMU_SECTION_سائيز);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_يادگيري >> 20, gd->شروعات_addr_sp);

	أرجع 0;
}
#جيڪڏهن_پورو

/* reserve memory for malloc() area */
ساكن صحيح reserve_malloc(فراغ)
{
	gd->شروعات_addr_sp = reserve_ڍير_aligned(ڪل_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      ڪل_MALLOC_LEN >> 10, gd->شروعات_addr_sp);
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_NONCACHED_يادگيري
	reserve_noncached();
#جيڪڏهن_پورو

	أرجع 0;
}

/* (permanently) allocate a Board Info struct */
ساكن صحيح reserve_board(فراغ)
{
	إذا (!gd->bd) {
		gd->شروعات_addr_sp = reserve_ڍير_aligned(حجم(بنية bd_info));
		gd->bd = (بنية bd_info *)map_sysmem(gd->شروعات_addr_sp,
						      حجم(بنية bd_info));
		memset(gd->bd, '\0', حجم(بنية bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      حجم(بنية bd_info), gd->شروعات_addr_sp);
	}
	أرجع 0;
}

ساكن صحيح reserve_global_معلومات(فراغ)
{
	gd->شروعات_addr_sp = reserve_ڍير_aligned(حجم(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->شروعات_addr_sp, حجم(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      حجم(gd_t), gd->شروعات_addr_sp);
	أرجع 0;
}

ساكن صحيح reserve_fdt(فراغ)
{
	إذا (!IS_ENABLED(CONFIG_جو_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		إذا (gd->fdt_blob) {
			gd->boardf->fdt_سائيز =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->شروعات_addr_sp = reserve_ڍير_aligned(
				gd->boardf->fdt_سائيز);
			gd->boardf->new_fdt = map_sysmem(gd->شروعات_addr_sp,
							 gd->boardf->fdt_سائيز);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_سائيز, gd->شروعات_addr_sp);
		}
	}

	أرجع 0;
}

ساكن صحيح reserve_bootstage(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_BOOTSTAGE
	صحيح سائيز = bootstage_get_سائيز(true);

	gd->شروعات_addr_sp = reserve_ڍير_aligned(سائيز);
	gd->boardf->new_bootstage = map_sysmem(gd->شروعات_addr_sp, سائيز);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", سائيز,
	      gd->شروعات_addr_sp);
#جيڪڏهن_پورو

	أرجع 0;
}

__weak صحيح arch_reserve_stacks(فراغ)
{
	أرجع 0;
}

ساكن صحيح reserve_stacks(فراغ)
{
	/* make stack pointer 16-byte aligned */
	gd->شروعات_addr_sp = reserve_ڍير_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	أرجع arch_reserve_stacks();
}

ساكن صحيح reserve_bloblist(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_BLOBLIST
	ulong سائيز = bloblist_get_ڪل_سائيز();

	إذا (سائيز < CONFIG_BLOBLIST_سائيز_RELOC)
		سائيز = CONFIG_BLOBLIST_سائيز_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->شروعات_addr_sp = ALIGN_هيٺ_ڏانهن(gd->شروعات_addr_sp - سائيز, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->شروعات_addr_sp, سائيز);
#جيڪڏهن_پورو

	أرجع 0;
}

ساكن صحيح display_new_sp(فراغ)
{
	debug("New Stack Pointer is: %08lx\n", gd->شروعات_addr_sp);

	أرجع 0;
}

__weak صحيح arch_تياري_bdinfo(فراغ)
{
	أرجع 0;
}

صحيح تياري_bdinfo(فراغ)
{
	أرجع arch_تياري_bdinfo();
}

#جيڪڏهن_وصف_ٿيل CONFIG_POST
ساكن صحيح init_post(فراغ)
{
	post_bootmode_init();
	post_لاڳيتو(عدم, POST_ROM | post_bootmode_get(0));

	أرجع 0;
}
#جيڪڏهن_پورو

ساكن صحيح reloc_fdt(فراغ)
{
	إذا (!IS_ENABLED(CONFIG_جو_EMBED)) {
		إذا (gd->boardf->new_fdt) {
			يادگيري_نقل(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	أرجع 0;
}

ساكن صحيح reloc_bootstage(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_BOOTSTAGE
	إذا (gd->flags & GD_FLG_SKIP_RELOC)
		أرجع 0;
	إذا (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#جيڪڏهن_پورو

	أرجع 0;
}

ساكن صحيح reloc_bloblist(فراغ)
{
#جيڪڏهن_وصف_ٿيل CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	إذا ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_سائيز == CONFIG_BLOBLIST_سائيز_RELOC) {
		debug("Not relocating bloblist\n");
		أرجع 0;
	}
	إذا (gd->boardf->new_bloblist) {
		ulong سائيز = bloblist_get_ڪل_سائيز();

		إذا (سائيز < CONFIG_BLOBLIST_سائيز_RELOC)
			سائيز = CONFIG_BLOBLIST_سائيز_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, سائيز);
		أرجع bloblist_reloc(gd->boardf->new_bloblist, سائيز);
	}
#جيڪڏهن_پورو

	أرجع 0;
}

فراغ mcheck_on_ramrelocation(size_t وٿي);
ساكن صحيح تياري_reloc(فراغ)
{
	إذا (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#جيڪڏهن_وصف_ٿيل CONFIG_متن_بنياد
#جيڪڏهن_وصف_ٿيل ARM
		gd->reloc_off = gd->relocaddr - (بنانشان ڊگهو)__image_copy_start;
#وإلاإذا defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)شروعات_2;
#وإلاإذا defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_متن_بنياد + 0x400);
#وإلاإذا !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_متن_بنياد;
#جيڪڏهن_پورو
#جيڪڏهن_پورو
	}

	يادگيري_نقل(gd->new_gd, (حرف *)gd, حجم(gd_t));

	إذا (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} وإلا {
#جيڪڏهن_وصف_ٿيل MCHECK_ڍير_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#جيڪڏهن_پورو
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_منزل_sysmem(gd->new_gd),
		      gd->شروعات_addr_sp);
	}

	أرجع 0;
}

#إذا CONFIG_IS_ENABLED(جو_BOARD_FIXUP)
ساكن صحيح fix_fdt(فراغ)
{
	أرجع board_fix_fdt((فراغ *)gd->fdt_blob);
}
#جيڪڏهن_پورو

/* ARM calls relocate_code from its crt0.S */
#إذا !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ساكن صحيح jump_منزل_copy(فراغ)
{
	إذا (gd->flags & GD_FLG_SKIP_RELOC)
		أرجع 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#إذا defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_تياري_gd(gd->new_gd);
# إذا CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->شروعات_addr_sp);
# وإلا
		board_init_f_r_trampoline(gd->شروعات_addr_sp);
# endif
#وإلا
	relocate_ڪوڊ(gd->شروعات_addr_sp, gd->new_gd, gd->relocaddr);
#جيڪڏهن_پورو

	أرجع 0;
}
#جيڪڏهن_پورو

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ساكن صحيح initf_bootstage(فراغ)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	صحيح ret;

	ret = bootstage_init(!from_spl);
	إذا (ret)
		أرجع ret;
	إذا (from_spl) {
		ret = bootstage_unstash_default();
		إذا (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			أرجع ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_شروعات_UBOOT_F, "board_init_f");

	أرجع 0;
}

ساكن صحيح initf_dm(فراغ)
{
	صحيح ret;

	إذا (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		أرجع 0;

	bootstage_شروعات(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	إذا (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_جاچ(true);
	وإلا
		ret = dm_init(false);
	إذا (ret)
		أرجع ret;

	ret = dm_autoprobe();
	إذا (ret)
		أرجع ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	إذا (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		إذا (ret)
			أرجع ret;
	}

	أرجع 0;
}

/* Architecture-specific memory reservation */
__weak صحيح reserve_arch(فراغ)
{
	أرجع 0;
}

__weak صحيح checkcpu(فراغ)
{
	أرجع 0;
}

__weak صحيح clear_bss(فراغ)
{
	أرجع 0;
}

ساكن صحيح initf_upl(فراغ)
{
	بنية upl *upl;
	صحيح ret;

	إذا (!IS_ENABLED(CONFIG_UPL_اندريون) || !(gd->flags & GD_FLG_UPL))
		أرجع 0;

	upl = يادگيري_ڏيو(حجم(بنية upl));
	إذا (!upl)
		أرجع -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	إذا (ret) {
		لکو("UPL handoff: read failure (err=%dE)\n", ret);
		أرجع ret;
	}
	gd_سيٽ_upl(upl);

	أرجع 0;
}

ساكن فراغ initcall_لاڳيتو_f(فراغ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(تياري_mon_len);
	INITCALL(initf_malloc);
#إذا CONFIG_IS_ENABLED(جو_CONTROL)
	INITCALL(fdtdec_تياري);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#جيڪڏهن_پورو
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#إذا CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#جيڪڏهن_پورو
	INITCALL(تياري_spl_handoff);
#إذا CONFIG_IS_ENABLED(CONSOLE_رڪارڊ_INIT_F)
	INITCALL(console_رڪارڊ_init);
#جيڪڏهن_پورو
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#إذا CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#جيڪڏهن_پورو
#إذا defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#جيڪڏهن_پورو
#إذا !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#جيڪڏهن_پورو
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_شرح);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_متن_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#إذا CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#جيڪڏهن_پورو
	/* display cpu info (and speed) */
#إذا CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#جيڪڏهن_پورو
#إذا CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#جيڪڏهن_پورو
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#جيڪڏهن_پورو
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#إذا CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#جيڪڏهن_پورو
	WATCHDOG_RESET();
#إذا defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#جيڪڏهن_پورو /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#إذا CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#جيڪڏهن_پورو
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
	INITCALL(تياري_ram_بنياد);
	INITCALL(dram_init_banksize);
	INITCALL(تياري_ram_config);
	INITCALL(تياري_dest_addr);
#إذا CONFIG_IS_ENABLED(جو_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(جو_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_ٿيل CFG_PRAM
	INITCALL(reserve_pram);
#جيڪڏهن_پورو
	INITCALL(reserve_ڦيرو_4k);
	INITCALL(تياري_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_معلومات);
	INITCALL(reserve_fdt);
#إذا CONFIG_IS_ENABLED(جو_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(جو_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#جيڪڏهن_پورو
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(تياري_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#إذا !CONFIG_IS_ENABLED(جو_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#جيڪڏهن_پورو
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(تياري_reloc);
#إذا CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_منزل_ram);
	INITCALL(do_elf_reloc_fixups);
#جيڪڏهن_پورو
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
#إذا !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_منزل_copy);
#جيڪڏهن_پورو
}

فراغ board_init_f(ulong boot_flags)
{
	بنية board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_لاڳيتو_f();

#إذا !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#جيڪڏهن_پورو
}

#إذا defined(CONFIG_X86) || defined(CONFIG_ARC)
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
ساكن فراغ initcall_لاڳيتو_f_r(فراغ)
{
#إذا !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#جيڪڏهن_پورو
}

فراغ board_init_f_r(فراغ)
{
	initcall_لاڳيتو_f_r();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#جيڪڏهن_وصف_ٿيل CONFIG_TIMER
	gd->timer = عدم;
#جيڪڏهن_پورو

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#جيڪڏهن_پورو /* CONFIG_X86 */
