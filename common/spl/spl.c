// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <bloblist.h>
#അന്തര്ഭാവ <binman_sym.h>
#അന്തര്ഭാവ <bootstage.h>
#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <handoff.h>
#അന്തര്ഭാവ <hang.h>
#അന്തര്ഭാവ <init.h>
#അന്തര്ഭാവ <irq_func.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <of_live.h>
#അന്തര്ഭാവ <serial.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <spl_load.h>
#അന്തര്ഭാവ <system-constants.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm-generic/gpio.h>
#അന്തര്ഭാവ <nand.h>
#അന്തര്ഭാവ <fat.h>
#അന്തര്ഭാവ <u-boot/crc.h>
#എങ്കിൽ CONFIG_IS_ENABLED(BANNER_PRINT)
#അന്തര്ഭാവ <timestamp.h>
#endif
#അന്തര്ഭാവ <version.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <dm/root.h>
#അന്തര്ഭാവ <dm/util.h>
#അന്തര്ഭാവ <dm/device-internal.h>
#അന്തര്ഭാവ <dm/uclass-internal.h>
#അന്തര്ഭാവ <linux/compiler.h>
#അന്തര്ഭാവ <fdt_support.h>
#അന്തര്ഭാവ <bootcount.h>
#അന്തര്ഭാവ <wdt.h>
#അന്തര്ഭാവ <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = ശൂന്യ;

#എങ്കിൽ CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#എങ്കിൽ defined(CONFIG_SPL_BUILD)
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
സ്ഥിതം ഘടന bd_info bdata __attribute__ ((section(".data")));

#എങ്കിൽ CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak ശൂന്യം show_boot_progress(പൂർണ്ണസംഖ്യ val) {}
#endif

#എങ്കിൽ defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak പൂർണ്ണസംഖ്യ dram_init_banksize(ശൂന്യം)
{
	തിരികെ 0;
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
#എങ്കിൽ CONFIG_IS_ENABLED(OS_BOOT)
__weak പൂർണ്ണസംഖ്യ spl_start_uboot(ശൂന്യം)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	തിരികെ 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
പൂർണ്ണസംഖ്യ __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 തിരികെ 1;
}

പൂർണ്ണസംഖ്യ __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 തിരികെ 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
ശൂന്യം __weak spl_perform_arch_fixups(ഘടന spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
ശൂന്യം __weak spl_perform_board_fixups(ഘടന spl_image_info *spl_image)
{
}

ശൂന്യം spl_fixup_fdt(ശൂന്യം *fdt_blob)
{
#എങ്കിൽ defined(CONFIG_SPL_OF_LIBFDT)
	പൂർണ്ണസംഖ്യ err;

	എങ്കിൽ (!fdt_blob)
		തിരികെ;

	err = fdt_check_header(fdt_blob);
	എങ്കിൽ (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		തിരികെ;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	എങ്കിൽ (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		തിരികെ;
	}

	err = arch_fixup_fdt(fdt_blob);
	എങ്കിൽ (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		തിരികെ;
	}
#endif
}

പൂർണ്ണസംഖ്യ spl_reserve_video_from_ram_മുകൾ(ശൂന്യം)
{
	എങ്കിൽ (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		പൂർണ്ണസംഖ്യ ret;

		addr = gd->ram_മുകൾ;
		ret = video_reserve(&addr);
		എങ്കിൽ (ret)
			തിരികെ ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	തിരികെ 0;
}

ulong spl_get_image_pos(ശൂന്യം)
{
	എങ്കിൽ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		തിരികെ BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	എങ്കിൽ (xpl_next_phase() == PHASE_VPL)
		തിരികെ binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#എങ്കിൽ defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	എങ്കിൽ (xpl_next_phase() == PHASE_SPL)
		തിരികെ binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#എങ്കിൽ defined(CONFIG_SPL_BUILD)
	തിരികെ binman_sym(ulong, u_boot_any, image_pos);
#endif

	തിരികെ BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(ശൂന്യം)
{
	എങ്കിൽ (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		തിരികെ BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	എങ്കിൽ (xpl_next_phase() == PHASE_VPL)
		തിരികെ binman_sym(ulong, u_boot_vpl_any, size);
#endif
	തിരികെ xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(ശൂന്യം)
{
#ifdef CONFIG_VPL
	എങ്കിൽ (xpl_next_phase() == PHASE_VPL)
		തിരികെ CONFIG_VPL_TEXT_BASE;
#endif
	തിരികെ xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak ശൂന്യം spl_board_prepare_for_linux(ശൂന്യം)
{
	/* Nothing to do! */
}

__weak ശൂന്യം spl_board_prepare_for_optee(ശൂന്യം *fdt)
{
}

__weak സ്ഥിരം അക്ഷരം *spl_board_loader_name(u32 boot_device)
{
	തിരികെ ശൂന്യ;
}

#എങ്കിൽ CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak ശൂന്യം __noreturn jump_to_image_optee(ഘടന spl_image_info *spl_image)
{
	spl_optee_entry(ശൂന്യ, ശൂന്യ, spl_image->fdt_addr,
			(ശൂന്യം *)spl_image->entry_point);
}
#endif

__weak ശൂന്യം spl_board_prepare_for_boot(ശൂന്യം)
{
	/* Nothing to do! */
}

__weak ഘടന legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	തിരികെ map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

ശൂന്യം spl_set_header_raw_uboot(ഘടന spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#എങ്കിൽ CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#അല്ലെങ്കിൽ
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	എങ്കിൽ (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} അല്ലെങ്കിൽ {
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

__weak പൂർണ്ണസംഖ്യ spl_parse_board_header(ഘടന spl_image_info *spl_image,
				  സ്ഥിരം ഘടന spl_boot_device *bootdev,
				  സ്ഥിരം ശൂന്യം *image_header, size_t size)
{
	തിരികെ -EINVAL;
}

__weak പൂർണ്ണസംഖ്യ spl_parse_legacy_header(ഘടന spl_image_info *spl_image,
				   സ്ഥിരം ഘടന legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	തിരികെ -EINVAL;
}

പൂർണ്ണസംഖ്യ spl_parse_image_header(ഘടന spl_image_info *spl_image,
			   സ്ഥിരം ഘടന spl_boot_device *bootdev,
			   സ്ഥിരം ഘടന legacy_img_hdr *header)
{
	പൂർണ്ണസംഖ്യ ret;

	എങ്കിൽ (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		എങ്കിൽ (!ret)
			തിരികെ ret;
	}
	എങ്കിൽ (image_get_magic(header) == IH_MAGIC) {
		പൂർണ്ണസംഖ്യ ret;

		ret = spl_parse_legacy_header(spl_image, header);
		എങ്കിൽ (ret)
			തിരികെ ret;
		തിരികെ 0;
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		എങ്കിൽ (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			തിരികെ 0;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		എങ്കിൽ (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			തിരികെ 0;
		}
	}

	എങ്കിൽ (!spl_parse_board_header(spl_image, bootdev, (സ്ഥിരം ശൂന്യം *)header,
				    വലുപ്പം(*header)))
		തിരികെ 0;

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} അല്ലെങ്കിൽ {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		തിരികെ -EINVAL;
	}

	തിരികെ 0;
}

#എങ്കിൽ SPL_LOAD_USERS > 1
പൂർണ്ണസംഖ്യ spl_load(ഘടന spl_image_info *spl_image,
	     സ്ഥിരം ഘടന spl_boot_device *bootdev, ഘടന spl_load_info *info,
	     size_t size, size_t offset)
{
	തിരികെ _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak ശൂന്യം __noreturn jump_to_image(ഘടന spl_image_info *spl_image)
{
	തരനിർവചനം ശൂന്യം __noreturn (*image_entry_noargs_t)(ശൂന്യം);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#എങ്കിൽ CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_spl_handoff(ശൂന്യം)
{
	ഘടന spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, വലുപ്പം(ഘടന spl_handoff));
	എങ്കിൽ (!ho)
		തിരികെ -ENOENT;

	തിരികെ 0;
}

__weak പൂർണ്ണസംഖ്യ handoff_arch_save(ഘടന spl_handoff *ho)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ write_spl_handoff(ശൂന്യം)
{
	ഘടന spl_handoff *ho;
	പൂർണ്ണസംഖ്യ ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, വലുപ്പം(ഘടന spl_handoff));
	എങ്കിൽ (!ho)
		തിരികെ -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	എങ്കിൽ (ret)
		തിരികെ ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	തിരികെ 0;
}
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ setup_spl_handoff(ശൂന്യം) { തിരികെ 0; }
സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ write_spl_handoff(ശൂന്യം) { തിരികെ 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
സ്ഥിതം എണ്ണൽ bootstage_id get_bootstage_id(bool start)
{
	എണ്ണൽ xpl_phase_t phase = xpl_phase();

	എങ്കിൽ (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		തിരികെ start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	അല്ലെങ്കിൽ എങ്കിൽ (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		തിരികെ start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	അല്ലെങ്കിൽ
		തിരികെ start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_common_init(bool setup_malloc)
{
	പൂർണ്ണസംഖ്യ ret;

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	എങ്കിൽ (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	എങ്കിൽ (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		തിരികെ ret;
	}
	എങ്കിൽ (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		എങ്കിൽ (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#എങ്കിൽ CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	എങ്കിൽ (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		തിരികെ ret;
	}
#endif
	എങ്കിൽ (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		എങ്കിൽ (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			തിരികെ ret;
		}
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (ഘടന device_കണ്ണി **)gd_of_വേര്_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		എങ്കിൽ (ret)
			തിരികെ ret;
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		എങ്കിൽ (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			തിരികെ ret;
		}

		ret = dm_autoprobe();
		എങ്കിൽ (ret)
			തിരികെ ret;
	}

	തിരികെ 0;
}

ശൂന്യം spl_set_bd(ശൂന്യം)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	എങ്കിൽ (!gd->bd)
		gd->bd = &bdata;
}

പൂർണ്ണസംഖ്യ spl_early_init(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	എങ്കിൽ (ret)
		തിരികെ ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ spl_init(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_അട്ടി_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	എങ്കിൽ (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		എങ്കിൽ (ret)
			തിരികെ ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	തിരികെ 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak ശൂന്യം board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak പൂർണ്ണസംഖ്യ spl_check_board_image(ഘടന spl_image_info *spl_image,
				 സ്ഥിരം ഘടന spl_boot_device *bootdev)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_load_image(ഘടന spl_image_info *spl_image,
			  ഘടന spl_image_loader *loader)
{
	പൂർണ്ണസംഖ്യ ret;
	ഘടന spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = ശൂന്യ;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	എങ്കിൽ (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned അക്ഷരം *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		എങ്കിൽ (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	എങ്കിൽ (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	തിരികെ ret;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ boot_from_devices(ഘടന spl_image_info *spl_image,
			     u32 spl_boot_list[], പൂർണ്ണസംഖ്യ എണ്ണം)
{
	ഘടന spl_image_loader *drv =
		ll_entry_start(ഘടന spl_image_loader, spl_image_loader);
	സ്ഥിരം പൂർണ്ണസംഖ്യ n_ents =
		ll_entry_എണ്ണം(ഘടന spl_image_loader, spl_image_loader);
	പൂർണ്ണസംഖ്യ ret = -ENODEV;
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < എണ്ണം && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		ഘടന spl_image_loader *loader;
		പൂർണ്ണസംഖ്യ bootdev = spl_boot_list[i];

		എങ്കിൽ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		വേണ്ടി (loader = drv; loader != drv + n_ents; loader++) {
			എങ്കിൽ (loader && bootdev != loader->boot_device)
				തുടരുക;
			എങ്കിൽ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			എങ്കിൽ (!ret) {
				spl_image->boot_device = bootdev;
				തിരികെ 0;
			}
			printf("Error: %d\n", ret);
		}

		എങ്കിൽ (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			എങ്കിൽ (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			അല്ലെങ്കിൽ
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	തിരികെ ret;
}

ശൂന്യം board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	ഘടന spl_image_info spl_image;
	പൂർണ്ണസംഖ്യ ret, os;
	ശൂന്യം *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	എങ്കിൽ (!(gd->flags & GD_FLG_SPL_INIT)) {
		എങ്കിൽ (spl_init())
			hang();
	}
	timer_init();
	എങ്കിൽ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		എങ്കിൽ (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(HANDOFF)) {
		പൂർണ്ണസംഖ്യ ret;

		ret = setup_spl_handoff();
		എങ്കിൽ (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	എങ്കിൽ (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	എങ്കിൽ (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		എങ്കിൽ (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	എങ്കിൽ (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	എങ്കിൽ (CONFIG_IS_ENABLED(DM_STATS)) {
		ഘടന dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', വലുപ്പം(spl_image));
	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (ശൂന്യം *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	എങ്കിൽ (ret) {
		എങ്കിൽ (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		അല്ലെങ്കിൽ
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	എങ്കിൽ (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		എങ്കിൽ (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (ശൂന്യം *)SPL_PAYLOAD_ARGS_ADDR;
		അല്ലെങ്കിൽ
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} അല്ലെങ്കിൽ {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	എങ്കിൽ (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		ഘടന udevice *dev;
		പൂർണ്ണസംഖ്യ rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		എങ്കിൽ (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			എങ്കിൽ (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		എങ്കിൽ (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		എങ്കിൽ (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	എങ്കിൽ (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		എങ്കിൽ (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	എങ്കിൽ (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		പൂർണ്ണസംഖ്യ ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		എങ്കിൽ (ret) {
			എങ്കിൽ (xpl_phase() == PHASE_VPL)
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
ശൂന്യം preloader_console_init(ശൂന്യം)
{
#എങ്കിൽ CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#എങ്കിൽ CONFIG_IS_ENABLED(BANNER_PRINT)
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
__weak ശൂന്യം spl_relocate_അട്ടി_check(ശൂന്യം)
{
#എങ്കിൽ CONFIG_IS_ENABLED(SYS_REPORT_അട്ടി_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong അട്ടി_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_അട്ടി);
	u8 *ptr = (u8 *)അട്ടി_bottom;
	ulong i;

	വേണ്ടി (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_അട്ടി); i++) {
		എങ്കിൽ (*ptr != CONFIG_VAL(SYS_അട്ടി_F_CHECK_BYTE))
			നിർത്തുക;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_അട്ടി) - i);
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
ulong spl_relocate_അട്ടി_gd(ശൂന്യം)
{
#എങ്കിൽ CONFIG_IS_ENABLED(അട്ടി_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_അട്ടി_R_ADDR;

	എങ്കിൽ (CONFIG_IS_ENABLED(SYS_REPORT_അട്ടി_F_USAGE))
		spl_relocate_അട്ടി_check();

#എങ്കിൽ defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	എങ്കിൽ (CONFIG_SPL_അട്ടി_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_അട്ടി_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_അട്ടി_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(വലുപ്പം(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (ശൂന്യം *)gd, വലുപ്പം(gd_t));
#എങ്കിൽ CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#എങ്കിൽ CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#എങ്കിൽ !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	തിരികെ ptr;
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif
}

#എങ്കിൽ defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
ശൂന്യം bootcount_store(ulong a)
{
}

ulong bootcount_load(ശൂന്യം)
{
	തിരികെ 0;
}
#endif
