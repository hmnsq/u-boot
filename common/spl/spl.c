// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <bloblist.h>
#અન્તર્ભાવ <binman_sym.h>
#અન્તર્ભાવ <bootstage.h>
#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <handoff.h>
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <init.h>
#અન્તર્ભાવ <irq_func.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <of_live.h>
#અન્તર્ભાવ <serial.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <system-constants.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm-generic/gpio.h>
#અન્તર્ભાવ <nand.h>
#અન્તર્ભાવ <fat.h>
#અન્તર્ભાવ <u-boot/crc.h>
#જો CONFIG_IS_ENABLED(BANNER_PRINT)
#અન્તર્ભાવ <timestamp.h>
#endif
#અન્તર્ભાવ <version.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <dm/root.h>
#અન્તર્ભાવ <dm/util.h>
#અન્તર્ભાવ <dm/device-internal.h>
#અન્તર્ભાવ <dm/uclass-internal.h>
#અન્તર્ભાવ <linux/compiler.h>
#અન્તર્ભાવ <fdt_support.h>
#અન્તર્ભાવ <bootcount.h>
#અન્તર્ભાવ <wdt.h>
#અન્તર્ભાવ <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = ખાલી;

#જો CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#જો defined(CONFIG_SPL_BUILD)
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
સ્થિર સંરચના bd_info bdata __attribute__ ((section(".data")));

#જો CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak શૂન્ય show_boot_progress(પૂર્ણાંક val) {}
#endif

#જો defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak પૂર્ણાંક dram_init_banksize(શૂન્ય)
{
	પાછા 0;
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
#જો CONFIG_IS_ENABLED(OS_BOOT)
__weak પૂર્ણાંક spl_start_uboot(શૂન્ય)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	પાછા 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
પૂર્ણાંક __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 પાછા 1;
}

પૂર્ણાંક __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 પાછા 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
શૂન્ય __weak spl_perform_arch_fixups(સંરચના spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
શૂન્ય __weak spl_perform_board_fixups(સંરચના spl_image_info *spl_image)
{
}

શૂન્ય spl_fixup_fdt(શૂન્ય *fdt_blob)
{
#જો defined(CONFIG_SPL_OF_LIBFDT)
	પૂર્ણાંક err;

	જો (!fdt_blob)
		પાછા;

	err = fdt_check_header(fdt_blob);
	જો (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		પાછા;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	જો (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		પાછા;
	}

	err = arch_fixup_fdt(fdt_blob);
	જો (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		પાછા;
	}
#endif
}

પૂર્ણાંક spl_reserve_video_from_ram_ટોચ(શૂન્ય)
{
	જો (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		પૂર્ણાંક ret;

		addr = gd->ram_ટોચ;
		ret = video_reserve(&addr);
		જો (ret)
			પાછા ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	પાછા 0;
}

ulong spl_get_image_pos(શૂન્ય)
{
	જો (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		પાછા BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	જો (xpl_next_phase() == PHASE_VPL)
		પાછા binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#જો defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	જો (xpl_next_phase() == PHASE_SPL)
		પાછા binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#જો defined(CONFIG_SPL_BUILD)
	પાછા binman_sym(ulong, u_boot_any, image_pos);
#endif

	પાછા BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(શૂન્ય)
{
	જો (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		પાછા BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	જો (xpl_next_phase() == PHASE_VPL)
		પાછા binman_sym(ulong, u_boot_vpl_any, size);
#endif
	પાછા xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(શૂન્ય)
{
#ifdef CONFIG_VPL
	જો (xpl_next_phase() == PHASE_VPL)
		પાછા CONFIG_VPL_TEXT_BASE;
#endif
	પાછા xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak શૂન્ય spl_board_prepare_for_linux(શૂન્ય)
{
	/* Nothing to do! */
}

__weak શૂન્ય spl_board_prepare_for_optee(શૂન્ય *fdt)
{
}

__weak અચલ અક્ષર *spl_board_loader_name(u32 boot_device)
{
	પાછા ખાલી;
}

#જો CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak શૂન્ય __noreturn jump_to_image_optee(સંરચના spl_image_info *spl_image)
{
	spl_optee_entry(ખાલી, ખાલી, spl_image->fdt_addr,
			(શૂન્ય *)spl_image->entry_point);
}
#endif

__weak શૂન્ય spl_board_prepare_for_boot(શૂન્ય)
{
	/* Nothing to do! */
}

__weak સંરચના legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	પાછા map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

શૂન્ય spl_set_header_raw_uboot(સંરચના spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#જો CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#નહીંતર
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	જો (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} નહીંતર {
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

__weak પૂર્ણાંક spl_parse_board_header(સંરચના spl_image_info *spl_image,
				  અચલ સંરચના spl_boot_device *bootdev,
				  અચલ શૂન્ય *image_header, size_t size)
{
	પાછા -EINVAL;
}

__weak પૂર્ણાંક spl_parse_legacy_header(સંરચના spl_image_info *spl_image,
				   અચલ સંરચના legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	પાછા -EINVAL;
}

પૂર્ણાંક spl_parse_image_header(સંરચના spl_image_info *spl_image,
			   અચલ સંરચના spl_boot_device *bootdev,
			   અચલ સંરચના legacy_img_hdr *header)
{
	પૂર્ણાંક ret;

	જો (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		જો (!ret)
			પાછા ret;
	}
	જો (image_get_magic(header) == IH_MAGIC) {
		પૂર્ણાંક ret;

		ret = spl_parse_legacy_header(spl_image, header);
		જો (ret)
			પાછા ret;
		પાછા 0;
	}

	જો (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	જો (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		જો (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			પાછા 0;
		}
	} નહીંતર જો (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		જો (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			પાછા 0;
		}
	}

	જો (!spl_parse_board_header(spl_image, bootdev, (અચલ શૂન્ય *)header,
				    કદ(*header)))
		પાછા 0;

	જો (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} નહીંતર {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		પાછા -EINVAL;
	}

	પાછા 0;
}

#જો SPL_LOAD_USERS > 1
પૂર્ણાંક spl_load(સંરચના spl_image_info *spl_image,
	     અચલ સંરચના spl_boot_device *bootdev, સંરચના spl_load_info *info,
	     size_t size, size_t offset)
{
	પાછા _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak શૂન્ય __noreturn jump_to_image(સંરચના spl_image_info *spl_image)
{
	પ્રકારવ્યાખ્યા શૂન્ય __noreturn (*image_entry_noargs_t)(શૂન્ય);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#જો CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
સ્થિર પૂર્ણાંક setup_spl_handoff(શૂન્ય)
{
	સંરચના spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, કદ(સંરચના spl_handoff));
	જો (!ho)
		પાછા -ENOENT;

	પાછા 0;
}

__weak પૂર્ણાંક handoff_arch_save(સંરચના spl_handoff *ho)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક write_spl_handoff(શૂન્ય)
{
	સંરચના spl_handoff *ho;
	પૂર્ણાંક ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, કદ(સંરચના spl_handoff));
	જો (!ho)
		પાછા -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	જો (ret)
		પાછા ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	પાછા 0;
}
#નહીંતર
સ્થિર અંતઃરેખ પૂર્ણાંક setup_spl_handoff(શૂન્ય) { પાછા 0; }
સ્થિર અંતઃરેખ પૂર્ણાંક write_spl_handoff(શૂન્ય) { પાછા 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
સ્થિર ગણના bootstage_id get_bootstage_id(bool start)
{
	ગણના xpl_phase_t phase = xpl_phase();

	જો (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		પાછા start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	નહીંતર જો (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		પાછા start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	નહીંતર
		પાછા start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

સ્થિર પૂર્ણાંક spl_common_init(bool setup_malloc)
{
	પૂર્ણાંક ret;

#જો CONFIG_IS_ENABLED(SYS_MALLOC_F)
	જો (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	જો (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		પાછા ret;
	}
	જો (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		જો (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#જો CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	જો (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		પાછા ret;
	}
#endif
	જો (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		જો (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			પાછા ret;
		}
	}
	જો (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (સંરચના device_ગાંઠ **)gd_of_મૂળ_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		જો (ret)
			પાછા ret;
	}
	જો (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		જો (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			પાછા ret;
		}

		ret = dm_autoprobe();
		જો (ret)
			પાછા ret;
	}

	પાછા 0;
}

શૂન્ય spl_set_bd(શૂન્ય)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	જો (!gd->bd)
		gd->bd = &bdata;
}

પૂર્ણાંક spl_early_init(શૂન્ય)
{
	પૂર્ણાંક ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	જો (ret)
		પાછા ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	પાછા 0;
}

પૂર્ણાંક spl_init(શૂન્ય)
{
	પૂર્ણાંક ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_ઢગલો_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	જો (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		જો (ret)
			પાછા ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	પાછા 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak શૂન્ય board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak પૂર્ણાંક spl_check_board_image(સંરચના spl_image_info *spl_image,
				 અચલ સંરચના spl_boot_device *bootdev)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_load_image(સંરચના spl_image_info *spl_image,
			  સંરચના spl_image_loader *loader)
{
	પૂર્ણાંક ret;
	સંરચના spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = ખાલી;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	જો (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned અક્ષર *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		જો (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	જો (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	પાછા ret;
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
સ્થિર પૂર્ણાંક boot_from_devices(સંરચના spl_image_info *spl_image,
			     u32 spl_boot_list[], પૂર્ણાંક ગણતરી)
{
	સંરચના spl_image_loader *drv =
		ll_entry_start(સંરચના spl_image_loader, spl_image_loader);
	અચલ પૂર્ણાંક n_ents =
		ll_entry_ગણતરી(સંરચના spl_image_loader, spl_image_loader);
	પૂર્ણાંક ret = -ENODEV;
	પૂર્ણાંક i;

	માટે (i = 0; i < ગણતરી && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		સંરચના spl_image_loader *loader;
		પૂર્ણાંક bootdev = spl_boot_list[i];

		જો (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		માટે (loader = drv; loader != drv + n_ents; loader++) {
			જો (loader && bootdev != loader->boot_device)
				ચાલુ;
			જો (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			જો (!ret) {
				spl_image->boot_device = bootdev;
				પાછા 0;
			}
			printf("Error: %d\n", ret);
		}

		જો (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			જો (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			નહીંતર
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	પાછા ret;
}

શૂન્ય board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	સંરચના spl_image_info spl_image;
	પૂર્ણાંક ret, os;
	શૂન્ય *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	જો (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	જો (!(gd->flags & GD_FLG_SPL_INIT)) {
		જો (spl_init())
			hang();
	}
	timer_init();
	જો (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		જો (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	જો (CONFIG_IS_ENABLED(HANDOFF)) {
		પૂર્ણાંક ret;

		ret = setup_spl_handoff();
		જો (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	જો (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	જો (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	જો (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	જો (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	જો (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		જો (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	જો (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	જો (CONFIG_IS_ENABLED(DM_STATS)) {
		સંરચના dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', કદ(spl_image));
	જો (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (શૂન્ય *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	જો (ret) {
		જો (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		નહીંતર
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	જો (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} નહીંતર જો (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} નહીંતર જો (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} નહીંતર જો (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} નહીંતર જો (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		જો (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (શૂન્ય *)SPL_PAYLOAD_ARGS_ADDR;
		નહીંતર
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} નહીંતર {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	જો (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	જો (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	જો (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		સંરચના udevice *dev;
		પૂર્ણાંક rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		જો (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			જો (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	જો (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		જો (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	જો (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		જો (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	જો (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		જો (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	જો (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		પૂર્ણાંક ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		જો (ret) {
			જો (xpl_phase() == PHASE_VPL)
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
શૂન્ય preloader_console_init(શૂન્ય)
{
#જો CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#જો CONFIG_IS_ENABLED(BANNER_PRINT)
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
__weak શૂન્ય spl_relocate_ઢગલો_check(શૂન્ય)
{
#જો CONFIG_IS_ENABLED(SYS_REPORT_ઢગલો_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong ઢગલો_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_ઢગલો);
	u8 *ptr = (u8 *)ઢગલો_bottom;
	ulong i;

	માટે (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_ઢગલો); i++) {
		જો (*ptr != CONFIG_VAL(SYS_ઢગલો_F_CHECK_BYTE))
			તોડો;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_ઢગલો) - i);
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
ulong spl_relocate_ઢગલો_gd(શૂન્ય)
{
#જો CONFIG_IS_ENABLED(ઢગલો_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_ઢગલો_R_ADDR;

	જો (CONFIG_IS_ENABLED(SYS_REPORT_ઢગલો_F_USAGE))
		spl_relocate_ઢગલો_check();

#જો defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	જો (CONFIG_SPL_ઢગલો_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_ઢગલો_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_ઢગલો_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(કદ(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (શૂન્ય *)gd, કદ(gd_t));
#જો CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#જો CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#જો !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	પાછા ptr;
#નહીંતર
	પાછા 0;
#endif
}

#જો defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
શૂન્ય bootcount_store(ulong a)
{
}

ulong bootcount_load(શૂન્ય)
{
	પાછા 0;
}
#endif
