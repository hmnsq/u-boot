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

#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bloblist.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootstage.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <clock_legacy.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cyclic.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <display_options.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env_internal.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <event.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fdtdec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fs.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <i2c.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <initcall.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memtop.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <os.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <post.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <relocate.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <serial.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sysreset.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <timer.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <trace.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <upl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <video.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/root.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/log2.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/sizes.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

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

#ਜੇ defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
ਸਥਿਰ ਪੂਰਨ init_func_watchdog_init(ਖਾਲੀ)
{
# ਜੇ defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	ਸਮਾਂ_ਸਾਰਣੀ();

	ਵਾਪਸ 0;
}

ਪੂਰਨ init_func_watchdog_reset(ਖਾਲੀ)
{
	ਸਮਾਂ_ਸਾਰਣੀ();

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CONFIG_WATCHDOG */

__weak ਖਾਲੀ board_ਜੋੜੋ_ram_info(ਪੂਰਨ use_default)
{
	/* please define platform specific board_add_ram_info() */
}

ਸਥਿਰ ਪੂਰਨ init_baud_ਦਰ(ਖਾਲੀ)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ display_ਪਾਠ_info(ਖਾਲੀ)
{
#ਜੇ !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_ਸ਼ੁਰੂ, bss_ਸਿਰਾ, ਪਾਠ_ਅਧਾਰ;

	bss_ਸ਼ੁਰੂ = (ulong)__bss_start;
	bss_ਸਿਰਾ = (ulong)__bss_end;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਪਾਠ_ਅਧਾਰ
	ਪਾਠ_ਅਧਾਰ = CONFIG_ਪਾਠ_ਅਧਾਰ;
#ਨਹੀਂਤਾਂ
	ਪਾਠ_ਅਧਾਰ = CONFIG_SYS_MONITOR_ਅਧਾਰ;
#ਜੇ_ਅੰਤ

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      ਪਾਠ_ਅਧਾਰ, bss_ਸ਼ੁਰੂ, bss_ਸਿਰਾ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYSRESET_PRINT_RESETINFO
ਸਥਿਰ ਪੂਰਨ print_resetinfo(ਖਾਲੀ)
{
	ਬਣਤਰ udevice *dev;
	ਅੱਖਰ ਸਥਿਤੀ[256];
	bool ਸਥਿਤੀ_printed = false;
	ਪੂਰਨ ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	ਲਈ (ret = uclass_ਪਹਿਲਾ_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_ਅਗਲਾ_device_check(&dev)) {
		ਜੇ (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			ਜਾਰੀ;
		}

		ਜੇ (!sysreset_get_ਸਥਿਤੀ(dev, ਸਥਿਤੀ, ਆਕਾਰ(ਸਥਿਤੀ))) {
			ਲਿਖੋ("%s%s", ਸਥਿਤੀ_printed ? " " : "", ਸਥਿਤੀ);
			ਸਥਿਤੀ_printed = true;
		}
	}
	ਜੇ (ਸਥਿਤੀ_printed)
		ਲਿਖੋ("\n");

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

#ਜੇ defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
ਸਥਿਰ ਪੂਰਨ print_cpuinfo(ਖਾਲੀ)
{
	ਬਣਤਰ udevice *dev;
	ਅੱਖਰ desc[512];
	ਪੂਰਨ ret;

	dev = cpu_get_current_dev();
	ਜੇ (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		ਵਾਪਸ -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, ਆਕਾਰ(desc));
	ਜੇ (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		ਵਾਪਸ ret;
	}

	ਲਿਖੋ("CPU:   %s\n", desc);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ announce_dram_init(ਖਾਲੀ)
{
	puts("DRAM:  ");
	ਵਾਪਸ 0;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ compute_ਮਾਪ_scale_val(ਮਾਪ, scale, val) ਕਰੋ { \
	scale = ilog2(ਮਾਪ) / 10 * 10; \
	val = (10 * ਮਾਪ + ((1ULL << scale) >> 1)) >> scale; \
	ਜੇ (val == 10240) { val = 10; scale += 10; } \
} ਜਦੋਂ (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
ਸਥਿਰ ਪੂਰਨ sizes_near(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ size1, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ size2)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ size1_scale, size1_val, size2_scale, size2_val;

	compute_ਮਾਪ_scale_val(size1, size1_scale, size1_val);
	compute_ਮਾਪ_scale_val(size2, size2_scale, size2_val);

	ਵਾਪਸ size1_scale == size2_scale && size1_val == size2_val;
}

ਸਥਿਰ ਪੂਰਨ show_dram_config(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਮਾਪ;
	ਪੂਰਨ i;

	debug("\nRAM Configuration:\n");
	ਲਈ (i = ਮਾਪ = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		ਮਾਪ += gd->dram[i].ਮਾਪ;
		debug("Bank #%d: %llx ", i,
		      (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)(gd->dram[i].ਸ਼ੁਰੂ));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
		print_ਮਾਪ(gd->dram[i].ਮਾਪ, "\n");
#ਜੇ_ਅੰਤ
	}
	debug("\nDRAM:  ");

	print_ਮਾਪ(gd->ram_ਮਾਪ, "");
	ਜੇ (!sizes_near(gd->ram_ਮਾਪ, ਮਾਪ)) {
		ਲਿਖੋ(" (total ");
		print_ਮਾਪ(ਮਾਪ, ")");
	}
	board_ਜੋੜੋ_ram_info(0);
	putc('\n');

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ dram_init_banksize(ਖਾਲੀ)
{
	gd->dram[0].ਸ਼ੁਰੂ = gd->ram_ਅਧਾਰ;
	gd->dram[0].ਮਾਪ = get_effective_memsize();

	ਵਾਪਸ 0;
}

#ਜੇ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
ਸਥਿਰ ਪੂਰਨ init_func_i2c(ਖਾਲੀ)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_mon_len(ਖਾਲੀ)
{
#ਜੇ defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#ਜਾਂਜੇ defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)ਸ਼ੁਰੂ_2;
#ਜਾਂਜੇ defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)ਸਿਰਾ - (ulong)_init;
#ਜਾਂਜੇ defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#ਜਾਂਜੇ defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)ਸਿਰਾ - (ulong)_init;
#ਜਾਂਜੇ defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#ਜਾਂਜੇ defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(ਸ਼ੁਰੂ_2);
#ਜਾਂਜੇ defined(CONFIG_SYS_MONITOR_ਅਧਾਰ)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_ਅਧਾਰ;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_spl_handoff(ਖਾਲੀ)
{
#ਜੇ CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_ਲੱਭੋ(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					ਆਕਾਰ(ਬਣਤਰ spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ arch_cpu_init(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ mach_cpu_init(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_ਸਿਖਰ(phys_ਮਾਪ_t ਕੁੱਲ_ਮਾਪ)
{
	ਜੇ (CONFIG_IS_ENABLED(RELOC_ADDR_ਸਿਖਰ))
		ਵਾਪਸ gd->ram_ਸਿਖਰ;

#ਜੇ defined(CFG_SYS_SDRAM_ਅਧਾਰ) && CFG_SYS_SDRAM_ਅਧਾਰ > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	ਜੇ (gd->ram_ਸਿਖਰ < CFG_SYS_SDRAM_ਅਧਾਰ)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ gd->ram_ਸਿਖਰ;
}

__weak ਪੂਰਨ arch_ਤਿਆਰੀ_dest_addr(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_ram_ਅਧਾਰ(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CFG_SYS_SDRAM_ਅਧਾਰ
	gd->ram_ਅਧਾਰ = CFG_SYS_SDRAM_ਅਧਾਰ;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_ram_config(ਖਾਲੀ)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	ਜੇ (CONFIG_IS_ENABLED(RELOC_ADDR_ਸਿਖਰ)) {
		ਪੂਰਨ i;
		phys_addr_t ਸਿਖਰ;

		gd->ram_ਮਾਪ = 0;
		ਲਈ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			ਸਿਖਰ = get_mem_ਸਿਖਰ(gd->dram[i].ਸ਼ੁਰੂ, gd->dram[i].ਮਾਪ,
					  ALIGN(gd->mon_len, SZ_1M),
					  (ਖਾਲੀ *)gd->fdt_blob);
			gd->ram_ਸਿਖਰ = max(ਸਿਖਰ, gd->ram_ਸਿਖਰ);
			gd->ram_ਮਾਪ += gd->dram[i].ਮਾਪ;
		}
	} ਨਹੀਂਤਾਂ {
		gd->ram_ਸਿਖਰ = gd->ram_ਅਧਾਰ + get_effective_memsize();
	}
	gd->ram_ਸਿਖਰ = board_get_usable_ram_ਸਿਖਰ(gd->mon_len);
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
#ਜੇ CONFIG_VAL(SYS_MEM_ਸਿਖਰ_HIDE)
	gd->ram_ਸਿਖਰ -= CONFIG_SYS_MEM_ਸਿਖਰ_HIDE;
	gd->ram_ਮਾਪ -= CONFIG_SYS_MEM_ਸਿਖਰ_HIDE;
#ਜੇ_ਅੰਤ

	debug("Ram top: %08llx\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)gd->ram_ਸਿਖਰ);
	debug("Ram size: %08llx\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)gd->ram_ਮਾਪ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_dest_addr(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	gd->relocaddr = gd->ram_ਸਿਖਰ;
	debug("Reloc addr: %08llX\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)gd->relocaddr);

	ret = arch_ਤਿਆਰੀ_dest_addr();
	ਜੇ (ret)
		ਵਾਪਸ ret;

	gd->initial_relocaddr = gd->relocaddr;
	ਵਾਪਸ 0;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CFG_PRAM
/* reserve protected RAM */
ਸਥਿਰ ਪੂਰਨ reserve_pram(ਖਾਲੀ)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
ਸਥਿਰ ਪੂਰਨ reserve_ਗੇੜਾ_4k(ਖਾਲੀ)
{
	gd->relocaddr &= ~(4096 - 1);
	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ arch_reserve_mmu(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_video_from_videoblob(ਖਾਲੀ)
{
	ਜੇ (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		ਬਣਤਰ video_handoff *ho;
		ਪੂਰਨ ret = 0;

		ho = bloblist_ਲੱਭੋ(BLOBLISTT_U_BOOT_VIDEO, ਆਕਾਰ(*ho));
		ਜੇ (!ho)
			ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		ਜੇ (ret)
			ਵਾਪਸ log_ਸੁਨੇਹਾ_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		ਜੇ (likely(gd->relocaddr > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ho->fb))
			gd->relocaddr = ho->fb;
	}

	ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_relocaddr_from_bloblist(ਖਾਲੀ)
{
	reserve_video_from_videoblob();

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_video(ਖਾਲੀ)
{
	ਜੇ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ਪੂਰਨ ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		ਜੇ (ret)
			ਵਾਪਸ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_trace(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_ਬਫਰ_ਮਾਪ;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_ਬਫਰ_ਮਾਪ);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)CONFIG_TRACE_ਬਫਰ_ਮਾਪ >> 10, gd->relocaddr);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_uboot(ਖਾਲੀ)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	ਜੇ (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	ਜੇ (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_ਕੋਡ)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#ਜੇ defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#ਜੇ_ਅੰਤ

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->ਸ਼ੁਰੂ_addr_sp = gd->relocaddr;

	ਵਾਪਸ 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ reserve_ਢੇਰ_aligned(size_t ਮਾਪ)
{
	ਵਾਪਸ ALIGN_ਥੱਲੇ(gd->ਸ਼ੁਰੂ_addr_sp - ਮਾਪ, 16);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_NONCACHED_ਸਮ੍ਰਿਤੀ
ਸਥਿਰ ਪੂਰਨ reserve_noncached(ਖਾਲੀ)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->ਸ਼ੁਰੂ_addr_sp = ALIGN(gd->ਸ਼ੁਰੂ_addr_sp, MMU_SECTION_ਮਾਪ) -
		MMU_SECTION_ਮਾਪ;
	gd->ਸ਼ੁਰੂ_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_ਸਮ੍ਰਿਤੀ,
				   MMU_SECTION_ਮਾਪ);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_ਸਮ੍ਰਿਤੀ >> 20, gd->ਸ਼ੁਰੂ_addr_sp);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

/* reserve memory for malloc() area */
ਸਥਿਰ ਪੂਰਨ reserve_malloc(ਖਾਲੀ)
{
	gd->ਸ਼ੁਰੂ_addr_sp = reserve_ਢੇਰ_aligned(ਕੁੱਲ_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      ਕੁੱਲ_MALLOC_LEN >> 10, gd->ਸ਼ੁਰੂ_addr_sp);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_NONCACHED_ਸਮ੍ਰਿਤੀ
	reserve_noncached();
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

/* (permanently) allocate a Board Info struct */
ਸਥਿਰ ਪੂਰਨ reserve_board(ਖਾਲੀ)
{
	ਜੇ (!gd->bd) {
		gd->ਸ਼ੁਰੂ_addr_sp = reserve_ਢੇਰ_aligned(ਆਕਾਰ(ਬਣਤਰ bd_info));
		gd->bd = (ਬਣਤਰ bd_info *)map_sysmem(gd->ਸ਼ੁਰੂ_addr_sp,
						      ਆਕਾਰ(ਬਣਤਰ bd_info));
		memset(gd->bd, '\0', ਆਕਾਰ(ਬਣਤਰ bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      ਆਕਾਰ(ਬਣਤਰ bd_info), gd->ਸ਼ੁਰੂ_addr_sp);
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_global_ਅੰਕੜੇ(ਖਾਲੀ)
{
	gd->ਸ਼ੁਰੂ_addr_sp = reserve_ਢੇਰ_aligned(ਆਕਾਰ(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->ਸ਼ੁਰੂ_addr_sp, ਆਕਾਰ(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      ਆਕਾਰ(gd_t), gd->ਸ਼ੁਰੂ_addr_sp);
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_fdt(ਖਾਲੀ)
{
	ਜੇ (!IS_ENABLED(CONFIG_ਦਾ_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		ਜੇ (gd->fdt_blob) {
			gd->boardf->fdt_ਮਾਪ =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->ਸ਼ੁਰੂ_addr_sp = reserve_ਢੇਰ_aligned(
				gd->boardf->fdt_ਮਾਪ);
			gd->boardf->new_fdt = map_sysmem(gd->ਸ਼ੁਰੂ_addr_sp,
							 gd->boardf->fdt_ਮਾਪ);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_ਮਾਪ, gd->ਸ਼ੁਰੂ_addr_sp);
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_bootstage(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BOOTSTAGE
	ਪੂਰਨ ਮਾਪ = bootstage_get_ਮਾਪ(true);

	gd->ਸ਼ੁਰੂ_addr_sp = reserve_ਢੇਰ_aligned(ਮਾਪ);
	gd->boardf->new_bootstage = map_sysmem(gd->ਸ਼ੁਰੂ_addr_sp, ਮਾਪ);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", ਮਾਪ,
	      gd->ਸ਼ੁਰੂ_addr_sp);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ arch_reserve_stacks(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reserve_stacks(ਖਾਲੀ)
{
	/* make stack pointer 16-byte aligned */
	gd->ਸ਼ੁਰੂ_addr_sp = reserve_ਢੇਰ_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	ਵਾਪਸ arch_reserve_stacks();
}

ਸਥਿਰ ਪੂਰਨ reserve_bloblist(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BLOBLIST
	ulong ਮਾਪ = bloblist_get_ਕੁੱਲ_ਮਾਪ();

	ਜੇ (ਮਾਪ < CONFIG_BLOBLIST_ਮਾਪ_RELOC)
		ਮਾਪ = CONFIG_BLOBLIST_ਮਾਪ_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->ਸ਼ੁਰੂ_addr_sp = ALIGN_ਥੱਲੇ(gd->ਸ਼ੁਰੂ_addr_sp - ਮਾਪ, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->ਸ਼ੁਰੂ_addr_sp, ਮਾਪ);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ display_new_sp(ਖਾਲੀ)
{
	debug("New Stack Pointer is: %08lx\n", gd->ਸ਼ੁਰੂ_addr_sp);

	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ arch_ਤਿਆਰੀ_bdinfo(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਪੂਰਨ ਤਿਆਰੀ_bdinfo(ਖਾਲੀ)
{
	ਵਾਪਸ arch_ਤਿਆਰੀ_bdinfo();
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_POST
ਸਥਿਰ ਪੂਰਨ init_post(ਖਾਲੀ)
{
	post_bootmode_init();
	post_ਲੜੀਵਾਰ(ਨਿਰਾ, POST_ROM | post_bootmode_get(0));

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ reloc_fdt(ਖਾਲੀ)
{
	ਜੇ (!IS_ENABLED(CONFIG_ਦਾ_EMBED)) {
		ਜੇ (gd->boardf->new_fdt) {
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reloc_bootstage(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BOOTSTAGE
	ਜੇ (gd->flags & GD_FLG_SKIP_RELOC)
		ਵਾਪਸ 0;
	ਜੇ (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ reloc_bloblist(ਖਾਲੀ)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	ਜੇ ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_ਮਾਪ == CONFIG_BLOBLIST_ਮਾਪ_RELOC) {
		debug("Not relocating bloblist\n");
		ਵਾਪਸ 0;
	}
	ਜੇ (gd->boardf->new_bloblist) {
		ulong ਮਾਪ = bloblist_get_ਕੁੱਲ_ਮਾਪ();

		ਜੇ (ਮਾਪ < CONFIG_BLOBLIST_ਮਾਪ_RELOC)
			ਮਾਪ = CONFIG_BLOBLIST_ਮਾਪ_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, ਮਾਪ);
		ਵਾਪਸ bloblist_reloc(gd->boardf->new_bloblist, ਮਾਪ);
	}
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਖਾਲੀ mcheck_on_ramrelocation(size_t ਵਿੱਥ);
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_reloc(ਖਾਲੀ)
{
	ਜੇ (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_ਪਾਠ_ਅਧਾਰ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ ARM
		gd->reloc_off = gd->relocaddr - (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)__image_copy_start;
#ਜਾਂਜੇ defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)ਸ਼ੁਰੂ_2;
#ਜਾਂਜੇ defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_ਪਾਠ_ਅਧਾਰ + 0x400);
#ਜਾਂਜੇ !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_ਪਾਠ_ਅਧਾਰ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
	}

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(gd->new_gd, (ਅੱਖਰ *)gd, ਆਕਾਰ(gd_t));

	ਜੇ (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MCHECK_ਢੇਰ_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#ਜੇ_ਅੰਤ
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_ਮੰਜ਼ਿਲ_sysmem(gd->new_gd),
		      gd->ਸ਼ੁਰੂ_addr_sp);
	}

	ਵਾਪਸ 0;
}

#ਜੇ CONFIG_IS_ENABLED(ਦਾ_BOARD_FIXUP)
ਸਥਿਰ ਪੂਰਨ fix_fdt(ਖਾਲੀ)
{
	ਵਾਪਸ board_fix_fdt((ਖਾਲੀ *)gd->fdt_blob);
}
#ਜੇ_ਅੰਤ

/* ARM calls relocate_code from its crt0.S */
#ਜੇ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

ਸਥਿਰ ਪੂਰਨ jump_ਮੰਜ਼ਿਲ_copy(ਖਾਲੀ)
{
	ਜੇ (gd->flags & GD_FLG_SKIP_RELOC)
		ਵਾਪਸ 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#ਜੇ defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_ਤਿਆਰੀ_gd(gd->new_gd);
# ਜੇ CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->ਸ਼ੁਰੂ_addr_sp);
# ਨਹੀਂਤਾਂ
		board_init_f_r_trampoline(gd->ਸ਼ੁਰੂ_addr_sp);
# endif
#ਨਹੀਂਤਾਂ
	relocate_ਕੋਡ(gd->ਸ਼ੁਰੂ_addr_sp, gd->new_gd, gd->relocaddr);
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
ਸਥਿਰ ਪੂਰਨ initf_bootstage(ਖਾਲੀ)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	ਪੂਰਨ ret;

	ret = bootstage_init(!from_spl);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	ਜੇ (from_spl) {
		ret = bootstage_unstash_default();
		ਜੇ (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			ਵਾਪਸ ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_ਸ਼ੁਰੂ_UBOOT_F, "board_init_f");

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initf_dm(ਖਾਲੀ)
{
	ਪੂਰਨ ret;

	ਜੇ (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		ਵਾਪਸ 0;

	bootstage_ਸ਼ੁਰੂ(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	ਜੇ (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_ਜਾਂਚ(true);
	ਨਹੀਂਤਾਂ
		ret = dm_init(false);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ret = dm_autoprobe();
	ਜੇ (ret)
		ਵਾਪਸ ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	ਜੇ (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ਵਾਪਸ 0;
}

/* Architecture-specific memory reservation */
__weak ਪੂਰਨ reserve_arch(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ checkcpu(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

__weak ਪੂਰਨ clear_bss(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ initf_upl(ਖਾਲੀ)
{
	ਬਣਤਰ upl *upl;
	ਪੂਰਨ ret;

	ਜੇ (!IS_ENABLED(CONFIG_UPL_ਵਿੱਚ) || !(gd->flags & GD_FLG_UPL))
		ਵਾਪਸ 0;

	upl = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ upl));
	ਜੇ (!upl)
		ਵਾਪਸ -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	ਜੇ (ret) {
		ਲਿਖੋ("UPL handoff: read failure (err=%dE)\n", ret);
		ਵਾਪਸ ret;
	}
	gd_ਸੈੱਟ_upl(upl);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ initcall_ਲੜੀਵਾਰ_f(ਖਾਲੀ)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(ਤਿਆਰੀ_mon_len);
	INITCALL(initf_malloc);
#ਜੇ CONFIG_IS_ENABLED(ਦਾ_CONTROL)
	INITCALL(fdtdec_ਤਿਆਰੀ);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#ਜੇ_ਅੰਤ
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#ਜੇ CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#ਜੇ_ਅੰਤ
	INITCALL(ਤਿਆਰੀ_spl_handoff);
#ਜੇ CONFIG_IS_ENABLED(CONSOLE_ਰਿਕਾਰਡ_INIT_F)
	INITCALL(console_ਰਿਕਾਰਡ_init);
#ਜੇ_ਅੰਤ
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#ਜੇ CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#ਜੇ_ਅੰਤ
#ਜੇ defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#ਜੇ_ਅੰਤ
#ਜੇ !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#ਜੇ_ਅੰਤ
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_ਦਰ);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_ਪਾਠ_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#ਜੇ CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#ਜੇ_ਅੰਤ
	/* display cpu info (and speed) */
#ਜੇ CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#ਜੇ_ਅੰਤ
#ਜੇ CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#ਜੇ_ਅੰਤ
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#ਜੇ CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#ਜੇ_ਅੰਤ
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#ਜੇ CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#ਜੇ_ਅੰਤ
	WATCHDOG_RESET();
#ਜੇ defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#ਜੇ_ਅੰਤ /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#ਜੇ CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#ਜੇ_ਅੰਤ
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
	INITCALL(ਤਿਆਰੀ_ram_ਅਧਾਰ);
	INITCALL(dram_init_banksize);
	INITCALL(ਤਿਆਰੀ_ram_config);
	INITCALL(ਤਿਆਰੀ_dest_addr);
#ਜੇ CONFIG_IS_ENABLED(ਦਾ_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(ਦਾ_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CFG_PRAM
	INITCALL(reserve_pram);
#ਜੇ_ਅੰਤ
	INITCALL(reserve_ਗੇੜਾ_4k);
	INITCALL(ਤਿਆਰੀ_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_ਅੰਕੜੇ);
	INITCALL(reserve_fdt);
#ਜੇ CONFIG_IS_ENABLED(ਦਾ_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(ਦਾ_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#ਜੇ_ਅੰਤ
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(ਤਿਆਰੀ_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#ਜੇ !CONFIG_IS_ENABLED(ਦਾ_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#ਜੇ_ਅੰਤ
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(ਤਿਆਰੀ_reloc);
#ਜੇ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_ਮੰਜ਼ਿਲ_ram);
	INITCALL(do_elf_reloc_fixups);
#ਜੇ_ਅੰਤ
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
#ਜੇ !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_ਮੰਜ਼ਿਲ_copy);
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ board_init_f(ulong boot_flags)
{
	ਬਣਤਰ board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_ਲੜੀਵਾਰ_f();

#ਜੇ !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#ਜੇ_ਅੰਤ
}

#ਜੇ defined(CONFIG_X86) || defined(CONFIG_ARC)
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
ਸਥਿਰ ਖਾਲੀ initcall_ਲੜੀਵਾਰ_f_r(ਖਾਲੀ)
{
#ਜੇ !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#ਜੇ_ਅੰਤ
}

ਖਾਲੀ board_init_f_r(ਖਾਲੀ)
{
	initcall_ਲੜੀਵਾਰ_f_r();

	/*
	 * The pre-relocation drivers may be using memory that has now gone
	 * away. Mark serial as unavailable - this will fall back to the debug
	 * UART if available.
	 *
	 * Do the same with log drivers since the memory may not be available.
	 */
	gd->flags &= ~(GD_FLG_SERIAL_READY | GD_FLG_LOG_READY);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_TIMER
	gd->timer = ਨਿਰਾ;
#ਜੇ_ਅੰਤ

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#ਜੇ_ਅੰਤ /* CONFIG_X86 */
