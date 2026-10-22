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

#include <config.h>
#include <bloblist.h>
#include <bootstage.h>
#include <clock_legacy.h>
#include <console.h>
#include <cpu.h>
#include <cpu_func.h>
#include <cyclic.h>
#include <display_options.h>
#include <dm.h>
#include <env.h>
#include <env_internal.h>
#include <event.h>
#include <fdtdec.h>
#include <fs.h>
#include <hang.h>
#include <i2c.h>
#include <init.h>
#include <initcall.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <memtop.h>
#include <os.h>
#include <post.h>
#include <relocate.h>
#include <serial.h>
#include <spl.h>
#include <sysreset.h>
#include <time.h>
#include <timer.h>
#include <trace.h>
#include <upl.h>
#include <video.h>
#include <watchdog.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/sections.h>
#include <dm/root.h>
#include <linux/errno.h>
#include <linux/log2.h>
#include <linux/sizes.h>

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

#གལ་སྲིད defined(CONFIG_WATCHDOG) || defined(CONFIG_HW_WATCHDOG)
static ཧྲིལ init_func_watchdog_init(void)
{
# གལ་སྲིད defined(CONFIG_HW_WATCHDOG) && \
	(defined(CONFIG_M68K) || defined(CONFIG_MICROBLAZE) || \
	defined(CONFIG_SH) || \
	defined(CONFIG_DESIGNWARE_WATCHDOG) || \
	defined(CONFIG_IMX_WATCHDOG))
	hw_watchdog_init();
	puts("       Watchdog enabled\n");
# endif
	schedule();

	ལོག 0;
}

ཧྲིལ init_func_watchdog_reset(void)
{
	schedule();

	ལོག 0;
}
#endif /* CONFIG_WATCHDOG */

__weak void board_add_ram_info(ཧྲིལ use_default)
{
	/* please define platform specific board_add_ram_info() */
}

static ཧྲིལ init_baud_rate(void)
{
	gd->baudrate = env_get_ulong("baudrate", 10, CONFIG_BAUDRATE);
	ལོག 0;
}

static ཧྲིལ display_text_info(void)
{
#གལ་སྲིད !defined(CONFIG_SANDBOX) && !defined(CONFIG_EFI_APP)
	ulong bss_start, bss_end, text_base;

	bss_start = (ulong)__bss_start;
	bss_end = (ulong)__bss_end;

#ifdef CONFIG_TEXT_BASE
	text_base = CONFIG_TEXT_BASE;
#དེ་མིན
	text_base = CONFIG_SYS_MONITOR_BASE;
#endif

	debug("U-Boot code: %08lX -> %08lX  BSS: -> %08lX\n",
	      text_base, bss_start, bss_end);
#endif

	ལོག 0;
}

#ifdef CONFIG_SYSRESET_PRINT_RESETINFO
static ཧྲིལ print_resetinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na status[256];
	bool status_printed = false;
	ཧྲིལ ret;

	/*
	 * Not all boards have sysreset drivers available during early
	 * boot, so don't fail if one can't be found.
	 */
	དོན་ལུ (ret = uclass_first_device_check(UCLASS_SYSRESET, &dev); dev;
	     ret = uclass_next_device_check(&dev)) {
		གལ་སྲིད (ret) {
			debug("%s: %s sysreset device (error: %d)\n",
			      __func__, dev->name, ret);
			continue;
		}

		གལ་སྲིད (!sysreset_get_status(dev, status, parimaa_na(status))) {
			printf("%s%s", status_printed ? " " : "", status);
			status_printed = true;
		}
	}
	གལ་སྲིད (status_printed)
		printf("\n");

	ལོག 0;
}
#endif

#གལ་སྲིད defined(CONFIG_DISPLAY_CPUINFO) && CONFIG_IS_ENABLED(CPU)
static ཧྲིལ print_cpuinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na desc[512];
	ཧྲིལ ret;

	dev = cpu_get_current_dev();
	གལ་སྲིད (!dev) {
		debug("%s: Could not get CPU device\n",
		      __func__);
		ལོག -ENODEV;
	}

	ret = cpu_get_desc(dev, desc, parimaa_na(desc));
	གལ་སྲིད (ret) {
		debug("%s: Could not get CPU description (err = %d)\n",
		      dev->name, ret);
		ལོག ret;
	}

	printf("CPU:   %s\n", desc);

	ལོག 0;
}
#endif

static ཧྲིལ announce_dram_init(void)
{
	puts("DRAM:  ");
	ལོག 0;
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
#define compute_size_scale_val(size, scale, val) do { \
	scale = ilog2(size) / 10 * 10; \
	val = (10 * size + ((1ULL << scale) >> 1)) >> scale; \
	གལ་སྲིད (val == 10240) { val = 10; scale += 10; } \
} སྐབས (0)

/*
 * Check if the sizes in their natural units written in decimal format with
 * one fraction number are same.
 */
static ཧྲིལ sizes_near(unsigned long long size1, unsigned long long size2)
{
	unsigned ཧྲིལ size1_scale, size1_val, size2_scale, size2_val;

	compute_size_scale_val(size1, size1_scale, size1_val);
	compute_size_scale_val(size2, size2_scale, size2_val);

	ལོག size1_scale == size2_scale && size1_val == size2_val;
}

static ཧྲིལ show_dram_config(void)
{
	unsigned long long size;
	ཧྲིལ i;

	debug("\nRAM Configuration:\n");
	དོན་ལུ (i = size = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		size += gd->dram[i].size;
		debug("Bank #%d: %llx ", i,
		      (unsigned long long)(gd->dram[i].start));
#ifdef DEBUG
		print_size(gd->dram[i].size, "\n");
#endif
	}
	debug("\nDRAM:  ");

	print_size(gd->ram_size, "");
	གལ་སྲིད (!sizes_near(gd->ram_size, size)) {
		printf(" (total ");
		print_size(size, ")");
	}
	board_add_ram_info(0);
	putc('\n');

	ལོག 0;
}

__weak ཧྲིལ dram_init_banksize(void)
{
	gd->dram[0].start = gd->ram_base;
	gd->dram[0].size = get_effective_memsize();

	ལོག 0;
}

#གལ་སྲིད CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
static ཧྲིལ init_func_i2c(void)
{
	puts("I2C:   ");
	i2c_init_all();
	puts("ready\n");
	ལོག 0;
}
#endif

static ཧྲིལ setup_mon_len(void)
{
#གལ་སྲིད defined(CONFIG_ARCH_NEXELL)
	gd->mon_len = (ulong)__bss_end - (ulong)__image_copy_start;
#elif defined(__ARM__) || defined(__MICROBLAZE__)
	gd->mon_len = (ulong)__bss_end - (ulong)_start;
#elif defined(CONFIG_SANDBOX) && !defined(__riscv)
	gd->mon_len = (ulong)_end - (ulong)_init;
#elif defined(CONFIG_SANDBOX)
	/* gcc does not provide _init in crti.o on RISC-V */
	gd->mon_len = 0;
#elif defined(CONFIG_EFI_APP)
	gd->mon_len = (ulong)_end - (ulong)_init;
#elif defined(CONFIG_NIOS2) || defined(CONFIG_XTENSA)
	gd->mon_len = CONFIG_SYS_MONITOR_LEN;
#elif defined(CONFIG_SH) || defined(CONFIG_RISCV)
	gd->mon_len = (ulong)(__bss_end) - (ulong)(_start);
#elif defined(CONFIG_SYS_MONITOR_BASE)
	/* TODO: use (ulong)__bss_end - (ulong)__text_start; ? */
	gd->mon_len = (ulong)__bss_end - CONFIG_SYS_MONITOR_BASE;
#endif
	ལོག 0;
}

static ཧྲིལ setup_spl_handoff(void)
{
#གལ་སྲིད CONFIG_IS_ENABLED(HANDOFF)
	gd->spl_handoff = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF,
					parimaa_na(sa_nracanaa spl_handoff));
	debug("Found SPL hand-off info %p\n", gd->spl_handoff);
#endif

	ལོག 0;
}

__weak ཧྲིལ arch_cpu_init(void)
{
	ལོག 0;
}

__weak ཧྲིལ mach_cpu_init(void)
{
	ལོག 0;
}

/* Get the top of usable RAM */
__weak phys_addr_t board_get_usable_ram_top(phys_size_t total_size)
{
	གལ་སྲིད (CONFIG_IS_ENABLED(RELOC_ADDR_TOP))
		ལོག gd->ram_top;

#གལ་སྲིད defined(CFG_SYS_SDRAM_BASE) && CFG_SYS_SDRAM_BASE > 0
	/*
	 * Detect whether we have so much RAM that it goes past the end of our
	 * 32-bit address space. If so, clip the usable RAM so it doesn't.
	 */
	གལ་སྲིད (gd->ram_top < CFG_SYS_SDRAM_BASE)
		/*
		 * Will wrap back to top of 32-bit space when reservations
		 * are made.
		 */
		ལོག 0;
#endif
	ལོག gd->ram_top;
}

__weak ཧྲིལ arch_setup_dest_addr(void)
{
	ལོག 0;
}

static ཧྲིལ setup_ram_base(void)
{
#ifdef CFG_SYS_SDRAM_BASE
	gd->ram_base = CFG_SYS_SDRAM_BASE;
#endif
	ལོག 0;
}

static ཧྲིལ setup_ram_config(void)
{
	debug("Monitor len: %08x\n", gd->mon_len);

	གལ་སྲིད (CONFIG_IS_ENABLED(RELOC_ADDR_TOP)) {
		ཧྲིལ i;
		phys_addr_t top;

		gd->ram_size = 0;
		དོན་ལུ (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
			top = get_mem_top(gd->dram[i].start, gd->dram[i].size,
					  ALIGN(gd->mon_len, SZ_1M),
					  (void *)gd->fdt_blob);
			gd->ram_top = max(top, gd->ram_top);
			gd->ram_size += gd->dram[i].size;
		}
	} དེ་མིན {
		gd->ram_top = gd->ram_base + get_effective_memsize();
	}
	gd->ram_top = board_get_usable_ram_top(gd->mon_len);
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
#གལ་སྲིད CONFIG_VAL(SYS_MEM_TOP_HIDE)
	gd->ram_top -= CONFIG_SYS_MEM_TOP_HIDE;
	gd->ram_size -= CONFIG_SYS_MEM_TOP_HIDE;
#endif

	debug("Ram top: %08llx\n", (unsigned long long)gd->ram_top);
	debug("Ram size: %08llx\n", (unsigned long long)gd->ram_size);

	ལོག 0;
}

static ཧྲིལ setup_dest_addr(void)
{
	ཧྲིལ ret;

	gd->relocaddr = gd->ram_top;
	debug("Reloc addr: %08llX\n", (unsigned long long)gd->relocaddr);

	ret = arch_setup_dest_addr();
	གལ་སྲིད (ret)
		ལོག ret;

	gd->initial_relocaddr = gd->relocaddr;
	ལོག 0;
}

#ifdef CFG_PRAM
/* reserve protected RAM */
static ཧྲིལ reserve_pram(void)
{
	ulong reg;

	reg = env_get_ulong("pram", 10, CFG_PRAM);
	gd->relocaddr -= (reg << 10);		/* size is in kB */
	debug("Reserving %ldk for protected RAM at %08lx\n", reg,
	      gd->relocaddr);
	ལོག 0;
}
#endif /* CFG_PRAM */

/* Round memory pointer down to next 4 kB limit */
static ཧྲིལ reserve_round_4k(void)
{
	gd->relocaddr &= ~(4096 - 1);
	ལོག 0;
}

__weak ཧྲིལ arch_reserve_mmu(void)
{
	ལོག 0;
}

static ཧྲིལ reserve_video_from_videoblob(void)
{
	གལ་སྲིད (IS_ENABLED(CONFIG_SPL_VIDEO_HANDOFF) && xpl_phase() > PHASE_SPL) {
		sa_nracanaa video_handoff *ho;
		ཧྲིལ ret = 0;

		ho = bloblist_find(BLOBLISTT_U_BOOT_VIDEO, parimaa_na(*ho));
		གལ་སྲིད (!ho)
			ལོག log_msg_ret("Missing video bloblist", -ENOENT);

		ret = video_reserve_from_bloblist(ho);
		གལ་སྲིད (ret)
			ལོག log_msg_ret("Invalid Video handoff info", ret);

		/* Sanity check fb from blob is before current relocaddr */
		གལ་སྲིད (likely(gd->relocaddr > (unsigned long)ho->fb))
			gd->relocaddr = ho->fb;
	}

	ལོག 0;
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
static ཧྲིལ setup_relocaddr_from_bloblist(void)
{
	reserve_video_from_videoblob();

	ལོག 0;
}

static ཧྲིལ reserve_video(void)
{
	གལ་སྲིད (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ཧྲིལ ret;

		addr = gd->relocaddr;
		ret = video_reserve(&addr);
		གལ་སྲིད (ret)
			ལོག ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ལོག 0;
}

static ཧྲིལ reserve_trace(void)
{
#ifdef CONFIG_TRACE
	gd->relocaddr -= CONFIG_TRACE_BUFFER_SIZE;
	gd->trace_buff = map_sysmem(gd->relocaddr, CONFIG_TRACE_BUFFER_SIZE);
	debug("Reserving %luk for trace data at: %08lx\n",
	      (unsigned long)CONFIG_TRACE_BUFFER_SIZE >> 10, gd->relocaddr);
#endif

	ལོག 0;
}

static ཧྲིལ reserve_uboot(void)
{
	/*
	 * This should be the first place GD_FLG_SKIP_RELOC is read from.
	 * Set GD_FLG_SKIP_RELOC flag if CONFIG_SKIP_RELOCATE is enabled.
	 */
	གལ་སྲིད (CONFIG_IS_ENABLED(SKIP_RELOCATE))
		gd->flags |= GD_FLG_SKIP_RELOC;

	གལ་སྲིད (!(gd->flags & GD_FLG_SKIP_RELOC) && !CONFIG_IS_ENABLED(SKIP_RELOCATE_CODE)) {
		/*
		 * reserve memory for U-Boot code, data & bss
		 * round down to next 4 kB limit
		 */
		gd->relocaddr -= gd->mon_len;
		gd->relocaddr &= ~(4096 - 1);
	#གལ་སྲིད defined(CONFIG_E500) || defined(CONFIG_MIPS)
		/* round down to next 64 kB limit so that IVPR stays aligned */
		gd->relocaddr &= ~(65536 - 1);
	#endif

		debug("Reserving %dk for U-Boot at: %08lx\n",
		      gd->mon_len >> 10, gd->relocaddr);
	}

	gd->start_addr_sp = gd->relocaddr;

	ལོག 0;
}

/*
 * reserve after start_addr_sp the requested size and make the stack pointer
 * 16-byte aligned, this alignment is needed for cast on the reserved memory
 * ref = x86_64 ABI: https://reviews.llvm.org/D30049: 16 bytes
 *     = ARMv8 Instruction Set Overview: quad word, 16 bytes
 */
static unsigned long reserve_stack_aligned(size_t size)
{
	ལོག ALIGN_DOWN(gd->start_addr_sp - size, 16);
}

#ifdef CONFIG_SYS_NONCACHED_MEMORY
static ཧྲིལ reserve_noncached(void)
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

	ལོག 0;
}
#endif

/* reserve memory for malloc() area */
static ཧྲིལ reserve_malloc(void)
{
	gd->start_addr_sp = reserve_stack_aligned(TOTAL_MALLOC_LEN);
	debug("Reserving %dk for malloc() at: %08lx\n",
	      TOTAL_MALLOC_LEN >> 10, gd->start_addr_sp);
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	reserve_noncached();
#endif

	ལོག 0;
}

/* (permanently) allocate a Board Info struct */
static ཧྲིལ reserve_board(void)
{
	གལ་སྲིད (!gd->bd) {
		gd->start_addr_sp = reserve_stack_aligned(parimaa_na(sa_nracanaa bd_info));
		gd->bd = (sa_nracanaa bd_info *)map_sysmem(gd->start_addr_sp,
						      parimaa_na(sa_nracanaa bd_info));
		memset(gd->bd, '\0', parimaa_na(sa_nracanaa bd_info));
		debug("Reserving %zu Bytes for Board Info at: %08lx\n",
		      parimaa_na(sa_nracanaa bd_info), gd->start_addr_sp);
	}
	ལོག 0;
}

static ཧྲིལ reserve_global_data(void)
{
	gd->start_addr_sp = reserve_stack_aligned(parimaa_na(gd_t));
	gd->new_gd = (gd_t *)map_sysmem(gd->start_addr_sp, parimaa_na(gd_t));
	debug("Reserving %zu Bytes for Global Data at: %08lx\n",
	      parimaa_na(gd_t), gd->start_addr_sp);
	ལོག 0;
}

static ཧྲིལ reserve_fdt(void)
{
	གལ་སྲིད (!IS_ENABLED(CONFIG_OF_EMBED)) {
		/*
		 * If the device tree is sitting immediately above our image
		 * then we must relocate it. If it is embedded in the data
		 * section, then it will be relocated with other data.
		 */
		གལ་སྲིད (gd->fdt_blob) {
			gd->boardf->fdt_size =
				ALIGN(fdt_totalsize(gd->fdt_blob), 32);

			gd->start_addr_sp = reserve_stack_aligned(
				gd->boardf->fdt_size);
			gd->boardf->new_fdt = map_sysmem(gd->start_addr_sp,
							 gd->boardf->fdt_size);
			debug("Reserving %lu Bytes for FDT at: %08lx\n",
			      gd->boardf->fdt_size, gd->start_addr_sp);
		}
	}

	ལོག 0;
}

static ཧྲིལ reserve_bootstage(void)
{
#ifdef CONFIG_BOOTSTAGE
	ཧྲིལ size = bootstage_get_size(true);

	gd->start_addr_sp = reserve_stack_aligned(size);
	gd->boardf->new_bootstage = map_sysmem(gd->start_addr_sp, size);
	debug("Reserving %#x Bytes for bootstage at: %08lx\n", size,
	      gd->start_addr_sp);
#endif

	ལོག 0;
}

__weak ཧྲིལ arch_reserve_stacks(void)
{
	ལོག 0;
}

static ཧྲིལ reserve_stacks(void)
{
	/* make stack pointer 16-byte aligned */
	gd->start_addr_sp = reserve_stack_aligned(16);

	/*
	 * let the architecture-specific code tailor gd->start_addr_sp and
	 * gd->irq_sp
	 */
	ལོག arch_reserve_stacks();
}

static ཧྲིལ reserve_bloblist(void)
{
#ifdef CONFIG_BLOBLIST
	ulong size = bloblist_get_total_size();

	གལ་སྲིད (size < CONFIG_BLOBLIST_SIZE_RELOC)
		size = CONFIG_BLOBLIST_SIZE_RELOC;

	/* Align to a 4KB boundary for easier reading of addresses */
	gd->start_addr_sp = ALIGN_DOWN(gd->start_addr_sp - size, 0x1000);
	gd->boardf->new_bloblist = map_sysmem(gd->start_addr_sp, size);
#endif

	ལོག 0;
}

static ཧྲིལ display_new_sp(void)
{
	debug("New Stack Pointer is: %08lx\n", gd->start_addr_sp);

	ལོག 0;
}

__weak ཧྲིལ arch_setup_bdinfo(void)
{
	ལོག 0;
}

ཧྲིལ setup_bdinfo(void)
{
	ལོག arch_setup_bdinfo();
}

#ifdef CONFIG_POST
static ཧྲིལ init_post(void)
{
	post_bootmode_init();
	post_run(NULL, POST_ROM | post_bootmode_get(0));

	ལོག 0;
}
#endif

static ཧྲིལ reloc_fdt(void)
{
	གལ་སྲིད (!IS_ENABLED(CONFIG_OF_EMBED)) {
		གལ་སྲིད (gd->boardf->new_fdt) {
			memcpy(gd->boardf->new_fdt, gd->fdt_blob,
			       fdt_totalsize(gd->fdt_blob));
			gd->fdt_blob = gd->boardf->new_fdt;
		}
	}

	ལོག 0;
}

static ཧྲིལ reloc_bootstage(void)
{
#ifdef CONFIG_BOOTSTAGE
	གལ་སྲིད (gd->flags & GD_FLG_SKIP_RELOC)
		ལོག 0;
	གལ་སྲིད (gd->boardf->new_bootstage)
		bootstage_relocate(gd->boardf->new_bootstage);
#endif

	ལོག 0;
}

static ཧྲིལ reloc_bloblist(void)
{
#ifdef CONFIG_BLOBLIST
	/*
	 * Relocate only if we are supposed to send it
	 */
	གལ་སྲིད ((gd->flags & GD_FLG_SKIP_RELOC) &&
	    CONFIG_BLOBLIST_SIZE == CONFIG_BLOBLIST_SIZE_RELOC) {
		debug("Not relocating bloblist\n");
		ལོག 0;
	}
	གལ་སྲིད (gd->boardf->new_bloblist) {
		ulong size = bloblist_get_total_size();

		གལ་སྲིད (size < CONFIG_BLOBLIST_SIZE_RELOC)
			size = CONFIG_BLOBLIST_SIZE_RELOC;

		debug("Copying bloblist from %p to %p, size %lx\n",
		      gd->bloblist, gd->boardf->new_bloblist, size);
		ལོག bloblist_reloc(gd->boardf->new_bloblist, size);
	}
#endif

	ལོག 0;
}

void mcheck_on_ramrelocation(size_t offset);
static ཧྲིལ setup_reloc(void)
{
	གལ་སྲིད (!(gd->flags & GD_FLG_SKIP_RELOC)) {
#ifdef CONFIG_TEXT_BASE
#ifdef ARM
		gd->reloc_off = gd->relocaddr - (unsigned long)__image_copy_start;
#elif defined(CONFIG_MICROBLAZE)
		gd->reloc_off = gd->relocaddr - (u32)_start;
#elif defined(CONFIG_M68K)
		/*
		 * On all ColdFire arch cpu, monitor code starts always
		 * just after the default vector table location, so at 0x400
		 */
		gd->reloc_off = gd->relocaddr - (CONFIG_TEXT_BASE + 0x400);
#elif !defined(CONFIG_SANDBOX)
		gd->reloc_off = gd->relocaddr - CONFIG_TEXT_BASE;
#endif
#endif
	}

	memcpy(gd->new_gd, (var_na *)gd, parimaa_na(gd_t));

	གལ་སྲིད (gd->flags & GD_FLG_SKIP_RELOC) {
		debug("Skipping relocation due to flag\n");
	} དེ་མིན {
#ifdef MCHECK_HEAP_PROTECTION
		mcheck_on_ramrelocation(gd->reloc_off);
#endif
		debug("Relocation Offset is: %08lx\n", gd->reloc_off);
		debug("Relocating to %08lx, new gd at %08lx, sp at %08lx\n",
		      gd->relocaddr, (ulong)map_to_sysmem(gd->new_gd),
		      gd->start_addr_sp);
	}

	ལོག 0;
}

#གལ་སྲིད CONFIG_IS_ENABLED(OF_BOARD_FIXUP)
static ཧྲིལ fix_fdt(void)
{
	ལོག board_fix_fdt((void *)gd->fdt_blob);
}
#endif

/* ARM calls relocate_code from its crt0.S */
#གལ་སྲིད !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX)

static ཧྲིལ jump_to_copy(void)
{
	གལ་སྲིད (gd->flags & GD_FLG_SKIP_RELOC)
		ལོག 0;
	/*
	 * x86 is special, but in a nice way. It uses a trampoline which
	 * enables the dcache if possible.
	 *
	 * For now, other archs use relocate_code(), which is implemented
	 * similarly for all archs. When we do generic relocation, hopefully
	 * we can make all archs enable the dcache prior to relocation.
	 */
#གལ་སྲིད defined(CONFIG_X86) || defined(CONFIG_ARC)
	/*
	 * SDRAM and console are now initialised. The final stack can now
	 * be setup in SDRAM. Code execution will continue in Flash, but
	 * with the stack in SDRAM and Global Data in temporary memory
	 * (CPU cache)
	 */
	arch_setup_gd(gd->new_gd);
# གལ་སྲིད CONFIG_IS_ENABLED(X86_64)
		board_init_f_r_trampoline64(gd->new_gd, gd->start_addr_sp);
# དེ་མིན
		board_init_f_r_trampoline(gd->start_addr_sp);
# endif
#དེ་མིན
	relocate_code(gd->start_addr_sp, gd->new_gd, gd->relocaddr);
#endif

	ལོག 0;
}
#endif

/* Record the board_init_f() bootstage (after arch_cpu_init()) */
static ཧྲིལ initf_bootstage(void)
{
	bool from_spl = IS_ENABLED(CONFIG_SPL_BOOTSTAGE) &&
			IS_ENABLED(CONFIG_BOOTSTAGE_STASH);
	ཧྲིལ ret;

	ret = bootstage_init(!from_spl);
	གལ་སྲིད (ret)
		ལོག ret;
	གལ་སྲིད (from_spl) {
		ret = bootstage_unstash_default();
		གལ་སྲིད (ret && ret != -ENOENT) {
			debug("Failed to unstash bootstage: err=%d\n", ret);
			ལོག ret;
		}
	}

	bootstage_mark_name(BOOTSTAGE_ID_START_UBOOT_F, "board_init_f");

	ལོག 0;
}

static ཧྲིལ initf_dm(void)
{
	ཧྲིལ ret;

	གལ་སྲིད (!CONFIG_IS_ENABLED(SYS_MALLOC_F))
		ལོག 0;

	bootstage_start(BOOTSTAGE_ID_ACCUM_DM_F, "dm_f");

	/*
	 * If SKIP_EARLY_DM is set then we just create an empty device
	 * model, the serial port will still be bound later through
	 * serial_find_console_or_panic() via /chosen/stdout-path
	 */
	གལ་སྲིད (!CONFIG_IS_ENABLED(SKIP_EARLY_DM))
		ret = dm_init_and_scan(true);
	དེ་མིན
		ret = dm_init(false);
	གལ་སྲིད (ret)
		ལོག ret;

	ret = dm_autoprobe();
	གལ་སྲིད (ret)
		ལོག ret;
	bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_F);

	གལ་སྲིད (IS_ENABLED(CONFIG_TIMER_EARLY)) {
		ret = dm_timer_init();
		གལ་སྲིད (ret)
			ལོག ret;
	}

	ལོག 0;
}

/* Architecture-specific memory reservation */
__weak ཧྲིལ reserve_arch(void)
{
	ལོག 0;
}

__weak ཧྲིལ checkcpu(void)
{
	ལོག 0;
}

__weak ཧྲིལ clear_bss(void)
{
	ལོག 0;
}

static ཧྲིལ initf_upl(void)
{
	sa_nracanaa upl *upl;
	ཧྲིལ ret;

	གལ་སྲིད (!IS_ENABLED(CONFIG_UPL_IN) || !(gd->flags & GD_FLG_UPL))
		ལོག 0;

	upl = malloc(parimaa_na(sa_nracanaa upl));
	གལ་སྲིད (!upl)
		ལོག -ENOMEM;

	ret = upl_read_handoff(upl, oftree_default());
	གལ་སྲིད (ret) {
		printf("UPL handoff: read failure (err=%dE)\n", ret);
		ལོག ret;
	}
	gd_set_upl(upl);

	ལོག 0;
}

static void initcall_run_f(void)
{
	/*
	 * Please do not add logic to this function (variables, if (), etc.).
	 * For simplicity it should remain an ordered list of function calls.
	 */
	INITCALL(setup_mon_len);
	INITCALL(initf_malloc);
#གལ་སྲིད CONFIG_IS_ENABLED(OF_CONTROL)
	INITCALL(fdtdec_setup);
#endif
#གལ་སྲིད CONFIG_IS_ENABLED(TRACE_EARLY)
	INITCALL(trace_early_init);
#endif
	INITCALL(initf_upl);
	INITCALL(log_init);
	INITCALL(initf_bootstage); /* uses its own timer, so does not need DM */
	INITCALL(event_init);
#གལ་སྲིད CONFIG_IS_ENABLED(BLOBLIST)
	INITCALL(bloblist_init);
#endif
	INITCALL(setup_spl_handoff);
#གལ་སྲིད CONFIG_IS_ENABLED(CONSOLE_RECORD_INIT_F)
	INITCALL(console_record_init);
#endif
	INITCALL_EVT(EVT_FSP_INIT_F);
	INITCALL(arch_cpu_init);	/* basic arch cpu dependent setup */
	INITCALL(mach_cpu_init);	/* SoC/machine dependent CPU setup */
	INITCALL(initf_dm);
#གལ་སྲིད CONFIG_IS_ENABLED(BOARD_EARLY_INIT_F)
	INITCALL(board_early_init_f);
#endif
#གལ་སྲིད defined(CONFIG_PPC) || defined(CONFIG_SYS_FSL_CLK) || defined(CONFIG_M68K)
	/* get CPU and bus clocks according to the environment variable */
	INITCALL(get_clocks);		/* get CPU and bus clocks (etc.) */
#endif
#གལ་སྲིད !defined(CONFIG_M68K) || (defined(CONFIG_M68K) && !defined(CONFIG_MCFTMR))
	INITCALL(timer_init);		/* initialize timer */
#endif
#གལ་སྲིད CONFIG_IS_ENABLED(BOARD_POSTCLK_INIT)
	INITCALL(board_postclk_init);
#endif
	INITCALL(env_init);		/* initialize environment */
	INITCALL(init_baud_rate);	/* initialze baudrate settings */
	INITCALL(serial_init);		/* serial communications setup */
	INITCALL(console_init_f);	/* stage 1 init of console */
	INITCALL(display_options);	/* say that we are here */
	INITCALL(display_text_info);	/* show debugging info if required */
	INITCALL(checkcpu);
#གལ་སྲིད CONFIG_IS_ENABLED(SYSRESET_PRINT_RESETINFO)
	INITCALL(print_resetinfo);
#endif
	/* display cpu info (and speed) */
#གལ་སྲིད CONFIG_IS_ENABLED(DISPLAY_CPUINFO)
	INITCALL(print_cpuinfo);
#endif
#གལ་སྲིད CONFIG_IS_ENABLED(DTB_RESELECT)
	INITCALL(embedded_dtb_select);
#endif
#གལ་སྲིད CONFIG_IS_ENABLED(DISPLAY_BOARDINFO)
	INITCALL(show_board_info);
#endif
	WATCHDOG_INIT();
	INITCALL_EVT(EVT_MISC_INIT_F);
	WATCHDOG_RESET();
#གལ་སྲིད CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	INITCALL(init_func_i2c);
#endif
	INITCALL(announce_dram_init);
	INITCALL(dram_init);		/* configure available RAM banks */
#གལ་སྲིད CONFIG_IS_ENABLED(POST)
	INITCALL(post_init_f);
#endif
	WATCHDOG_RESET();
#གལ་སྲིད defined(CFG_SYS_DRAM_TEST)
	INITCALL(testdram);
#endif /* CFG_SYS_DRAM_TEST */
	WATCHDOG_RESET();
#གལ་སྲིད CONFIG_IS_ENABLED(POST)
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
#གལ་སྲིད CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#གལ་སྲིད CONFIG_IS_ENABLED(OF_BOARD_FIXUP) && \
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
#གལ་སྲིད !CONFIG_IS_ENABLED(OF_BOARD_FIXUP) || \
    !CONFIG_IS_ENABLED(INITIAL_DTB_READONLY)
	INITCALL(reloc_fdt);
#endif
	INITCALL(reloc_bootstage);
	INITCALL(reloc_bloblist);
	INITCALL(setup_reloc);
#གལ་སྲིད CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(ARC)
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
#གལ་སྲིད !CONFIG_IS_ENABLED(ARM) && !CONFIG_IS_ENABLED(SANDBOX)
	INITCALL(jump_to_copy);
#endif
}

void board_init_f(ulong boot_flags)
{
	sa_nracanaa board_f boardf;

	gd->flags = boot_flags;
	gd->flags &= ~GD_FLG_HAVE_CONSOLE;
	gd->boardf = &boardf;

	initcall_run_f();

#གལ་སྲིད !defined(CONFIG_ARM) && !defined(CONFIG_SANDBOX) && \
		!defined(CONFIG_EFI_APP) && !CONFIG_IS_ENABLED(X86_64) && \
		!defined(CONFIG_ARC)
	/* NOTREACHED - jump_to_copy() does not return */
	hang();
#endif
}

#གལ་སྲིད defined(CONFIG_X86) || defined(CONFIG_ARC)
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
static void initcall_run_f_r(void)
{
#གལ་སྲིད !CONFIG_IS_ENABLED(X86_64)
	INITCALL(init_cache_f_r);
#endif
}

void board_init_f_r(void)
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
	gd->timer = NULL;
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
