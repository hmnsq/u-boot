// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#समावेश <config.h>
#समावेश <bloblist.h>
#समावेश <binman_sym.h>
#समावेश <bootstage.h>
#समावेश <dm.h>
#समावेश <handoff.h>
#समावेश <hang.h>
#समावेश <init.h>
#समावेश <irq_func.h>
#समावेश <log.h>
#समावेश <mapmem.h>
#समावेश <of_live.h>
#समावेश <serial.h>
#समावेश <spl.h>
#समावेश <spl_load.h>
#समावेश <system-constants.h>
#समावेश <asm/global_data.h>
#समावेश <asm-generic/gpio.h>
#समावेश <nand.h>
#समावेश <fat.h>
#समावेश <u-boot/crc.h>
#अगर CONFIG_IS_ENABLED(BANNER_PRINT)
#समावेश <timestamp.h>
#पूर्ण_अगर
#समावेश <version.h>
#समावेश <image.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <dm/root.h>
#समावेश <dm/util.h>
#समावेश <dm/device-internal.h>
#समावेश <dm/uclass-internal.h>
#समावेश <linux/compiler.h>
#समावेश <fdt_support.h>
#समावेश <bootcount.h>
#समावेश <wdt.h>
#समावेश <video.h>

DECLARE_GLOBAL_तथ्य_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = शून्य;

#अगर CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#अगर defined(CONFIG_SPL_BUILD)
binman_sym_declare(ulong, u_boot_any, image_pos);
binman_sym_declare(ulong, u_boot_any, आकार);
#पूर्ण_अगर

#अगर_घोषित CONFIG_TPL
binman_sym_declare(ulong, u_boot_spl_any, image_pos);
binman_sym_declare(ulong, u_boot_spl_any, आकार);
#पूर्ण_अगर

#अगर_घोषित CONFIG_VPL
binman_sym_declare(ulong, u_boot_vpl_any, image_pos);
binman_sym_declare(ulong, u_boot_vpl_any, आकार);
#पूर्ण_अगर

#पूर्ण_अगर /* BINMAN_UBOOT_SYMBOLS */

/* Define board data structure */
अटल काष्ठा bd_info bdata __attribute__ ((section(".data")));

#अगर CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak व्योम show_boot_progress(पूर्णांक val) {}
#पूर्ण_अगर

#अगर defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak पूर्णांक dram_init_banksize(व्योम)
{
	वापस 0;
}
#पूर्ण_अगर

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
#अगर CONFIG_IS_ENABLED(OS_BOOT)
__weak पूर्णांक spl_start_uboot(व्योम)
{
	माला_दो(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	माला_दो(PHASE_PROMPT "Direct Linux boot not active!\n");
	वापस 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
पूर्णांक __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 वापस 1;
}

पूर्णांक __weak booti_setup(ulong image, ulong *relocated_addr, ulong *आकार, bool force_reloc)
{
	 वापस 1;
}
#पूर्ण_अगर

/* Weak default function for arch specific fixups to the spl_image_info */
व्योम __weak spl_perform_arch_fixups(काष्ठा spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
व्योम __weak spl_perform_board_fixups(काष्ठा spl_image_info *spl_image)
{
}

व्योम spl_fixup_fdt(व्योम *fdt_blob)
{
#अगर defined(CONFIG_SPL_OF_LIBFDT)
	पूर्णांक err;

	अगर (!fdt_blob)
		वापस;

	err = fdt_check_header(fdt_blob);
	अगर (err < 0) {
		म_लिखो("fdt_root: %s\n", fdt_strerror(err));
		वापस;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	अगर (err == 0) {
		म_लिखो(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		वापस;
	}

	err = arch_fixup_fdt(fdt_blob);
	अगर (err) {
		म_लिखो(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		वापस;
	}
#पूर्ण_अगर
}

पूर्णांक spl_reserve_video_from_ram_शीर्ष(व्योम)
{
	अगर (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		पूर्णांक ret;

		addr = gd->ram_शीर्ष;
		ret = video_reserve(&addr);
		अगर (ret)
			वापस ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((अचिन्हित दीर्घ)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	वापस 0;
}

ulong spl_get_image_pos(व्योम)
{
	अगर (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		वापस BINMAN_SYM_MISSING;

#अगर_घोषित CONFIG_VPL
	अगर (xpl_अग्र_phase() == PHASE_VPL)
		वापस binman_sym(ulong, u_boot_vpl_any, image_pos);
#पूर्ण_अगर
#अगर defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	अगर (xpl_अग्र_phase() == PHASE_SPL)
		वापस binman_sym(ulong, u_boot_spl_any, image_pos);
#पूर्ण_अगर
#अगर defined(CONFIG_SPL_BUILD)
	वापस binman_sym(ulong, u_boot_any, image_pos);
#पूर्ण_अगर

	वापस BINMAN_SYM_MISSING;
}

ulong spl_get_image_आकार(व्योम)
{
	अगर (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		वापस BINMAN_SYM_MISSING;

#अगर_घोषित CONFIG_VPL
	अगर (xpl_अग्र_phase() == PHASE_VPL)
		वापस binman_sym(ulong, u_boot_vpl_any, आकार);
#पूर्ण_अगर
	वापस xpl_अग्र_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, आकार) :
		binman_sym(ulong, u_boot_any, आकार);
}

ulong spl_get_image_text_base(व्योम)
{
#अगर_घोषित CONFIG_VPL
	अगर (xpl_अग्र_phase() == PHASE_VPL)
		वापस CONFIG_VPL_TEXT_BASE;
#पूर्ण_अगर
	वापस xpl_अग्र_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak व्योम spl_board_prepare_for_linux(व्योम)
{
	/* Nothing to do! */
}

__weak व्योम spl_board_prepare_for_optee(व्योम *fdt)
{
}

__weak स्थिर अक्षर *spl_board_loader_name(u32 boot_device)
{
	वापस शून्य;
}

#अगर CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak व्योम __noreturn jump_to_image_optee(काष्ठा spl_image_info *spl_image)
{
	spl_optee_entry(शून्य, शून्य, spl_image->fdt_addr,
			(व्योम *)spl_image->entry_point);
}
#पूर्ण_अगर

__weak व्योम spl_board_prepare_for_boot(व्योम)
{
	/* Nothing to do! */
}

__weak काष्ठा legacy_img_hdr *spl_get_load_बफर(ssize_t offset, माप_प्रकार आकार)
{
	वापस map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

व्योम spl_set_header_raw_uboot(काष्ठा spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#अगर CONFIG_SYS_MONITOR_LEN != 0
	spl_image->आकार = CONFIG_SYS_MONITOR_LEN;
#अन्यथा
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->आकार = 200 * 1024;
#पूर्ण_अगर

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	अगर (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->आकार = spl_get_image_आकार();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} अन्यथा {
		spl_image->entry_point = CONFIG_SYS_UBOOT_START;
		spl_image->load_addr = CONFIG_TEXT_BASE;
		log_debug("Default load addr %lx (u_boot_pos=%lx)\n",
			  spl_image->load_addr, u_boot_pos);
	}
	spl_image->os = IH_OS_U_BOOT;
	spl_image->name = xpl_name(xpl_अग्र_phase());
	log_debug("Next phase: %s at %lx size %lx\n", spl_image->name,
		  spl_image->load_addr, (ulong)spl_image->आकार);
}

__weak पूर्णांक spl_parse_board_header(काष्ठा spl_image_info *spl_image,
				  स्थिर काष्ठा spl_boot_device *bootdev,
				  स्थिर व्योम *image_header, माप_प्रकार आकार)
{
	वापस -EINVAL;
}

__weak पूर्णांक spl_parse_legacy_header(काष्ठा spl_image_info *spl_image,
				   स्थिर काष्ठा legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	वापस -EINVAL;
}

पूर्णांक spl_parse_image_header(काष्ठा spl_image_info *spl_image,
			   स्थिर काष्ठा spl_boot_device *bootdev,
			   स्थिर काष्ठा legacy_img_hdr *header)
{
	पूर्णांक ret;

	अगर (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		अगर (!ret)
			वापस ret;
	}
	अगर (image_get_magic(header) == IH_MAGIC) {
		पूर्णांक ret;

		ret = spl_parse_legacy_header(spl_image, header);
		अगर (ret)
			वापस ret;
		वापस 0;
	}

	अगर (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, आकार;

		अगर (!booti_setup((ulong)header, &start, &आकार, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->आकार = आकार;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->आकार);
			वापस 0;
		}
	} अन्यथा अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		अगर (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->आकार = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->आकार);
			वापस 0;
		}
	}

	अगर (!spl_parse_board_header(spl_image, bootdev, (स्थिर व्योम *)header,
				    माप(*header)))
		वापस 0;

	अगर (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} अन्यथा {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		वापस -EINVAL;
	}

	वापस 0;
}

#अगर SPL_LOAD_USERS > 1
पूर्णांक spl_load(काष्ठा spl_image_info *spl_image,
	     स्थिर काष्ठा spl_boot_device *bootdev, काष्ठा spl_load_info *info,
	     माप_प्रकार आकार, माप_प्रकार offset)
{
	वापस _spl_load(spl_image, bootdev, info, आकार, offset);
}
#पूर्ण_अगर

__weak व्योम __noreturn jump_to_image(काष्ठा spl_image_info *spl_image)
{
	प्रकार व्योम __noreturn (*image_entry_noargs_t)(व्योम);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#अगर CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
अटल पूर्णांक setup_spl_handoff(व्योम)
{
	काष्ठा spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, माप(काष्ठा spl_handoff));
	अगर (!ho)
		वापस -ENOENT;

	वापस 0;
}

__weak पूर्णांक handoff_arch_save(काष्ठा spl_handoff *ho)
{
	वापस 0;
}

अटल पूर्णांक write_spl_handoff(व्योम)
{
	काष्ठा spl_handoff *ho;
	पूर्णांक ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, माप(काष्ठा spl_handoff));
	अगर (!ho)
		वापस -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	अगर (ret)
		वापस ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	वापस 0;
}
#अन्यथा
अटल अंतरभूत पूर्णांक setup_spl_handoff(व्योम) { वापस 0; }
अटल अंतरभूत पूर्णांक write_spl_handoff(व्योम) { वापस 0; }

#पूर्ण_अगर /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
अटल क्रमागत bootstage_id get_bootstage_id(bool start)
{
	क्रमागत xpl_phase_t phase = xpl_phase();

	अगर (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		वापस start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	अन्यथा अगर (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		वापस start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	अन्यथा
		वापस start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

अटल पूर्णांक spl_common_init(bool setup_malloc)
{
	पूर्णांक ret;

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	अगर (setup_malloc) {
#अगर_घोषित CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#पूर्ण_अगर
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#पूर्ण_अगर
	ret = bootstage_init(xpl_is_first_phase());
	अगर (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		वापस ret;
	}
	अगर (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		अगर (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#अगर CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	अगर (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		वापस ret;
	}
#पूर्ण_अगर
	अगर (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		अगर (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			वापस ret;
		}
	}
	अगर (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (काष्ठा device_गाँठ **)gd_of_मूल_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		अगर (ret)
			वापस ret;
	}
	अगर (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		अगर (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			वापस ret;
		}

		ret = dm_autoprobe();
		अगर (ret)
			वापस ret;
	}

	वापस 0;
}

व्योम spl_set_bd(व्योम)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	अगर (!gd->bd)
		gd->bd = &bdata;
}

पूर्णांक spl_early_init(व्योम)
{
	पूर्णांक ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	अगर (ret)
		वापस ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	वापस 0;
}

पूर्णांक spl_init(व्योम)
{
	पूर्णांक ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_ढेर_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	अगर (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		अगर (ret)
			वापस ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	वापस 0;
}

#अगर_अघोषित BOOT_DEVICE_NONE
#घोषणा BOOT_DEVICE_NONE 0xdeadbeef
#पूर्ण_अगर

__weak व्योम board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak पूर्णांक spl_check_board_image(काष्ठा spl_image_info *spl_image,
				 स्थिर काष्ठा spl_boot_device *bootdev)
{
	वापस 0;
}

अटल पूर्णांक spl_load_image(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_image_loader *loader)
{
	पूर्णांक ret;
	काष्ठा spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = शून्य;

	ret = loader->load_image(spl_image, &bootdev);
#अगर_घोषित CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	अगर (!ret && spl_image->dcrc_लंबाई) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (अचिन्हित अक्षर *)spl_image->dcrc_तथ्य,
				      spl_image->dcrc_लंबाई, CHUNKSZ_CRC32);
		अगर (dcrc != spl_image->dcrc) {
			माला_दो("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#पूर्ण_अगर
	अगर (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	वापस ret;
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
अटल पूर्णांक boot_from_devices(काष्ठा spl_image_info *spl_image,
			     u32 spl_boot_list[], पूर्णांक गिनती)
{
	काष्ठा spl_image_loader *drv =
		ll_entry_start(काष्ठा spl_image_loader, spl_image_loader);
	स्थिर पूर्णांक n_ents =
		ll_entry_गिनती(काष्ठा spl_image_loader, spl_image_loader);
	पूर्णांक ret = -ENODEV;
	पूर्णांक i;

	क्रम (i = 0; i < गिनती && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		काष्ठा spl_image_loader *loader;
		पूर्णांक bootdev = spl_boot_list[i];

		अगर (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		क्रम (loader = drv; loader != drv + n_ents; loader++) {
			अगर (loader && bootdev != loader->boot_device)
				जारी;
			अगर (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				म_लिखो("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			अगर (!ret) {
				spl_image->boot_device = bootdev;
				वापस 0;
			}
			म_लिखो("Error: %d\n", ret);
		}

		अगर (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			अगर (CONFIG_IS_ENABLED(SHOW_ERRORS))
				म_लिखो(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			अन्यथा
				म_लिखो(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	वापस ret;
}

व्योम board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	काष्ठा spl_image_info spl_image;
	पूर्णांक ret, os;
	व्योम *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	अगर (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_आकार);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	अगर (!(gd->flags & GD_FLG_SPL_INIT)) {
		अगर (spl_init())
			hang();
	}
	timer_init();
	अगर (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		अगर (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			माला_दो(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	अगर (CONFIG_IS_ENABLED(HANDOFF)) {
		पूर्णांक ret;

		ret = setup_spl_handoff();
		अगर (ret) {
			माला_दो(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	अगर (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	अगर (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	अगर (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	अगर (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		अगर (ret)
			माला_दो(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	अगर (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	अगर (CONFIG_IS_ENABLED(DM_STATS)) {
		काष्ठा dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	स_रखो(&spl_image, '\0', माप(spl_image));
	अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (व्योम *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_आकार(spl_boot_list));
	अगर (ret) {
		अगर (CONFIG_IS_ENABLED(SHOW_ERRORS))
			म_लिखो(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		अन्यथा
			माला_दो(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	अगर (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_अग्र_phase()));
	} अन्यथा अगर (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} अन्यथा अगर (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} अन्यथा अगर (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} अन्यथा अगर (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		अगर (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (व्योम *)SPL_PAYLOAD_ARGS_ADDR;
		अन्यथा
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} अन्यथा {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	अगर (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_आकार))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	अगर (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	अगर (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		काष्ठा udevice *dev;
		पूर्णांक rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		अगर (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			अगर (rc)
				म_लिखो("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	अगर (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		अगर (ret)
			म_लिखो(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	अगर (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		अगर (ret) {
			म_लिखो(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	अगर (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		अगर (ret)
			म_लिखो("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	अगर (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		पूर्णांक ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		अगर (ret) {
			अगर (xpl_phase() == PHASE_VPL)
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
व्योम preloader_console_init(व्योम)
{
#अगर CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#अगर CONFIG_IS_ENABLED(BANNER_PRINT)
	माला_दो("\nU-Boot " PHASE_NAME " " PLAIN_VERSION " (" U_BOOT_DATE " - "
	     U_BOOT_TIME " " U_BOOT_TZ ")\n");
#पूर्ण_अगर
#अगर_घोषित CONFIG_SPL_DISPLAY_PRINT
	spl_display_print();
#पूर्ण_अगर
#पूर्ण_अगर
}

/**
 * This function is called before the stack is changed from initial stack to
 * relocated stack. It tries to dump the stack size used
 */
__weak व्योम spl_relocate_ढेर_check(व्योम)
{
#अगर CONFIG_IS_ENABLED(SYS_REPORT_ढेर_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong ढेर_bottom = init_sp - CONFIG_VAL(आकार_LIMIT_PROVIDE_ढेर);
	u8 *ptr = (u8 *)ढेर_bottom;
	ulong i;

	क्रम (i = 0; i < CONFIG_VAL(आकार_LIMIT_PROVIDE_ढेर); i++) {
		अगर (*ptr != CONFIG_VAL(SYS_ढेर_F_CHECK_BYTE))
			अवरोध;
		ptr++;
	}
	म_लिखो("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(आकार_LIMIT_PROVIDE_ढेर) - i);
#पूर्ण_अगर
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
ulong spl_relocate_ढेर_gd(व्योम)
{
#अगर CONFIG_IS_ENABLED(ढेर_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ढेर_R_ADDR;

	अगर (CONFIG_IS_ENABLED(SYS_REPORT_ढेर_F_USAGE))
		spl_relocate_ढेर_check();

#अगर defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	अगर (CONFIG_SPL_ढेर_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ढेर_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_ढेर_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#पूर्ण_अगर
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(माप(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	स_नकल(new_gd, (व्योम *)gd, माप(gd_t));
#अगर CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#पूर्ण_अगर
#अगर CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#पूर्ण_अगर
#अगर !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#पूर्ण_अगर
	वापस ptr;
#अन्यथा
	वापस 0;
#पूर्ण_अगर
}

#अगर defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
व्योम bootcount_store(ulong a)
{
}

ulong bootcount_load(व्योम)
{
	वापस 0;
}
#पूर्ण_अगर
