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
#जर CONFIG_IS_ENABLED(BANNER_PRINT)
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

#जर CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#जर defined(CONFIG_SPL_BUILD)
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

#जर CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak शून्य show_boot_progress(पूर्णांक val) {}
#endif

#जर defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak पूर्णांक dram_init_banksize(शून्य)
{
	परत 0;
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
#जर CONFIG_IS_ENABLED(OS_BOOT)
__weak पूर्णांक spl_start_uboot(शून्य)
{
	माला_दो(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	माला_दो(PHASE_PROMPT "Direct Linux boot not active!\n");
	परत 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
पूर्णांक __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 परत 1;
}

पूर्णांक __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 परत 1;
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
#जर defined(CONFIG_SPL_OF_LIBFDT)
	पूर्णांक err;

	जर (!fdt_blob)
		परत;

	err = fdt_check_header(fdt_blob);
	जर (err < 0) {
		म_लिखो("fdt_root: %s\n", fdt_strerror(err));
		परत;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	जर (err == 0) {
		म_लिखो(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		परत;
	}

	err = arch_fixup_fdt(fdt_blob);
	जर (err) {
		म_लिखो(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		परत;
	}
#endif
}

पूर्णांक spl_reserve_video_from_ram_शीर्ष(शून्य)
{
	जर (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		पूर्णांक ret;

		addr = gd->ram_शीर्ष;
		ret = video_reserve(&addr);
		जर (ret)
			परत ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((अचिन्हित दीर्घ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	परत 0;
}

ulong spl_get_image_pos(शून्य)
{
	जर (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		परत BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	जर (xpl_next_phase() == PHASE_VPL)
		परत binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#जर defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	जर (xpl_next_phase() == PHASE_SPL)
		परत binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#जर defined(CONFIG_SPL_BUILD)
	परत binman_sym(ulong, u_boot_any, image_pos);
#endif

	परत BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(शून्य)
{
	जर (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		परत BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	जर (xpl_next_phase() == PHASE_VPL)
		परत binman_sym(ulong, u_boot_vpl_any, size);
#endif
	परत xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(शून्य)
{
#ifdef CONFIG_VPL
	जर (xpl_next_phase() == PHASE_VPL)
		परत CONFIG_VPL_TEXT_BASE;
#endif
	परत xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
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

__weak स्थिरांक अक्षर *spl_board_loader_name(u32 boot_device)
{
	परत रिक्त;
}

#जर CONFIG_IS_ENABLED(OPTEE_IMAGE)
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

__weak संरचना legacy_img_hdr *spl_get_load_buffer(ssize_t offset, माप_प्रकार size)
{
	परत map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

शून्य spl_set_header_raw_uboot(संरचना spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#जर CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#अन्यथा
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	जर (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} अन्यथा {
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

__weak पूर्णांक spl_parse_board_header(संरचना spl_image_info *spl_image,
				  स्थिरांक संरचना spl_boot_device *bootdev,
				  स्थिरांक शून्य *image_header, माप_प्रकार size)
{
	परत -EINVAL;
}

__weak पूर्णांक spl_parse_legacy_header(संरचना spl_image_info *spl_image,
				   स्थिरांक संरचना legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	परत -EINVAL;
}

पूर्णांक spl_parse_image_header(संरचना spl_image_info *spl_image,
			   स्थिरांक संरचना spl_boot_device *bootdev,
			   स्थिरांक संरचना legacy_img_hdr *header)
{
	पूर्णांक ret;

	जर (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		जर (!ret)
			परत ret;
	}
	जर (image_get_magic(header) == IH_MAGIC) {
		पूर्णांक ret;

		ret = spl_parse_legacy_header(spl_image, header);
		जर (ret)
			परत ret;
		परत 0;
	}

	जर (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	जर (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		जर (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			परत 0;
		}
	} अन्यथा जर (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		जर (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			परत 0;
		}
	}

	जर (!spl_parse_board_header(spl_image, bootdev, (स्थिरांक शून्य *)header,
				    आकार(*header)))
		परत 0;

	जर (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} अन्यथा {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		परत -EINVAL;
	}

	परत 0;
}

#जर SPL_LOAD_USERS > 1
पूर्णांक spl_load(संरचना spl_image_info *spl_image,
	     स्थिरांक संरचना spl_boot_device *bootdev, संरचना spl_load_info *info,
	     माप_प्रकार size, माप_प्रकार offset)
{
	परत _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak शून्य __noreturn jump_to_image(संरचना spl_image_info *spl_image)
{
	प्रकारव्याख्या शून्य __noreturn (*image_entry_noargs_t)(शून्य);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#जर CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
स्थिर पूर्णांक setup_spl_handoff(शून्य)
{
	संरचना spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, आकार(संरचना spl_handoff));
	जर (!ho)
		परत -ENOENT;

	परत 0;
}

__weak पूर्णांक handoff_arch_save(संरचना spl_handoff *ho)
{
	परत 0;
}

स्थिर पूर्णांक write_spl_handoff(शून्य)
{
	संरचना spl_handoff *ho;
	पूर्णांक ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, आकार(संरचना spl_handoff));
	जर (!ho)
		परत -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	जर (ret)
		परत ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	परत 0;
}
#अन्यथा
स्थिर ओळीत पूर्णांक setup_spl_handoff(शून्य) { परत 0; }
स्थिर ओळीत पूर्णांक write_spl_handoff(शून्य) { परत 0; }

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

	जर (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		परत start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	अन्यथा जर (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		परत start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	अन्यथा
		परत start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

स्थिर पूर्णांक spl_common_init(bool setup_malloc)
{
	पूर्णांक ret;

#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	जर (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	जर (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		परत ret;
	}
	जर (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		जर (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#जर CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	जर (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		परत ret;
	}
#endif
	जर (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		जर (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			परत ret;
		}
	}
	जर (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (संरचना device_गाठ **)gd_of_मूळ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		जर (ret)
			परत ret;
	}
	जर (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		जर (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			परत ret;
		}

		ret = dm_autoprobe();
		जर (ret)
			परत ret;
	}

	परत 0;
}

शून्य spl_set_bd(शून्य)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	जर (!gd->bd)
		gd->bd = &bdata;
}

पूर्णांक spl_early_init(शून्य)
{
	पूर्णांक ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	जर (ret)
		परत ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	परत 0;
}

पूर्णांक spl_init(शून्य)
{
	पूर्णांक ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_चळत_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	जर (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		जर (ret)
			परत ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	परत 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak शून्य board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak पूर्णांक spl_check_board_image(संरचना spl_image_info *spl_image,
				 स्थिरांक संरचना spl_boot_device *bootdev)
{
	परत 0;
}

स्थिर पूर्णांक spl_load_image(संरचना spl_image_info *spl_image,
			  संरचना spl_image_loader *loader)
{
	पूर्णांक ret;
	संरचना spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = रिक्त;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	जर (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (अचिन्हित अक्षर *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		जर (dcrc != spl_image->dcrc) {
			माला_दो("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	जर (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	परत ret;
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
स्थिर पूर्णांक boot_from_devices(संरचना spl_image_info *spl_image,
			     u32 spl_boot_list[], पूर्णांक मोजणी)
{
	संरचना spl_image_loader *drv =
		ll_entry_start(संरचना spl_image_loader, spl_image_loader);
	स्थिरांक पूर्णांक n_ents =
		ll_entry_मोजणी(संरचना spl_image_loader, spl_image_loader);
	पूर्णांक ret = -ENODEV;
	पूर्णांक i;

	साठी (i = 0; i < मोजणी && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		संरचना spl_image_loader *loader;
		पूर्णांक bootdev = spl_boot_list[i];

		जर (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		साठी (loader = drv; loader != drv + n_ents; loader++) {
			जर (loader && bootdev != loader->boot_device)
				सुरूठेवा;
			जर (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				म_लिखो("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			जर (!ret) {
				spl_image->boot_device = bootdev;
				परत 0;
			}
			म_लिखो("Error: %d\n", ret);
		}

		जर (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			जर (CONFIG_IS_ENABLED(SHOW_ERRORS))
				म_लिखो(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			अन्यथा
				म_लिखो(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	परत ret;
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
	पूर्णांक ret, os;
	शून्य *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	जर (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	जर (!(gd->flags & GD_FLG_SPL_INIT)) {
		जर (spl_init())
			hang();
	}
	timer_init();
	जर (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		जर (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			माला_दो(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	जर (CONFIG_IS_ENABLED(HANDOFF)) {
		पूर्णांक ret;

		ret = setup_spl_handoff();
		जर (ret) {
			माला_दो(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	जर (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	जर (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	जर (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	जर (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	जर (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		जर (ret)
			माला_दो(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	जर (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	जर (CONFIG_IS_ENABLED(DM_STATS)) {
		संरचना dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	स_रखो(&spl_image, '\0', आकार(spl_image));
	जर (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (शून्य *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	जर (ret) {
		जर (CONFIG_IS_ENABLED(SHOW_ERRORS))
			म_लिखो(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		अन्यथा
			माला_दो(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	जर (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} अन्यथा जर (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} अन्यथा जर (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} अन्यथा जर (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} अन्यथा जर (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		जर (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (शून्य *)SPL_PAYLOAD_ARGS_ADDR;
		अन्यथा
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} अन्यथा {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	जर (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	जर (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	जर (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		संरचना udevice *dev;
		पूर्णांक rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		जर (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			जर (rc)
				म_लिखो("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	जर (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		जर (ret)
			म_लिखो(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	जर (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		जर (ret) {
			म_लिखो(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	जर (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		जर (ret)
			म_लिखो("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	जर (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		पूर्णांक ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		जर (ret) {
			जर (xpl_phase() == PHASE_VPL)
				म_लिखो("jump failed %d\n", ret);
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
#जर CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#जर CONFIG_IS_ENABLED(BANNER_PRINT)
	माला_दो("\nU-Boot " PHASE_NAME " " PLAIN_VERSION " (" U_BOOT_DATE " - "
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
__weak शून्य spl_relocate_चळत_check(शून्य)
{
#जर CONFIG_IS_ENABLED(SYS_REPORT_चळत_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong चळत_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_चळत);
	u8 *ptr = (u8 *)चळत_bottom;
	ulong i;

	साठी (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_चळत); i++) {
		जर (*ptr != CONFIG_VAL(SYS_चळत_F_CHECK_BYTE))
			थांबा;
		ptr++;
	}
	म_लिखो("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_चळत) - i);
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
ulong spl_relocate_चळत_gd(शून्य)
{
#जर CONFIG_IS_ENABLED(चळत_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_चळत_R_ADDR;

	जर (CONFIG_IS_ENABLED(SYS_REPORT_चळत_F_USAGE))
		spl_relocate_चळत_check();

#जर defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	जर (CONFIG_SPL_चळत_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_चळत_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_चळत_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(आकार(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	स_नकल(new_gd, (शून्य *)gd, आकार(gd_t));
#जर CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#जर CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#जर !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	परत ptr;
#अन्यथा
	परत 0;
#endif
}

#जर defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
शून्य bootcount_store(ulong a)
{
}

ulong bootcount_load(शून्य)
{
	परत 0;
}
#endif
