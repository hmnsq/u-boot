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
#यदि CONFIG_IS_ENABLED(BANNER_PRINT)
#अन्तर्भाव <timestamp.h>
#यदि_अन्तः
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

DECLARE_GLOBAL_दत्त_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = रिक्त;

#यदि CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#यदि defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, आकार);
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, आकार);
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, आकार);
#यदि_अन्तः

#यदि_अन्तः /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
स्थिर संरचना bd_info bdata __attribute__ ((section(".data")));

#यदि CONFIG_IS_ENABLED(SHOW_BOOT_प्रगतिः)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak शून्य show_boot_प्रगतिः(पूर्णाङ्क val) {}
#यदि_अन्तः

#यदि defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak पूर्णाङ्क dram_init_banksize(शून्य)
{
	प्रत्यागम 0;
}
#यदि_अन्तः

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
#यदि CONFIG_IS_ENABLED(OS_BOOT)
__weak पूर्णाङ्क spl_start_uboot(शून्य)
{
	पङ्क्तिं_लिख(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	पङ्क्तिं_लिख(PHASE_PROMPT "Direct Linux boot not active!\n");
	प्रत्यागम 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
पूर्णाङ्क __weak bootz_सज्जता(ulong image, ulong *start, ulong *अन्तः_2)
{
	 प्रत्यागम 1;
}

पूर्णाङ्क __weak booti_सज्जता(ulong image, ulong *relocated_addr, ulong *आकार, bool force_reloc)
{
	 प्रत्यागम 1;
}
#यदि_अन्तः

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
#यदि defined(CONFIG_SPL_सम्बन्धि_LIBFDT)
	पूर्णाङ्क err;

	यदि (!fdt_blob)
		प्रत्यागम;

	err = fdt_check_शीर्षकम्(fdt_blob);
	यदि (err < 0) {
		प्रारूप_लिख("fdt_root: %s\n", fdt_strerror(err));
		प्रत्यागम;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_गन्तव्यम्_minimum(fdt_blob, 0);
	यदि (err == 0) {
		प्रारूप_लिख(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		प्रत्यागम;
	}

	err = arch_fixup_fdt(fdt_blob);
	यदि (err) {
		प्रारूप_लिख(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		प्रत्यागम;
	}
#यदि_अन्तः
}

पूर्णाङ्क spl_reserve_video_from_ram_शिखरम्(शून्य)
{
	यदि (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		पूर्णाङ्क ret;

		addr = gd->ram_शिखरम्;
		ret = video_reserve(&addr);
		यदि (ret)
			प्रत्यागम ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((अचिह्नित दीर्घ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	प्रत्यागम 0;
}

ulong spl_get_image_pos(शून्य)
{
	यदि (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		प्रत्यागम BINMAN_SYM_MISSING;

#यदि_परिभाषितम् CONFIG_VPL
	यदि (xpl_अग्र_phase() == PHASE_VPL)
		प्रत्यागम binman_sym(ulong, u_boot_vpl_any, image_pos);
#यदि_अन्तः
#यदि defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	यदि (xpl_अग्र_phase() == PHASE_SPL)
		प्रत्यागम binman_sym(ulong, u_boot_spl_any, image_pos);
#यदि_अन्तः
#यदि defined(CONFIG_SPL_BUILD)
	प्रत्यागम binman_sym(ulong, u_boot_any, image_pos);
#यदि_अन्तः

	प्रत्यागम BINMAN_SYM_MISSING;
}

ulong spl_get_image_आकार(शून्य)
{
	यदि (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		प्रत्यागम BINMAN_SYM_MISSING;

#यदि_परिभाषितम् CONFIG_VPL
	यदि (xpl_अग्र_phase() == PHASE_VPL)
		प्रत्यागम binman_sym(ulong, u_boot_vpl_any, आकार);
#यदि_अन्तः
	प्रत्यागम xpl_अग्र_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, आकार) :
		binman_sym(ulong, u_boot_any, आकार);
}

ulong spl_get_image_text_आधारः(शून्य)
{
#यदि_परिभाषितम् CONFIG_VPL
	यदि (xpl_अग्र_phase() == PHASE_VPL)
		प्रत्यागम CONFIG_VPL_TEXT_आधारः;
#यदि_अन्तः
	प्रत्यागम xpl_अग्र_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_आधारः :
		CONFIG_TEXT_आधारः;
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
	प्रत्यागम रिक्त;
}

#यदि CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak शून्य __noreturn jump_गन्तव्यम्_image_optee(संरचना spl_image_info *spl_image)
{
	spl_optee_प्रविष्टिः(रिक्त, रिक्त, spl_image->fdt_addr,
			(शून्य *)spl_image->प्रविष्टिः_point);
}
#यदि_अन्तः

__weak शून्य spl_board_prepare_for_boot(शून्य)
{
	/* Nothing to do! */
}

__weak संरचना legacy_img_hdr *spl_get_आरोपय_बफर(ssize_t विस्थापनम्, size_t आकार)
{
	प्रत्यागम map_sysmem(CONFIG_TEXT_आधारः + विस्थापनम्, 0);
}

शून्य spl_समुच्चयः_शीर्षकम्_raw_uboot(संरचना spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#यदि CONFIG_SYS_MONITOR_LEN != 0
	spl_image->आकार = CONFIG_SYS_MONITOR_LEN;
#अन्यथा
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->आकार = 200 * 1024;
#यदि_अन्तः

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	यदि (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->प्रविष्टिः_point = spl_get_image_text_आधारः();
		spl_image->आरोपय_addr = spl_get_image_text_आधारः();
		spl_image->आकार = spl_get_image_आकार();
		log_debug("Next load addr %lx\n", spl_image->आरोपय_addr);
	} अन्यथा {
		spl_image->प्रविष्टिः_point = CONFIG_SYS_UBOOT_START;
		spl_image->आरोपय_addr = CONFIG_TEXT_आधारः;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->आरोपय_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_अग्र_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->आरोपय_addr, (ulong)spl_image->आकार);
}

__weak पूर्णाङ्क spl_parse_board_शीर्षकम्(संरचना spl_image_info *spl_image,
				  ध्रुव संरचना spl_boot_device *bootdev,
				  ध्रुव शून्य *image_शीर्षकम्, size_t आकार)
{
	प्रत्यागम -EINVAL;
}

__weak पूर्णाङ्क spl_parse_legacy_शीर्षकम्(संरचना spl_image_info *spl_image,
				   ध्रुव संरचना legacy_img_hdr *शीर्षकम्)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	प्रत्यागम -EINVAL;
}

पूर्णाङ्क spl_parse_image_शीर्षकम्(संरचना spl_image_info *spl_image,
			   ध्रुव संरचना spl_boot_device *bootdev,
			   ध्रुव संरचना legacy_img_hdr *शीर्षकम्)
{
	पूर्णाङ्क ret;

	यदि (CONFIG_IS_ENABLED(आरोपय_FIT_पूर्णम्)) {
		ret = spl_आरोपय_fit_image(spl_image, शीर्षकम्);

		यदि (!ret)
			प्रत्यागम ret;
	}
	यदि (image_get_magic(शीर्षकम्) == IH_MAGIC) {
		पूर्णाङ्क ret;

		ret = spl_parse_legacy_शीर्षकम्(spl_image, शीर्षकम्);
		यदि (ret)
			प्रत्यागम ret;
		प्रत्यागम 0;
	}

	यदि (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, आकार;

		यदि (!booti_सज्जता((ulong)शीर्षकम्, &start, &आकार, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->आरोपय_addr = start;
			spl_image->प्रविष्टिः_point = start;
			spl_image->आकार = आकार;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->आरोपय_addr, spl_image->आकार);
			प्रत्यागम 0;
		}
	} अन्यथा यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, अन्तः_2;

		यदि (!bootz_सज्जता((ulong)शीर्षकम्, &start, &अन्तः_2)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->आरोपय_addr = CONFIG_SYS_आरोपय_ADDR;
			spl_image->प्रविष्टिः_point = CONFIG_SYS_आरोपय_ADDR;
			spl_image->आकार = अन्तः_2 - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->आरोपय_addr, spl_image->आकार);
			प्रत्यागम 0;
		}
	}

	यदि (!spl_parse_board_शीर्षकम्(spl_image, bootdev, (ध्रुव शून्य *)शीर्षकम्,
				    परिमाण(*शीर्षकम्)))
		प्रत्यागम 0;

	यदि (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      शीर्षकम्->ih_magic);
		spl_समुच्चयः_शीर्षकम्_raw_uboot(spl_image);
	} अन्यथा {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

#यदि SPL_आरोपय_USERS > 1
पूर्णाङ्क spl_आरोपय(संरचना spl_image_info *spl_image,
	     ध्रुव संरचना spl_boot_device *bootdev, संरचना spl_आरोपय_info *info,
	     size_t आकार, size_t विस्थापनम्)
{
	प्रत्यागम spl_आरोपय_2(spl_image, bootdev, info, आकार, विस्थापनम्);
}
#यदि_अन्तः

__weak शून्य __noreturn jump_गन्तव्यम्_image(संरचना spl_image_info *spl_image)
{
	प्रकारनिर्देश शून्य __noreturn (*image_प्रविष्टिः_noargs_t)(शून्य);

	image_प्रविष्टिः_noargs_t image_प्रविष्टिः =
		(image_प्रविष्टिः_noargs_t)spl_image->प्रविष्टिः_point;

	debug("image entry point: 0x%lx\n", spl_image->प्रविष्टिः_point);
	image_प्रविष्टिः();
}

#यदि CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
स्थिर पूर्णाङ्क सज्जता_spl_handoff(शून्य)
{
	संरचना spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, परिमाण(संरचना spl_handoff));
	यदि (!ho)
		प्रत्यागम -ENOENT;

	प्रत्यागम 0;
}

__weak पूर्णाङ्क handoff_arch_save(संरचना spl_handoff *ho)
{
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क write_spl_handoff(शून्य)
{
	संरचना spl_handoff *ho;
	पूर्णाङ्क ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, परिमाण(संरचना spl_handoff));
	यदि (!ho)
		प्रत्यागम -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	यदि (ret)
		प्रत्यागम ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	प्रत्यागम 0;
}
#अन्यथा
स्थिर अन्तःस्थ पूर्णाङ्क सज्जता_spl_handoff(शून्य) { प्रत्यागम 0; }
स्थिर अन्तःस्थ पूर्णाङ्क write_spl_handoff(शून्य) { प्रत्यागम 0; }

#यदि_अन्तः /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
स्थिर गणना bootstage_id get_bootstage_id(bool start)
{
	गणना xpl_phase_t phase = xpl_phase();

	यदि (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		प्रत्यागम start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_अन्तः_TPL;
	अन्यथा यदि (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		प्रत्यागम start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_अन्तः_VPL;
	अन्यथा
		प्रत्यागम start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_अन्तः_SPL;
}

स्थिर पूर्णाङ्क spl_common_init(bool सज्जता_malloc)
{
	पूर्णाङ्क ret;

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	यदि (सज्जता_malloc) {
#यदि_परिभाषितम् CFG_MALLOC_F_ADDR
		gd->malloc_आधारः = CFG_MALLOC_F_ADDR;
#यदि_अन्तः
		gd->malloc_सीमा = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#यदि_अन्तः
	ret = bootstage_init(xpl_is_first_phase());
	यदि (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		प्रत्यागम ret;
	}
	यदि (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		यदि (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#यदि CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	यदि (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		प्रत्यागम ret;
	}
#यदि_अन्तः
	यदि (CONFIG_IS_ENABLED(सम्बन्धि_REAL)) {
		ret = fdtdec_सज्जता();
		यदि (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			प्रत्यागम ret;
		}
	}
	यदि (CONFIG_IS_ENABLED(सम्बन्धि_जीवितम्)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_सम्बन्धि_जीवितम्, "of_live");
		ret = सम्बन्धि_जीवितम्_build(gd->fdt_blob,
				    (संरचना device_ग्रन्थिः **)gd_सम्बन्धि_मूलम्_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_सम्बन्धि_जीवितम्);
		यदि (ret)
			प्रत्यागम ret;
	}
	यदि (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_अवलोकनम्(!CONFIG_IS_ENABLED(सम्बन्धि_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		यदि (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			प्रत्यागम ret;
		}

		ret = dm_autoprobe();
		यदि (ret)
			प्रत्यागम ret;
	}

	प्रत्यागम 0;
}

शून्य spl_समुच्चयः_bd(शून्य)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	यदि (!gd->bd)
		gd->bd = &bdata;
}

पूर्णाङ्क spl_early_init(शून्य)
{
	पूर्णाङ्क ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	यदि (ret)
		प्रत्यागम ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	प्रत्यागम 0;
}

पूर्णाङ्क spl_init(शून्य)
{
	पूर्णाङ्क ret;
	bool सज्जता_malloc = !(IS_ENABLED(CONFIG_SPL_राशिः_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	यदि (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(सज्जता_malloc);
		यदि (ret)
			प्रत्यागम ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	प्रत्यागम 0;
}

#यदि_अपरिभाषितम् BOOT_DEVICE_NONE
#परिभाषय BOOT_DEVICE_NONE 0xdeadbeef
#यदि_अन्तः

__weak शून्य board_boot_क्रमः(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak पूर्णाङ्क spl_check_board_image(संरचना spl_image_info *spl_image,
				 ध्रुव संरचना spl_boot_device *bootdev)
{
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क spl_आरोपय_image(संरचना spl_image_info *spl_image,
			  संरचना spl_image_loader *loader)
{
	पूर्णाङ्क ret;
	संरचना spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = रिक्त;

	ret = loader->आरोपय_image(spl_image, &bootdev);
#यदि_परिभाषितम् CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	यदि (!ret && spl_image->dcrc_दीर्घता) {
		/* check data crc */
		ulong dcrc = चक्रीयपरीक्षा_32_wd(0, (अचिह्नित वर्ण *)spl_image->dcrc_दत्त,
				      spl_image->dcrc_दीर्घता, CHUNKSZ_चक्रीयपरीक्षा_32);
		यदि (dcrc != spl_image->dcrc) {
			पङ्क्तिं_लिख("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#यदि_अन्तः
	यदि (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	प्रत्यागम ret;
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
		ll_प्रविष्टिः_start(संरचना spl_image_loader, spl_image_loader);
	ध्रुव पूर्णाङ्क n_ents =
		ll_प्रविष्टिः_गणनम्(संरचना spl_image_loader, spl_image_loader);
	पूर्णाङ्क ret = -ENODEV;
	पूर्णाङ्क i;

	प्रति (i = 0; i < गणनम् && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		संरचना spl_image_loader *loader;
		पूर्णाङ्क bootdev = spl_boot_list[i];

		यदि (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		प्रति (loader = drv; loader != drv + n_ents; loader++) {
			यदि (loader && bootdev != loader->boot_device)
				अनुवर्त;
			यदि (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				प्रारूप_लिख("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_आरोपय_image(spl_image, loader);
			यदि (!ret) {
				spl_image->boot_device = bootdev;
				प्रत्यागम 0;
			}
			प्रारूप_लिख("Error: %d\n", ret);
		}

		यदि (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			यदि (CONFIG_IS_ENABLED(SHOW_ERRORS))
				प्रारूप_लिख(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			अन्यथा
				प्रारूप_लिख(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	प्रत्यागम ret;
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
	spl_jump_गन्तव्यम्_image_t jumper = &jump_गन्तव्यम्_image;
	संरचना spl_image_info spl_image;
	पूर्णाङ्क ret, os;
	शून्य *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_समुच्चयः_bd();

	यदि (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_आकार);
		gd->flags |= GD_FLG_पूर्णम्_MALLOC_INIT;
	}
	यदि (!(gd->flags & GD_FLG_SPL_INIT)) {
		यदि (spl_init())
			hang();
	}
	timer_init();
	यदि (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		यदि (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			पङ्क्तिं_लिख(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	यदि (CONFIG_IS_ENABLED(HANDOFF)) {
		पूर्णाङ्क ret;

		ret = सज्जता_spl_handoff();
		यदि (ret) {
			पङ्क्तिं_लिख(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	यदि (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	यदि (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	यदि (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	यदि (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		यदि (ret)
			पङ्क्तिं_लिख(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	यदि (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	यदि (CONFIG_IS_ENABLED(DM_STATS)) {
		संरचना dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	स्मृतिं_पूरय(&spl_image, '\0', परिमाण(spl_image));
	यदि (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (शून्य *)SPL_PAYLOAD_तर्काः_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_क्रमः(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_आकार(spl_boot_list));
	यदि (ret) {
		यदि (CONFIG_IS_ENABLED(SHOW_ERRORS))
			प्रारूप_लिख(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		अन्यथा
			पङ्क्तिं_लिख(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	यदि (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_अग्र_phase()));
	} अन्यथा यदि (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} अन्यथा यदि (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_गन्तव्यम्_image_optee;
	} अन्यथा यदि (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} अन्यथा यदि (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		यदि (CONFIG_IS_ENABLED(OS_BOOT_तर्काः))
			fdt = (शून्य *)SPL_PAYLOAD_तर्काः_ADDR;
		अन्यथा
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_गन्तव्यम्_image_linux;
	} अन्यथा {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	यदि (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_आकार))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	यदि (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	यदि (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		संरचना udevice *dev;
		पूर्णाङ्क rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		यदि (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_प्रसामान्यम्);
			यदि (rc)
				प्रारूप_लिख("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	यदि (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		यदि (ret)
			प्रारूप_लिख(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	यदि (CONFIG_IS_ENABLED(UPL_बहिर्गमनम्) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		यदि (ret) {
			प्रारूप_लिख(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	यदि (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		यदि (ret)
			प्रारूप_लिख("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	यदि (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		पूर्णाङ्क ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		यदि (ret) {
			यदि (xpl_phase() == PHASE_VPL)
				प्रारूप_लिख("jump failed %d\n", ret);
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
#यदि CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#यदि CONFIG_IS_ENABLED(BANNER_PRINT)
	पङ्क्तिं_लिख("\nU-Boot " PHASE_NAME " " PLAIN_संस्करणम् " (" U_BOOT_DATE " - "
	     U_BOOT_TIME " " U_BOOT_TZ ")\n");
#यदि_अन्तः
#यदि_परिभाषितम् CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#यदि_अन्तः
#यदि_अन्तः
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak शून्य spl_relocate_राशिः_check(शून्य)
{
#यदि CONFIG_IS_ENABLED(SYS_REPORT_राशिः_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong राशिः_bottom = init_sp - CONFIG_VAL(आकार_सीमा_PROVIDE_राशिः);
	u8 *ptr = (u8 *)राशिः_bottom;
	ulong i;

	प्रति (i = 0; i < CONFIG_VAL(आकार_सीमा_PROVIDE_राशिः); i++) {
		यदि (*ptr != CONFIG_VAL(SYS_राशिः_F_CHECK_BYTE))
			भङ्ग;
		ptr++;
	}
	प्रारूप_लिख("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(आकार_सीमा_PROVIDE_राशिः) - i);
#यदि_अन्तः
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
#यदि CONFIG_IS_ENABLED(राशिः_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_राशिः_R_ADDR;

	यदि (CONFIG_IS_ENABLED(SYS_REPORT_राशिः_F_USAGE))
		spl_relocate_राशिः_check();

#यदि defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	यदि (CONFIG_SPL_राशिः_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_राशिः_R_MALLOC_SIMPLE_LEN;
		gd->malloc_आधारः = ptr;
		gd->malloc_सीमा = CONFIG_SPL_राशिः_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#यदि_अन्तः
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(परिमाण(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	स्मृतिं_प्रतिलिख(new_gd, (शून्य *)gd, परिमाण(gd_t));
#यदि CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#यदि_अन्तः
#यदि CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#यदि_अन्तः
#यदि !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#यदि_अन्तः
	प्रत्यागम ptr;
#अन्यथा
	प्रत्यागम 0;
#यदि_अन्तः
}

#यदि defined(CONFIG_BOOTCOUNT_सीमा) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_सीमा)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_सीमा)))
शून्य bootcount_store(ulong a)
{
}

ulong bootcount_आरोपय(शून्य)
{
	प्रत्यागम 0;
}
#यदि_अन्तः
