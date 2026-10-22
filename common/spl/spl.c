// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#অন্তর্ভাব <config.h>
#অন্তর্ভাব <bloblist.h>
#অন্তর্ভাব <binman_sym.h>
#অন্তর্ভাব <bootstage.h>
#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <handoff.h>
#অন্তর্ভাব <hang.h>
#অন্তর্ভাব <init.h>
#অন্তর্ভাব <irq_func.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <of_live.h>
#অন্তর্ভাব <serial.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <spl_load.h>
#অন্তর্ভাব <system-constants.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <asm-generic/gpio.h>
#অন্তর্ভাব <nand.h>
#অন্তর্ভাব <fat.h>
#অন্তর্ভাব <u-boot/crc.h>
#যদি CONFIG_IS_ENABLED(BANNER_PRINT)
#অন্তর্ভাব <timestamp.h>
#endif
#অন্তর্ভাব <version.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <dm/root.h>
#অন্তর্ভাব <dm/util.h>
#অন্তর্ভাব <dm/device-internal.h>
#অন্তর্ভাব <dm/uclass-internal.h>
#অন্তর্ভাব <linux/compiler.h>
#অন্তর্ভাব <fdt_support.h>
#অন্তর্ভাব <bootcount.h>
#অন্তর্ভাব <wdt.h>
#অন্তর্ভাব <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = রিক্ত;

#যদি CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#যদি defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, size);
#endif

#ifdef CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, size);
#endif

#ifdef CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, size);
#endif

#endif /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
স্থির সংরচনা bd_info bdata __attribute__ ((section(".data")));

#যদি CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak শূন্য show_boot_progress(পূর্ণাঙ্ক val) {}
#endif

#যদি defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak পূর্ণাঙ্ক dram_init_banksize(শূন্য)
{
	প্রত্যাগম 0;
}
#endif

/*
 * Default function to determine if u-boot or the OS should
 * be started. This implementation always returns 1.
 *
 * Please implement your own board specific funcion to do this.
 *
 * RETURN
 * 0 to not start u-boot
 * positive if u-boot should start
 */
#যদি CONFIG_IS_ENABLED(OS_BOOT)
__weak পূর্ণাঙ্ক spl_start_uboot(শূন্য)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	প্রত্যাগম 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
পূর্ণাঙ্ক __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 প্রত্যাগম 1;
}

পূর্ণাঙ্ক __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 প্রত্যাগম 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
শূন্য __weak spl_perform_arch_fixups(সংরচনা spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
শূন্য __weak spl_perform_board_fixups(সংরচনা spl_image_info *spl_image)
{
}

শূন্য spl_fixup_fdt(শূন্য *fdt_blob)
{
#যদি defined(CONFIG_SPL_OF_LIBFDT)
	পূর্ণাঙ্ক err;

	যদি (!fdt_blob)
		প্রত্যাগম;

	err = fdt_check_header(fdt_blob);
	যদি (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		প্রত্যাগম;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	যদি (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		প্রত্যাগম;
	}

	err = arch_fixup_fdt(fdt_blob);
	যদি (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		প্রত্যাগম;
	}
#endif
}

পূর্ণাঙ্ক spl_reserve_video_from_ram_শীর্ষ(শূন্য)
{
	যদি (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		পূর্ণাঙ্ক ret;

		addr = gd->ram_শীর্ষ;
		ret = video_reserve(&addr);
		যদি (ret)
			প্রত্যাগম ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	প্রত্যাগম 0;
}

ulong spl_get_image_pos(শূন্য)
{
	যদি (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		প্রত্যাগম BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	যদি (xpl_next_phase() == PHASE_VPL)
		প্রত্যাগম binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#যদি defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	যদি (xpl_next_phase() == PHASE_SPL)
		প্রত্যাগম binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#যদি defined(CONFIG_SPL_BUILD)
	প্রত্যাগম binman_sym(ulong, u_boot_any, image_pos);
#endif

	প্রত্যাগম BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(শূন্য)
{
	যদি (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		প্রত্যাগম BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	যদি (xpl_next_phase() == PHASE_VPL)
		প্রত্যাগম binman_sym(ulong, u_boot_vpl_any, size);
#endif
	প্রত্যাগম xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(শূন্য)
{
#ifdef CONFIG_VPL
	যদি (xpl_next_phase() == PHASE_VPL)
		প্রত্যাগম CONFIG_VPL_TEXT_BASE;
#endif
	প্রত্যাগম xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak শূন্য spl_board_prepare_for_linux(শূন্য)
{
	/* Nothing to do! */
}

__weak শূন্য spl_board_prepare_for_optee(শূন্য *fdt)
{
}

__weak ধ্রুব বর্ণ *spl_board_loader_name(u32 boot_device)
{
	প্রত্যাগম রিক্ত;
}

#যদি CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak শূন্য __noreturn jump_to_image_optee(সংরচনা spl_image_info *spl_image)
{
	spl_optee_entry(রিক্ত, রিক্ত, spl_image->fdt_addr,
			(শূন্য *)spl_image->entry_point);
}
#endif

__weak শূন্য spl_board_prepare_for_boot(শূন্য)
{
	/* Nothing to do! */
}

__weak সংরচনা legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	প্রত্যাগম map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

শূন্য spl_set_header_raw_uboot(সংরচনা spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#যদি CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#অন্যথা
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	যদি (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} অন্যথা {
		spl_image->entry_point = CONFIG_SYS_UBOOT_START;
		spl_image->load_addr = CONFIG_TEXT_BASE;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->load_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_next_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->load_addr, (ulong)spl_image->size);
}

__weak পূর্ণাঙ্ক spl_parse_board_header(সংরচনা spl_image_info *spl_image,
				  ধ্রুব সংরচনা spl_boot_device *bootdev,
				  ধ্রুব শূন্য *image_header, size_t size)
{
	প্রত্যাগম -EINVAL;
}

__weak পূর্ণাঙ্ক spl_parse_legacy_header(সংরচনা spl_image_info *spl_image,
				   ধ্রুব সংরচনা legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	প্রত্যাগম -EINVAL;
}

পূর্ণাঙ্ক spl_parse_image_header(সংরচনা spl_image_info *spl_image,
			   ধ্রুব সংরচনা spl_boot_device *bootdev,
			   ধ্রুব সংরচনা legacy_img_hdr *header)
{
	পূর্ণাঙ্ক ret;

	যদি (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		যদি (!ret)
			প্রত্যাগম ret;
	}
	যদি (image_get_magic(header) == IH_MAGIC) {
		পূর্ণাঙ্ক ret;

		ret = spl_parse_legacy_header(spl_image, header);
		যদি (ret)
			প্রত্যাগম ret;
		প্রত্যাগম 0;
	}

	যদি (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
		/*
		 * CONFIG_SPL_PANIC_ON_RAW_IMAGE is defined when the
		 * code which loads images in SPL cannot guarantee that
		 * absolutely all read errors will be reported.
		 * An example is the LPC32XX MLC NAND driver, which
		 * will consider that a completely unreadable NAND block
		 * is bad, and thus should be skipped silently.
		 */
		panic("** no mkimage signature but raw image not supported");
	}

	যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		যদি (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			প্রত্যাগম 0;
		}
	} অন্যথা যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		যদি (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			প্রত্যাগম 0;
		}
	}

	যদি (!spl_parse_board_header(spl_image, bootdev, (ধ্রুব শূন্য *)header,
				    পরিমাণ(*header)))
		প্রত্যাগম 0;

	যদি (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} অন্যথা {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		প্রত্যাগম -EINVAL;
	}

	প্রত্যাগম 0;
}

#যদি SPL_LOAD_USERS > 1
পূর্ণাঙ্ক spl_load(সংরচনা spl_image_info *spl_image,
	     ধ্রুব সংরচনা spl_boot_device *bootdev, সংরচনা spl_load_info *info,
	     size_t size, size_t offset)
{
	প্রত্যাগম _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak শূন্য __noreturn jump_to_image(সংরচনা spl_image_info *spl_image)
{
	প্রকারনির্দেশ শূন্য __noreturn (*image_entry_noargs_t)(শূন্য);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#যদি CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
স্থির পূর্ণাঙ্ক setup_spl_handoff(শূন্য)
{
	সংরচনা spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, পরিমাণ(সংরচনা spl_handoff));
	যদি (!ho)
		প্রত্যাগম -ENOENT;

	প্রত্যাগম 0;
}

__weak পূর্ণাঙ্ক handoff_arch_save(সংরচনা spl_handoff *ho)
{
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক write_spl_handoff(শূন্য)
{
	সংরচনা spl_handoff *ho;
	পূর্ণাঙ্ক ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, পরিমাণ(সংরচনা spl_handoff));
	যদি (!ho)
		প্রত্যাগম -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	যদি (ret)
		প্রত্যাগম ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	প্রত্যাগম 0;
}
#অন্যথা
স্থির অন্তঃস্থ পূর্ণাঙ্ক setup_spl_handoff(শূন্য) { প্রত্যাগম 0; }
স্থির অন্তঃস্থ পূর্ণাঙ্ক write_spl_handoff(শূন্য) { প্রত্যাগম 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
স্থির গণনা bootstage_id get_bootstage_id(bool start)
{
	গণনা xpl_phase_t phase = xpl_phase();

	যদি (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		প্রত্যাগম start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	অন্যথা যদি (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		প্রত্যাগম start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	অন্যথা
		প্রত্যাগম start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

স্থির পূর্ণাঙ্ক spl_common_init(bool setup_malloc)
{
	পূর্ণাঙ্ক ret;

#যদি CONFIG_IS_ENABLED(SYS_MALLOC_F)
	যদি (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	যদি (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		প্রত্যাগম ret;
	}
	যদি (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		যদি (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#যদি CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	যদি (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		প্রত্যাগম ret;
	}
#endif
	যদি (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		যদি (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			প্রত্যাগম ret;
		}
	}
	যদি (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (সংরচনা device_গ্রন্থি **)gd_of_মূল_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		যদি (ret)
			প্রত্যাগম ret;
	}
	যদি (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		যদি (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			প্রত্যাগম ret;
		}

		ret = dm_autoprobe();
		যদি (ret)
			প্রত্যাগম ret;
	}

	প্রত্যাগম 0;
}

শূন্য spl_set_bd(শূন্য)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	যদি (!gd->bd)
		gd->bd = &bdata;
}

পূর্ণাঙ্ক spl_early_init(শূন্য)
{
	পূর্ণাঙ্ক ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	যদি (ret)
		প্রত্যাগম ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক spl_init(শূন্য)
{
	পূর্ণাঙ্ক ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_স্তূপ_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	যদি (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		যদি (ret)
			প্রত্যাগম ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	প্রত্যাগম 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak শূন্য board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak পূর্ণাঙ্ক spl_check_board_image(সংরচনা spl_image_info *spl_image,
				 ধ্রুব সংরচনা spl_boot_device *bootdev)
{
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক spl_load_image(সংরচনা spl_image_info *spl_image,
			  সংরচনা spl_image_loader *loader)
{
	পূর্ণাঙ্ক ret;
	সংরচনা spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = রিক্ত;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	যদি (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned বর্ণ *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		যদি (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	যদি (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	প্রত্যাগম ret;
}

/**
 * boot_from_devices() - Try loading a booting U-Boot from a list of devices
 *
 * @spl_image: Place to put the image details if successful
 * @spl_boot_list: List of boot devices to try
 * @count: Number of elements in spl_boot_list
 * Return: 0 if OK, -ENODEV if there were no boot devices
 *	if CONFIG_SHOW_ERRORS is enabled, returns -ENXIO if there were
 *	devices but none worked
 */
স্থির পূর্ণাঙ্ক boot_from_devices(সংরচনা spl_image_info *spl_image,
			     u32 spl_boot_list[], পূর্ণাঙ্ক গুনতি)
{
	সংরচনা spl_image_loader *drv =
		ll_entry_start(সংরচনা spl_image_loader, spl_image_loader);
	ধ্রুব পূর্ণাঙ্ক n_ents =
		ll_entry_গুনতি(সংরচনা spl_image_loader, spl_image_loader);
	পূর্ণাঙ্ক ret = -ENODEV;
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < গুনতি && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		সংরচনা spl_image_loader *loader;
		পূর্ণাঙ্ক bootdev = spl_boot_list[i];

		যদি (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		প্রতি (loader = drv; loader != drv + n_ents; loader++) {
			যদি (loader && bootdev != loader->boot_device)
				অনুবর্ত;
			যদি (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			যদি (!ret) {
				spl_image->boot_device = bootdev;
				প্রত্যাগম 0;
			}
			printf("Error: %d\n", ret);
		}

		যদি (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			যদি (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			অন্যথা
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	প্রত্যাগম ret;
}

শূন্য board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	সংরচনা spl_image_info spl_image;
	পূর্ণাঙ্ক ret, os;
	শূন্য *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	যদি (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	যদি (!(gd->flags & GD_FLG_SPL_INIT)) {
		যদি (spl_init())
			hang();
	}
	timer_init();
	যদি (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		যদি (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	যদি (CONFIG_IS_ENABLED(HANDOFF)) {
		পূর্ণাঙ্ক ret;

		ret = setup_spl_handoff();
		যদি (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	যদি (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	যদি (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	যদি (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	যদি (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		যদি (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	যদি (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	যদি (CONFIG_IS_ENABLED(DM_STATS)) {
		সংরচনা dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', পরিমাণ(spl_image));
	যদি (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (শূন্য *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	যদি (ret) {
		যদি (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		অন্যথা
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	যদি (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} অন্যথা যদি (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} অন্যথা যদি (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} অন্যথা যদি (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} অন্যথা যদি (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		যদি (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (শূন্য *)SPL_PAYLOAD_ARGS_ADDR;
		অন্যথা
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} অন্যথা {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	যদি (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	যদি (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	যদি (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		সংরচনা udevice *dev;
		পূর্ণাঙ্ক rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		যদি (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			যদি (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	যদি (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		যদি (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	যদি (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		যদি (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	যদি (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		যদি (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	যদি (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		পূর্ণাঙ্ক ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		যদি (ret) {
			যদি (xpl_phase() == PHASE_VPL)
				printf("jump failed %d\n", ret);
			hang();
		}
	}

	jumper(&spl_image);
}

/*
 * This requires UART clocks to be enabled.  In order for this to work the
 * caller must ensure that the gd pointer is valid.
 */
শূন্য preloader_console_init(শূন্য)
{
#যদি CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#যদি CONFIG_IS_ENABLED(BANNER_PRINT)
	puts("\nU-Boot " PHASE_NAME " " PLAIN_VERSION " (" U_BOOT_DATE " - "
	     U_BOOT_TIME " " U_BOOT_TZ ")\n");
#endif
#ifdef CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#endif
#endif
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak শূন্য spl_relocate_স্তূপ_check(শূন্য)
{
#যদি CONFIG_IS_ENABLED(SYS_REPORT_স্তূপ_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong স্তূপ_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_স্তূপ);
	u8 *ptr = (u8 *)স্তূপ_bottom;
	ulong i;

	প্রতি (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_স্তূপ); i++) {
		যদি (*ptr != CONFIG_VAL(SYS_স্তূপ_F_CHECK_BYTE))
			ভঙ্গ;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_স্তূপ) - i);
#endif
}

/**
 * spl_relocate_stack_gd() - Relocate stack ready for board_init_r() execution
 *
 * Sometimes board_init_f() runs with a stack in SRAM but we want to use SDRAM
 * for the main board_init_r() execution. This is typically because we need
 * more stack space for things like the MMC sub-system.
 *
 * This function calculates the stack position, copies the global_data into
 * place, sets the new gd (except for ARM, for which setting GD within a C
 * function may not always work) and returns the new stack position. The
 * caller is responsible for setting up the sp register and, in the case
 * of ARM, setting up gd.
 *
 * All of this is done using the same layout and alignments as done in
 * board_init_f_init_reserve() / board_init_f_alloc_reserve().
 *
 * Return: new stack location, or 0 to use the same stack
 */
ulong spl_relocate_স্তূপ_gd(শূন্য)
{
#যদি CONFIG_IS_ENABLED(স্তূপ_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_স্তূপ_R_ADDR;

	যদি (CONFIG_IS_ENABLED(SYS_REPORT_স্তূপ_F_USAGE))
		spl_relocate_স্তূপ_check();

#যদি defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	যদি (CONFIG_SPL_স্তূপ_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_স্তূপ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_স্তূপ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(পরিমাণ(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (শূন্য *)gd, পরিমাণ(gd_t));
#যদি CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#যদি CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#যদি !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	প্রত্যাগম ptr;
#অন্যথা
	প্রত্যাগম 0;
#endif
}

#যদি defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
শূন্য bootcount_store(ulong a)
{
}

ulong bootcount_load(শূন্য)
{
	প্রত্যাগম 0;
}
#endif
