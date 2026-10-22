// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <bloblist.h>
#ಅನ್ತರ್ಭಾವ <binman_sym.h>
#ಅನ್ತರ್ಭಾವ <bootstage.h>
#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <handoff.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <irq_func.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <of_live.h>
#ಅನ್ತರ್ಭಾವ <serial.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <spl_load.h>
#ಅನ್ತರ್ಭಾವ <system-constants.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm-generic/gpio.h>
#ಅನ್ತರ್ಭಾವ <nand.h>
#ಅನ್ತರ್ಭಾವ <fat.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಯದಿ CONFIG_IS_ENABLED(BANNER_PRINT)
#ಅನ್ತರ್ಭಾವ <timestamp.h>
#endif
#ಅನ್ತರ್ಭಾವ <version.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <dm/root.h>
#ಅನ್ತರ್ಭಾವ <dm/util.h>
#ಅನ್ತರ್ಭಾವ <dm/device-internal.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass-internal.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>
#ಅನ್ತರ್ಭಾವ <fdt_support.h>
#ಅನ್ತರ್ಭಾವ <bootcount.h>
#ಅನ್ತರ್ಭಾವ <wdt.h>
#ಅನ್ತರ್ಭಾವ <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = ರಿಕ್ತ;

#ಯದಿ CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#ಯದಿ defined(CONFIG_SPL_BUILD)
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
ಸ್ಥಿರ ಸಂರಚನಾ bd_info bdata __attribute__ ((section(".data")));

#ಯದಿ CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak ಶೂನ್ಯ show_boot_progress(ಪೂರ್ಣಾಙ್ಕ val) {}
#endif

#ಯದಿ defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak ಪೂರ್ಣಾಙ್ಕ dram_init_banksize(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
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
#ಯದಿ CONFIG_IS_ENABLED(OS_BOOT)
__weak ಪೂರ್ಣಾಙ್ಕ spl_start_uboot(ಶೂನ್ಯ)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	ಪ್ರತ್ಯಾಗಮ 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
ಪೂರ್ಣಾಙ್ಕ __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 ಪ್ರತ್ಯಾಗಮ 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
ಶೂನ್ಯ __weak spl_perform_arch_fixups(ಸಂರಚನಾ spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
ಶೂನ್ಯ __weak spl_perform_board_fixups(ಸಂರಚನಾ spl_image_info *spl_image)
{
}

ಶೂನ್ಯ spl_fixup_fdt(ಶೂನ್ಯ *fdt_blob)
{
#ಯದಿ defined(CONFIG_SPL_OF_LIBFDT)
	ಪೂರ್ಣಾಙ್ಕ err;

	ಯದಿ (!fdt_blob)
		ಪ್ರತ್ಯಾಗಮ;

	err = fdt_check_header(fdt_blob);
	ಯದಿ (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		ಪ್ರತ್ಯಾಗಮ;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	ಯದಿ (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		ಪ್ರತ್ಯಾಗಮ;
	}

	err = arch_fixup_fdt(fdt_blob);
	ಯದಿ (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		ಪ್ರತ್ಯಾಗಮ;
	}
#endif
}

ಪೂರ್ಣಾಙ್ಕ spl_reserve_video_from_ram_ತುದಿ(ಶೂನ್ಯ)
{
	ಯದಿ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		ಪೂರ್ಣಾಙ್ಕ ret;

		addr = gd->ram_ತುದಿ;
		ret = video_reserve(&addr);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ulong spl_get_image_pos(ಶೂನ್ಯ)
{
	ಯದಿ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ಪ್ರತ್ಯಾಗಮ BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	ಯದಿ (xpl_next_phase() == PHASE_VPL)
		ಪ್ರತ್ಯಾಗಮ binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#ಯದಿ defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	ಯದಿ (xpl_next_phase() == PHASE_SPL)
		ಪ್ರತ್ಯಾಗಮ binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#ಯದಿ defined(CONFIG_SPL_BUILD)
	ಪ್ರತ್ಯಾಗಮ binman_sym(ulong, u_boot_any, image_pos);
#endif

	ಪ್ರತ್ಯಾಗಮ BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(ಶೂನ್ಯ)
{
	ಯದಿ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		ಪ್ರತ್ಯಾಗಮ BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	ಯದಿ (xpl_next_phase() == PHASE_VPL)
		ಪ್ರತ್ಯಾಗಮ binman_sym(ulong, u_boot_vpl_any, size);
#endif
	ಪ್ರತ್ಯಾಗಮ xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(ಶೂನ್ಯ)
{
#ifdef CONFIG_VPL
	ಯದಿ (xpl_next_phase() == PHASE_VPL)
		ಪ್ರತ್ಯಾಗಮ CONFIG_VPL_TEXT_BASE;
#endif
	ಪ್ರತ್ಯಾಗಮ xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak ಶೂನ್ಯ spl_board_prepare_for_linux(ಶೂನ್ಯ)
{
	/* Nothing to do! */
}

__weak ಶೂನ್ಯ spl_board_prepare_for_optee(ಶೂನ್ಯ *fdt)
{
}

__weak ಧ್ರುವ ವರ್ಣ *spl_board_loader_name(u32 boot_device)
{
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

#ಯದಿ CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak ಶೂನ್ಯ __noreturn jump_to_image_optee(ಸಂರಚನಾ spl_image_info *spl_image)
{
	spl_optee_entry(ರಿಕ್ತ, ರಿಕ್ತ, spl_image->fdt_addr,
			(ಶೂನ್ಯ *)spl_image->entry_point);
}
#endif

__weak ಶೂನ್ಯ spl_board_prepare_for_boot(ಶೂನ್ಯ)
{
	/* Nothing to do! */
}

__weak ಸಂರಚನಾ legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

ಶೂನ್ಯ spl_set_header_raw_uboot(ಸಂರಚನಾ spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#ಯದಿ CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#ಅನ್ಯಥಾ
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	ಯದಿ (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} ಅನ್ಯಥಾ {
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

__weak ಪೂರ್ಣಾಙ್ಕ spl_parse_board_header(ಸಂರಚನಾ spl_image_info *spl_image,
				  ಧ್ರುವ ಸಂರಚನಾ spl_boot_device *bootdev,
				  ಧ್ರುವ ಶೂನ್ಯ *image_header, size_t size)
{
	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

__weak ಪೂರ್ಣಾಙ್ಕ spl_parse_legacy_header(ಸಂರಚನಾ spl_image_info *spl_image,
				   ಧ್ರುವ ಸಂರಚನಾ legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	ಪ್ರತ್ಯಾಗಮ -EINVAL;
}

ಪೂರ್ಣಾಙ್ಕ spl_parse_image_header(ಸಂರಚನಾ spl_image_info *spl_image,
			   ಧ್ರುವ ಸಂರಚನಾ spl_boot_device *bootdev,
			   ಧ್ರುವ ಸಂರಚನಾ legacy_img_hdr *header)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	ಯದಿ (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		ಯದಿ (!ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}
	ಯದಿ (image_get_magic(header) == IH_MAGIC) {
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = spl_parse_legacy_header(spl_image, header);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		ಯದಿ (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	} ಅನ್ಯಥಾ ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		ಯದಿ (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	}

	ಯದಿ (!spl_parse_board_header(spl_image, bootdev, (ಧ್ರುವ ಶೂನ್ಯ *)header,
				    ಪರಿಮಾಣ(*header)))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} ಅನ್ಯಥಾ {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		ಪ್ರತ್ಯಾಗಮ -EINVAL;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ SPL_LOAD_USERS > 1
ಪೂರ್ಣಾಙ್ಕ spl_load(ಸಂರಚನಾ spl_image_info *spl_image,
	     ಧ್ರುವ ಸಂರಚನಾ spl_boot_device *bootdev, ಸಂರಚನಾ spl_load_info *info,
	     size_t size, size_t offset)
{
	ಪ್ರತ್ಯಾಗಮ _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak ಶೂನ್ಯ __noreturn jump_to_image(ಸಂರಚನಾ spl_image_info *spl_image)
{
	ಪ್ರಕಾರನಿರ್ದೇಶ ಶೂನ್ಯ __noreturn (*image_entry_noargs_t)(ಶೂನ್ಯ);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#ಯದಿ CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_spl_handoff(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, ಪರಿಮಾಣ(ಸಂರಚನಾ spl_handoff));
	ಯದಿ (!ho)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;

	ಪ್ರತ್ಯಾಗಮ 0;
}

__weak ಪೂರ್ಣಾಙ್ಕ handoff_arch_save(ಸಂರಚನಾ spl_handoff *ho)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ write_spl_handoff(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ spl_handoff *ho;
	ಪೂರ್ಣಾಙ್ಕ ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, ಪರಿಮಾಣ(ಸಂರಚನಾ spl_handoff));
	ಯದಿ (!ho)
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	ಪ್ರತ್ಯಾಗಮ 0;
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ setup_spl_handoff(ಶೂನ್ಯ) { ಪ್ರತ್ಯಾಗಮ 0; }
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ write_spl_handoff(ಶೂನ್ಯ) { ಪ್ರತ್ಯಾಗಮ 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ಸ್ಥಿರ ಗಣನಾ bootstage_id get_bootstage_id(bool start)
{
	ಗಣನಾ xpl_phase_t phase = xpl_phase();

	ಯದಿ (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		ಪ್ರತ್ಯಾಗಮ start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	ಅನ್ಯಥಾ ಯದಿ (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		ಪ್ರತ್ಯಾಗಮ start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	ಅನ್ಯಥಾ
		ಪ್ರತ್ಯಾಗಮ start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_common_init(bool setup_malloc)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

#ಯದಿ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಯದಿ (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	ಯದಿ (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		ಪ್ರತ್ಯಾಗಮ ret;
	}
	ಯದಿ (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		ಯದಿ (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#ಯದಿ CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	ಯದಿ (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		ಪ್ರತ್ಯಾಗಮ ret;
	}
#endif
	ಯದಿ (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		ಯದಿ (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			ಪ್ರತ್ಯಾಗಮ ret;
		}
	}
	ಯದಿ (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (ಸಂರಚನಾ device_ಗಂಟು **)gd_of_ಬೇರು_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}
	ಯದಿ (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		ಯದಿ (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			ಪ್ರತ್ಯಾಗಮ ret;
		}

		ret = dm_autoprobe();
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ spl_set_bd(ಶೂನ್ಯ)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	ಯದಿ (!gd->bd)
		gd->bd = &bdata;
}

ಪೂರ್ಣಾಙ್ಕ spl_early_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ spl_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_ರಾಶಿ_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	ಯದಿ (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak ಶೂನ್ಯ board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak ಪೂರ್ಣಾಙ್ಕ spl_check_board_image(ಸಂರಚನಾ spl_image_info *spl_image,
				 ಧ್ರುವ ಸಂರಚನಾ spl_boot_device *bootdev)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ spl_load_image(ಸಂರಚನಾ spl_image_info *spl_image,
			  ಸಂರಚನಾ spl_image_loader *loader)
{
	ಪೂರ್ಣಾಙ್ಕ ret;
	ಸಂರಚನಾ spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = ರಿಕ್ತ;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	ಯದಿ (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned ವರ್ಣ *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		ಯದಿ (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	ಯದಿ (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	ಪ್ರತ್ಯಾಗಮ ret;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ boot_from_devices(ಸಂರಚನಾ spl_image_info *spl_image,
			     u32 spl_boot_list[], ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
	ಸಂರಚನಾ spl_image_loader *drv =
		ll_entry_start(ಸಂರಚನಾ spl_image_loader, spl_image_loader);
	ಧ್ರುವ ಪೂರ್ಣಾಙ್ಕ n_ents =
		ll_entry_ಎಣಿಕೆ(ಸಂರಚನಾ spl_image_loader, spl_image_loader);
	ಪೂರ್ಣಾಙ್ಕ ret = -ENODEV;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ಎಣಿಕೆ && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		ಸಂರಚನಾ spl_image_loader *loader;
		ಪೂರ್ಣಾಙ್ಕ bootdev = spl_boot_list[i];

		ಯದಿ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		ಪ್ರತಿ (loader = drv; loader != drv + n_ents; loader++) {
			ಯದಿ (loader && bootdev != loader->boot_device)
				ಅನುವರ್ತ;
			ಯದಿ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			ಯದಿ (!ret) {
				spl_image->boot_device = bootdev;
				ಪ್ರತ್ಯಾಗಮ 0;
			}
			printf("Error: %d\n", ret);
		}

		ಯದಿ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			ಯದಿ (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			ಅನ್ಯಥಾ
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	ಸಂರಚನಾ spl_image_info spl_image;
	ಪೂರ್ಣಾಙ್ಕ ret, os;
	ಶೂನ್ಯ *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	ಯದಿ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	ಯದಿ (!(gd->flags & GD_FLG_SPL_INIT)) {
		ಯದಿ (spl_init())
			hang();
	}
	timer_init();
	ಯದಿ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		ಯದಿ (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	ಯದಿ (CONFIG_IS_ENABLED(HANDOFF)) {
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = setup_spl_handoff();
		ಯದಿ (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	ಯದಿ (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	ಯದಿ (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	ಯದಿ (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	ಯದಿ (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		ಯದಿ (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	ಯದಿ (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	ಯದಿ (CONFIG_IS_ENABLED(DM_STATS)) {
		ಸಂರಚನಾ dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', ಪರಿಮಾಣ(spl_image));
	ಯದಿ (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (ಶೂನ್ಯ *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	ಯದಿ (ret) {
		ಯದಿ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		ಅನ್ಯಥಾ
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	ಯದಿ (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} ಅನ್ಯಥಾ ಯದಿ (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} ಅನ್ಯಥಾ ಯದಿ (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} ಅನ್ಯಥಾ ಯದಿ (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} ಅನ್ಯಥಾ ಯದಿ (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		ಯದಿ (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (ಶೂನ್ಯ *)SPL_PAYLOAD_ARGS_ADDR;
		ಅನ್ಯಥಾ
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} ಅನ್ಯಥಾ {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	ಯದಿ (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	ಯದಿ (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	ಯದಿ (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		ಸಂರಚನಾ udevice *dev;
		ಪೂರ್ಣಾಙ್ಕ rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		ಯದಿ (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			ಯದಿ (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	ಯದಿ (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		ಯದಿ (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	ಯದಿ (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		ಯದಿ (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	ಯದಿ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		ಯದಿ (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	ಯದಿ (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		ಪೂರ್ಣಾಙ್ಕ ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		ಯದಿ (ret) {
			ಯದಿ (xpl_phase() == PHASE_VPL)
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
ಶೂನ್ಯ preloader_console_init(ಶೂನ್ಯ)
{
#ಯದಿ CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#ಯದಿ CONFIG_IS_ENABLED(BANNER_PRINT)
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
__weak ಶೂನ್ಯ spl_relocate_ರಾಶಿ_check(ಶೂನ್ಯ)
{
#ಯದಿ CONFIG_IS_ENABLED(SYS_REPORT_ರಾಶಿ_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong ರಾಶಿ_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_ರಾಶಿ);
	u8 *ptr = (u8 *)ರಾಶಿ_bottom;
	ulong i;

	ಪ್ರತಿ (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_ರಾಶಿ); i++) {
		ಯದಿ (*ptr != CONFIG_VAL(SYS_ರಾಶಿ_F_CHECK_BYTE))
			ಭಙ್ಗ;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_ರಾಶಿ) - i);
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
ulong spl_relocate_ರಾಶಿ_gd(ಶೂನ್ಯ)
{
#ಯದಿ CONFIG_IS_ENABLED(ರಾಶಿ_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ರಾಶಿ_R_ADDR;

	ಯದಿ (CONFIG_IS_ENABLED(SYS_REPORT_ರಾಶಿ_F_USAGE))
		spl_relocate_ರಾಶಿ_check();

#ಯದಿ defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ಯದಿ (CONFIG_SPL_ರಾಶಿ_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ರಾಶಿ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_ರಾಶಿ_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(ಪರಿಮಾಣ(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (ಶೂನ್ಯ *)gd, ಪರಿಮಾಣ(gd_t));
#ಯದಿ CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#ಯದಿ CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#ಯದಿ !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	ಪ್ರತ್ಯಾಗಮ ptr;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ 0;
#endif
}

#ಯದಿ defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
ಶೂನ್ಯ bootcount_store(ulong a)
{
}

ulong bootcount_load(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif
