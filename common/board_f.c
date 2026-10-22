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

#समावेश <config.h>
#समावेश <bloblist.h>
#समावेश <bootstage.h>
#समावेश <clock_legacy.h>
#समावेश <console.h>
#समावेश <cpu.h>
#समावेश <cpu_func.h>
#समावेश <cyclic.h>
#समावेश <display_options.h>
#समावेश <dm.h>
#समावेश <env.h>
#समावेश <env_internal.h>
#समावेश <event.h>
#समावेश <fdtdec.h>
#समावेश <fs.h>
#समावेश <hang.h>
#समावेश <i2c.h>
#समावेश <init.h>
#समावेश <initcall.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <memtop.h>
#समावेश <os.h>
#समावेश <post.h>
#समावेश <relocate.h>
#समावेश <serial.h>
#समावेश <spl.h>
#समावेश <sysreset.h>
#समावेश <time.h>
#समावेश <timer.h>
#समावेश <trace.h>
#समावेश <upl.h>
#समावेश <video.h>
#समावेश <watchdog.h>
#समावेश <asm/cache.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <asm/sections.h>
#समावेश <dm/root.h>
#समावेश <linux/errno.h>
#समावेश <linux/log2.h>
#समावेश <linux/sizes.h>

DECLARE_GLOBAL_तथ्य_PTR;

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

#अगर defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
अटल पूर्णांक init_func_watchdog_init(व्योम)
{
# अगर defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	माला_दो("       Watchdog enabled\n");
# endif
	schedule();

	वापस 0;
}

पूर्णांक init_func_watchdog_reset(व्योम)
{
	schedule();

	वापस 0;
}
#पूर्ण_अगर /* CONFIG_WATCHDOG */

__weak व्योम board_add_ram_info(पूर्णांक use_default)
{
	/* please define platform specific board_add_ram_info() */
}

अटल पूर्णांक init_baud_rate(व्योम)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	वापस 0;
}

अटल पूर्णांक display_text_info(व्योम)
{
#अगर !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#अगर_घोषित CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#अन्यथा
	text_base = CONFIG_SYS_MONITOR_BASE;
#पूर्ण_अगर

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#पूर्ण_अगर

	वापस 0;
}

#अगर_घोषित CONFIG_SYSRESET_PRINT_RESETINFO
अटल पूर्णांक print_resetinfo(व्योम)
{
	काष्ठा udevice *dev;
	अक्षर status[256];
	bool status_printed = false;
	पूर्णांक ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	क्रम (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_अग्र_device_check(&dev)) {
		अगर (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			जारी;
		}

		अगर (!sysreset_get_status(dev, status, माप(status))) {
			म_लिखो("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	अगर (status_printed)
		म_लिखो("\n");

	वापस 0;
}
#पूर्ण_अगर

#अगर defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
अटल पूर्णांक print_cpuinfo(व्योम)
{
	काष्ठा udevice *dev;
	अक्षर desc[512];
	पूर्णांक ret;

	dev = cpu_get_current_dev();
	अगर (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		वापस -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, माप(desc));
	अगर (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		वापस ret;
	}

	म_लिखो("CPU:   %s\n", desc);

	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक announce_dram_init(व्योम)
{
	माला_दो("DRAM:  ");
	वापस 0;
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
#घोषणा compute_आकार_scale_val(आकार, scale, val) करो { \
	scale = ilog2(आकार) / 10 * 10; \
	val = (10 * आकार + ((1ULL << scale) >> 1)) >> scale; \
	अगर (val == 10240) { val = 10; scale += 10; } \
} जबतक (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
अटल पूर्णांक sizes_near(अचिन्हित दीर्घ दीर्घ size1, अचिन्हित दीर्घ दीर्घ size2)
{
	अचिन्हित पूर्णांक size1_scale, size1_val, size2_scale, size2_val;

	compute_आकार_scale_val(size1, size1_scale, size1_val);
	compute_आकार_scale_val(size2, size2_scale, size2_val);

	वापस size1_scale == size2_scale && size1_val == size2_val;
}

अटल पूर्णांक show_dram_config(व्योम)
{
	अचिन्हित दीर्घ दीर्घ आकार;
	पूर्णांक i;

	debug("\nRAM Configuration:\n");
	क्रम (i = आकार = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		आकार += gd->dram[i].आकार;
		debug("Bank #%d: %llx ", i,
		      (अचिन्हित दीर्घ दीर्घ)(gd->dram[i].start));
#अगर_घोषित DEBUG
		print_आकार(gd->dram[i].आकार, "\n");
#पूर्ण_अगर
	}
	debug("\nDRAM:  ");

	print_आकार(gd->ram_आकार, "");
	अगर (!sizes_near(gd->ram_आकार, आकार)) {
		म_लिखो(" (total ");
		print_आकार(आकार, ")");
	}
	board_add_ram_info(0);
	अ_दो('\n');

	वापस 0;
}

__weak पूर्णांक dram_init_banksize(व्योम)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].आकार = get_effective_memsize();

	वापस 0;
}

#अगर CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
अटल पूर्णांक init_func_i2c(व्योम)
{
	माला_दो("I2C:   ");
	i2c_init_all();
	माला_दो("ready\n");
	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक setup_mon_len(व्योम)
{
#अगर defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#या_अगर defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#या_अगर defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#या_अगर defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#या_अगर defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#या_अगर defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#या_अगर defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#या_अगर defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#पूर्ण_अगर
	वापस 0;
}

अटल पूर्णांक setup_spl_handoff(व्योम)
{
#अगर CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					माप(काष्ठा spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#पूर्ण_अगर

	वापस 0;
}

__weak पूर्णांक arch_cpu_init(व्योम)
{
	वापस 0;
}

__weak पूर्णांक mach_cpu_init(व्योम)
{
	वापस 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_शीर्ष(phys_आकार_t योग_आकार)
{
	अगर (CONFIG_IS_ENABLED(RELOC_ADDR_शीर्ष))
		वापस gd->ram_शीर्ष;

#अगर defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	अगर (gd->ram_शीर्ष < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		वापस 0;
#पूर्ण_अगर
	वापस gd->ram_शीर्ष;
}

__weak पूर्णांक arch_setup_dest_addr(व्योम)
{
	वापस 0;
}

अटल पूर्णांक setup_ram_base(व्योम)
{
#अगर_घोषित CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#पूर्ण_अगर
	वापस 0;
}

अटल पूर्णांक setup_ram_config(व्योम)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	अगर (CONFIG_IS_ENABLED(RELOC_ADDR_शीर्ष)) {
		पूर्णांक i;
		phys_addr_t शीर्ष;

		gd->ram_आकार = 0;
		क्रम (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			शीर्ष = get_mem_शीर्ष(gd->dram[i].start, gd->dram[i].आकार,
					  ALIGN(gd->mon_len, SZ_1M),
					  (व्योम *)gd->fdt_blob);
			gd->ram_शीर्ष = max(शीर्ष, gd->ram_शीर्ष);
			gd->ram_आकार += gd->dram[i].आकार;
		}
	} अन्यथा {
		gd->ram_शीर्ष = gd->ram_base + get_effective_memsize();
	}
	gd->ram_शीर्ष = board_get_usable_ram_शीर्ष(gd->mon_len);
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
#अगर CONFIG_VAL(SYS_MEM_शीर्ष_HIDE)
	gd->ram_शीर्ष -= CONFIG_SYS_MEM_शीर्ष_HIDE;
	gd->ram_आकार -= CONFIG_SYS_MEM_शीर्ष_HIDE;
#पूर्ण_अगर

	debug("Ram top: %08llx\n", (अचिन्हित दीर्घ दीर्घ)gd->ram_शीर्ष);
	debug("Ram size: %08llx\n", (अचिन्हित दीर्घ दीर्घ)gd->ram_आकार);

	वापस 0;
}

अटल पूर्णांक setup_dest_addr(व्योम)
{
	पूर्णांक ret;

	gd->relocaddr = gd->ram_शीर्ष;
	debug("Reloc addr: %08llX\n", (अचिन्हित दीर्घ दीर्घ)gd->relocaddr);

	ret = arch_setup_dest_addr();
	अगर (ret)
		वापस ret;

	gd->initial_relocaddr = gd->relocaddr;
	वापस 0;
}

#अगर_घोषित CFG_PRAM
/* reserve protected RAM */
अटल पूर्णांक reserve_pram(व्योम)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	वापस 0;
}
#पूर्ण_अगर /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
अटल पूर्णांक reserve_round_4k(व्योम)
{
	gd->relocaddr &= ~(4096 - 1);
	वापस 0;
}

__weak पूर्णांक arch_reserve_mmu(व्योम)
{
	वापस 0;
}

अटल पूर्णांक reserve_video_from_videoblob(व्योम)
{
	अगर (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		काष्ठा video_handoff *ho;
		पूर्णांक ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, माप(*ho));
		अगर (!ho)
			वापस log_संदेश_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		अगर (ret)
			वापस log_संदेश_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		अगर (likely(gd->relocaddr > (अचिन्हित दीर्घ)ho->fb))
			gd->relocaddr = ho->fb;
	}

	वापस 0;
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
अटल पूर्णांक setup_relocaddr_from_bloblist(व्योम)
{
	reserve_video_from_videoblob();

	वापस 0;
}

अटल पूर्णांक reserve_video(व्योम)
{
	अगर (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		पूर्णांक ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		अगर (ret)
			वापस ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((अचिन्हित दीर्घ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	वापस 0;
}

अटल पूर्णांक reserve_trace(व्योम)
{
#अगर_घोषित CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_बफर_आकार;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_बफर_आकार);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (अचिन्हित दीर्घ)CONFIG_TRACE_बफर_आकार >> 10, gd->relocaddr);
#पूर्ण_अगर

	वापस 0;
}

अटल पूर्णांक reserve_uboot(व्योम)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	अगर (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	अगर (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#अगर defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#पूर्ण_अगर

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	वापस 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
अटल अचिन्हित दीर्घ reserve_ढेर_aligned(माप_प्रकार आकार)
{
	वापस ALIGN_DOWN(gd->start_addr_sp - आकार, 16);
}

#अगर_घोषित CONFIG_SYS_NONCACHED_MEMORY
अटल पूर्णांक reserve_noncached(व्योम)
{
	/*
	 * The value of gd->start_addr_sp must match the value of
	 * mem_malloc_start calculated in board_r.c:initr_malloc(), which is
	 * passed to dlmalloc.c:mem_malloc_init() and then used by
	 * cache.c:noncached_init()
	 *
	 * These calculations must match the code in cache.c:noncached_init()
	 */
	gd->start_addr_sp = ALIGN(gd->start_addr_sp, MMU_SECTION_आकार) -
		MMU_SECTION_आकार;
	gd->start_addr_sp -= ALIGN(CONFIG_SYS_NONCACHED_MEMORY,
				   MMU_SECTION_आकार);
	debug("Reserving %dM for noncached_alloc() at: %08lx\n",
	      CONFIG_SYS_NONCACHED_MEMORY >> 20, gd->start_addr_sp);

	वापस 0;
}
#पूर्ण_अगर

/* reserve memory for malloc() area */
अटल पूर्णांक reserve_malloc(व्योम)
{
	gd->start_addr_sp = reserve_ढेर_aligned(योग_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      योग_MALLOC_LEN >> 10, gd->start_addr_sp);
#अगर_घोषित CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#पूर्ण_अगर

	वापस 0;
}

/* (permanently) allocate a Board Info struct */
अटल पूर्णांक reserve_board(व्योम)
{
	अगर (!gd->bd) {
		gd->start_addr_sp = reserve_ढेर_aligned(माप(काष्ठा bd_info));
		gd->bd = (काष्ठा bd_info *)map_sysmem(gd->start_addr_sp,
						      माप(काष्ठा bd_info));
		स_रखो(gd->bd, '\0', माप(काष्ठा bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      माप(काष्ठा bd_info), gd->start_addr_sp);
	}
	वापस 0;
}

अटल पूर्णांक reserve_global_तथ्य(व्योम)
{
	gd->start_addr_sp = reserve_ढेर_aligned(माप(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, माप(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      माप(gd_t), gd->start_addr_sp);
	वापस 0;
}

अटल पूर्णांक reserve_fdt(व्योम)
{
	अगर (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		अगर (gd->fdt_blob) {
			gd->boardf->fdt_आकार =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_ढेर_aligned(
				gd->boardf->fdt_आकार);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_आकार);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_आकार, gd->start_addr_sp);
		}
	}

	वापस 0;
}

अटल पूर्णांक reserve_bootstage(व्योम)
{
#अगर_घोषित CONFIG_BOOTSTAGE
	पूर्णांक आकार = bootstage_get_आकार(true);

	gd->start_addr_sp = reserve_ढेर_aligned(आकार);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, आकार);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", आकार,
	      gd->start_addr_sp);
#पूर्ण_अगर

	वापस 0;
}

__weak पूर्णांक arch_reserve_stacks(व्योम)
{
	वापस 0;
}

अटल पूर्णांक reserve_stacks(व्योम)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_ढेर_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	वापस arch_reserve_stacks();
}

अटल पूर्णांक reserve_bloblist(व्योम)
{
#अगर_घोषित CONFIG_BLOBLIST
	ulong आकार = bloblist_get_योग_आकार();

	अगर (आकार < CONFIG_BLOBLIST_आकार_RELOC)
		आकार = CONFIG_BLOBLIST_आकार_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - आकार, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, आकार);
#पूर्ण_अगर

	वापस 0;
}

अटल पूर्णांक display_new_sp(व्योम)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	वापस 0;
}

__weak पूर्णांक arch_setup_bdinfo(व्योम)
{
	वापस 0;
}

पूर्णांक setup_bdinfo(व्योम)
{
	वापस arch_setup_bdinfo();
}

#अगर_घोषित CONFIG_POST
अटल पूर्णांक init_post(व्योम)
{
	post_bootmode_init();
	post_run(शून्य, POST_ROM | post_bootmode_get(0));

	वापस 0;
}
#पूर्ण_अगर

अटल पूर्णांक reloc_fdt(व्योम)
{
	अगर (!IS_ENABLED(CONFIG_OF_EMBED)) {
		अगर (gd->boardf->new_fdt) {
			स_नकल(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	वापस 0;
}

अटल पूर्णांक reloc_bootstage(व्योम)
{
#अगर_घोषित CONFIG_BOOTSTAGE
	अगर (gd->flags & GD_FLG_SKIP_RELOC)
		वापस 0;
	अगर (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#पूर्ण_अगर

	वापस 0;
}

अटल पूर्णांक reloc_bloblist(व्योम)
{
#अगर_घोषित CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	अगर ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_आकार == CONFIG_BLOBLIST_आकार_RELOC) {
		debug("Not relocating bloblist\n");
		वापस 0;
	}
	अगर (gd->boardf->new_bloblist) {
		ulong आकार = bloblist_get_योग_आकार();

		अगर (आकार < CONFIG_BLOBLIST_आकार_RELOC)
			आकार = CONFIG_BLOBLIST_आकार_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, आकार);
		वापस bloblist_reloc(gd->boardf->new_bloblist, आकार);
	}
#पूर्ण_अगर

	वापस 0;
}

व्योम mcheck_on_ramrelocation(माप_प्रकार offset);
अटल पूर्णांक setup_reloc(व्योम)
{
	अगर (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#अगर_घोषित CONFIG_TEXT_BASE
#अगर_घोषित ARM
		gd->reloc_off = gd->relocaddr - (अचिन्हित दीर्घ)__image_copy_start;
#या_अगर defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#या_अगर defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#या_अगर !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#पूर्ण_अगर
#पूर्ण_अगर
	}

	स_नकल(gd->new_gd, (अक्षर *)gd, माप(gd_t));

	अगर (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} अन्यथा {
#अगर_घोषित MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#पूर्ण_अगर
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	वापस 0;
}

#अगर CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
अटल पूर्णांक fix_fdt(व्योम)
{
	वापस board_fix_fdt((व्योम *)gd->fdt_blob);
}
#पूर्ण_अगर

/* ARM calls relocate_code from its crt0.S */
#अगर !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

अटल पूर्णांक jump_to_copy(व्योम)
{
	अगर (gd->flags & GD_FLG_SKIP_RELOC)
		वापस 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#अगर defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# अगर CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# अन्यथा
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#अन्यथा
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#पूर्ण_अगर

	वापस 0;
}
#पूर्ण_अगर

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
अटल पूर्णांक initf_bootstage(व्योम)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	पूर्णांक ret;

	ret = bootstage_init(!from_spl);
	अगर (ret)
		वापस ret;
	अगर (from_spl) {
		ret = bootstage_unstash_default();
		अगर (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			वापस ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	वापस 0;
}

अटल पूर्णांक initf_dm(व्योम)
{
	पूर्णांक ret;

	अगर (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		वापस 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	अगर (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	अन्यथा
		ret = dm_init(false);
	अगर (ret)
		वापस ret;

	ret = dm_autoprobe();
	अगर (ret)
		वापस ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	अगर (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		अगर (ret)
			वापस ret;
	}

	वापस 0;
}

/* Architecture-specific memory reservation */
__weak पूर्णांक reserve_arch(व्योम)
{
	वापस 0;
}

__weak पूर्णांक checkcpu(व्योम)
{
	वापस 0;
}

__weak पूर्णांक clear_bss(व्योम)
{
	वापस 0;
}

अटल पूर्णांक initf_upl(व्योम)
{
	काष्ठा upl *upl;
	पूर्णांक ret;

	अगर (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		वापस 0;

	upl = दो_स्मृति(माप(काष्ठा upl));
	अगर (!upl)
		वापस -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	अगर (ret) {
		म_लिखो("UPL handoff: read failure (err=%dE)\n", ret);
		वापस ret;
	}
	gd_set_upl(upl);

	वापस 0;
}

अटल व्योम initcall_run_f(व्योम)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#अगर CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#पूर्ण_अगर
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#अगर CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#पूर्ण_अगर
	INITCALL(setup_spl_handoff);
#अगर CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#पूर्ण_अगर
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#अगर CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#पूर्ण_अगर
#अगर defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#पूर्ण_अगर
#अगर !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#पूर्ण_अगर
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#अगर CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#पूर्ण_अगर
	/* display cpu info (and speed) */
#अगर CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#पूर्ण_अगर
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#अगर CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#पूर्ण_अगर
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#अगर CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#पूर्ण_अगर
	WATCHDOG_RESET();
#अगर defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#पूर्ण_अगर /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#अगर CONFIG_IS_ENABLED(POST)
	INITCALL(init_post);
#पूर्ण_अगर
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
#अगर CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
    !CONFIG_IS_ENABLED(OF_INITIAL_DTB_READONLY)
	INITCALL(fix_fdt);
#पूर्ण_अगर
#अगर_घोषित CFG_PRAM
	INITCALL(reserve_pram);
#पूर्ण_अगर
	INITCALL(reserve_round_4k);
	INITCALL(setup_relocaddr_from_bloblist);
	INITCALL(arch_reserve_mmu);
	INITCALL(reserve_video);
	INITCALL(reserve_trace);
	INITCALL(reserve_uboot);
	INITCALL(reserve_malloc);
	INITCALL(reserve_board);
	INITCALL(reserve_global_तथ्य);
	INITCALL(reserve_fdt);
#अगर CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
    CONFIG_IS_ENABLED(OF_INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
	INITCALL(fix_fdt);
#पूर्ण_अगर
	INITCALL(reserve_bootstage);
	INITCALL(reserve_bloblist);
	INITCALL(reserve_arch);
	INITCALL(reserve_stacks);
	INITCALL(show_dram_config);
	WATCHDOG_RESET();
	INITCALL(setup_bdinfo);
	INITCALL(display_new_sp);
	WATCHDOG_RESET();
#अगर !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#पूर्ण_अगर
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#अगर CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
	INITCALL(copy_uboot_to_ram);
	INITCALL(do_elf_reloc_fixups);
#पूर्ण_अगर
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
#अगर !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#पूर्ण_अगर
}

व्योम board_init_f(ulong boot_flags)
{
	काष्ठा board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#अगर !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#पूर्ण_अगर
}

#अगर defined(CONFIG_X86) || defined(CONFIG_ARC)
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
अटल व्योम initcall_run_f_r(व्योम)
{
#अगर !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#पूर्ण_अगर
}

व्योम board_init_f_r(व्योम)
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
#अगर_घोषित CONFIG_TIMER
	gd->timer = शून्य;
#पूर्ण_अगर

	/*
	 * U-Boot has been copied into SDRAM, the BSS has been cleared etc.
	 * Transfer execution from Flash to RAM by calculating the address
	 * of the in-RAM copy of board_init_r() and calling it
	 */
	(board_init_r + gd->reloc_off)((gd_t *)gd, gd->relocaddr);

	/* NOTREACHED - board_init_r() does not return */
	hang();
}
#पूर्ण_अगर /* CONFIG_X86 */
