// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010
 * Texas Instruments, <www.ti.com>
 *
 * Aneesh V <aneesh@ti.com>
 */

#شامل <config.h>
#شامل <bloblist.h>
#شامل <binman_sym.h>
#شامل <bootstage.h>
#شامل <dm.h>
#شامل <handoff.h>
#شامل <hang.h>
#شامل <init.h>
#شامل <irq_func.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <of_live.h>
#شامل <serial.h>
#شامل <spl.h>
#شامل <spl_load.h>
#شامل <system-constants.h>
#شامل <asm/global_data.h>
#شامل <asm-generic/gpio.h>
#شامل <nand.h>
#شامل <fat.h>
#شامل <u-boot/crc.h>
#اگر CONFIG_IS_ENABLED(BANNER_PRINT)
#شامل <timestamp.h>
#endif
#شامل <version.h>
#شامل <image.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <dm/root.h>
#شامل <dm/util.h>
#شامل <dm/device-internal.h>
#شامل <dm/uclass-internal.h>
#شامل <linux/compiler.h>
#شامل <fdt_support.h>
#شامل <bootcount.h>
#شامل <wdt.h>
#شامل <video.h>

DECLARE_GLOBAL_DATA_PTR;
DECLARE_BINMAN_MAGIC_SYM;

u32 *boot_params_ptr = پوچ;

#اگر CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS)
/* See spl.h for information about this */
#اگر defined(CONFIG_SPL_BUILD)
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
ایستا ساختار bd_info bdata __attribute__ ((section(".data")));

#اگر CONFIG_IS_ENABLED(SHOW_BOOT_PROGRESS)
/*
 * Board-specific Platform code can reimplement show_boot_progress () if needed
 */
__weak تهی show_boot_progress(صحیح val) {}
#endif

#اگر defined(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) || \
	defined(CONFIG_SPL_ATF)
/* weak, default platform-specific function to initialize dram banks */
__weak صحیح dram_init_banksize(تهی)
{
	بازگشت 0;
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
#اگر CONFIG_IS_ENABLED(OS_BOOT)
__weak صحیح spl_start_uboot(تهی)
{
	puts(PHASE_PROMPT
	     "Please implement spl_start_uboot() for your board\n");
	puts(PHASE_PROMPT "Direct Linux boot not active!\n");
	بازگشت 1;
}

/*
 * Weak default function for arch specific zImage check. Return zero
 * and fill start and end address if image is recognized.
 */
صحیح __weak bootz_setup(ulong image, ulong *start, ulong *end)
{
	 بازگشت 1;
}

صحیح __weak booti_setup(ulong image, ulong *relocated_addr, ulong *size, bool force_reloc)
{
	 بازگشت 1;
}
#endif

/* Weak default function for arch specific fixups to the spl_image_info */
تهی __weak spl_perform_arch_fixups(ساختار spl_image_info *spl_image)
{
}

/* Weak default function for board specific fixups to the spl_image_info */
تهی __weak spl_perform_board_fixups(ساختار spl_image_info *spl_image)
{
}

تهی spl_fixup_fdt(تهی *fdt_blob)
{
#اگر defined(CONFIG_SPL_OF_LIBFDT)
	صحیح err;

	اگر (!fdt_blob)
		بازگشت;

	err = fdt_check_header(fdt_blob);
	اگر (err < 0) {
		printf("fdt_root: %s\n", fdt_strerror(err));
		بازگشت;
	}

	/* fixup the memory dt node */
	err = fdt_shrink_to_minimum(fdt_blob, 0);
	اگر (err == 0) {
		printf(PHASE_PROMPT "fdt_shrink_to_minimum err - %d\n", err);
		بازگشت;
	}

	err = arch_fixup_fdt(fdt_blob);
	اگر (err) {
		printf(PHASE_PROMPT "arch_fixup_fdt err - %d\n", err);
		بازگشت;
	}
#endif
}

صحیح spl_reserve_video_from_ram_بالاترین(تهی)
{
	اگر (CONFIG_IS_ENABLED(VIDEO)) {
		ulong addr;
		صحیح ret;

		addr = gd->ram_بالاترین;
		ret = video_reserve(&addr);
		اگر (ret)
			بازگشت ret;
		debug("Reserving %luk for video at: %08lx\n",
		      ((unsigned long)gd->relocaddr - addr) >> 10, addr);
		gd->relocaddr = addr;
	}

	بازگشت 0;
}

ulong spl_get_image_pos(تهی)
{
	اگر (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		بازگشت BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	اگر (xpl_next_phase() == PHASE_VPL)
		بازگشت binman_sym(ulong, u_boot_vpl_any, image_pos);
#endif
#اگر defined(CONFIG_TPL) && !defined(CONFIG_VPL)
	اگر (xpl_next_phase() == PHASE_SPL)
		بازگشت binman_sym(ulong, u_boot_spl_any, image_pos);
#endif
#اگر defined(CONFIG_SPL_BUILD)
	بازگشت binman_sym(ulong, u_boot_any, image_pos);
#endif

	بازگشت BINMAN_SYM_MISSING;
}

ulong spl_get_image_size(تهی)
{
	اگر (!CONFIG_IS_ENABLED(BINMAN_UBOOT_SYMBOLS))
		بازگشت BINMAN_SYM_MISSING;

#ifdef CONFIG_VPL
	اگر (xpl_next_phase() == PHASE_VPL)
		بازگشت binman_sym(ulong, u_boot_vpl_any, size);
#endif
	بازگشت xpl_next_phase() == PHASE_SPL ?
		binman_sym(ulong, u_boot_spl_any, size) :
		binman_sym(ulong, u_boot_any, size);
}

ulong spl_get_image_text_base(تهی)
{
#ifdef CONFIG_VPL
	اگر (xpl_next_phase() == PHASE_VPL)
		بازگشت CONFIG_VPL_TEXT_BASE;
#endif
	بازگشت xpl_next_phase() == PHASE_SPL ? CONFIG_SPL_TEXT_BASE :
		CONFIG_TEXT_BASE;
}

/*
 * Weak default function for board specific cleanup/preparation before
 * Linux boot. Some boards/platforms might not need it, so just provide
 * an empty stub here.
 */
__weak تهی spl_board_prepare_for_linux(تهی)
{
	/* Nothing to do! */
}

__weak تهی spl_board_prepare_for_optee(تهی *fdt)
{
}

__weak ثابت نویسه *spl_board_loader_name(u32 boot_device)
{
	بازگشت پوچ;
}

#اگر CONFIG_IS_ENABLED(OPTEE_IMAGE)
__weak تهی __noreturn jump_to_image_optee(ساختار spl_image_info *spl_image)
{
	spl_optee_entry(پوچ, پوچ, spl_image->fdt_addr,
			(تهی *)spl_image->entry_point);
}
#endif

__weak تهی spl_board_prepare_for_boot(تهی)
{
	/* Nothing to do! */
}

__weak ساختار legacy_img_hdr *spl_get_load_buffer(ssize_t offset, size_t size)
{
	بازگشت map_sysmem(CONFIG_TEXT_BASE + offset, 0);
}

تهی spl_set_header_raw_uboot(ساختار spl_image_info *spl_image)
{
	ulong u_boot_pos = spl_get_image_pos();

#اگر CONFIG_SYS_MONITOR_LEN != 0
	spl_image->size = CONFIG_SYS_MONITOR_LEN;
#وگرنه
	/* Unknown U-Boot size, let's assume it will not be more than 200 KB */
	spl_image->size = 200 * 1024;
#endif

	/*
	 * Binman error cases: address of the end of the previous region or the
	 * start of the image's entry area (usually 0) if there is no previous
	 * region.
	 */
	اگر (u_boot_pos && u_boot_pos != BINMAN_SYM_MISSING) {
		/* Binman does not support separated entry addresses */
		spl_image->entry_point = spl_get_image_text_base();
		spl_image->load_addr = spl_get_image_text_base();
		spl_image->size = spl_get_image_size();
		log_debug("Next load addr %lx\n", spl_image->load_addr);
	} وگرنه {
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

__weak صحیح spl_parse_board_header(ساختار spl_image_info *spl_image,
				  ثابت ساختار spl_boot_device *bootdev,
				  ثابت تهی *image_header, size_t size)
{
	بازگشت -EINVAL;
}

__weak صحیح spl_parse_legacy_header(ساختار spl_image_info *spl_image,
				   ثابت ساختار legacy_img_hdr *header)
{
	/* LEGACY image not supported */
	debug("Legacy boot image support not enabled, proceeding to other boot methods\n");
	بازگشت -EINVAL;
}

صحیح spl_parse_image_header(ساختار spl_image_info *spl_image,
			   ثابت ساختار spl_boot_device *bootdev,
			   ثابت ساختار legacy_img_hdr *header)
{
	صحیح ret;

	اگر (CONFIG_IS_ENABLED(LOAD_FIT_FULL)) {
		ret = spl_load_fit_image(spl_image, header);

		اگر (!ret)
			بازگشت ret;
	}
	اگر (image_get_magic(header) == IH_MAGIC) {
		صحیح ret;

		ret = spl_parse_legacy_header(spl_image, header);
		اگر (ret)
			بازگشت ret;
		بازگشت 0;
	}

	اگر (IS_ENABLED(CONFIG_SPL_PANIC_ON_RAW_IMAGE)) {
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

	اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT) && IS_ENABLED(CONFIG_SPL_BOOTI)) {
		ulong start, size;

		اگر (!booti_setup((ulong)header, &start, &size, 0)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = start;
			spl_image->entry_point = start;
			spl_image->size = size;
			debug(PHASE_PROMPT
			      "payload Image, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			بازگشت 0;
		}
	} وگرنه اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT) &&
		   IS_ENABLED(CONFIG_SPL_BOOTZ)) {
		ulong start, end;

		اگر (!bootz_setup((ulong)header, &start, &end)) {
			spl_image->name = "Linux";
			spl_image->os = IH_OS_LINUX;
			spl_image->load_addr = CONFIG_SYS_LOAD_ADDR;
			spl_image->entry_point = CONFIG_SYS_LOAD_ADDR;
			spl_image->size = end - start;
			debug(PHASE_PROMPT
			      "payload zImage, load addr: 0x%lx size: %d\n",
			      spl_image->load_addr, spl_image->size);
			بازگشت 0;
		}
	}

	اگر (!spl_parse_board_header(spl_image, bootdev, (ثابت تهی *)header,
				    اندازه(*header)))
		بازگشت 0;

	اگر (IS_ENABLED(CONFIG_SPL_RAW_IMAGE_SUPPORT)) {
		/* Signature not found - assume u-boot.bin */
		debug("mkimage signature not found - ih_magic = %x\n",
		      header->ih_magic);
		spl_set_header_raw_uboot(spl_image);
	} وگرنه {
		/* RAW image not supported, proceed to other boot methods. */
		debug("Raw boot image support not enabled, proceeding to other boot methods\n");
		بازگشت -EINVAL;
	}

	بازگشت 0;
}

#اگر SPL_LOAD_USERS > 1
صحیح spl_load(ساختار spl_image_info *spl_image,
	     ثابت ساختار spl_boot_device *bootdev, ساختار spl_load_info *info,
	     size_t size, size_t offset)
{
	بازگشت _spl_load(spl_image, bootdev, info, size, offset);
}
#endif

__weak تهی __noreturn jump_to_image(ساختار spl_image_info *spl_image)
{
	تعریفنوع تهی __noreturn (*image_entry_noargs_t)(تهی);

	image_entry_noargs_t image_entry =
		(image_entry_noargs_t)spl_image->entry_point;

	debug("image entry point: 0x%lx\n", spl_image->entry_point);
	image_entry();
}

#اگر CONFIG_IS_ENABLED(HANDOFF)
/**
 * Set up the SPL hand-off information
 *
 * This is initially empty (zero) but can be written by
 */
ایستا صحیح setup_spl_handoff(تهی)
{
	ساختار spl_handoff *ho;

	ho = bloblist_ensure(BLOBLISTT_U_BOOT_SPL_HANDOFF, اندازه(ساختار spl_handoff));
	اگر (!ho)
		بازگشت -ENOENT;

	بازگشت 0;
}

__weak صحیح handoff_arch_save(ساختار spl_handoff *ho)
{
	بازگشت 0;
}

ایستا صحیح write_spl_handoff(تهی)
{
	ساختار spl_handoff *ho;
	صحیح ret;

	ho = bloblist_find(BLOBLISTT_U_BOOT_SPL_HANDOFF, اندازه(ساختار spl_handoff));
	اگر (!ho)
		بازگشت -ENOENT;
	handoff_save_dram(ho);
	ret = handoff_arch_save(ho);
	اگر (ret)
		بازگشت ret;
	debug(PHASE_PROMPT "Wrote SPL handoff\n");

	بازگشت 0;
}
#وگرنه
ایستا درونخطی صحیح setup_spl_handoff(تهی) { بازگشت 0; }
ایستا درونخطی صحیح write_spl_handoff(تهی) { بازگشت 0; }

#endif /* HANDOFF */

/**
 * get_bootstage_id() - Get the bootstage ID to emit
 *
 * @start: true if this is for starting SPL, false for ending it
 * Return: bootstage ID to use
 */
ایستا شمارش bootstage_id get_bootstage_id(bool start)
{
	شمارش xpl_phase_t phase = xpl_phase();

	اگر (IS_ENABLED(CONFIG_TPL_BUILD) && phase == PHASE_TPL)
		بازگشت start ? BOOTSTAGE_ID_START_TPL : BOOTSTAGE_ID_END_TPL;
	وگرنه اگر (IS_ENABLED(CONFIG_VPL_BUILD) && phase == PHASE_VPL)
		بازگشت start ? BOOTSTAGE_ID_START_VPL : BOOTSTAGE_ID_END_VPL;
	وگرنه
		بازگشت start ? BOOTSTAGE_ID_START_SPL : BOOTSTAGE_ID_END_SPL;
}

ایستا صحیح spl_common_init(bool setup_malloc)
{
	صحیح ret;

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (setup_malloc) {
#ifdef CFG_MALLOC_F_ADDR
		gd->malloc_base = CFG_MALLOC_F_ADDR;
#endif
		gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
		gd->malloc_ptr = 0;
	}
#endif
	ret = bootstage_init(xpl_is_first_phase());
	اگر (ret) {
		debug("%s: Failed to set up bootstage: ret=%d\n", __func__,
		      ret);
		بازگشت ret;
	}
	اگر (!xpl_is_first_phase()) {
		ret = bootstage_unstash_default();
		اگر (ret)
			log_debug("Failed to unstash bootstage: ret=%d\n", ret);
	}
	bootstage_mark_name(get_bootstage_id(true), xpl_name(xpl_phase()));
#اگر CONFIG_IS_ENABLED(LOG)
	ret = log_init();
	اگر (ret) {
		debug("%s: Failed to set up logging\n", __func__);
		بازگشت ret;
	}
#endif
	اگر (CONFIG_IS_ENABLED(OF_REAL)) {
		ret = fdtdec_setup();
		اگر (ret) {
			debug("fdtdec_setup() returned error %d\n", ret);
			بازگشت ret;
		}
	}
	اگر (CONFIG_IS_ENABLED(OF_LIVE)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_OF_LIVE, "of_live");
		ret = of_live_build(gd->fdt_blob,
				    (ساختار device_گره **)gd_of_ریشه_ptr());
		bootstage_accum(BOOTSTAGE_ID_ACCUM_OF_LIVE);
		اگر (ret)
			بازگشت ret;
	}
	اگر (CONFIG_IS_ENABLED(DM)) {
		bootstage_start(BOOTSTAGE_ID_ACCUM_DM_SPL,
				xpl_phase() == PHASE_TPL ? "dm tpl" : "dm_spl");
		/* With CONFIG_SPL_OF_PLATDATA, bring in all devices */
		ret = dm_init_and_scan(!CONFIG_IS_ENABLED(OF_PLATDATA));
		bootstage_accum(BOOTSTAGE_ID_ACCUM_DM_SPL);
		اگر (ret) {
			debug("dm_init_and_scan() returned error %d\n", ret);
			بازگشت ret;
		}

		ret = dm_autoprobe();
		اگر (ret)
			بازگشت ret;
	}

	بازگشت 0;
}

تهی spl_set_bd(تهی)
{
	/*
	 * NOTE: On some platforms (e.g. x86) bdata may be in flash and not
	 * writeable.
	 */
	اگر (!gd->bd)
		gd->bd = &bdata;
}

صحیح spl_early_init(تهی)
{
	صحیح ret;

	debug("%s\n", __func__);

	ret = spl_common_init(true);
	اگر (ret)
		بازگشت ret;
	gd->flags |= GD_FLG_SPL_EARLY_INIT;

	بازگشت 0;
}

صحیح spl_init(تهی)
{
	صحیح ret;
	bool setup_malloc = !(IS_ENABLED(CONFIG_SPL_پشته_R) &&
			IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIMPLE));

	debug("%s\n", __func__);

	اگر (!(gd->flags & GD_FLG_SPL_EARLY_INIT)) {
		ret = spl_common_init(setup_malloc);
		اگر (ret)
			بازگشت ret;
	}
	gd->flags |= GD_FLG_SPL_INIT;

	بازگشت 0;
}

#ifndef BOOT_DEVICE_NONE
#define BOOT_DEVICE_NONE 0xdeadbeef
#endif

__weak تهی board_boot_order(u32 *spl_boot_list)
{
	spl_boot_list[0] = spl_boot_device();
}

__weak صحیح spl_check_board_image(ساختار spl_image_info *spl_image,
				 ثابت ساختار spl_boot_device *bootdev)
{
	بازگشت 0;
}

ایستا صحیح spl_load_image(ساختار spl_image_info *spl_image,
			  ساختار spl_image_loader *loader)
{
	صحیح ret;
	ساختار spl_boot_device bootdev;

	bootdev.boot_device = loader->boot_device;
	bootdev.boot_device_name = پوچ;

	ret = loader->load_image(spl_image, &bootdev);
#ifdef CONFIG_SPL_LEGACY_IMAGE_CRC_CHECK
	اگر (!ret && spl_image->dcrc_length) {
		/* check data crc */
		ulong dcrc = crc32_wd(0, (unsigned نویسه *)spl_image->dcrc_data,
				      spl_image->dcrc_length, CHUNKSZ_CRC32);
		اگر (dcrc != spl_image->dcrc) {
			puts("SPL: Image data CRC check failed!\n");
			ret = -EINVAL;
		}
	}
#endif
	اگر (!ret)
		ret = spl_check_board_image(spl_image, &bootdev);

	بازگشت ret;
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
ایستا صحیح boot_from_devices(ساختار spl_image_info *spl_image,
			     u32 spl_boot_list[], صحیح شمار)
{
	ساختار spl_image_loader *drv =
		ll_entry_start(ساختار spl_image_loader, spl_image_loader);
	ثابت صحیح n_ents =
		ll_entry_شمار(ساختار spl_image_loader, spl_image_loader);
	صحیح ret = -ENODEV;
	صحیح i;

	برای (i = 0; i < شمار && spl_boot_list[i] != BOOT_DEVICE_NONE; i++) {
		ساختار spl_image_loader *loader;
		صحیح bootdev = spl_boot_list[i];

		اگر (CONFIG_IS_ENABLED(SHOW_ERRORS))
			ret = -ENXIO;
		برای (loader = drv; loader != drv + n_ents; loader++) {
			اگر (loader && bootdev != loader->boot_device)
				ادامه;
			اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
				printf("Trying to boot from %s\n",
				       spl_loader_name(loader));
			}

			ret = spl_load_image(spl_image, loader);
			اگر (!ret) {
				spl_image->boot_device = bootdev;
				بازگشت 0;
			}
			printf("Error: %d\n", ret);
		}

		اگر (!CONFIG_IS_ENABLED(SILENT_CONSOLE)) {
			اگر (CONFIG_IS_ENABLED(SHOW_ERRORS))
				printf(PHASE_PROMPT
				       "Unsupported Boot Device %d\n",
				       bootdev);
			وگرنه
				printf(PHASE_PROMPT
				       "Unsupported Boot Device!\n");
		}
	}

	بازگشت ret;
}

تهی board_init_r(gd_t *dummy1, ulong dummy2)
{
	u32 spl_boot_list[] = {
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
		BOOT_DEVICE_NONE,
	};
	spl_jump_to_image_t jumper = &jump_to_image;
	ساختار spl_image_info spl_image;
	صحیح ret, os;
	تهی *fdt;

	debug(">>" PHASE_PROMPT "board_init_r()\n");

	spl_set_bd();

	اگر (IS_ENABLED(CONFIG_SPL_SYS_MALLOC)) {
		mem_malloc_init(SPL_SYS_MALLOC_START, SPL_SYS_MALLOC_SIZE);
		gd->flags |= GD_FLG_FULL_MALLOC_INIT;
	}
	اگر (!(gd->flags & GD_FLG_SPL_INIT)) {
		اگر (spl_init())
			hang();
	}
	timer_init();
	اگر (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_init();
		اگر (ret) {
			debug("%s: Failed to set up bloblist: ret=%d\n",
			      __func__, ret);
			puts(PHASE_PROMPT "Cannot set up bloblist\n");
			hang();
		}
	}
	اگر (CONFIG_IS_ENABLED(HANDOFF)) {
		صحیح ret;

		ret = setup_spl_handoff();
		اگر (ret) {
			puts(PHASE_PROMPT "Cannot set up SPL handoff\n");
			hang();
		}
	}

	اگر (CONFIG_IS_ENABLED(SOC_INIT))
		spl_soc_init();

	اگر (IS_ENABLED(CONFIG_SPL_WATCHDOG) && CONFIG_IS_ENABLED(WDT))
		initr_watchdog();

	اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT) || CONFIG_IS_ENABLED(HANDOFF) ||
	    IS_ENABLED(CONFIG_SPL_ATF) || IS_ENABLED(CONFIG_SPL_NET))
		dram_init_banksize();

	اگر (IS_ENABLED(CONFIG_SPL_LMB))
		lmb_init();

	اگر (CONFIG_IS_ENABLED(PCI) && !(gd->flags & GD_FLG_DM_DEAD)) {
		ret = pci_init();
		اگر (ret)
			puts(PHASE_PROMPT "Cannot initialize PCI\n");
		/* Don't fail. We still can try other boot methods. */
	}

	اگر (CONFIG_IS_ENABLED(BOARD_INIT))
		spl_board_init();

	bootcount_inc();

	/* Dump driver model states to aid analysis */
	اگر (CONFIG_IS_ENABLED(DM_STATS)) {
		ساختار dm_stats mem;

		dm_get_mem(&mem);
		dm_dump_mem(&mem);
	}

	memset(&spl_image, '\0', اندازه(spl_image));
	اگر (IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image.arg = (تهی *)SPL_PAYLOAD_ARGS_ADDR;
	spl_image.boot_device = BOOT_DEVICE_NONE;
	board_boot_order(spl_boot_list);

	ret = boot_from_devices(&spl_image, spl_boot_list,
				ARRAY_SIZE(spl_boot_list));
	اگر (ret) {
		اگر (CONFIG_IS_ENABLED(SHOW_ERRORS))
			printf(PHASE_PROMPT "failed to boot from all boot devices (err=%d)\n",
			       ret);
		وگرنه
			puts(PHASE_PROMPT "failed to boot from all boot devices\n");
		hang();
	}

	spl_perform_arch_fixups(&spl_image);
	spl_perform_board_fixups(&spl_image);

	os = spl_image.os;
	اگر (os == IH_OS_U_BOOT) {
		debug("Jumping to %s...\n", xpl_name(xpl_next_phase()));
	} وگرنه اگر (CONFIG_IS_ENABLED(ATF) && os == IH_OS_ARM_TRUSTED_FIRMWARE) {
		debug("Jumping to U-Boot via ARM Trusted Firmware\n");
		spl_fixup_fdt(spl_image_fdt_addr(&spl_image));
		jumper = &spl_invoke_atf;
	} وگرنه اگر (CONFIG_IS_ENABLED(OPTEE_IMAGE) && os == IH_OS_TEE) {
		debug("Jumping to U-Boot via OP-TEE\n");
		spl_board_prepare_for_optee(spl_image_fdt_addr(&spl_image));
		jumper = &jump_to_image_optee;
	} وگرنه اگر (CONFIG_IS_ENABLED(OPENSBI) && os == IH_OS_OPENSBI) {
		debug("Jumping to U-Boot via RISC-V OpenSBI\n");
		jumper = &spl_invoke_opensbi;
	} وگرنه اگر (CONFIG_IS_ENABLED(OS_BOOT) && os == IH_OS_LINUX) {
		debug("Jumping to Linux\n");
		اگر (CONFIG_IS_ENABLED(OS_BOOT_ARGS))
			fdt = (تهی *)SPL_PAYLOAD_ARGS_ADDR;
		وگرنه
			fdt = spl_image_fdt_addr(&spl_image);
		spl_fixup_fdt(fdt);
		spl_board_prepare_for_linux();
		spl_image.arg = fdt;
		jumper = &jump_to_image_linux;
	} وگرنه {
		debug("Unsupported OS image.. Jumping nevertheless..\n");
	}
	اگر (CONFIG_IS_ENABLED(SYS_MALLOC_F) &&
	    !IS_ENABLED(CONFIG_SPL_SYS_MALLOC_SIZE))
		debug("SPL malloc() used 0x%x bytes (%d KB)\n",
		      gd_malloc_ptr(), gd_malloc_ptr() / 1024);

	bootstage_mark_name(get_bootstage_id(false), "end phase");
	ret = bootstage_stash_default();
	اگر (ret)
		debug("Failed to stash bootstage: err=%d\n", ret);

	اگر (IS_ENABLED(CONFIG_SPL_VIDEO_REMOVE)) {
		ساختار udevice *dev;
		صحیح rc;

		rc = uclass_find_device(UCLASS_VIDEO, 0, &dev);
		اگر (!rc && dev) {
			rc = device_remove(dev, DM_REMOVE_NORMAL);
			اگر (rc)
				printf("Cannot remove video device '%s' (err=%d)\n",
				       dev->name, rc);
		}
	}
	اگر (CONFIG_IS_ENABLED(HANDOFF)) {
		ret = write_spl_handoff();
		اگر (ret)
			printf(PHASE_PROMPT
			       "SPL hand-off write failed (err=%d)\n", ret);
	}
	اگر (CONFIG_IS_ENABLED(UPL_OUT) && (gd->flags & GD_FLG_UPL)) {
		ret = spl_write_upl_handoff(&spl_image);
		اگر (ret) {
			printf(PHASE_PROMPT
			       "UPL hand-off write failed (err=%d)\n", ret);
			hang();
		}
	}
	اگر (CONFIG_IS_ENABLED(BLOBLIST)) {
		ret = bloblist_finish();
		اگر (ret)
			printf("Warning: Failed to finish bloblist (ret=%d)\n",
			       ret);
	}

	spl_board_prepare_for_boot();

	اگر (CONFIG_IS_ENABLED(RELOC_LOADER)) {
		صحیح ret;

		ret = spl_reloc_jump(&spl_image, jumper);
		اگر (ret) {
			اگر (xpl_phase() == PHASE_VPL)
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
تهی preloader_console_init(تهی)
{
#اگر CONFIG_IS_ENABLED(SERIAL)
	gd->baudrate = CONFIG_BAUDRATE;

	serial_init();		/* serial communications setup */

	gd->flags |= GD_FLG_HAVE_CONSOLE;

#اگر CONFIG_IS_ENABLED(BANNER_PRINT)
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
__weak تهی spl_relocate_پشته_check(تهی)
{
#اگر CONFIG_IS_ENABLED(SYS_REPORT_پشته_F_USAGE)
	ulong init_sp = gd->start_addr_sp;
	ulong پشته_bottom = init_sp - CONFIG_VAL(SIZE_LIMIT_PROVIDE_پشته);
	u8 *ptr = (u8 *)پشته_bottom;
	ulong i;

	برای (i = 0; i < CONFIG_VAL(SIZE_LIMIT_PROVIDE_پشته); i++) {
		اگر (*ptr != CONFIG_VAL(SYS_پشته_F_CHECK_BYTE))
			بشکن;
		ptr++;
	}
	printf("SPL initial stack usage: %lu bytes\n",
	       CONFIG_VAL(SIZE_LIMIT_PROVIDE_پشته) - i);
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
ulong spl_relocate_پشته_gd(تهی)
{
#اگر CONFIG_IS_ENABLED(پشته_R)
	gd_t *new_gd;
	ulong ptr = CONFIG_SPL_پشته_R_ADDR;

	اگر (CONFIG_IS_ENABLED(SYS_REPORT_پشته_F_USAGE))
		spl_relocate_پشته_check();

#اگر defined(CONFIG_SPL_SYS_MALLOC_SIMPLE) && CONFIG_IS_ENABLED(SYS_MALLOC_F)
	اگر (CONFIG_SPL_پشته_R_MALLOC_SIMPLE_LEN) {
		debug("SPL malloc() before relocation used 0x%x bytes (%d KB)\n",
		      gd->malloc_ptr, gd->malloc_ptr / 1024);
		ptr -= CONFIG_SPL_پشته_R_MALLOC_SIMPLE_LEN;
		gd->malloc_base = ptr;
		gd->malloc_limit = CONFIG_SPL_پشته_R_MALLOC_SIMPLE_LEN;
		gd->malloc_ptr = 0;
	}
#endif
	/* Get stack position: use 8-byte alignment for ABI compliance */
	ptr -= roundup(اندازه(gd_t), 16);
	gd->start_addr_sp = ptr;
	new_gd = (gd_t *)ptr;
	memcpy(new_gd, (تهی *)gd, اندازه(gd_t));
#اگر CONFIG_IS_ENABLED(DM)
	dm_fixup_for_gd_move(new_gd);
#endif
#اگر CONFIG_IS_ENABLED(LOG)
	log_fixup_for_gd_move(new_gd);
#endif
#اگر !defined(CONFIG_ARM) && !defined(CONFIG_RISCV)
	gd = new_gd;
#endif
	بازگشت ptr;
#وگرنه
	بازگشت 0;
#endif
}

#اگر defined(CONFIG_BOOTCOUNT_LIMIT) && \
	((!defined(CONFIG_TPL_BUILD) && !defined(CONFIG_SPL_BOOTCOUNT_LIMIT)) || \
	 (defined(CONFIG_TPL_BUILD) && !defined(CONFIG_TPL_BOOTCOUNT_LIMIT)))
تهی bootcount_store(ulong a)
{
}

ulong bootcount_load(تهی)
{
	بازگشت 0;
}
#endif
