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

DECLARE_GLOBAL_ڈیٹا_PTR;

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
ساکن صحیح init_func_watchdog_init(خالی)
{
# اگر defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	جدول_وقت();

	واپس 0;
}

صحیح init_func_watchdog_reset(خالی)
{
	جدول_وقت();

	واپس 0;
}
#اگر_ختم /* CONFIG_WATCHDOG */

__weak خالی board_جوڑو_ram_info(صحیح use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ساکن صحیح init_baud_شرح(خالی)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	واپس 0;
}

ساکن صحیح display_متن_info(خالی)
{
#اگر !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_آغاز, bss_انتہا, متن_بنیاد;

	bss_آغاز = (ulong)__bss_start;
	bss_انتہا = (ulong)__bss_end;

#اگر_تعریف_شدہ CONFIG_متن_بنیاد
	متن_بنیاد = CONFIG_متن_بنیاد;
#ورنہ
	متن_بنیاد = CONFIG_SYS_MONITOR_بنیاد;
#اگر_ختم

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      متن_بنیاد, bss_آغاز, bss_انتہا);
#اگر_ختم

	واپس 0;
}

#اگر_تعریف_شدہ CONFIG_SYSRESET_PRINT_RESETINFO
ساکن صحیح print_resetinfo(خالی)
{
	ساخت udevice *dev;
	حرف حالت_2[256];
	bool حالت_printed = false;
	صحیح ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	برائے (ret = uclass_پہلا_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_اگلا_device_check(&dev)) {
		اگر (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			جاری;
		}

		اگر (!sysreset_get_حالت(dev, حالت_2, حجم(حالت_2))) {
			لکھو("%s%s", حالت_printed ? " " : "", حالت_2);
			حالت_printed = true;
		}
	}
	اگر (حالت_printed)
		لکھو("\n");

	واپس 0;
}
#اگر_ختم

#اگر defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ساکن صحیح print_cpuinfo(خالی)
{
	ساخت udevice *dev;
	حرف desc[512];
	صحیح ret;

	dev = cpu_get_current_dev();
	اگر (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		واپس -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, حجم(desc));
	اگر (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		واپس ret;
	}

	لکھو("CPU:   %s\n", desc);

	واپس 0;
}
#اگر_ختم

ساکن صحیح announce_dram_init(خالی)
{
	puts("DRAM:  ");
	واپس 0;
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
#تعریف_کرو compute_سائز_scale_val(سائز, scale, val) کرو { \
	scale = ilog2(سائز) / 10 * 10; \
	val = (10 * سائز + ((1ULL << scale) >> 1)) >> scale; \
	اگر (val == 10240) { val = 10; scale += 10; } \
} جب (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ساکن صحیح sizes_near(غیرعلامتی طویل طویل size1, غیرعلامتی طویل طویل size2)
{
	غیرعلامتی صحیح size1_scale, size1_val, size2_scale, size2_val;

	compute_سائز_scale_val(size1, size1_scale, size1_val);
	compute_سائز_scale_val(size2, size2_scale, size2_val);

	واپس size1_scale == size2_scale && size1_val == size2_val;
}

ساکن صحیح show_dram_config(خالی)
{
	غیرعلامتی طویل طویل سائز;
	صحیح i;

	debug("\nRAM Configuration:\n");
	برائے (i = سائز = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		سائز += gd->dram[i].سائز;
		debug("Bank #%d: %llx ", i,
		      (غیرعلامتی طویل طویل)(gd->dram[i].آغاز));
#اگر_تعریف_شدہ DEBUG
		print_سائز(gd->dram[i].سائز, "\n");
#اگر_ختم
	}
	debug("\nDRAM:  ");

	print_سائز(gd->ram_سائز, "");
	اگر (!sizes_near(gd->ram_سائز, سائز)) {
		لکھو(" (total ");
		print_سائز(سائز, ")");
	}
	board_جوڑو_ram_info(0);
	putc('\n');

	واپس 0;
}

__weak صحیح dram_init_banksize(خالی)
{
	gd->dram[0].آغاز = gd->ram_بنیاد;
	gd->dram[0].سائز = get_effective_memsize();

	واپس 0;
}

#اگر CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ساکن صحیح init_func_i2c(خالی)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	واپس 0;
}
#اگر_ختم

ساکن صحیح تیاری_mon_len(خالی)
{
#اگر defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#وگرنہ_اگر defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)آغاز_2;
#وگرنہ_اگر defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)انتہا - (ulong)_init;
#وگرنہ_اگر defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#وگرنہ_اگر defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)انتہا - (ulong)_init;
#وگرنہ_اگر defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#وگرنہ_اگر defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(آغاز_2);
#وگرنہ_اگر defined(CONFIG_SYS_MONITOR_بنیاد)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_بنیاد;
#اگر_ختم
	واپس 0;
}

ساکن صحیح تیاری_spl_handoff(خالی)
{
#اگر CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_ڈھونڈو(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					حجم(ساخت spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#اگر_ختم

	واپس 0;
}

__weak صحیح arch_cpu_init(خالی)
{
	واپس 0;
}

__weak صحیح mach_cpu_init(خالی)
{
	واپس 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_چوٹی(phys_سائز_t کل_سائز)
{
	اگر (CONFIG_IS_ENABLED(RELOC_ADDR_چوٹی))
		واپس gd->ram_چوٹی;

#اگر defined(CFG_SYS_SDRAM_بنیاد) && CFG_SYS_SDRAM_بنیاد > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	اگر (gd->ram_چوٹی < CFG_SYS_SDRAM_بنیاد)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		واپس 0;
#اگر_ختم
	واپس gd->ram_چوٹی;
}

__weak صحیح arch_تیاری_dest_addr(خالی)
{
	واپس 0;
}

ساکن صحیح تیاری_ram_بنیاد(خالی)
{
#اگر_تعریف_شدہ CFG_SYS_SDRAM_بنیاد
	gd->ram_بنیاد = CFG_SYS_SDRAM_بنیاد;
#اگر_ختم
	واپس 0;
}

ساکن صحیح تیاری_ram_config(خالی)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	اگر (CONFIG_IS_ENABLED(RELOC_ADDR_چوٹی)) {
		صحیح i;
		phys_addr_t چوٹی;

		gd->ram_سائز = 0;
		برائے (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			چوٹی = get_mem_چوٹی(gd->dram[i].آغاز, gd->dram[i].سائز,
					  ALIGN(gd->mon_len, SZ_1M),
					  (خالی *)gd->fdt_blob);
			gd->ram_چوٹی = max(چوٹی, gd->ram_چوٹی);
			gd->ram_سائز += gd->dram[i].سائز;
		}
	} ورنہ {
		gd->ram_چوٹی = gd->ram_بنیاد + get_effective_memsize();
	}
	gd->ram_چوٹی = board_get_usable_ram_چوٹی(gd->mon_len);
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
#اگر CONFIG_VAL(SYS_MEM_چوٹی_HIDE)
	gd->ram_چوٹی -= CONFIG_SYS_MEM_چوٹی_HIDE;
	gd->ram_سائز -= CONFIG_SYS_MEM_چوٹی_HIDE;
#اگر_ختم

	debug("Ram top: %08llx\n", (غیرعلامتی طویل طویل)gd->ram_چوٹی);
	debug("Ram size: %08llx\n", (غیرعلامتی طویل طویل)gd->ram_سائز);

	واپس 0;
}

ساکن صحیح تیاری_dest_addr(خالی)
{
	صحیح ret;

	gd->relocaddr = gd->ram_چوٹی;
	debug("Reloc addr: %08llX\n", (غیرعلامتی طویل طویل)gd->relocaddr);

	ret = arch_تیاری_dest_addr();
	اگر (ret)
		واپس ret;

	gd->initial_relocaddr = gd->relocaddr;
	واپس 0;
}

#اگر_تعریف_شدہ CFG_PRAM
/* reserve protected RAM */
ساکن صحیح reserve_pram(خالی)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	واپس 0;
}
#اگر_ختم /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ساکن صحیح reserve_باری_4k(خالی)
{
	gd->relocaddr &= ~(4096 - 1);
	واپس 0;
}

__weak صحیح arch_reserve_mmu(خالی)
{
	واپس 0;
}

ساکن صحیح reserve_video_from_videoblob(خالی)
{
	اگر (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ساخت video_handoff *ho;
		صحیح ret = 0;

		ho = bloblist_ڈھونڈو(BLOBLISTT_U_BOOT_VIDEO, حجم(*ho));
		اگر (!ho)
			واپس log_پیغام_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		اگر (ret)
			واپس log_پیغام_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		اگر (likely(gd->relocaddr > (غیرعلامتی طویل)ho->fb))
			gd->relocaddr = ho->fb;
	}

	واپس 0;
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
ساکن صحیح تیاری_relocaddr_from_bloblist(خالی)
{
	reserve_video_from_videoblob();

	واپس 0;
}

ساکن صحیح reserve_video(خالی)
{
	اگر (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحیح ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		اگر (ret)
			واپس ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((غیرعلامتی طویل)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	واپس 0;
}

ساکن صحیح reserve_trace(خالی)
{
#اگر_تعریف_شدہ CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_بفر_سائز;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_بفر_سائز);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (غیرعلامتی طویل)CONFIG_TRACE_بفر_سائز >> 10, gd->relocaddr);
#اگر_ختم

	واپس 0;
}

ساکن صحیح reserve_uboot(خالی)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	اگر (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	اگر (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_رمز)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#اگر defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#اگر_ختم

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->آغاز_addr_sp = gd->relocaddr;

	واپس 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ساکن غیرعلامتی طویل reserve_ڈھیر_aligned(size_t سائز)
{
	واپس ALIGN_نیچے_کو(gd->آغاز_addr_sp - سائز, 16);
}

#اگر_تعریف_شدہ CONFIG_SYS_NONCACHED_حافظہ
ساکن صحیح reserve_noncached(خالی)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->آغاز_addr_sp = ALIGN(gd->آغاز_addr_sp, MMU_SECTION_سائز) -
		MMU_SECTION_سائز;
	gd->آغاز_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_حافظہ,
				   MMU_SECTION_سائز);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_حافظہ >> 20, gd->آغاز_addr_sp);

	واپس 0;
}
#اگر_ختم

/* reserve memory for malloc() area */
ساکن صحیح reserve_malloc(خالی)
{
	gd->آغاز_addr_sp = reserve_ڈھیر_aligned(کل_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      کل_MALLOC_LEN >> 10, gd->آغاز_addr_sp);
#اگر_تعریف_شدہ CONFIG_SYS_NONCACHED_حافظہ
	reserve_noncached();
#اگر_ختم

	واپس 0;
}

/* (permanently) allocate a Board Info struct */
ساکن صحیح reserve_board(خالی)
{
	اگر (!gd->bd) {
		gd->آغاز_addr_sp = reserve_ڈھیر_aligned(حجم(ساخت bd_info));
		gd->bd = (ساخت bd_info *)map_sysmem(gd->آغاز_addr_sp,
						      حجم(ساخت bd_info));
		memset(gd->bd, '\0', حجم(ساخت bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      حجم(ساخت bd_info), gd->آغاز_addr_sp);
	}
	واپس 0;
}

ساکن صحیح reserve_global_ڈیٹا(خالی)
{
	gd->آغاز_addr_sp = reserve_ڈھیر_aligned(حجم(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->آغاز_addr_sp, حجم(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      حجم(gd_t), gd->آغاز_addr_sp);
	واپس 0;
}

ساکن صحیح reserve_fdt(خالی)
{
	اگر (!IS_ENABLED(CONFIG_کا_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		اگر (gd->fdt_blob) {
			gd->boardf->fdt_سائز =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->آغاز_addr_sp = reserve_ڈھیر_aligned(
				gd->boardf->fdt_سائز);
			gd->boardf->new_fdt = map_sysmem(gd->آغاز_addr_sp,
							 gd->boardf->fdt_سائز);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_سائز, gd->آغاز_addr_sp);
		}
	}

	واپس 0;
}

ساکن صحیح reserve_bootstage(خالی)
{
#اگر_تعریف_شدہ CONFIG_BOOTSTAGE
	صحیح سائز = bootstage_get_سائز(true);

	gd->آغاز_addr_sp = reserve_ڈھیر_aligned(سائز);
	gd->boardf->new_bootstage = map_sysmem(gd->آغاز_addr_sp, سائز);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", سائز,
	      gd->آغاز_addr_sp);
#اگر_ختم

	واپس 0;
}

__weak صحیح arch_reserve_stacks(خالی)
{
	واپس 0;
}

ساکن صحیح reserve_stacks(خالی)
{
	/* make stack pointer 16-byte aligned */
	gd->آغاز_addr_sp = reserve_ڈھیر_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	واپس arch_reserve_stacks();
}

ساکن صحیح reserve_bloblist(خالی)
{
#اگر_تعریف_شدہ CONFIG_BLOBLIST
	ulong سائز = bloblist_get_کل_سائز();

	اگر (سائز < CONFIG_BLOBLIST_سائز_RELOC)
		سائز = CONFIG_BLOBLIST_سائز_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->آغاز_addr_sp = ALIGN_نیچے_کو(gd->آغاز_addr_sp - سائز, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->آغاز_addr_sp, سائز);
#اگر_ختم

	واپس 0;
}

ساکن صحیح display_new_sp(خالی)
{
	debug("New Stack Pointer is: %08lx\n", gd->آغاز_addr_sp);

	واپس 0;
}

__weak صحیح arch_تیاری_bdinfo(خالی)
{
	واپس 0;
}

صحیح تیاری_bdinfo(خالی)
{
	واپس arch_تیاری_bdinfo();
}

#اگر_تعریف_شدہ CONFIG_POST
ساکن صحیح init_post(خالی)
{
	post_bootmode_init();
	post_تسلسل(تہی, POST_ROM | post_bootmode_get(0));

	واپس 0;
}
#اگر_ختم

ساکن صحیح reloc_fdt(خالی)
{
	اگر (!IS_ENABLED(CONFIG_کا_EMBED)) {
		اگر (gd->boardf->new_fdt) {
			حافظہ_نقل(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	واپس 0;
}

ساکن صحیح reloc_bootstage(خالی)
{
#اگر_تعریف_شدہ CONFIG_BOOTSTAGE
	اگر (gd->flags & GD_FLG_SKIP_RELOC)
		واپس 0;
	اگر (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#اگر_ختم

	واپس 0;
}

ساکن صحیح reloc_bloblist(خالی)
{
#اگر_تعریف_شدہ CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	اگر ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_سائز == CONFIG_BLOBLIST_سائز_RELOC) {
		debug("Not relocating bloblist\n");
		واپس 0;
	}
	اگر (gd->boardf->new_bloblist) {
		ulong سائز = bloblist_get_کل_سائز();

		اگر (سائز < CONFIG_BLOBLIST_سائز_RELOC)
			سائز = CONFIG_BLOBLIST_سائز_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, سائز);
		واپس bloblist_reloc(gd->boardf->new_bloblist, سائز);
	}
#اگر_ختم

	واپس 0;
}

خالی mcheck_on_ramrelocation(size_t انحراف);
ساکن صحیح تیاری_reloc(خالی)
{
	اگر (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#اگر_تعریف_شدہ CONFIG_متن_بنیاد
#اگر_تعریف_شدہ ARM
		gd->reloc_off = gd->relocaddr - (غیرعلامتی طویل)__image_copy_start;
#وگرنہ_اگر defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)آغاز_2;
#وگرنہ_اگر defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_متن_بنیاد + 0x400);
#وگرنہ_اگر !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_متن_بنیاد;
#اگر_ختم
#اگر_ختم
	}

	حافظہ_نقل(gd->new_gd, (حرف *)gd, حجم(gd_t));

	اگر (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} ورنہ {
#اگر_تعریف_شدہ MCHECK_ڈھیر_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#اگر_ختم
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_مقصد_sysmem(gd->new_gd),
		      gd->آغاز_addr_sp);
	}

	واپس 0;
}

#اگر CONFIG_IS_ENABLED(کا_BOARD_FIXUP)
ساکن صحیح fix_fdt(خالی)
{
	واپس board_fix_fdt((خالی *)gd->fdt_blob);
}
#اگر_ختم

/* ARM calls relocate_code from its crt0.S */
#اگر !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ساکن صحیح jump_مقصد_copy(خالی)
{
	اگر (gd->flags & GD_FLG_SKIP_RELOC)
		واپس 0;
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
	arch_تیاری_gd(gd->new_gd);
# اگر CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->آغاز_addr_sp);
# ورنہ
		board_init_f_r_trampoline(gd->آغاز_addr_sp);
# endif
#ورنہ
	relocate_رمز(gd->آغاز_addr_sp, gd->new_gd, gd->relocaddr);
#اگر_ختم

	واپس 0;
}
#اگر_ختم

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ساکن صحیح initf_bootstage(خالی)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	صحیح ret;

	ret = bootstage_init(!from_spl);
	اگر (ret)
		واپس ret;
	اگر (from_spl) {
		ret = bootstage_unstash_default();
		اگر (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			واپس ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_آغاز_UBOOT_F, "board_init_f");

	واپس 0;
}

ساکن صحیح initf_dm(خالی)
{
	صحیح ret;

	اگر (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		واپس 0;

	bootstage_آغاز(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	اگر (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_جائزہ(true);
	ورنہ
		ret = dm_init(false);
	اگر (ret)
		واپس ret;

	ret = dm_autoprobe();
	اگر (ret)
		واپس ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	اگر (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		اگر (ret)
			واپس ret;
	}

	واپس 0;
}

/* Architecture-specific memory reservation */
__weak صحیح reserve_arch(خالی)
{
	واپس 0;
}

__weak صحیح checkcpu(خالی)
{
	واپس 0;
}

__weak صحیح clear_bss(خالی)
{
	واپس 0;
}

ساکن صحیح initf_upl(خالی)
{
	ساخت upl *upl;
	صحیح ret;

	اگر (!IS_ENABLED(CONFIG_UPL_اندر) || !(gd->flags & GD_FLG_UPL))
		واپس 0;

	upl = حافظہ_دو(حجم(ساخت upl));
	اگر (!upl)
		واپس -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	اگر (ret) {
		لکھو("UPL handoff: read failure (err=%dE)\n", ret);
		واپس ret;
	}
	gd_سیٹ_upl(upl);

	واپس 0;
}

ساکن خالی initcall_تسلسل_f(خالی)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(تیاری_mon_len);
	INITCALL(initf_malloc);
#اگر CONFIG_IS_ENABLED(کا_CONTROL)
	INITCALL(fdtdec_تیاری);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#اگر_ختم
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#اگر CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#اگر_ختم
	INITCALL(تیاری_spl_handoff);
#اگر CONFIG_IS_ENABLED(CONSOLE_ریکارڈ_INIT_F)
	INITCALL(console_ریکارڈ_init);
#اگر_ختم
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#اگر CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#اگر_ختم
#اگر defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#اگر_ختم
#اگر !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#اگر_ختم
#اگر CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#اگر_ختم
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_شرح);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_متن_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#اگر CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#اگر_ختم
	/* display cpu info (and speed) */
#اگر CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#اگر_ختم
#اگر CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#اگر_ختم
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#اگر_ختم
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#اگر_ختم
	WATCHDOG_RESET();
#اگر defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#اگر_ختم /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#اگر CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#اگر_ختم
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
	INITCALL(تیاری_ram_بنیاد);
	INITCALL(dram_init_banksize);
	INITCALL(تیاری_ram_config);
	INITCALL(تیاری_dest_addr);
#اگر CONFIG_IS_ENABLED(کا_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(کا_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#اگر_ختم
#اگر_تعریف_شدہ CFG_PRAM
	INITCALL(reserve_pram);
#اگر_ختم
	INITCALL(reserve_باری_4k);
	INITCALL(تیاری_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_ڈیٹا);
	INITCALL(reserve_fdt);
#اگر CONFIG_IS_ENABLED(کا_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(کا_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#اگر_ختم
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(تیاری_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#اگر !CONFIG_IS_ENABLED(کا_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#اگر_ختم
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(تیاری_reloc);
#اگر CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_مقصد_ram);
	INITCALL(do_elf_reloc_fixups);
#اگر_ختم
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
	INITCALL(jump_مقصد_copy);
#اگر_ختم
}

خالی board_init_f(ulong boot_flags)
{
	ساخت board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_تسلسل_f();

#اگر !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#اگر_ختم
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
ساکن خالی initcall_تسلسل_f_r(خالی)
{
#اگر !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#اگر_ختم
}

خالی board_init_f_r(خالی)
{
	initcall_تسلسل_f_r();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#اگر_تعریف_شدہ CONFIG_TIMER
	gd->timer = تہی;
#اگر_ختم

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#اگر_ختم /* CONFIG_X86 */
