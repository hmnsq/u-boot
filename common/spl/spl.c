// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#அந்தர்பாவ <config.h>
#அந்தர்பாவ <bloblist.h>
#அந்தர்பாவ <binman_sym.h>
#அந்தர்பாவ <bootstage.h>
#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <handoff.h>
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <init.h>
#அந்தர்பாவ <irq_func.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <of_live.h>
#அந்தர்பாவ <serial.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <spl_load.h>
#அந்தர்பாவ <system-constants.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm-generic/gpio.h>
#அந்தர்பாவ <nand.h>
#அந்தர்பாவ <fat.h>
#அந்தர்பாவ <u-boot/crc.h>
#எனில் CONFIG_IS_ENABLED(BANNER_PRINT)
#அந்தர்பாவ <timestamp.h>
#endif
#அந்தர்பாவ <version.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <dm/root.h>
#அந்தர்பாவ <dm/util.h>
#அந்தர்பாவ <dm/device-internal.h>
#அந்தர்பாவ <dm/uclass-internal.h>
#அந்தர்பாவ <linux/compiler.h>
#அந்தர்பாவ <fdt_support.h>
#அந்தர்பாவ <bootcount.h>
#அந்தர்பாவ <wdt.h>
#அந்தர்பாவ <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = வெற்று;

#எனில் CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#எனில் defined(CONFIG_SPL_BUILD)
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
நிலையான கட்டமைப்பு bd_info bdata __attribute__ ((section(".data")));

#எனில் CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak வெறுமை show_boot_progress(முழு val) {}
#endif

#எனில் defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak முழு dram_init_banksize(வெறுமை)
{
	திரும்பு 0;
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
#எனில் CONFIG_IS_ENABLED(OS_BOOT)
__weak முழு spl_start_uboot(வெறுமை)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	திரும்பு 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
முழு __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 திரும்பு 1;
}

முழு __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 திரும்பு 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
வெறுமை __weak spl_perform_arch_fixups(கட்டமைப்பு spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
வெறுமை __weak spl_perform_board_fixups(கட்டமைப்பு spl_image_info *spl_image)
{
}

வெறுமை spl_fixup_fdt(வெறுமை *fdt_blob)
{
#எனில் defined(CONFIG_SPL_OF_LIBFDT)
	முழு err;

	எனில் (!fdt_blob)
		திரும்பு;

	err = fdt_check_header(fdt_blob);
	எனில் (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		திரும்பு;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	எனில் (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		திரும்பு;
	}

	err = arch_fixup_fdt(fdt_blob);
	எனில் (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		திரும்பு;
	}
#endif
}

முழு spl_reserve_video_from_ram_உச்சி(வெறுமை)
{
	எனில் (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		முழு ret;

		addr = gd->ram_உச்சி;
		ret = video_reserve(&addr);
		எனில் (ret)
			திரும்பு ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	திரும்பு 0;
}

ulong spl_get_image_pos(வெறுமை)
{
	எனில் (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		திரும்பு BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	எனில் (xpl_next_phase() == PHASE_VPL)
		திரும்பு binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#எனில் defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	எனில் (xpl_next_phase() == PHASE_SPL)
		திரும்பு binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#எனில் defined(CONFIG_SPL_BUILD)
	திரும்பு binman_sym(ulong, u_boot_any, image_pos);
#endif

	திரும்பு BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(வெறுமை)
{
	எனில் (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		திரும்பு BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	எனில் (xpl_next_phase() == PHASE_VPL)
		திரும்பு binman_sym(ulong, u_boot_vpl_any, size);
#endif
	திரும்பு xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(வெறுமை)
{
#ifdef CONFIG_VPL
	எனில் (xpl_next_phase() == PHASE_VPL)
		திரும்பு CONFIG_VPL_TEXT_BASE;
#endif
	திரும்பு xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak வெறுமை spl_board_prepare_for_linux(வெறுமை)
{
	/* Nothing to do! */
}

__weak வெறுமை spl_board_prepare_for_optee(வெறுமை *fdt)
{
}

__weak மாறிலி எழுத்து *spl_board_loader_name(u32 boot_device)
{
	திரும்பு வெற்று;
}

#எனில் CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak வெறுமை __noreturn jump_to_image_optee(கட்டமைப்பு spl_image_info *spl_image)
{
	spl_optee_entry(வெற்று, வெற்று, spl_image->fdt_addr,
			(வெறுமை *)spl_image->entry_point);
}
#endif

__weak வெறுமை spl_board_prepare_for_boot(வெறுமை)
{
	/* Nothing to do! */
}

__weak கட்டமைப்பு legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	திரும்பு map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

வெறுமை spl_set_header_raw_uboot(கட்டமைப்பு spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#எனில் CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#இல்லையெனில்
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	எனில் (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} இல்லையெனில் {
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

__weak முழு spl_parse_board_header(கட்டமைப்பு spl_image_info *spl_image,
				  மாறிலி கட்டமைப்பு spl_boot_device *bootdev,
				  மாறிலி வெறுமை *image_header, size_t size)
{
	திரும்பு -EINVAL;
}

__weak முழு spl_parse_legacy_header(கட்டமைப்பு spl_image_info *spl_image,
				   மாறிலி கட்டமைப்பு legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	திரும்பு -EINVAL;
}

முழு spl_parse_image_header(கட்டமைப்பு spl_image_info *spl_image,
			   மாறிலி கட்டமைப்பு spl_boot_device *bootdev,
			   மாறிலி கட்டமைப்பு legacy_img_hdr *header)
{
	முழு ret;

	எனில் (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		எனில் (!ret)
			திரும்பு ret;
	}
	எனில் (image_get_magic(header) == IH_MAGIC) {
		முழு ret;

		ret = spl_parse_legacy_header(spl_image, header);
		எனில் (ret)
			திரும்பு ret;
		திரும்பு 0;
	}

	எனில் (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		எனில் (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			திரும்பு 0;
		}
	} இல்லையெனில் எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		எனில் (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			திரும்பு 0;
		}
	}

	எனில் (!spl_parse_board_header(spl_image, bootdev, (மாறிலி வெறுமை *)header,
				    அளவு(*header)))
		திரும்பு 0;

	எனில் (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} இல்லையெனில் {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		திரும்பு -EINVAL;
	}

	திரும்பு 0;
}

#எனில் SPL_LOAD_USERS > 1
முழு spl_load(கட்டமைப்பு spl_image_info *spl_image,
	     மாறிலி கட்டமைப்பு spl_boot_device *bootdev, கட்டமைப்பு spl_load_info *info,
	     size_t size, size_t offset)
{
	திரும்பு _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak வெறுமை __noreturn jump_to_image(கட்டமைப்பு spl_image_info *spl_image)
{
	வகைவரை வெறுமை __noreturn (*image_entry_noargs_t)(வெறுமை);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#எனில் CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
நிலையான முழு setup_spl_handoff(வெறுமை)
{
	கட்டமைப்பு spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, அளவு(கட்டமைப்பு spl_handoff));
	எனில் (!ho)
		திரும்பு -ENOENT;

	திரும்பு 0;
}

__weak முழு handoff_arch_save(கட்டமைப்பு spl_handoff *ho)
{
	திரும்பு 0;
}

நிலையான முழு write_spl_handoff(வெறுமை)
{
	கட்டமைப்பு spl_handoff *ho;
	முழு ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, அளவு(கட்டமைப்பு spl_handoff));
	எனில் (!ho)
		திரும்பு -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	எனில் (ret)
		திரும்பு ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	திரும்பு 0;
}
#இல்லையெனில்
நிலையான உள்வரி முழு setup_spl_handoff(வெறுமை) { திரும்பு 0; }
நிலையான உள்வரி முழு write_spl_handoff(வெறுமை) { திரும்பு 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
நிலையான எண்ணிக்கை bootstage_id get_bootstage_id(bool start)
{
	எண்ணிக்கை xpl_phase_t phase = xpl_phase();

	எனில் (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		திரும்பு start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	இல்லையெனில் எனில் (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		திரும்பு start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	இல்லையெனில்
		திரும்பு start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

நிலையான முழு spl_common_init(bool setup_malloc)
{
	முழு ret;

#எனில் CONFIG_IS_ENABLED(SYS_MALLOC_F)
	எனில் (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	எனில் (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		திரும்பு ret;
	}
	எனில் (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		எனில் (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#எனில் CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	எனில் (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		திரும்பு ret;
	}
#endif
	எனில் (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		எனில் (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			திரும்பு ret;
		}
	}
	எனில் (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (கட்டமைப்பு device_முனை **)gd_of_வேர்_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		எனில் (ret)
			திரும்பு ret;
	}
	எனில் (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		எனில் (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			திரும்பு ret;
		}

		ret = dm_autoprobe();
		எனில் (ret)
			திரும்பு ret;
	}

	திரும்பு 0;
}

வெறுமை spl_set_bd(வெறுமை)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	எனில் (!gd->bd)
		gd->bd = &bdata;
}

முழு spl_early_init(வெறுமை)
{
	முழு ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	எனில் (ret)
		திரும்பு ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	திரும்பு 0;
}

முழு spl_init(வெறுமை)
{
	முழு ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_அடுக்கு_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	எனில் (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		எனில் (ret)
			திரும்பு ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	திரும்பு 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak வெறுமை board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak முழு spl_check_board_image(கட்டமைப்பு spl_image_info *spl_image,
				 மாறிலி கட்டமைப்பு spl_boot_device *bootdev)
{
	திரும்பு 0;
}

நிலையான முழு spl_load_image(கட்டமைப்பு spl_image_info *spl_image,
			  கட்டமைப்பு spl_image_loader *loader)
{
	முழு ret;
	கட்டமைப்பு spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = வெற்று;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	எனில் (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned எழுத்து *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		எனில் (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	எனில் (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	திரும்பு ret;
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
நிலையான முழு boot_from_devices(கட்டமைப்பு spl_image_info *spl_image,
			     u32 spl_boot_list[], முழு மொத்தம்)
{
	கட்டமைப்பு spl_image_loader *drv =
		ll_entry_start(கட்டமைப்பு spl_image_loader, spl_image_loader);
	மாறிலி முழு n_ents =
		ll_entry_மொத்தம்(கட்டமைப்பு spl_image_loader, spl_image_loader);
	முழு ret = -ENODEV;
	முழு i;

	க்கு (i = 0; i < மொத்தம் && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		கட்டமைப்பு spl_image_loader *loader;
		முழு bootdev = spl_boot_list[i];

		எனில் (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		க்கு (loader = drv; loader != drv + n_ents; loader++) {
			எனில் (loader && bootdev != loader->boot_device)
				தொடர்;
			எனில் (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			எனில் (!ret) {
				spl_image->boot_device = bootdev;
				திரும்பு 0;
			}
			printf("Error: %d\n", ret);
		}

		எனில் (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			எனில் (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			இல்லையெனில்
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	திரும்பு ret;
}

வெறுமை board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	கட்டமைப்பு spl_image_info spl_image;
	முழு ret, os;
	வெறுமை *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	எனில் (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	எனில் (!(gd->flags & GD_FLG_SPL_INIT)) {
		எனில் (spl_init())
			hang();
	}
	timer_init();
	எனில் (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		எனில் (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	எனில் (CONFIG_IS_ENABLED(HANDOFF)) {
		முழு ret;

		ret = setup_spl_handoff();
		எனில் (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	எனில் (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	எனில் (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	எனில் (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	எனில் (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		எனில் (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	எனில் (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	எனில் (CONFIG_IS_ENABLED(DM_STATS)) {
		கட்டமைப்பு dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', அளவு(spl_image));
	எனில் (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (வெறுமை *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	எனில் (ret) {
		எனில் (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		இல்லையெனில்
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	எனில் (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} இல்லையெனில் எனில் (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} இல்லையெனில் எனில் (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} இல்லையெனில் எனில் (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} இல்லையெனில் எனில் (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		எனில் (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (வெறுமை *)SPL_PAYLOAD_ARGS_ADDR;
		இல்லையெனில்
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} இல்லையெனில் {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	எனில் (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	எனில் (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	எனில் (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		கட்டமைப்பு udevice *dev;
		முழு rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		எனில் (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			எனில் (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	எனில் (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		எனில் (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	எனில் (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		எனில் (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	எனில் (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		எனில் (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	எனில் (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		முழு ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		எனில் (ret) {
			எனில் (xpl_phase() == PHASE_VPL)
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
வெறுமை preloader_console_init(வெறுமை)
{
#எனில் CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#எனில் CONFIG_IS_ENABLED(BANNER_PRINT)
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
__weak வெறுமை spl_relocate_அடுக்கு_check(வெறுமை)
{
#எனில் CONFIG_IS_ENABLED(SYS_REPORT_அடுக்கு_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong அடுக்கு_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_அடுக்கு);
	u8 *ptr = (u8 *)அடுக்கு_bottom;
	ulong i;

	க்கு (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_அடுக்கு); i++) {
		எனில் (*ptr != CONFIG_VAL(SYS_அடுக்கு_F_CHECK_BYTE))
			நிறுத்து;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_அடுக்கு) - i);
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
ulong spl_relocate_அடுக்கு_gd(வெறுமை)
{
#எனில் CONFIG_IS_ENABLED(அடுக்கு_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_அடுக்கு_R_ADDR;

	எனில் (CONFIG_IS_ENABLED(SYS_REPORT_அடுக்கு_F_USAGE))
		spl_relocate_அடுக்கு_check();

#எனில் defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	எனில் (CONFIG_SPL_அடுக்கு_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_அடுக்கு_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_அடுக்கு_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(அளவு(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (வெறுமை *)gd, அளவு(gd_t));
#எனில் CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#எனில் CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#எனில் !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	திரும்பு ptr;
#இல்லையெனில்
	திரும்பு 0;
#endif
}

#எனில் defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
வெறுமை bootcount_store(ulong a)
{
}

ulong bootcount_load(வெறுமை)
{
	திரும்பு 0;
}
#endif
