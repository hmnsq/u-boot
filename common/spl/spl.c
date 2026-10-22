// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#अन्तर्भाव <config.h>
#अन्तर्भाव <bloblist.h>
#अन्तर्भाव <binman_sym.h>
#अन्तर्भाव <bootstage.h>
#अन्तर्भाव <dm.h>
#अन्तर्भाव <handoff.h>
#अन्तर्भाव <hang.h>
#अन्तर्भाव <init.h>
#अन्तर्भाव <irq_func.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <of_live.h>
#अन्तर्भाव <serial.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <system-constants.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm-generic/gpio.h>
#अन्तर्भाव <nand.h>
#अन्तर्भाव <fat.h>
#अन्तर्भाव <u-boot/crc.h>
#सचे CONFIG_IS_ENABLED(BANNER_PRINT)
#अन्तर्भाव <timestamp.h>
#endif
#अन्तर्भाव <version.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <dm/root.h>
#अन्तर्भाव <dm/util.h>
#अन्तर्भाव <dm/device-internal.h>
#अन्तर्भाव <dm/uclass-internal.h>
#अन्तर्भाव <linux/compiler.h>
#अन्तर्भाव <fdt_support.h>
#अन्तर्भाव <bootcount.h>
#अन्तर्भाव <wdt.h>
#अन्तर्भाव <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = रिक्त;

#सचे CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#सचे defined(CONFIG_SPL_BUILD)
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
स्थिर संरचना bd_info bdata __attribute__ ((section(".data")));

#सचे CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak शून्य show_boot_progress(पूर्णाङ्क val) {}
#endif

#सचे defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak पूर्णाङ्क dram_init_banksize(शून्य)
{
	पटिनिवत्त 0;
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
#सचे CONFIG_IS_ENABLED(OS_BOOT)
__weak पूर्णाङ्क spl_start_uboot(शून्य)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	पटिनिवत्त 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
पूर्णाङ्क __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 पटिनिवत्त 1;
}

पूर्णाङ्क __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 पटिनिवत्त 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
शून्य __weak spl_perform_arch_fixups(संरचना spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
शून्य __weak spl_perform_board_fixups(संरचना spl_image_info *spl_image)
{
}

शून्य spl_fixup_fdt(शून्य *fdt_blob)
{
#सचे defined(CONFIG_SPL_OF_LIBFDT)
	पूर्णाङ्क err;

	सचे (!fdt_blob)
		पटिनिवत्त;

	err = fdt_check_header(fdt_blob);
	सचे (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		पटिनिवत्त;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	सचे (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		पटिनिवत्त;
	}

	err = arch_fixup_fdt(fdt_blob);
	सचे (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		पटिनिवत्त;
	}
#endif
}

पूर्णाङ्क spl_reserve_video_from_ram_शिखरम्(शून्य)
{
	सचे (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		पूर्णाङ्क ret;

		addr = gd->ram_शिखरम्;
		ret = video_reserve(&addr);
		सचे (ret)
			पटिनिवत्त ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	पटिनिवत्त 0;
}

ulong spl_get_image_pos(शून्य)
{
	सचे (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		पटिनिवत्त BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	सचे (xpl_next_phase() == PHASE_VPL)
		पटिनिवत्त binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#सचे defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	सचे (xpl_next_phase() == PHASE_SPL)
		पटिनिवत्त binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#सचे defined(CONFIG_SPL_BUILD)
	पटिनिवत्त binman_sym(ulong, u_boot_any, image_pos);
#endif

	पटिनिवत्त BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(शून्य)
{
	सचे (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		पटिनिवत्त BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	सचे (xpl_next_phase() == PHASE_VPL)
		पटिनिवत्त binman_sym(ulong, u_boot_vpl_any, size);
#endif
	पटिनिवत्त xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(शून्य)
{
#ifdef CONFIG_VPL
	सचे (xpl_next_phase() == PHASE_VPL)
		पटिनिवत्त CONFIG_VPL_TEXT_BASE;
#endif
	पटिनिवत्त xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak शून्य spl_board_prepare_for_linux(शून्य)
{
	/* Nothing to do! */
}

__weak शून्य spl_board_prepare_for_optee(शून्य *fdt)
{
}

__weak ध्रुव वर्ण *spl_board_loader_name(u32 boot_device)
{
	पटिनिवत्त रिक्त;
}

#सचे CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak शून्य __noreturn jump_to_image_optee(संरचना spl_image_info *spl_image)
{
	spl_optee_entry(रिक्त, रिक्त, spl_image->fdt_addr,
			(शून्य *)spl_image->entry_point);
}
#endif

__weak शून्य spl_board_prepare_for_boot(शून्य)
{
	/* Nothing to do! */
}

__weak संरचना legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	पटिनिवत्त map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

शून्य spl_set_header_raw_uboot(संरचना spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#सचे CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#अञ्ञथा
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	सचे (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} अञ्ञथा {
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

__weak पूर्णाङ्क spl_parse_board_header(संरचना spl_image_info *spl_image,
				  ध्रुव संरचना spl_boot_device *bootdev,
				  ध्रुव शून्य *image_header, size_t size)
{
	पटिनिवत्त -EINVAL;
}

__weak पूर्णाङ्क spl_parse_legacy_header(संरचना spl_image_info *spl_image,
				   ध्रुव संरचना legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	पटिनिवत्त -EINVAL;
}

पूर्णाङ्क spl_parse_image_header(संरचना spl_image_info *spl_image,
			   ध्रुव संरचना spl_boot_device *bootdev,
			   ध्रुव संरचना legacy_img_hdr *header)
{
	पूर्णाङ्क ret;

	सचे (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		सचे (!ret)
			पटिनिवत्त ret;
	}
	सचे (image_get_magic(header) == IH_MAGIC) {
		पूर्णाङ्क ret;

		ret = spl_parse_legacy_header(spl_image, header);
		सचे (ret)
			पटिनिवत्त ret;
		पटिनिवत्त 0;
	}

	सचे (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		सचे (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			पटिनिवत्त 0;
		}
	} अञ्ञथा सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		सचे (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			पटिनिवत्त 0;
		}
	}

	सचे (!spl_parse_board_header(spl_image, bootdev, (ध्रुव शून्य *)header,
				    परिमाण(*header)))
		पटिनिवत्त 0;

	सचे (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} अञ्ञथा {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		पटिनिवत्त -EINVAL;
	}

	पटिनिवत्त 0;
}

#सचे SPL_LOAD_USERS > 1
पूर्णाङ्क spl_load(संरचना spl_image_info *spl_image,
	     ध्रुव संरचना spl_boot_device *bootdev, संरचना spl_load_info *info,
	     size_t size, size_t offset)
{
	पटिनिवत्त _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak शून्य __noreturn jump_to_image(संरचना spl_image_info *spl_image)
{
	प्रकारनिर्देश शून्य __noreturn (*image_entry_noargs_t)(शून्य);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#सचे CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
स्थिर पूर्णाङ्क setup_spl_handoff(शून्य)
{
	संरचना spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, परिमाण(संरचना spl_handoff));
	सचे (!ho)
		पटिनिवत्त -ENOENT;

	पटिनिवत्त 0;
}

__weak पूर्णाङ्क handoff_arch_save(संरचना spl_handoff *ho)
{
	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क write_spl_handoff(शून्य)
{
	संरचना spl_handoff *ho;
	पूर्णाङ्क ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, परिमाण(संरचना spl_handoff));
	सचे (!ho)
		पटिनिवत्त -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	सचे (ret)
		पटिनिवत्त ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	पटिनिवत्त 0;
}
#अञ्ञथा
स्थिर अन्तःस्थ पूर्णाङ्क setup_spl_handoff(शून्य) { पटिनिवत्त 0; }
स्थिर अन्तःस्थ पूर्णाङ्क write_spl_handoff(शून्य) { पटिनिवत्त 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
स्थिर गणना bootstage_id get_bootstage_id(bool start)
{
	गणना xpl_phase_t phase = xpl_phase();

	सचे (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		पटिनिवत्त start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	अञ्ञथा सचे (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		पटिनिवत्त start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	अञ्ञथा
		पटिनिवत्त start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

स्थिर पूर्णाङ्क spl_common_init(bool setup_malloc)
{
	पूर्णाङ्क ret;

#सचे CONFIG_IS_ENABLED(SYS_MALLOC_F)
	सचे (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	सचे (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		पटिनिवत्त ret;
	}
	सचे (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		सचे (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#सचे CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	सचे (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		पटिनिवत्त ret;
	}
#endif
	सचे (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		सचे (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			पटिनिवत्त ret;
		}
	}
	सचे (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (संरचना device_ग्रन्थिः **)gd_of_मूलम्_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		सचे (ret)
			पटिनिवत्त ret;
	}
	सचे (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		सचे (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			पटिनिवत्त ret;
		}

		ret = dm_autoprobe();
		सचे (ret)
			पटिनिवत्त ret;
	}

	पटिनिवत्त 0;
}

शून्य spl_set_bd(शून्य)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	सचे (!gd->bd)
		gd->bd = &bdata;
}

पूर्णाङ्क spl_early_init(शून्य)
{
	पूर्णाङ्क ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	सचे (ret)
		पटिनिवत्त ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	पटिनिवत्त 0;
}

पूर्णाङ्क spl_init(शून्य)
{
	पूर्णाङ्क ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_राशिः_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	सचे (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		सचे (ret)
			पटिनिवत्त ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	पटिनिवत्त 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak शून्य board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak पूर्णाङ्क spl_check_board_image(संरचना spl_image_info *spl_image,
				 ध्रुव संरचना spl_boot_device *bootdev)
{
	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क spl_load_image(संरचना spl_image_info *spl_image,
			  संरचना spl_image_loader *loader)
{
	पूर्णाङ्क ret;
	संरचना spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = रिक्त;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	सचे (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned वर्ण *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		सचे (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	सचे (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	पटिनिवत्त ret;
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
स्थिर पूर्णाङ्क boot_from_devices(संरचना spl_image_info *spl_image,
			     u32 spl_boot_list[], पूर्णाङ्क गणनम्)
{
	संरचना spl_image_loader *drv =
		ll_entry_start(संरचना spl_image_loader, spl_image_loader);
	ध्रुव पूर्णाङ्क n_ents =
		ll_entry_गणनम्(संरचना spl_image_loader, spl_image_loader);
	पूर्णाङ्क ret = -ENODEV;
	पूर्णाङ्क i;

	अत्थाय (i = 0; i < गणनम् && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		संरचना spl_image_loader *loader;
		पूर्णाङ्क bootdev = spl_boot_list[i];

		सचे (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		अत्थाय (loader = drv; loader != drv + n_ents; loader++) {
			सचे (loader && bootdev != loader->boot_device)
				अनुवर्त;
			सचे (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			सचे (!ret) {
				spl_image->boot_device = bootdev;
				पटिनिवत्त 0;
			}
			printf("Error: %d\n", ret);
		}

		सचे (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			सचे (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			अञ्ञथा
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	पटिनिवत्त ret;
}

शून्य board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	संरचना spl_image_info spl_image;
	पूर्णाङ्क ret, os;
	शून्य *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	सचे (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	सचे (!(gd->flags & GD_FLG_SPL_INIT)) {
		सचे (spl_init())
			hang();
	}
	timer_init();
	सचे (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		सचे (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	सचे (CONFIG_IS_ENABLED(HANDOFF)) {
		पूर्णाङ्क ret;

		ret = setup_spl_handoff();
		सचे (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	सचे (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	सचे (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	सचे (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	सचे (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		सचे (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	सचे (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	सचे (CONFIG_IS_ENABLED(DM_STATS)) {
		संरचना dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', परिमाण(spl_image));
	सचे (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (शून्य *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	सचे (ret) {
		सचे (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		अञ्ञथा
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	सचे (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} अञ्ञथा सचे (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} अञ्ञथा सचे (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} अञ्ञथा सचे (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} अञ्ञथा सचे (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		सचे (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (शून्य *)SPL_PAYLOAD_ARGS_ADDR;
		अञ्ञथा
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} अञ्ञथा {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	सचे (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	सचे (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	सचे (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		संरचना udevice *dev;
		पूर्णाङ्क rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		सचे (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			सचे (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	सचे (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		सचे (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	सचे (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		सचे (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	सचे (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		सचे (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	सचे (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		पूर्णाङ्क ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		सचे (ret) {
			सचे (xpl_phase() == PHASE_VPL)
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
शून्य preloader_console_init(शून्य)
{
#सचे CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#सचे CONFIG_IS_ENABLED(BANNER_PRINT)
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
__weak शून्य spl_relocate_राशिः_check(शून्य)
{
#सचे CONFIG_IS_ENABLED(SYS_REPORT_राशिः_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong राशिः_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_राशिः);
	u8 *ptr = (u8 *)राशिः_bottom;
	ulong i;

	अत्थाय (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_राशिः); i++) {
		सचे (*ptr != CONFIG_VAL(SYS_राशिः_F_CHECK_BYTE))
			भङ्ग;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_राशिः) - i);
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
ulong spl_relocate_राशिः_gd(शून्य)
{
#सचे CONFIG_IS_ENABLED(राशिः_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_राशिः_R_ADDR;

	सचे (CONFIG_IS_ENABLED(SYS_REPORT_राशिः_F_USAGE))
		spl_relocate_राशिः_check();

#सचे defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	सचे (CONFIG_SPL_राशिः_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_राशिः_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_राशिः_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(परिमाण(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (शून्य *)gd, परिमाण(gd_t));
#सचे CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#सचे CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#सचे !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	पटिनिवत्त ptr;
#अञ्ञथा
	पटिनिवत्त 0;
#endif
}

#सचे defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
शून्य bootcount_store(ulong a)
{
}

ulong bootcount_load(शून्य)
{
	पटिनिवत्त 0;
}
#endif
